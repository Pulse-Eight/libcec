/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2012 Pulse-Eight Limited.  All rights reserved.
 * libCEC(R) is an original work, containing original code.
 *
 * libCEC(R) is a trademark of Pulse-Eight Limited.
 *
 * This program is dual-licensed; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * Alternatively, you can license this library under a commercial license,
 * please contact Pulse-Eight Licensing for more information.
 *
 * For more information contact:
 * Pulse-Eight Licensing       <license@pulse-eight.com>
 *     http://www.pulse-eight.com/
 *     http://www.pulse-eight.net/
 */

#include "USBCECAdapterMessageQueue.h"
#include "USBCECAdapterCommunication.h"
#include "../platform/sockets/socket.h"
#include "../LibCEC.h"

using namespace CEC;
using namespace PLATFORM;

CCECAdapterMessageQueueEntry::CCECAdapterMessageQueueEntry(CCECAdapterMessage *message) :
    m_message(message),
    m_iPacketsLeft(message->IsTranmission() ? message->Size() / 4 : 1),
    m_bSucceeded(false),
    m_bWaiting(true) {}

CCECAdapterMessageQueueEntry::~CCECAdapterMessageQueueEntry(void) { }

void CCECAdapterMessageQueueEntry::Broadcast(void)
{
  CLockObject lock(m_mutex);
  m_condition.Broadcast();
}

bool CCECAdapterMessageQueueEntry::MessageReceived(const CCECAdapterMessage &message)
{
  bool bSendSignal(false);
  bool bHandled(false);

  PLATFORM::CLockObject lock(m_mutex);
  if (!IsResponse(message))
  {
    /* we received a message from the adapter that's not a response to this command */
    if (!message.IsTranmission())
    {
      /* we received something that's not a transmission while waiting for an ack to this command, so this command failed */

      //TODO verify whether we're not failing too soon
      CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - %s - not a response %s - failed", __FUNCTION__, ToString(), CCECAdapterMessage::ToString(message.Message()));
      m_message->state = ADAPTER_MESSAGE_STATE_SENT_NOT_ACKED;
      bSendSignal = true;
    }
  }
  else
  {
    /* we received a response, so this message is handled */
    bHandled = true;
    switch (message.Message())
    {
    case MSGCODE_COMMAND_ACCEPTED:
      bSendSignal = MessageReceivedCommandAccepted(message);
      break;
    case MSGCODE_TRANSMIT_SUCCEEDED:
      bSendSignal = MessageReceivedTransmitSucceeded(message);
      break;
    default:
      bSendSignal = MessageReceivedResponse(message);
      break;
    }
  }

  /* signal the waiting thread when we're done */
  if (bSendSignal)
  {
    m_bSucceeded = true;
    m_condition.Signal();
  }

  return bHandled;
}

bool CCECAdapterMessageQueueEntry::Wait(uint32_t iTimeout)
{
  bool bReturn(false);
  /* wait until we receive a signal when the tranmission succeeded */
  {
    CLockObject lock(m_mutex);
    bReturn = m_bSucceeded ? true : m_condition.Wait(m_mutex, m_bSucceeded, iTimeout);
    m_bWaiting = false;
  }
  return bReturn;
}

bool CCECAdapterMessageQueueEntry::IsWaiting(void)
{
  CLockObject lock(m_mutex);
  return m_bWaiting;
}

cec_adapter_messagecode CCECAdapterMessageQueueEntry::MessageCode(void)
{
  return m_message->Message();
}

bool CCECAdapterMessageQueueEntry::IsResponse(const CCECAdapterMessage &msg)
{
  cec_adapter_messagecode msgCode = msg.Message();
  return msgCode == MessageCode() ||
         msgCode == MSGCODE_TIMEOUT_ERROR ||
         msgCode == MSGCODE_COMMAND_ACCEPTED ||
         msgCode == MSGCODE_COMMAND_REJECTED ||
         (m_message->IsTranmission() && msgCode == MSGCODE_HIGH_ERROR) ||
         (m_message->IsTranmission() && msgCode == MSGCODE_LOW_ERROR) ||
         (m_message->IsTranmission() && msgCode == MSGCODE_RECEIVE_FAILED) ||
         (m_message->IsTranmission() && msgCode == MSGCODE_TRANSMIT_FAILED_LINE) ||
         (m_message->IsTranmission() && msgCode == MSGCODE_TRANSMIT_FAILED_ACK) ||
         (m_message->IsTranmission() && msgCode == MSGCODE_TRANSMIT_FAILED_TIMEOUT_DATA) ||
         (m_message->IsTranmission() && msgCode == MSGCODE_TRANSMIT_FAILED_TIMEOUT_LINE) ||
         (m_message->IsTranmission() && msgCode == MSGCODE_TRANSMIT_SUCCEEDED);
}

const char *CCECAdapterMessageQueueEntry::ToString(void) const
{
  /* CEC transmissions got the 'set ack polarity' msgcode, which doesn't look nice */
  if (m_message->IsTranmission())
    return "CEC transmission";
  else
    return CCECAdapterMessage::ToString(m_message->Message());
}

bool CCECAdapterMessageQueueEntry::MessageReceivedCommandAccepted(const CCECAdapterMessage &message)
{
  bool bSendSignal(false);
  if (m_iPacketsLeft == 0)
  {
    /* we received a "command accepted", but we're not waiting for one anymore */
    CLibCEC::AddLog(CEC_LOG_ERROR, "%s - received unexpected 'command accepted' message", ToString());
    m_message->state = ADAPTER_MESSAGE_STATE_ERROR;
    bSendSignal = true;
  }
  else
  {
    /* decrease number of acks we're waiting on by 1 */
    if (m_iPacketsLeft > 0)
      m_iPacketsLeft--;

    /* log this message */
    CStdString strLog;
    strLog.Format("%s - command accepted", ToString());
    if (m_iPacketsLeft > 0)
      strLog.AppendFormat(" - waiting for %d more", m_iPacketsLeft);
    CLibCEC::AddLog(CEC_LOG_DEBUG, strLog);

    /* no more packets left and not a transmission, so we're done */
    if (!m_message->IsTranmission() && m_iPacketsLeft == 0)
    {
      m_message->state = ADAPTER_MESSAGE_STATE_SENT_ACKED;
      m_message->response = message.packet;
      bSendSignal = true;
    }
  }
  return bSendSignal;
}

bool CCECAdapterMessageQueueEntry::MessageReceivedTransmitSucceeded(const CCECAdapterMessage &message)
{
  if (m_iPacketsLeft == 0)
  {
    /* transmission succeeded, so we're done */
    CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - transmit succeeded", ToString());
    m_message->state = ADAPTER_MESSAGE_STATE_SENT_ACKED;
    m_message->response = message.packet;
  }
  else
  {
    /* error, we expected more acks */
    CLibCEC::AddLog(CEC_LOG_WARNING, "%s - received 'transmit succeeded' but not enough 'command accepted' messages (%d left)", ToString(), m_iPacketsLeft);
    m_message->state = ADAPTER_MESSAGE_STATE_ERROR;
  }
  return true;
}

bool CCECAdapterMessageQueueEntry::MessageReceivedResponse(const CCECAdapterMessage &message)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - received response", ToString());
  m_message->response = message.packet;
  if (m_message->IsTranmission())
    m_message->state = message.Message() == MSGCODE_TRANSMIT_SUCCEEDED ? ADAPTER_MESSAGE_STATE_SENT_ACKED : ADAPTER_MESSAGE_STATE_SENT_NOT_ACKED;
  else
    m_message->state = ADAPTER_MESSAGE_STATE_SENT_ACKED;
  return true;
}


CCECAdapterMessageQueue::~CCECAdapterMessageQueue(void)
{
  Clear();
}

void CCECAdapterMessageQueue::Clear(void)
{
  CLockObject lock(m_mutex);
  CCECAdapterMessageQueueEntry *message(NULL);
  while (m_messages.Pop(message))
    message->Broadcast();
}

void CCECAdapterMessageQueue::MessageReceived(const CCECAdapterMessage &msg)
{
  CLockObject lock(m_mutex);
  CCECAdapterMessageQueueEntry *message = GetNextQueuedEntry();

  /* send the received message to the first entry in the queue */
  bool bHandled = message ? message->MessageReceived(msg) : false;

  if (!message || !bHandled)
  {
    /* the message wasn't handled */
    bool bIsError(m_com->HandlePoll(msg));
    CLibCEC::AddLog(bIsError ? CEC_LOG_WARNING : CEC_LOG_DEBUG, msg.ToString());

    /* push this message to the current frame */
    if (!bIsError && msg.PushToCecCommand(m_currentCECFrame))
    {
      /* and push the current frame back over the callback method when a full command was received */
      if (m_com->IsInitialised())
        m_com->m_callback->OnCommandReceived(m_currentCECFrame);

      /* clear the current frame */
      m_currentCECFrame.Clear();
    }
  }
}

void CCECAdapterMessageQueue::AddData(uint8_t *data, size_t iLen)
{
  for (size_t iPtr = 0; iPtr < iLen; iPtr++)
  {
    bool bFullMessage(false);
    {
      CLockObject lock(m_mutex);
      bFullMessage = m_incomingAdapterMessage.PushReceivedByte(data[iPtr]);
    }

    if (bFullMessage)
    {
      /* a full message was received */
      CCECAdapterMessage newMessage;
      newMessage.packet = m_incomingAdapterMessage.packet;
      MessageReceived(newMessage);

      /* clear the current message */
      CLockObject lock(m_mutex);
      m_incomingAdapterMessage.Clear();
    }
  }
}

bool CCECAdapterMessageQueue::Write(CCECAdapterMessage *msg)
{
  msg->state = ADAPTER_MESSAGE_STATE_WAITING_TO_BE_SENT;

  /* set the correct line timeout */
  if (msg->IsTranmission())
  {
    if (msg->tries == 1)
      m_com->SetLineTimeout(msg->lineTimeout);
    else
      m_com->SetLineTimeout(msg->retryTimeout);
  }

  CCECAdapterMessageQueueEntry *entry(NULL);
  /* add to the wait for ack queue */
  if (msg->Message() != MSGCODE_START_BOOTLOADER)
  {
    entry = new CCECAdapterMessageQueueEntry(msg);
    PLATFORM::CLockObject lock(m_mutex);
    m_messages.Push(entry);
  }

  /* TODO write the message async */
  if (!m_com->WriteToDevice(msg))
  {
    /* error! */
    Clear();
    return false;
  }

  if (entry && !entry->Wait(msg->transmit_timeout <= 5 ? CEC_DEFAULT_TRANSMIT_WAIT : msg->transmit_timeout))
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "command '%s' was not acked by the controller", CCECAdapterMessage::ToString(msg->Message()));
    msg->state = ADAPTER_MESSAGE_STATE_SENT_NOT_ACKED;
    return false;
  }

  return true;
}

CCECAdapterMessageQueueEntry *CCECAdapterMessageQueue::GetNextQueuedEntry(void)
{
  CCECAdapterMessageQueueEntry *message(NULL);
  while (message == NULL && m_messages.Peek(message))
  {
    if (!message->IsWaiting())
    {
      /* delete old messages */
      m_messages.Pop(message);
      delete message;
      message = NULL;
    }
  }
  return message;
}
