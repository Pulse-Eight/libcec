/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2015 Pulse-Eight Limited.  All rights reserved.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
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

#include "env.h"

#if defined(HAVE_RPI_API)
#include "RPiCECAdapterMessageQueue.h"
#include "p8-platform/util/StringUtils.h"

// use vc_cec_send_message2() if defined and vc_cec_send_message() if not
//#define RPI_USE_SEND_MESSAGE2

#include "RPiCECAdapterCommunication.h"
#include "LibCEC.h"
#include "CECTypeUtils.h"

extern "C" {
#include <interface/vmcs_host/vc_cecservice.h>
#include <interface/vchiq_arm/vchiq.h>
}

using namespace CEC;
using namespace P8PLATFORM;

#define LIB_CEC m_com->m_callback->GetLib()

// initialise new msg with unsuccesfull status, also
// set default return state to "UNKNOWN" - instead
// of NACK (which has special meaning for dev POLLing)
CRPiCECAdapterMessageQueueEntry::CRPiCECAdapterMessageQueueEntry(CRPiCECAdapterMessageQueue *queue, const cec_command &command) :
    m_queue(queue),
    m_command(command),
    m_retval(VC_CEC_ERROR_BUSY),
    m_bSucceeded(false)
{

}

void CRPiCECAdapterMessageQueueEntry::Broadcast(void)
{
  CLockObject lock(m_mutex);
  m_condition.Broadcast();
}

bool CRPiCECAdapterMessageQueueEntry::MessageReceived(cec_opcode opcode, cec_logical_address initiator, cec_logical_address destination, uint32_t response)
{
  if ((m_command.opcode_set && m_command.opcode == opcode &&
     m_command.initiator == initiator &&
     m_command.destination == destination)
     ||
     (!m_command.opcode_set &&
     m_command.destination == destination))
  {
    CLockObject lock(m_mutex);
    m_retval = response;
    m_bSucceeded = true;
    m_condition.Signal();
    return true;
  }

  return false;
}

bool CRPiCECAdapterMessageQueueEntry::Wait(uint32_t iTimeout)
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

bool CRPiCECAdapterMessageQueueEntry::IsWaiting(void)
{
  CLockObject lock(m_mutex);
  return m_bWaiting;
}

void CRPiCECAdapterMessageQueue::Clear(void)
{
  CLockObject lock(m_mutex);
  m_messages.clear();
}

void CRPiCECAdapterMessageQueue::MessageReceived(cec_opcode opcode, cec_logical_address initiator, cec_logical_address destination, uint32_t response)
{
  bool bHandled(false);
  CLockObject lock(m_mutex);
  /* send the received message to each entry in the queue until it is handled */
  for (std::map<uint64_t, CRPiCECAdapterMessageQueueEntry *>::iterator it = m_messages.begin(); !bHandled && it != m_messages.end(); it++)
    bHandled = it->second->MessageReceived(opcode, initiator, destination, response);

  if (!bHandled)
    LIB_CEC->AddLog(CEC_LOG_WARNING, "unhandled response received: opcode=%x initiator=%x destination=%x response=%x", (int)opcode, (int)initiator, (int)destination, response);
}

uint32_t CRPiCECAdapterMessageQueueEntry::Result() const
{
  return m_retval;
}

cec_adapter_message_state CRPiCECAdapterMessageQueue::Write(const cec_command &command, bool &bRetry, uint32_t iLineTimeout, bool bIsReply, VC_CEC_ERROR_T &vcReply)
{
  // handle POLL (msg like '11') in a special way - the way it was
  // originally designed by BCM, expected to happen and documented
  // in API docs (/opt/vc/includes)
  // due to often (more than 20% test cases - CEC bus with 8 devices)
  // irregularities on returned status, repeat until we get SAME
  // result twice in a row
  if (!command.opcode_set && command.destination == command.initiator)
  {
    int iReturnPrev = -1;
    int iReturn = 0;

    while((iReturn = vc_cec_poll_address((CEC_AllDevices_T)command.destination)) != iReturnPrev)
      iReturnPrev = iReturn;
    if (iReturn == 0)
      return ADAPTER_MESSAGE_STATE_SENT_ACKED;
    else if (iReturn > 0)
      return ADAPTER_MESSAGE_STATE_SENT_NOT_ACKED;
    else
      return ADAPTER_MESSAGE_STATE_WAITING_TO_BE_SENT;
  }

  CRPiCECAdapterMessageQueueEntry *entry = new CRPiCECAdapterMessageQueueEntry(this, command);
  uint64_t iEntryId(0);
  /* add to the wait for ack queue */
  {
    CLockObject lock(m_mutex);
    iEntryId = m_iNextMessage++;
    m_messages.insert(std::make_pair(iEntryId, entry));
  }

#if defined(RPI_USE_SEND_MESSAGE2)
  VC_CEC_MESSAGE_T message;
  message.initiator = (CEC_AllDevices_T)command.initiator;
  message.follower = (CEC_AllDevices_T)command.destination;
  message.length = 1;

  if (command.opcode_set)
  {
    message.length += 1;
    message.payload[0] = command.opcode;

    message.length += command.parameters.size;
    for (uint8_t iPtr = 0; iPtr < command.parameters.size; iPtr++)
      message.payload[iPtr + 1] = command.parameters.At(iPtr);
  }

#ifdef CEC_DEBUGGING
  std::string strDump;
  strDump = StringUtils::Format("len = %d, payload = %X%X", message.length, (int)message.initiator, (int)message.follower);
  for (uint8_t iPtr = 0; iPtr < message.length - 1; iPtr++)
    strDump += StringUtils::Format(":%02X", message.payload[iPtr]);
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "sending data: %s", strDump.c_str());
#endif

  int iReturn = vc_cec_send_message2(&message);
#else
  uint8_t payload[32];
  uint32_t iLength(0);

  if (command.opcode_set)
  {
    iLength += 1;
    payload[0] = command.opcode;

    iLength += command.parameters.size;
    for (uint8_t iPtr = 0; iPtr < command.parameters.size; iPtr++)
      payload[iPtr + 1] = command.parameters.At(iPtr);
  }

#ifdef CEC_DEBUGGING
  std:string strDump;
  strDump = StringUtils::Format("len = %d, payload = %X%X", iLength, (int)command.initiator, (int)command.destination);
  for (uint8_t iPtr = 0; iPtr < iLength; iPtr++)
    strDump += StringUtils::Format(":%02X", payload[iPtr]);
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "sending data: %s", strDump.c_str());
#endif

  int iReturn = vc_cec_send_message((uint32_t)command.destination, command.opcode_set ? (uint8_t*)&payload : NULL, iLength, bIsReply);
#endif

  bRetry = false;
  if (iReturn != VCHIQ_SUCCESS)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "sending command '%s' failed (%d)", CCECTypeUtils::ToString(command.opcode), iReturn);
    CLockObject lock(m_mutex);
    delete entry;
    m_messages.erase(iEntryId);
    return ADAPTER_MESSAGE_STATE_ERROR;
  }

  cec_adapter_message_state bReturn(ADAPTER_MESSAGE_STATE_ERROR);
  if (entry)
  {
    if (entry->Wait(iLineTimeout))
    {
      int status = entry->Result();

      if (status == VC_CEC_ERROR_NO_ACK)
        bReturn = ADAPTER_MESSAGE_STATE_SENT_NOT_ACKED;
      else if (status == VC_CEC_SUCCESS)
        bReturn = ADAPTER_MESSAGE_STATE_SENT_ACKED;
      else
        bReturn = ADAPTER_MESSAGE_STATE_SENT;
    }
    else
    {
      bRetry = true;
      LIB_CEC->AddLog(CEC_LOG_DEBUG, "command '%s' timeout", CCECTypeUtils::ToString(command.opcode));
      CEvent::Sleep(CEC_DEFAULT_TRANSMIT_RETRY_WAIT);
      bReturn = ADAPTER_MESSAGE_STATE_WAITING_TO_BE_SENT;
    }

    vcReply = (VC_CEC_ERROR_T)entry->Result();

    CLockObject lock(m_mutex);
    m_messages.erase(iEntryId);
    delete entry;
  }

  return bReturn;
}

#endif
