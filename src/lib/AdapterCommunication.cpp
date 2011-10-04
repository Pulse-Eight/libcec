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
    m_controller(controller),
    m_inbuf(NULL),
    m_iInbufSize(0),
    m_iInbufUsed(0),
    m_bStarted(false),
    m_bStop(false)
{
  m_port = new CSerialPort;
}

CAdapterCommunication::~CAdapterCommunication(void)
{
  m_port->Close();
  m_port = NULL;
}

bool CAdapterCommunication::Open(const char *strPort, int iBaudRate /* = 38400 */, int iTimeoutMs /* = 10000 */)
{
  CLockObject lock(&m_commMutex);
  if (m_bStarted)
    return false;

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

  CCondition::Sleep(CEC_SETTLE_DOWN_TIME);

  m_bStop = false;
  m_bStarted = true;

  if (CreateThread())
  {
    m_controller->AddLog(CEC_LOG_DEBUG, "reader thread created");
    return true;
  }
  else
  {
    m_controller->AddLog(CEC_LOG_DEBUG, "could not create a reader thread");
  }

  return false;
}

void CAdapterCommunication::Close(void)
{
  StopThread();
  m_port->Close();
}

void *CAdapterCommunication::Process(void)
{
  while (!m_bStop)
  {
    if (!ReadFromDevice(250))
    {
      m_bStarted = false;
      break;
    }

    CCondition::Sleep(50);
  }

  m_controller->AddLog(CEC_LOG_DEBUG, "reader thread terminated");

  CLockObject lock(&m_commMutex);
  m_bStarted = false;
  return NULL;
}

bool CAdapterCommunication::ReadFromDevice(int iTimeout)
{
  uint8_t buff[1024];
  CLockObject lock(&m_commMutex);
  int iBytesRead = m_port->Read(buff, sizeof(buff), iTimeout);
  lock.Leave();
  if (iBytesRead < 0)
  {
    CStdString strError;
    strError.Format("error reading from serial port: %s", m_port->GetError().c_str());
    m_controller->AddLog(CEC_LOG_ERROR, strError);
    return false;
  }
  else if (iBytesRead > 0)
    AddData(buff, iBytesRead);

  return true;
}

void CAdapterCommunication::AddData(uint8_t *data, int iLen)
{
  CLockObject lock(&m_bufferMutex);
  if (iLen + m_iInbufUsed > m_iInbufSize)
  {
    m_iInbufSize = iLen + m_iInbufUsed;
    m_inbuf = (uint8_t*)realloc(m_inbuf, m_iInbufSize);
  }

  memcpy(m_inbuf + m_iInbufUsed, data, iLen);
  m_iInbufUsed += iLen;
  lock.Leave();
  m_condition.Signal();
}

bool CAdapterCommunication::Write(const cec_frame &data)
{
  CLockObject lock(&m_commMutex);

  if (m_port->Write(data) != data.size())
  {
    CStdString strError;
    strError.Format("error writing to serial port: %s", m_port->GetError().c_str());
    m_controller->AddLog(CEC_LOG_ERROR, strError);
    return false;
  }

  m_controller->AddLog(CEC_LOG_DEBUG, "command sent");

  CCondition::Sleep((int) data.size() * 24 /*data*/ + 5 /*start bit (4.5 ms)*/ + 50 /* to be on the safe side */);

  return true;
}

bool CAdapterCommunication::Read(cec_frame &msg, int iTimeout)
{
  CLockObject lock(&m_bufferMutex);

  if (m_iInbufUsed < 1)
    m_condition.Wait(&m_bufferMutex, iTimeout);

  if (m_iInbufUsed < 1)
    return false;

  //search for first start of message
  int startpos = -1;
  for (int i = 0; i < m_iInbufUsed; i++)
  {
    if (m_inbuf[i] == MSGSTART)
    {
      startpos = i;
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
  int endpos = -1;
  for (int i = 1; i < m_iInbufUsed; i++)
  {
    if (m_inbuf[i] == MSGEND)
    {
      endpos = i;
      break;
    }
    else if (m_inbuf[i] == MSGSTART)
    {
      startpos = i;
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
    for (int i = 1; i < endpos; i++)
    {
      if (isesc)
      {
        msg.push_back(m_inbuf[i] + (uint8_t)ESCOFFSET);
        isesc = false;
      }
      else if (m_inbuf[i] == MSGESC)
      {
        isesc = true;
      }
      else
      {
        msg.push_back(m_inbuf[i]);
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
