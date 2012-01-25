/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011 Pulse-Eight Limited.  All rights reserved.
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

#include "AdapterCommunication.h"

#include "AdapterMessage.h"
#include "../CECProcessor.h"
#include "../platform/serialport/serialport.h"
#include "../LibCEC.h"

using namespace std;
using namespace CEC;
using namespace PLATFORM;

CAdapterCommunication::CAdapterCommunication(CCECProcessor *processor) :
    m_port(NULL),
    m_processor(processor),
    m_iLineTimeout(0),
    m_iFirmwareVersion(CEC_FW_VERSION_UNKNOWN)
{
  m_port = new PLATFORM::CSerialPort;
}

CAdapterCommunication::~CAdapterCommunication(void)
{
  Close();

  if (m_port)
  {
    delete m_port;
    m_port = NULL;
  }
}

bool CAdapterCommunication::Open(const char *strPort, uint16_t iBaudRate /* = 38400 */, uint32_t iTimeoutMs /* = 10000 */)
{
  uint64_t iNow = GetTimeMs();
  uint64_t iTimeout = iNow + iTimeoutMs;

  CLockObject lock(m_mutex);

  if (!m_port)
  {
    CLibCEC::AddLog(CEC_LOG_ERROR, "port is NULL");
    return false;
  }

  if (IsOpen())
  {
    CLibCEC::AddLog(CEC_LOG_ERROR, "port is already open");
    return true;
  }

  CStdString strError;
  bool bConnected(false);
  while (!bConnected && iNow < iTimeout)
  {
    if ((bConnected = m_port->Open(strPort, iBaudRate)) == false)
    {
      strError.Format("error opening serial port '%s': %s", strPort, m_port->GetError().c_str());
      Sleep(250);
      iNow = GetTimeMs();
    }
  }

  if (!bConnected)
  {
    CLibCEC::AddLog(CEC_LOG_ERROR, strError);
    return false;
  }

  CLibCEC::AddLog(CEC_LOG_DEBUG, "connection opened");

  //clear any input bytes
  uint8_t buff[1];
  while (m_port->Read(buff, 1, 5) == 1) {}

  if (CreateThread())
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "communication thread started");
    return true;
  }
  else
  {
    CLibCEC::AddLog(CEC_LOG_ERROR, "could not create a communication thread");
  }

  return false;
}

void CAdapterCommunication::Close(void)
{
  CLockObject lock(m_mutex);
  m_rcvCondition.Broadcast();
  StopThread();
}

void *CAdapterCommunication::Process(void)
{
  while (!IsStopped())
  {
    ReadFromDevice(50);
    Sleep(5);
    WriteNextCommand();
  }

  CCECAdapterMessage *msg(NULL);
  if (m_outBuffer.Pop(msg))
    msg->condition.Broadcast();

  return NULL;
}

bool CAdapterCommunication::Write(CCECAdapterMessage *data)
{
  bool bReturn(false);

  CLockObject lock(data->mutex);
  data->state = ADAPTER_MESSAGE_STATE_WAITING_TO_BE_SENT;
  m_outBuffer.Push(data);
  data->condition.Wait(data->mutex);

  if (data->state != ADAPTER_MESSAGE_STATE_SENT)
  {
    CLibCEC::AddLog(CEC_LOG_ERROR, "command was not sent");
  }
  else if (data->expectControllerAck)
  {
    bReturn = WaitForAck(*data);
    if (bReturn)
    {
      if (data->isTransmission)
        data->state = ADAPTER_MESSAGE_STATE_SENT_ACKED;
    }
    else
    {
      data->state = ADAPTER_MESSAGE_STATE_SENT_NOT_ACKED;
      CLibCEC::AddLog(CEC_LOG_DEBUG, "did not receive ack");
    }
  }
  else
  {
    bReturn = true;
  }

  return bReturn;
}

bool CAdapterCommunication::Read(CCECAdapterMessage &msg, uint32_t iTimeout)
{
  CLockObject lock(m_mutex);

  msg.Clear();
  uint64_t iNow = GetTimeMs();
  uint64_t iTarget = iNow + iTimeout;
  bool bGotFullMessage(false);
  bool bNextIsEscaped(false);
  bool bGotStart(false);

  while(!bGotFullMessage && iNow < iTarget)
  {
    uint8_t buf = 0;
    if (!m_inBuffer.Pop(buf))
    {
      if (!m_rcvCondition.Wait(m_mutex, (uint32_t) (iTarget - iNow)))
        return false;
    }

    if (!bGotStart)
    {
      if (buf == MSGSTART)
        bGotStart = true;
      continue;
    }
    else if (buf == MSGSTART) //we found a msgstart before msgend, this is not right, remove
    {
      if (msg.Size() > 0)
        CLibCEC::AddLog(CEC_LOG_WARNING, "received MSGSTART before MSGEND, removing previous buffer contents");
      msg.Clear();
      bGotStart = true;
    }

    if (buf == MSGEND)
    {
      bGotFullMessage = true;
    }
    else if (bNextIsEscaped)
    {
      msg.PushBack(buf + (uint8_t)ESCOFFSET);
      bNextIsEscaped = false;
    }
    else if (buf == MSGESC)
      bNextIsEscaped = true;
    else
      msg.PushBack(buf);
  }

  if (bGotFullMessage)
    msg.state = ADAPTER_MESSAGE_STATE_INCOMING;

  return bGotFullMessage;
}

std::string CAdapterCommunication::GetError(void) const
{
  return m_port->GetError();
}

bool CAdapterCommunication::StartBootloader(void)
{
  bool bReturn(false);
  if (!IsRunning())
    return bReturn;

  CLibCEC::AddLog(CEC_LOG_DEBUG, "starting the bootloader");
  CCECAdapterMessage *output = new CCECAdapterMessage;

  output->PushBack(MSGSTART);
  output->PushEscaped(MSGCODE_START_BOOTLOADER);
  output->PushBack(MSGEND);
  output->isTransmission = false;
  output->expectControllerAck = false;

  if ((bReturn = Write(output)) == false)
    CLibCEC::AddLog(CEC_LOG_ERROR, "could not start the bootloader");
  delete output;

  return bReturn;
}

bool CAdapterCommunication::PingAdapter(void)
{
  bool bReturn(false);
  if (!IsRunning())
    return bReturn;

  CLibCEC::AddLog(CEC_LOG_DEBUG, "sending ping");
  CCECAdapterMessage *output = new CCECAdapterMessage;

  output->PushBack(MSGSTART);
  output->PushEscaped(MSGCODE_PING);
  output->PushBack(MSGEND);
  output->isTransmission = false;

  if ((bReturn = Write(output)) == false)
    CLibCEC::AddLog(CEC_LOG_ERROR, "could not ping the adapter");
  delete output;

  return bReturn;
}

uint16_t CAdapterCommunication::GetFirmwareVersion(void)
{
  uint16_t iReturn(m_iFirmwareVersion);
  if (!IsRunning())
    return iReturn;

  if (iReturn == CEC_FW_VERSION_UNKNOWN)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting the firmware version");
    CCECAdapterMessage *output = new CCECAdapterMessage;

    output->PushBack(MSGSTART);
    output->PushEscaped(MSGCODE_FIRMWARE_VERSION);
    output->PushBack(MSGEND);
    output->isTransmission = false;
    output->expectControllerAck = false;

    SendMessageToAdapter(output);
    delete output;

    CCECAdapterMessage input;
    if (!Read(input, CEC_DEFAULT_TRANSMIT_WAIT) || input.Message() != MSGCODE_FIRMWARE_VERSION || input.Size() != 3)
      CLibCEC::AddLog(CEC_LOG_ERROR, "no or invalid firmware version");
    else
    {
      m_iFirmwareVersion = (input[1] << 8 | input[2]);
      iReturn = m_iFirmwareVersion;
    }
  }

  return iReturn;
}

bool CAdapterCommunication::SetLineTimeout(uint8_t iTimeout)
{
  bool bReturn(m_iLineTimeout != iTimeout);

  if (!bReturn)
  {
    CCECAdapterMessage *output = new CCECAdapterMessage;

    output->PushBack(MSGSTART);
    output->PushEscaped(MSGCODE_TRANSMIT_IDLETIME);
    output->PushEscaped(iTimeout);
    output->PushBack(MSGEND);
    output->isTransmission = false;

    if ((bReturn = Write(output)) == false)
      CLibCEC::AddLog(CEC_LOG_ERROR, "could not set the idletime");
    delete output;
  }

  return bReturn;
}

bool CAdapterCommunication::SetAckMask(uint16_t iMask)
{
  bool bReturn(false);
  CStdString strLog;
  strLog.Format("setting ackmask to %2x", iMask);
  CLibCEC::AddLog(CEC_LOG_DEBUG, strLog.c_str());

  CCECAdapterMessage *output = new CCECAdapterMessage;

  output->PushBack(MSGSTART);
  output->PushEscaped(MSGCODE_SET_ACK_MASK);
  output->PushEscaped(iMask >> 8);
  output->PushEscaped((uint8_t)iMask);
  output->PushBack(MSGEND);
  output->isTransmission = false;

  if ((bReturn = Write(output)) == false)
    CLibCEC::AddLog(CEC_LOG_ERROR, "could not set the ackmask");
  delete output;

  return bReturn;
}

bool CAdapterCommunication::IsOpen(void)
{
  return !IsStopped() && m_port->IsOpen() && IsRunning();
}

bool CAdapterCommunication::WaitForAck(CCECAdapterMessage &message)
{
  bool bError(false);
  bool bTransmitSucceeded(false);
  uint8_t iPacketsLeft(message.Size() / 4);

  int64_t iNow = GetTimeMs();
  int64_t iTargetTime = iNow + message.transmit_timeout;

  while (!bTransmitSucceeded && !bError && (message.transmit_timeout == 0 || iNow < iTargetTime))
  {
    CCECAdapterMessage msg;
    int32_t iWait = (int32_t)(iTargetTime - iNow);
    if (iWait <= 5 || message.transmit_timeout <= 5)
      iWait = CEC_DEFAULT_TRANSMIT_WAIT;

    if (!Read(msg, iWait))
    {
      iNow = GetTimeMs();
      continue;
    }

    if (msg.Message() == MSGCODE_FRAME_START && msg.IsACK())
    {
      m_processor->HandlePoll(msg.Initiator(), msg.Destination());
      iNow = GetTimeMs();
      continue;
    }

    if (msg.Message() == MSGCODE_RECEIVE_FAILED &&
        m_processor->HandleReceiveFailed())
    {
      iNow = GetTimeMs();
      continue;
    }

    bError = msg.IsError();
    if (bError)
    {
      message.reply = msg.Message();
      CLibCEC::AddLog(CEC_LOG_DEBUG, msg.ToString());
    }
    else
    {
      switch(msg.Message())
      {
      case MSGCODE_COMMAND_ACCEPTED:
        CLibCEC::AddLog(CEC_LOG_DEBUG, msg.ToString());
        if (iPacketsLeft > 0)
          iPacketsLeft--;
        if (!message.isTransmission && iPacketsLeft == 0)
          bTransmitSucceeded = true;
        break;
      case MSGCODE_TRANSMIT_SUCCEEDED:
        CLibCEC::AddLog(CEC_LOG_DEBUG, msg.ToString());
        bTransmitSucceeded = (iPacketsLeft == 0);
        bError = !bTransmitSucceeded;
        message.reply = MSGCODE_TRANSMIT_SUCCEEDED;
        break;
      default:
        // ignore other data while waiting
        break;
      }

      iNow = GetTimeMs();
    }
  }

  return bTransmitSucceeded && !bError;
}

void CAdapterCommunication::AddData(uint8_t *data, uint8_t iLen)
{
  CLockObject lock(m_mutex);
  for (uint8_t iPtr = 0; iPtr < iLen; iPtr++)
    m_inBuffer.Push(data[iPtr]);

  m_rcvCondition.Signal();
}

bool CAdapterCommunication::ReadFromDevice(uint32_t iTimeout)
{
  int32_t iBytesRead;
  uint8_t buff[1024];
  if (!m_port)
    return false;

  CLockObject lock(m_mutex);
  iBytesRead = m_port->Read(buff, sizeof(buff), iTimeout);
  if (iBytesRead < 0 || iBytesRead > 256)
  {
    CStdString strError;
    strError.Format("error reading from serial port: %s", m_port->GetError().c_str());
    CLibCEC::AddLog(CEC_LOG_ERROR, strError);
    return false;
  }
  else if (iBytesRead > 0)
    AddData(buff, (uint8_t) iBytesRead);

  return iBytesRead > 0;
}

void CAdapterCommunication::SendMessageToAdapter(CCECAdapterMessage *msg)
{
  CLockObject adapterLock(m_mutex);
  CLockObject lock(msg->mutex);
  if (m_port->Write(msg->packet.data, msg->Size()) != (int32_t) msg->Size())
  {
    CStdString strError;
    strError.Format("error writing to serial port: %s", m_port->GetError().c_str());
    CLibCEC::AddLog(CEC_LOG_ERROR, strError);
    msg->state = ADAPTER_MESSAGE_STATE_ERROR;
  }
  else
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "command sent");
    msg->state = ADAPTER_MESSAGE_STATE_SENT;
  }
  msg->condition.Signal();
}

void CAdapterCommunication::WriteNextCommand(void)
{
  CCECAdapterMessage *msg(NULL);
  if (m_outBuffer.Pop(msg))
    SendMessageToAdapter(msg);
}
