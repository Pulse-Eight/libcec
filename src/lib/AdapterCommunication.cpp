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

#include "LibCEC.h"
#include "platform/serialport.h"
#include "util/StdString.h"
#include "platform/timeutils.h"

using namespace std;
using namespace CEC;

CAdapterCommunication::CAdapterCommunication(CLibCEC *controller) :
    m_port(NULL),
    m_controller(controller)
{
  m_port = new CSerialPort;
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
  CLockObject lock(&m_commMutex);
  if (!m_port)
  {
    m_controller->AddLog(CEC_LOG_ERROR, "port is NULL");
    return false;
  }

  if (IsOpen())
  {
    m_controller->AddLog(CEC_LOG_ERROR, "port is already open");
  }

  if (!m_port->Open(strPort, iBaudRate))
  {
    CStdString strError;
    strError.Format("error opening serial port '%s': %s", strPort, m_port->GetError().c_str());
    m_controller->AddLog(CEC_LOG_ERROR, strError);
    return false;
  }

  m_controller->AddLog(CEC_LOG_DEBUG, "connection opened");

  //clear any input bytes
  uint8_t buff[1024];
  m_port->Read(buff, sizeof(buff), 500);

  if (CreateThread())
  {
    m_controller->AddLog(CEC_LOG_DEBUG, "communication thread created");
    return true;
  }
  else
  {
    m_controller->AddLog(CEC_LOG_DEBUG, "could not create a communication thread");
  }

  return false;
}

void CAdapterCommunication::Close(void)
{
  CLockObject lock(&m_commMutex);
  StopThread();

  m_rcvCondition.Broadcast();
}

void *CAdapterCommunication::Process(void)
{
  m_controller->AddLog(CEC_LOG_DEBUG, "communication thread started");

  while (!IsStopped())
  {
    {
      CLockObject lock(&m_commMutex);
      ReadFromDevice(100);
    }

    if (!IsStopped())
      Sleep(5);
  }

  return NULL;
}

bool CAdapterCommunication::ReadFromDevice(uint32_t iTimeout)
{
  int32_t iBytesRead;
  uint8_t buff[1024];
  if (!m_port)
    return false;

  iBytesRead = m_port->Read(buff, sizeof(buff), iTimeout);
  if (iBytesRead < 0 || iBytesRead > 256)
  {
    CStdString strError;
    strError.Format("error reading from serial port: %s", m_port->GetError().c_str());
    m_controller->AddLog(CEC_LOG_ERROR, strError);
    return false;
  }
  else if (iBytesRead > 0)
    AddData(buff, (uint8_t) iBytesRead);

  return iBytesRead > 0;
}

void CAdapterCommunication::AddData(uint8_t *data, uint8_t iLen)
{
  CLockObject lock(&m_bufferMutex);
  for (unsigned int iPtr = 0; iPtr < iLen; iPtr++)
    m_inBuffer.Push(data[iPtr]);

  m_rcvCondition.Signal();
}

bool CAdapterCommunication::Write(const cec_adapter_message &data)
{
  CLockObject lock(&m_commMutex);
  if (m_port->Write(data) != (int32_t) data.size())
  {
    CStdString strError;
    strError.Format("error writing to serial port: %s", m_port->GetError().c_str());
    m_controller->AddLog(CEC_LOG_ERROR, strError);
    return false;
  }

  m_controller->AddLog(CEC_LOG_DEBUG, "command sent");
  CCondition::Sleep((uint32_t) data.size() * (uint32_t)24 /*data*/ + (uint32_t)5 /*start bit (4.5 ms)*/);

  return true;
}

bool CAdapterCommunication::Read(cec_adapter_message &msg, uint32_t iTimeout)
{
  CLockObject lock(&m_bufferMutex);

  msg.clear();
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
      if (!m_rcvCondition.Wait(&m_bufferMutex, iTarget - iNow))
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
      m_controller->AddLog(CEC_LOG_ERROR, "received MSGSTART before MSGEND");
      msg.clear();
      bGotStart = true;
    }

    if (buf == MSGEND)
    {
      bGotFullMessage = true;
    }
    else if (bNextIsEscaped)
    {
      msg.push_back(buf + (uint8_t)ESCOFFSET);
      bNextIsEscaped = false;
    }
    else if (buf == MSGESC)
      bNextIsEscaped = true;
    else
      msg.push_back(buf);
  }

  return bGotFullMessage;
}

std::string CAdapterCommunication::GetError(void) const
{
  return m_port->GetError();
}

bool CAdapterCommunication::StartBootloader(void)
{
  if (!IsRunning())
    return false;

  m_controller->AddLog(CEC_LOG_DEBUG, "starting the bootloader");
  cec_adapter_message output;
  output.clear();

  output.push_back(MSGSTART);
  PushEscaped(output, MSGCODE_START_BOOTLOADER);
  output.push_back(MSGEND);

  if (!Write(output))
  {
    m_controller->AddLog(CEC_LOG_ERROR, "could not start the bootloader");
    return false;
  }
  m_controller->AddLog(CEC_LOG_DEBUG, "bootloader start command transmitted");
  return true;
}

void CAdapterCommunication::PushEscaped(cec_adapter_message &vec, uint8_t byte)
{
  if (byte >= MSGESC && byte != MSGSTART)
  {
    vec.push_back(MSGESC);
    vec.push_back(byte - ESCOFFSET);
  }
  else
  {
    vec.push_back(byte);
  }
}

bool CAdapterCommunication::SetAckMask(uint16_t iMask)
{
  if (!IsRunning())
    return false;

  CStdString strLog;
  strLog.Format("setting ackmask to %2x", iMask);
  m_controller->AddLog(CEC_LOG_DEBUG, strLog.c_str());

  cec_adapter_message output;
  output.clear();

  output.push_back(MSGSTART);
  PushEscaped(output, MSGCODE_SET_ACK_MASK);
  PushEscaped(output, iMask >> 8);
  PushEscaped(output, (uint8_t)iMask);
  output.push_back(MSGEND);

  if (!Write(output))
  {
    m_controller->AddLog(CEC_LOG_ERROR, "could not set the ackmask");
    return false;
  }

  return true;
}

bool CAdapterCommunication::PingAdapter(void)
{
  if (!IsRunning())
    return false;

  m_controller->AddLog(CEC_LOG_DEBUG, "sending ping");
  cec_adapter_message output;
  output.clear();

  output.push_back(MSGSTART);
  PushEscaped(output, MSGCODE_PING);
  output.push_back(MSGEND);

  if (!Write(output))
  {
    m_controller->AddLog(CEC_LOG_ERROR, "could not send ping command");
    return false;
  }

  m_controller->AddLog(CEC_LOG_DEBUG, "ping tranmitted");

  // TODO check for pong
  return true;
}

bool CAdapterCommunication::IsOpen(void) const
{
  return !IsStopped() && m_port->IsOpen();
}

void CAdapterCommunication::FormatAdapterMessage(const cec_command &command, cec_adapter_message &packet)
{
  packet.clear();

  //set ack polarity to high when transmitting to the broadcast address
  //set ack polarity low when transmitting to any other address
  packet.push_back(MSGSTART);
  PushEscaped(packet, MSGCODE_TRANSMIT_ACK_POLARITY);
  if (command.destination == CECDEVICE_BROADCAST)
    PushEscaped(packet, CEC_TRUE);
  else
    PushEscaped(packet, CEC_FALSE);
  packet.push_back(MSGEND);

  // add source and destination
  packet.push_back(MSGSTART);
  PushEscaped(packet, MSGCODE_TRANSMIT);
  packet.push_back(((uint8_t)command.initiator << 4) + (uint8_t)command.destination);
  packet.push_back(MSGEND);

  // add opcode
  packet.push_back(MSGSTART);
  PushEscaped(packet, command.parameters.empty() ? (uint8_t)MSGCODE_TRANSMIT_EOM : (uint8_t)MSGCODE_TRANSMIT);
  packet.push_back((uint8_t) command.opcode);
  packet.push_back(MSGEND);

  // add parameters
  for (int8_t iPtr = 0; iPtr < command.parameters.size; iPtr++)
  {
    packet.push_back(MSGSTART);

    if (iPtr == command.parameters.size - 1)
      PushEscaped(packet, MSGCODE_TRANSMIT_EOM);
    else
      PushEscaped(packet, MSGCODE_TRANSMIT);

    PushEscaped(packet, command.parameters[iPtr]);

    packet.push_back(MSGEND);
  }
}

