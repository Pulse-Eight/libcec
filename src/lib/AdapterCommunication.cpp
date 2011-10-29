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
  StopThread();

  m_rcvCondition.Broadcast();
}

void *CAdapterCommunication::Process(void)
{
  m_controller->AddLog(CEC_LOG_DEBUG, "communication thread started");

  while (!IsStopped())
  {
    ReadFromDevice(500);
    WriteNextCommand();
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

void CAdapterCommunication::WriteNextCommand(void)
{
  CCECAdapterMessage msg;
  if (m_outBuffer.Pop(msg))
  {
    if (m_port->Write(msg) != (int32_t) msg.size())
    {
      CStdString strError;
      strError.Format("error writing to serial port: %s", m_port->GetError().c_str());
      m_controller->AddLog(CEC_LOG_ERROR, strError);
    }
    else
    {
      m_controller->AddLog(CEC_LOG_DEBUG, "command sent");
      CCondition::Sleep((uint32_t) msg.size() * (uint32_t)24 /*data*/ + (uint32_t)5 /*start bit (4.5 ms)*/);
    }
  }
}

bool CAdapterCommunication::Write(const CCECAdapterMessage &data)
{
  m_outBuffer.Push(data);
  return true;
}

bool CAdapterCommunication::Read(CCECAdapterMessage &msg, uint32_t iTimeout)
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
  CCECAdapterMessage output;

  output.push_back(MSGSTART);
  output.push_escaped(MSGCODE_START_BOOTLOADER);
  output.push_back(MSGEND);

  if (!Write(output))
  {
    m_controller->AddLog(CEC_LOG_ERROR, "could not start the bootloader");
    return false;
  }
  m_controller->AddLog(CEC_LOG_DEBUG, "bootloader start command transmitted");
  return true;
}

bool CAdapterCommunication::SetAckMask(uint16_t iMask)
{
  if (!IsRunning())
    return false;

  CStdString strLog;
  strLog.Format("setting ackmask to %2x", iMask);
  m_controller->AddLog(CEC_LOG_DEBUG, strLog.c_str());

  CCECAdapterMessage output;

  output.push_back(MSGSTART);
  output.push_escaped(MSGCODE_SET_ACK_MASK);
  output.push_escaped(iMask >> 8);
  output.push_escaped((uint8_t)iMask);
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
  CCECAdapterMessage output;

  output.push_back(MSGSTART);
  output.push_escaped(MSGCODE_PING);
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
