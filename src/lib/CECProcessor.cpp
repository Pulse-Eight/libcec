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

#include "CECProcessor.h"

#include "AdapterCommunication.h"
#include "devices/CECBusDevice.h"
#include "LibCEC.h"
#include "util/StdString.h"
#include "platform/timeutils.h"

using namespace CEC;
using namespace std;

CCECProcessor::CCECProcessor(CLibCEC *controller, CAdapterCommunication *serComm, const char *strDeviceName, cec_logical_address iLogicalAddress /* = CECDEVICE_PLAYBACKDEVICE1 */, uint16_t iPhysicalAddress /* = CEC_DEFAULT_PHYSICAL_ADDRESS*/) :
    m_iLogicalAddress(iLogicalAddress),
    m_strDeviceName(strDeviceName),
    m_communication(serComm),
    m_controller(controller),
    m_bMonitor(false)
{
  for (int iPtr = 0; iPtr < 16; iPtr++)
    m_busDevices[iPtr] = new CCECBusDevice(this, (cec_logical_address) iPtr, iPtr == iLogicalAddress ? iPhysicalAddress : 0);
}

CCECProcessor::~CCECProcessor(void)
{
  StopThread();
  m_communication = NULL;
  m_controller = NULL;
  for (unsigned int iPtr = 0; iPtr < 16; iPtr++)
    delete m_busDevices[iPtr];
}

bool CCECProcessor::Start(void)
{
  if (!m_communication || !m_communication->IsOpen())
  {
    m_controller->AddLog(CEC_LOG_ERROR, "connection is closed");
    return false;
  }

  if (!SetLogicalAddress(m_iLogicalAddress))
  {
    m_controller->AddLog(CEC_LOG_ERROR, "could not set the logical address");
    return false;
  }

  if (CreateThread())
    return true;
  else
    m_controller->AddLog(CEC_LOG_ERROR, "could not create a processor thread");

  return false;
}

void *CCECProcessor::Process(void)
{
  m_controller->AddLog(CEC_LOG_DEBUG, "processor thread started");

  cec_command           command;
  CCECAdapterMessage    msg;
  CCECAdapterMessagePtr msgPtr;

  while (!IsStopped())
  {
    bool bParseFrame(false);
    command.clear();
    msg.clear();

    {
      CLockObject lock(&m_mutex);
      if (m_frameBuffer.Pop(msgPtr))
        bParseFrame = ParseMessage(msgPtr);
      else if (m_communication->IsOpen() && m_communication->Read(msg, 50))
      {
        msgPtr = CCECAdapterMessagePtr(new CCECAdapterMessage(msg));
        bParseFrame = ParseMessage(msgPtr);
      }

      bParseFrame &= !IsStopped();
      if (bParseFrame)
        command = m_currentframe;
    }

    if (bParseFrame)
      ParseCommand(command);

    m_controller->CheckKeypressTimeout();

    for (unsigned int iDevicePtr = 0; iDevicePtr < 16; iDevicePtr++)
      m_busDevices[iDevicePtr]->PollVendorId();

    if (!IsStopped())
      Sleep(5);
  }

  return NULL;
}

bool CCECProcessor::SetActiveView(void)
{
  if (!IsRunning())
    return false;

  return m_busDevices[m_iLogicalAddress]->BroadcastActiveView();
}

bool CCECProcessor::SetInactiveView(void)
{
  if (!IsRunning())
    return false;

  return m_busDevices[m_iLogicalAddress]->BroadcastInactiveView();
}

void CCECProcessor::LogOutput(const cec_command &data)
{
  CStdString strTx;
  strTx.Format("<< %02x:%02x", ((uint8_t)data.initiator << 4) + (uint8_t)data.destination, (uint8_t)data.opcode);

  for (uint8_t iPtr = 0; iPtr < data.parameters.size; iPtr++)
    strTx.AppendFormat(":%02x", data.parameters[iPtr]);
  m_controller->AddLog(CEC_LOG_TRAFFIC, strTx.c_str());
}

bool CCECProcessor::SetLogicalAddress(cec_logical_address iLogicalAddress)
{
  if (m_iLogicalAddress != iLogicalAddress)
  {
    CStdString strLog;
    strLog.Format("<< setting logical address to %1x", iLogicalAddress);
    m_controller->AddLog(CEC_LOG_NOTICE, strLog.c_str());

    m_iLogicalAddress = iLogicalAddress;
    return m_communication && m_communication->SetAckMask(0x1 << (uint8_t)m_iLogicalAddress);
  }

  return true;
}

bool CCECProcessor::SetPhysicalAddress(uint16_t iPhysicalAddress)
{
  m_busDevices[m_iLogicalAddress]->SetPhysicalAddress(iPhysicalAddress);
  return m_busDevices[m_iLogicalAddress]->BroadcastActiveView();
}

bool CCECProcessor::SwitchMonitoring(bool bEnable)
{
  CStdString strLog;
  strLog.Format("== %s monitoring mode ==", bEnable ? "enabling" : "disabling");
  m_controller->AddLog(CEC_LOG_NOTICE, strLog.c_str());

  m_bMonitor = bEnable;
  if (bEnable)
    return m_communication && m_communication->SetAckMask(0);
  else
    return m_communication && m_communication->SetAckMask(0x1 << (uint8_t)m_iLogicalAddress);
}

bool CCECProcessor::Transmit(const cec_command &data, bool bWaitForAck /* = true */)
{
  bool bReturn(false);
  LogOutput(data);

  CCECAdapterMessagePtr output(new CCECAdapterMessage(data));

  CLockObject lock(&m_mutex);
  {
    CLockObject msgLock(&output->mutex);
    if (!m_communication || !m_communication->Write(output))
      return bReturn;
    else
      output->condition.Wait(&output->mutex);
  }

  if (bWaitForAck)
  {
    bool bError(false);
    if ((bReturn = WaitForAck(&bError, output->size(), 1000)) == false)
      m_controller->AddLog(CEC_LOG_ERROR, "did not receive ack");
  }
  else
  {
    bReturn = true;
  }

  return bReturn;
}

void CCECProcessor::TransmitAbort(cec_logical_address address, cec_opcode opcode, ECecAbortReason reason /* = CEC_ABORT_REASON_UNRECOGNIZED_OPCODE */)
{
  m_controller->AddLog(CEC_LOG_DEBUG, "<< transmitting abort message");

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, address, CEC_OPCODE_FEATURE_ABORT);
  command.parameters.push_back((uint8_t)opcode);
  command.parameters.push_back((uint8_t)reason);

  Transmit(command);
}

bool CCECProcessor::WaitForAck(bool *bError, uint8_t iLength, uint32_t iTimeout /* = 1000 */)
{
  bool bTransmitSucceeded = false;
  uint8_t iPacketsLeft(iLength / 4);
  *bError = false;

  int64_t iNow = GetTimeMs();
  int64_t iTargetTime = iNow + (uint64_t) iTimeout;

  while (!bTransmitSucceeded && !*bError && (iTimeout == 0 || iNow < iTargetTime))
  {
    CCECAdapterMessage msg;

    if (!m_communication->Read(msg, iTimeout > 0 ? (int32_t)(iTargetTime - iNow) : 1000))
    {
      iNow = GetTimeMs();
      continue;
    }

    switch(msg.message())
    {
    case MSGCODE_TIMEOUT_ERROR:
    case MSGCODE_HIGH_ERROR:
    case MSGCODE_LOW_ERROR:
      {
        CStdString logStr;
        if (msg.message() == MSGCODE_TIMEOUT_ERROR)
          logStr = "MSGCODE_TIMEOUT";
        else if (msg.message() == MSGCODE_HIGH_ERROR)
          logStr = "MSGCODE_HIGH_ERROR";
        else
          logStr = "MSGCODE_LOW_ERROR";

        int iLine      = (msg.size() >= 3) ? (msg[1] << 8) | (msg[2]) : 0;
        uint32_t iTime = (msg.size() >= 7) ? (msg[3] << 24) | (msg[4] << 16) | (msg[5] << 8) | (msg[6]) : 0;
        logStr.AppendFormat(" line:%i", iLine);
        logStr.AppendFormat(" time:%u", iTime);
        m_controller->AddLog(CEC_LOG_WARNING, logStr.c_str());
        *bError = true;
      }
      break;
    case MSGCODE_COMMAND_ACCEPTED:
      m_controller->AddLog(CEC_LOG_DEBUG, "MSGCODE_COMMAND_ACCEPTED");
      iPacketsLeft--;
      break;
    case MSGCODE_TRANSMIT_SUCCEEDED:
      m_controller->AddLog(CEC_LOG_DEBUG, "MSGCODE_TRANSMIT_SUCCEEDED");
      bTransmitSucceeded = (iPacketsLeft == 0);
      *bError = !bTransmitSucceeded;
      break;
    case MSGCODE_RECEIVE_FAILED:
      m_controller->AddLog(CEC_LOG_WARNING, "MSGCODE_RECEIVE_FAILED");
      *bError = true;
      break;
    case MSGCODE_COMMAND_REJECTED:
      m_controller->AddLog(CEC_LOG_WARNING, "MSGCODE_COMMAND_REJECTED");
      *bError = true;
      break;
    case MSGCODE_TRANSMIT_FAILED_LINE:
      m_controller->AddLog(CEC_LOG_WARNING, "MSGCODE_TRANSMIT_FAILED_LINE");
      *bError = true;
      break;
    case MSGCODE_TRANSMIT_FAILED_ACK:
      m_controller->AddLog(CEC_LOG_WARNING, "MSGCODE_TRANSMIT_FAILED_ACK");
      *bError = true;
      break;
    case MSGCODE_TRANSMIT_FAILED_TIMEOUT_DATA:
      m_controller->AddLog(CEC_LOG_WARNING, "MSGCODE_TRANSMIT_FAILED_TIMEOUT_DATA");
      *bError = true;
      break;
    case MSGCODE_TRANSMIT_FAILED_TIMEOUT_LINE:
      m_controller->AddLog(CEC_LOG_WARNING, "MSGCODE_TRANSMIT_FAILED_TIMEOUT_LINE");
      *bError = true;
      break;
    default:
      CCECAdapterMessagePtr msgPtr = CCECAdapterMessagePtr(new CCECAdapterMessage(msg));
      m_frameBuffer.Push(msgPtr);
      break;
    }

    iNow = GetTimeMs();
  }

  return bTransmitSucceeded && !*bError;
}

bool CCECProcessor::ParseMessage(CCECAdapterMessagePtr msg)
{
  bool bEom = false;

  if (msg->empty())
    return bEom;

  CStdString logStr;

  switch(msg->message())
  {
  case MSGCODE_NOTHING:
    m_controller->AddLog(CEC_LOG_DEBUG, "MSGCODE_NOTHING");
    break;
  case MSGCODE_TIMEOUT_ERROR:
  case MSGCODE_HIGH_ERROR:
  case MSGCODE_LOW_ERROR:
    {
      if (msg->message() == MSGCODE_TIMEOUT_ERROR)
        logStr = "MSGCODE_TIMEOUT";
      else if (msg->message() == MSGCODE_HIGH_ERROR)
        logStr = "MSGCODE_HIGH_ERROR";
      else
        logStr = "MSGCODE_LOW_ERROR";

      int iLine      = (msg->size() >= 3) ? (msg->at(1) << 8) | (msg->at(2)) : 0;
      uint32_t iTime = (msg->size() >= 7) ? (msg->at(3) << 24) | (msg->at(4) << 16) | (msg->at(5) << 8) | (msg->at(6)) : 0;
      logStr.AppendFormat(" line:%i", iLine);
      logStr.AppendFormat(" time:%u", iTime);
      m_controller->AddLog(CEC_LOG_WARNING, logStr.c_str());
    }
    break;
  case MSGCODE_FRAME_START:
    {
      logStr = "MSGCODE_FRAME_START";
      m_currentframe.clear();
      if (msg->size() >= 2)
      {
        logStr.AppendFormat(" initiator:%u destination:%u ack:%s %s", msg->initiator(), msg->destination(), msg->ack() ? "high" : "low", msg->eom() ? "eom" : "");
        m_currentframe.initiator   = msg->initiator();
        m_currentframe.destination = msg->destination();
        m_currentframe.ack         = msg->ack();
        m_currentframe.eom         = msg->eom();
      }
      m_controller->AddLog(CEC_LOG_DEBUG, logStr.c_str());
    }
    break;
  case MSGCODE_FRAME_DATA:
    {
      logStr = "MSGCODE_FRAME_DATA";
      if (msg->size() >= 2)
      {
        uint8_t iData = msg->at(1);
        logStr.AppendFormat(" %02x", iData);
        m_currentframe.push_back(iData);
        m_currentframe.eom = msg->eom();
      }
      m_controller->AddLog(CEC_LOG_DEBUG, logStr.c_str());

      bEom = msg->eom();
    }
    break;
  case MSGCODE_COMMAND_ACCEPTED:
    m_controller->AddLog(CEC_LOG_DEBUG, "MSGCODE_COMMAND_ACCEPTED");
    break;
  case MSGCODE_TRANSMIT_SUCCEEDED:
    m_controller->AddLog(CEC_LOG_DEBUG, "MSGCODE_TRANSMIT_SUCCEEDED");
    break;
  case MSGCODE_RECEIVE_FAILED:
    m_controller->AddLog(CEC_LOG_WARNING, "MSGCODE_RECEIVE_FAILED");
    break;
  case MSGCODE_COMMAND_REJECTED:
    m_controller->AddLog(CEC_LOG_WARNING, "MSGCODE_COMMAND_REJECTED");
    break;
  case MSGCODE_TRANSMIT_FAILED_LINE:
    m_controller->AddLog(CEC_LOG_WARNING, "MSGCODE_TRANSMIT_FAILED_LINE");
    break;
  case MSGCODE_TRANSMIT_FAILED_ACK:
    m_controller->AddLog(CEC_LOG_WARNING, "MSGCODE_TRANSMIT_FAILED_ACK");
    break;
  case MSGCODE_TRANSMIT_FAILED_TIMEOUT_DATA:
    m_controller->AddLog(CEC_LOG_WARNING, "MSGCODE_TRANSMIT_FAILED_TIMEOUT_DATA");
    break;
  case MSGCODE_TRANSMIT_FAILED_TIMEOUT_LINE:
    m_controller->AddLog(CEC_LOG_WARNING, "MSGCODE_TRANSMIT_FAILED_TIMEOUT_LINE");
    break;
  default:
    break;
  }

  return bEom;
}

void CCECProcessor::ParseCommand(cec_command &command)
{
  CStdString dataStr;
  dataStr.Format(">> %1x%1x:%02x", command.initiator, command.destination, command.opcode);
  for (uint8_t iPtr = 0; iPtr < command.parameters.size; iPtr++)
    dataStr.AppendFormat(":%02x", (unsigned int)command.parameters[iPtr]);
  m_controller->AddLog(CEC_LOG_TRAFFIC, dataStr.c_str());

  if (!m_bMonitor)
    m_busDevices[(uint8_t)command.initiator]->HandleCommand(command);
}

uint16_t CCECProcessor::GetPhysicalAddress(void) const
{
  return m_busDevices[m_iLogicalAddress]->GetPhysicalAddress();
}

void CCECProcessor::SetCurrentButton(cec_user_control_code iButtonCode)
{
  m_controller->SetCurrentButton(iButtonCode);
}

void CCECProcessor::AddCommand(const cec_command &command)
{
  m_controller->AddCommand(command);
}

void CCECProcessor::AddKey(void)
{
  m_controller->AddKey();
}

void CCECProcessor::AddLog(cec_log_level level, const CStdString &strMessage)
{
  m_controller->AddLog(level, strMessage);
}
