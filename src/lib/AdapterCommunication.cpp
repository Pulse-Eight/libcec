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

#include "CECProcessor.h"
#include "platform/serialport.h"
#include "util/StdString.h"
#include "platform/timeutils.h"

using namespace std;
using namespace CEC;

CCECAdapterMessage::CCECAdapterMessage(const cec_command &command)
{
  clear();

  //set ack polarity to high when transmitting to the broadcast address
  //set ack polarity low when transmitting to any other address
  push_back(MSGSTART);
  push_escaped(MSGCODE_TRANSMIT_ACK_POLARITY);
  if (command.destination == CECDEVICE_BROADCAST)
    push_escaped(CEC_TRUE);
  else
    push_escaped(CEC_FALSE);
  push_back(MSGEND);

  // add source and destination
  push_back(MSGSTART);
  push_escaped(command.opcode_set == 0 ? (uint8_t)MSGCODE_TRANSMIT_EOM : (uint8_t)MSGCODE_TRANSMIT);
  push_back(((uint8_t)command.initiator << 4) + (uint8_t)command.destination);
  push_back(MSGEND);

  // add opcode
  if (command.opcode_set == 1)
  {
    push_back(MSGSTART);
    push_escaped(command.parameters.IsEmpty() ? (uint8_t)MSGCODE_TRANSMIT_EOM : (uint8_t)MSGCODE_TRANSMIT);
    push_back((uint8_t) command.opcode);
    push_back(MSGEND);

    // add parameters
    for (int8_t iPtr = 0; iPtr < command.parameters.size; iPtr++)
    {
      push_back(MSGSTART);

      if (iPtr == command.parameters.size - 1)
        push_escaped( MSGCODE_TRANSMIT_EOM);
      else
        push_escaped(MSGCODE_TRANSMIT);

      push_escaped(command.parameters[iPtr]);

      push_back(MSGEND);
    }
  }

  // set timeout
  transmit_timeout = command.transmit_timeout;
}

CCECAdapterMessage &CCECAdapterMessage::operator =(const CCECAdapterMessage &msg)
{
  packet = msg.packet;
  state  = msg.state;
  return *this;
}

CStdString CCECAdapterMessage::MessageCodeAsString(void) const
{
  CStdString strMsg;
  switch (message())
  {
  case MSGCODE_NOTHING:
    strMsg = "NOTHING";
    break;
  case MSGCODE_PING:
    strMsg = "PING";
    break;
  case MSGCODE_TIMEOUT_ERROR:
    strMsg = "TIMEOUT";
    break;
  case MSGCODE_HIGH_ERROR:
    strMsg = "HIGH_ERROR";
    break;
  case MSGCODE_LOW_ERROR:
    strMsg = "LOW_ERROR";
    break;
  case MSGCODE_FRAME_START:
    strMsg = "FRAME_START";
    break;
  case MSGCODE_FRAME_DATA:
    strMsg = "FRAME_DATA";
    break;
  case MSGCODE_RECEIVE_FAILED:
    strMsg = "RECEIVE_FAILED";
    break;
  case MSGCODE_COMMAND_ACCEPTED:
    strMsg = "COMMAND_ACCEPTED";
    break;
  case MSGCODE_COMMAND_REJECTED:
    strMsg = "COMMAND_REJECTED";
    break;
  case MSGCODE_SET_ACK_MASK:
    strMsg = "SET_ACK_MASK";
    break;
  case MSGCODE_TRANSMIT:
    strMsg = "TRANSMIT";
    break;
  case MSGCODE_TRANSMIT_EOM:
    strMsg = "TRANSMIT_EOM";
    break;
  case MSGCODE_TRANSMIT_IDLETIME:
    strMsg = "TRANSMIT_IDLETIME";
    break;
  case MSGCODE_TRANSMIT_ACK_POLARITY:
    strMsg = "TRANSMIT_ACK_POLARITY";
    break;
  case MSGCODE_TRANSMIT_LINE_TIMEOUT:
    strMsg = "TRANSMIT_LINE_TIMEOUT";
    break;
  case MSGCODE_TRANSMIT_SUCCEEDED:
    strMsg = "TRANSMIT_SUCCEEDED";
    break;
  case MSGCODE_TRANSMIT_FAILED_LINE:
    strMsg = "TRANSMIT_FAILED_LINE";
    break;
  case MSGCODE_TRANSMIT_FAILED_ACK:
    strMsg = "TRANSMIT_FAILED_ACK";
    break;
  case MSGCODE_TRANSMIT_FAILED_TIMEOUT_DATA:
    strMsg = "TRANSMIT_FAILED_TIMEOUT_DATA";
    break;
  case MSGCODE_TRANSMIT_FAILED_TIMEOUT_LINE:
    strMsg = "TRANSMIT_FAILED_TIMEOUT_LINE";
    break;
  case MSGCODE_FIRMWARE_VERSION:
    strMsg = "FIRMWARE_VERSION";
    break;
  case MSGCODE_START_BOOTLOADER:
    strMsg = "START_BOOTLOADER";
    break;
  case MSGCODE_FRAME_EOM:
    strMsg = "FRAME_EOM";
    break;
  case MSGCODE_FRAME_ACK:
    strMsg = "FRAME_ACK";
    break;
  }

  return strMsg;
}

CStdString CCECAdapterMessage::ToString(void) const
{
  CStdString strMsg;
  strMsg = MessageCodeAsString();

  switch (message())
  {
  case MSGCODE_TIMEOUT_ERROR:
  case MSGCODE_HIGH_ERROR:
  case MSGCODE_LOW_ERROR:
    {
      int iLine      = (size() >= 3) ? (at(1) << 8) | at(2) : 0;
      uint32_t iTime = (size() >= 7) ? (at(3) << 24) | (at(4) << 16) | (at(5) << 8) | at(6) : 0;
      strMsg.AppendFormat(" line:%i", iLine);
      strMsg.AppendFormat(" time:%u", iTime);
    }
    break;
  case MSGCODE_FRAME_START:
    if (size() >= 2)
      strMsg.AppendFormat(" initiator:%1x destination:%1x ack:%s %s", initiator(), destination(), ack() ? "high" : "low", eom() ? "eom" : "");
    break;
  case MSGCODE_FRAME_DATA:
    if (size() >= 2)
      strMsg.AppendFormat(" %02x %s", at(1), eom() ? "eom" : "");
    break;
  default:
    break;
  }

  return strMsg;
}

bool CCECAdapterMessage::is_error(void) const
{
  cec_adapter_messagecode code = message();
  return (code == MSGCODE_HIGH_ERROR ||
    code == MSGCODE_LOW_ERROR ||
    code == MSGCODE_RECEIVE_FAILED ||
    code == MSGCODE_COMMAND_REJECTED ||
    code ==  MSGCODE_TRANSMIT_LINE_TIMEOUT ||
    code == MSGCODE_TRANSMIT_FAILED_LINE ||
    code ==  MSGCODE_TRANSMIT_FAILED_ACK ||
    code == MSGCODE_TRANSMIT_FAILED_TIMEOUT_DATA ||
    code == MSGCODE_TRANSMIT_FAILED_TIMEOUT_LINE);
}

void CCECAdapterMessage::push_escaped(uint8_t byte)
{
  if (byte >= MSGESC)
  {
    push_back(MSGESC);
    push_back(byte - ESCOFFSET);
  }
  else
    push_back(byte);
}

CAdapterCommunication::CAdapterCommunication(CCECProcessor *processor) :
    m_port(NULL),
    m_processor(processor)
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
  CLockObject lock(&m_mutex);
  if (!m_port)
  {
    m_processor->AddLog(CEC_LOG_ERROR, "port is NULL");
    return false;
  }

  if (IsOpen())
  {
    m_processor->AddLog(CEC_LOG_ERROR, "port is already open");
  }

  if (!m_port->Open(strPort, iBaudRate))
  {
    CStdString strError;
    strError.Format("error opening serial port '%s': %s", strPort, m_port->GetError().c_str());
    m_processor->AddLog(CEC_LOG_ERROR, strError);
    return false;
  }

  m_processor->AddLog(CEC_LOG_DEBUG, "connection opened");

  //clear any input bytes
  uint8_t buff[1024];
  while (m_port->Read(buff, sizeof(buff), 500) > 0) {}

  if (CreateThread())
  {
    m_startCondition.Wait(&m_mutex);
    m_processor->AddLog(CEC_LOG_DEBUG, "communication thread started");
    return true;
  }
  else
  {
    m_processor->AddLog(CEC_LOG_DEBUG, "could not create a communication thread");
  }

  return false;
}

void CAdapterCommunication::Close(void)
{
  CLockObject lock(&m_mutex);
  m_startCondition.Broadcast();
  m_rcvCondition.Broadcast();
  StopThread();
}

void *CAdapterCommunication::Process(void)
{
  {
    CLockObject lock(&m_mutex);
    m_startCondition.Signal();
  }

  while (!IsStopped())
  {
    ReadFromDevice(500);
    Sleep(5);
    WriteNextCommand();
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
    m_processor->AddLog(CEC_LOG_ERROR, strError);
    return false;
  }
  else if (iBytesRead > 0)
    AddData(buff, (uint8_t) iBytesRead);

  return iBytesRead > 0;
}

void CAdapterCommunication::AddData(uint8_t *data, uint8_t iLen)
{
  CLockObject lock(&m_mutex);
  for (unsigned int iPtr = 0; iPtr < iLen; iPtr++)
    m_inBuffer.Push(data[iPtr]);

  m_rcvCondition.Signal();
}

void CAdapterCommunication::WriteNextCommand(void)
{
  CCECAdapterMessage *msg;
  if (m_outBuffer.Pop(msg))
    SendMessageToAdapter(msg);
}

void CAdapterCommunication::SendMessageToAdapter(CCECAdapterMessage *msg)
{
  CLockObject lock(&msg->mutex);
  if (m_port->Write(msg) != (int32_t) msg->size())
  {
    CStdString strError;
    strError.Format("error writing to serial port: %s", m_port->GetError().c_str());
    m_processor->AddLog(CEC_LOG_ERROR, strError);
    msg->state = ADAPTER_MESSAGE_STATE_ERROR;
  }
  else
  {
    m_processor->AddLog(CEC_LOG_DEBUG, "command sent");
    CCondition::Sleep((uint32_t) msg->size() * 24 /*data*/ + 5 /*start bit (4.5 ms)*/);
    msg->state = ADAPTER_MESSAGE_STATE_SENT;
  }
  msg->condition.Signal();
}

bool CAdapterCommunication::Write(CCECAdapterMessage *data)
{
  data->state = ADAPTER_MESSAGE_STATE_WAITING;
  m_outBuffer.Push(data);
  return true;
}

bool CAdapterCommunication::Read(CCECAdapterMessage &msg, uint32_t iTimeout)
{
  CLockObject lock(&m_mutex);

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
      if (!m_rcvCondition.Wait(&m_mutex, (uint32_t) (iTarget - iNow)))
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
      if (msg.size() > 0)
        m_processor->AddLog(CEC_LOG_WARNING, "received MSGSTART before MSGEND, removing previous buffer contents");
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

  if (bGotFullMessage)
    msg.state = ADAPTER_MESSAGE_STATE_RECEIVED;

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

  m_processor->AddLog(CEC_LOG_DEBUG, "starting the bootloader");
  CCECAdapterMessage *output = new CCECAdapterMessage;

  output->push_back(MSGSTART);
  output->push_escaped(MSGCODE_START_BOOTLOADER);
  output->push_back(MSGEND);

  SendMessageToAdapter(output);
  bReturn = output->state == ADAPTER_MESSAGE_STATE_SENT;
  delete output;

  return bReturn;
}

bool CAdapterCommunication::PingAdapter(void)
{
  bool bReturn(false);
  if (!IsRunning())
    return bReturn;

  m_processor->AddLog(CEC_LOG_DEBUG, "sending ping");
  CCECAdapterMessage *output = new CCECAdapterMessage;

  output->push_back(MSGSTART);
  output->push_escaped(MSGCODE_PING);
  output->push_back(MSGEND);

  SendMessageToAdapter(output);
  bReturn = output->state == ADAPTER_MESSAGE_STATE_SENT;
  delete output;

  return bReturn;
}

bool CAdapterCommunication::IsOpen(void) const
{
  return !IsStopped() && m_port->IsOpen() && IsRunning();
}
