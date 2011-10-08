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

using namespace std;
using namespace CEC;

CAdapterCommunication::CAdapterCommunication(CLibCEC *controller) :
    m_port(NULL),
    m_controller(controller),
    m_inbuf(NULL),
    m_iInbufSize(0),
    m_iInbufUsed(0)
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

  if (m_inbuf)
    free(m_inbuf);
}

bool CAdapterCommunication::Open(const char *strPort, uint16_t iBaudRate /* = 38400 */, uint32_t iTimeoutMs /* = 10000 */)
{
  CLockObject lock(&m_mutex);
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
  m_port->Read(buff, sizeof(buff), 50);

  Sleep(CEC_SETTLE_DOWN_TIME);

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
  m_rcvCondition.Broadcast();

  CLockObject lock(&m_mutex);
  StopThread();

  if (m_inbuf)
  {
    free(m_inbuf);
    m_inbuf = NULL;
    m_iInbufSize = 0;
    m_iInbufUsed = 0;
  }
}

void *CAdapterCommunication::Process(void)
{
  m_controller->AddLog(CEC_LOG_DEBUG, "communication thread started");

  while (!IsStopped())
  {
    {
      CLockObject lock(&m_mutex);
      if (ReadFromDevice(50))
        m_rcvCondition.Signal();
    }

    if (!IsStopped())
      Sleep(50);
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
    StopThread(false);
    return false;
  }
  else if (iBytesRead > 0)
    AddData(buff, (uint8_t) iBytesRead);

  return iBytesRead > 0;
}

void CAdapterCommunication::AddData(uint8_t *data, uint8_t iLen)
{
  if (m_iInbufUsed + iLen > m_iInbufSize)
  {
    m_iInbufSize = m_iInbufUsed + iLen;
    m_inbuf = (uint8_t*)realloc(m_inbuf, m_iInbufSize);
  }

  memcpy(m_inbuf + m_iInbufUsed, data, iLen);
  m_iInbufUsed += iLen;
}

bool CAdapterCommunication::Write(const cec_frame &data)
{
  {
    CLockObject lock(&m_mutex);
    if (m_port->Write(data) != (int32_t) data.size)
    {
      CStdString strError;
      strError.Format("error writing to serial port: %s", m_port->GetError().c_str());
      m_controller->AddLog(CEC_LOG_ERROR, strError);
      return false;
    }

    m_controller->AddLog(CEC_LOG_DEBUG, "command sent");
  }

  return true;
}

bool CAdapterCommunication::Read(cec_frame &msg, uint32_t iTimeout)
{
  CLockObject lock(&m_mutex);

  if (m_iInbufUsed < 1)
  {
    if (!m_rcvCondition.Wait(&m_mutex, iTimeout))
      return false;
  }

  if (m_iInbufUsed < 1 || IsStopped())
    return false;

  //search for first start of message
  int16_t startpos = -1;
  for (int16_t iPtr = 0; iPtr < m_iInbufUsed; iPtr++)
  {
    if (m_inbuf[iPtr] == MSGSTART)
    {
      startpos = iPtr;
      break;
    }
  }

  if (startpos == -1)
    return false;

  //move anything from the first start of message to the beginning of the buffer
  if (startpos > 0)
  {
    memmove(m_inbuf, m_inbuf + startpos, m_iInbufUsed - startpos);
    m_iInbufUsed -= startpos;
  }

  if (m_iInbufUsed < 2)
    return false;

  //look for end of message
  startpos = -1;
  int16_t endpos = -1;
  for (int16_t iPtr = 1; iPtr < m_iInbufUsed; iPtr++)
  {
    if (m_inbuf[iPtr] == MSGEND)
    {
      endpos = iPtr;
      break;
    }
    else if (m_inbuf[iPtr] == MSGSTART)
    {
      startpos = iPtr;
      break;
    }
  }

  if (startpos > 0) //we found a msgstart before msgend, this is not right, remove
  {
    m_controller->AddLog(CEC_LOG_ERROR, "received MSGSTART before MSGEND");
    memmove(m_inbuf, m_inbuf + startpos, m_iInbufUsed - startpos);
    m_iInbufUsed -= startpos;
    return false;
  }

  if (endpos > 0) //found a MSGEND
  {
    msg.clear();
    bool isesc = false;
    for (int16_t iPtr = 1; iPtr < endpos; iPtr++)
    {
      if (isesc)
      {
        msg.push_back(m_inbuf[iPtr] + (uint8_t)ESCOFFSET);
        isesc = false;
      }
      else if (m_inbuf[iPtr] == MSGESC)
      {
        isesc = true;
      }
      else
      {
        msg.push_back(m_inbuf[iPtr]);
      }
    }

    if (endpos + 1 < m_iInbufUsed)
      memmove(m_inbuf, m_inbuf + endpos + 1, m_iInbufUsed - endpos - 1);

    m_iInbufUsed -= endpos + 1;

    return true;
  }

  return false;
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
  cec_frame output;
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

void CAdapterCommunication::PushEscaped(cec_frame &vec, uint8_t byte)
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

  cec_frame output;
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
  cec_frame output;
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
