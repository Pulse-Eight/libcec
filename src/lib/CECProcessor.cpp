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
  m_startCondition.Broadcast();
  StopThread();
  m_communication = NULL;
  m_controller = NULL;
  for (unsigned int iPtr = 0; iPtr < 16; iPtr++)
    delete m_busDevices[iPtr];
}

bool CCECProcessor::Start(void)
{
  CLockObject lock(&m_mutex);
  if (!m_communication || !m_communication->IsOpen())
  {
    m_controller->AddLog(CEC_LOG_ERROR, "connection is closed");
    return false;
  }

  if (CreateThread())
  {
    if (!m_startCondition.Wait(&m_mutex))
    {
      m_controller->AddLog(CEC_LOG_ERROR, "could not create a processor thread");
      return false;
    }
    return true;
  }
  else
    m_controller->AddLog(CEC_LOG_ERROR, "could not create a processor thread");

  return false;
}

void *CCECProcessor::Process(void)
{
  cec_command           command;
  CCECAdapterMessage    msg;

  SetAckMask(0x1 << (uint8_t)m_iLogicalAddress);

  {
    CLockObject lock(&m_mutex);
    m_controller->AddLog(CEC_LOG_DEBUG, "processor thread started");
    m_startCondition.Signal();
  }

  while (!IsStopped())
  {
    bool bParseFrame(false);
    command.clear();
    msg.clear();

    {
      CLockObject lock(&m_mutex);
      if (m_communication->IsOpen() && m_communication->Read(msg, 50))
      {
        m_controller->AddLog(msg.is_error() ? CEC_LOG_WARNING : CEC_LOG_DEBUG, msg.ToString());
        bParseFrame = ParseMessage(msg) && !IsStopped();
      }

      if (bParseFrame)
        command = m_currentframe;
    }

    if (bParseFrame)
      ParseCommand(command);

    Sleep(5);

    m_controller->CheckKeypressTimeout();

    for (unsigned int iDevicePtr = 0; iDevicePtr < 16; iDevicePtr++)
      m_busDevices[iDevicePtr]->PollVendorId();

    Sleep(5);
  }

  return NULL;
}

bool CCECProcessor::SetActiveView(void)
{
  if (!IsRunning())
    return false;

  if (m_iLogicalAddress != CECDEVICE_UNKNOWN && m_busDevices[m_iLogicalAddress])
    return m_busDevices[m_iLogicalAddress]->BroadcastActiveView();
  return false;
}

bool CCECProcessor::SetInactiveView(void)
{
  if (!IsRunning())
    return false;

  if (m_iLogicalAddress != CECDEVICE_UNKNOWN && m_busDevices[m_iLogicalAddress])
    return m_busDevices[m_iLogicalAddress]->BroadcastInactiveView();
  return false;
}

void CCECProcessor::LogOutput(const cec_command &data)
{
  CStdString strTx;
  strTx.Format("<< %02x", ((uint8_t)data.initiator << 4) + (uint8_t)data.destination);
  if (data.opcode_set)
      strTx.AppendFormat(":%02x", (uint8_t)data.opcode);

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
    return SetAckMask(0x1 << (uint8_t)m_iLogicalAddress);
  }

  return true;
}

bool CCECProcessor::SetPhysicalAddress(uint16_t iPhysicalAddress)
{
  if (m_iLogicalAddress != CECDEVICE_UNKNOWN && m_busDevices[m_iLogicalAddress])
  {
    m_busDevices[m_iLogicalAddress]->SetPhysicalAddress(iPhysicalAddress);
    return m_busDevices[m_iLogicalAddress]->BroadcastActiveView();
  }
  return false;
}

bool CCECProcessor::SwitchMonitoring(bool bEnable)
{
  CStdString strLog;
  strLog.Format("== %s monitoring mode ==", bEnable ? "enabling" : "disabling");
  m_controller->AddLog(CEC_LOG_NOTICE, strLog.c_str());

  m_bMonitor = bEnable;
  if (bEnable)
    return SetAckMask(0);
  else
    return SetAckMask(0x1 << (uint8_t)m_iLogicalAddress);
}

bool CCECProcessor::PollDevice(cec_logical_address iAddress)
{
  if (iAddress != CECDEVICE_UNKNOWN && m_busDevices[iAddress])
    return m_busDevices[iAddress]->PollDevice();
  return false;
}

cec_version CCECProcessor::GetDeviceCecVersion(cec_logical_address iAddress)
{
  return m_busDevices[iAddress]->GetCecVersion();
}

bool CCECProcessor::GetDeviceMenuLanguage(cec_logical_address iAddress, cec_menu_language *language)
{
  if (m_busDevices[iAddress])
  {
    *language = m_busDevices[iAddress]->GetMenuLanguage();
    return (strcmp(language->language, "???") != 0);
  }
  return false;
}

uint64_t CCECProcessor::GetDeviceVendorId(cec_logical_address iAddress)
{
  if (m_busDevices[iAddress])
    return m_busDevices[iAddress]->GetVendorId();
  return false;
}

cec_power_status CCECProcessor::GetDevicePowerStatus(cec_logical_address iAddress)
{
  if (m_busDevices[iAddress])
    return m_busDevices[iAddress]->GetPowerStatus();
  return CEC_POWER_STATUS_UNKNOWN;
}

bool CCECProcessor::Transmit(const cec_command &data)
{
  bool bReturn(false);
  LogOutput(data);

  CCECAdapterMessage *output = new CCECAdapterMessage(data);
  bReturn = Transmit(output);
  delete output;

  return bReturn;
}

bool CCECProcessor::Transmit(CCECAdapterMessage *output)
{
  bool bReturn(false);
  CLockObject lock(&m_mutex);
  {
    CLockObject msgLock(&output->mutex);
    if (!m_communication || !m_communication->Write(output))
      return bReturn;
    else
    {
      output->condition.Wait(&output->mutex);
      if (output->state != ADAPTER_MESSAGE_STATE_SENT)
      {
        m_controller->AddLog(CEC_LOG_ERROR, "command was not sent");
        return bReturn;
      }
    }

    if (output->transmit_timeout > 0)
    {
      if ((bReturn = WaitForTransmitSucceeded(output->size(), output->transmit_timeout)) == false)
        m_controller->AddLog(CEC_LOG_ERROR, "did not receive ack");
    }
    else
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

bool CCECProcessor::WaitForTransmitSucceeded(uint8_t iLength, uint32_t iTimeout /* = 1000 */)
{
  bool bError(false);
  bool bTransmitSucceeded(false);
  uint8_t iPacketsLeft(iLength / 4);

  int64_t iNow = GetTimeMs();
  int64_t iTargetTime = iNow + (uint64_t) iTimeout;

  while (!bTransmitSucceeded && !bError && (iTimeout == 0 || iNow < iTargetTime))
  {
    CCECAdapterMessage msg;

    if (!m_communication->Read(msg, iTimeout > 0 ? (int32_t)(iTargetTime - iNow) : 1000))
    {
      iNow = GetTimeMs();
      continue;
    }

    if ((bError = msg.is_error()) == false)
    {
      m_controller->AddLog(bError ? CEC_LOG_WARNING : CEC_LOG_DEBUG, msg.ToString());

      switch(msg.message())
      {
      case MSGCODE_COMMAND_ACCEPTED:
        if (iPacketsLeft > 0)
          iPacketsLeft--;
        break;
      case MSGCODE_TRANSMIT_SUCCEEDED:
        bTransmitSucceeded = (iPacketsLeft == 0);
        bError = !bTransmitSucceeded;
        break;
      default:
        ParseMessage(msg);
      }

      iNow = GetTimeMs();
    }
  }

  return bTransmitSucceeded && !bError;
}

bool CCECProcessor::ParseMessage(const CCECAdapterMessage &msg)
{
  bool bEom = false;

  if (msg.empty())
    return bEom;

  switch(msg.message())
  {
  case MSGCODE_FRAME_START:
    {
      m_currentframe.clear();
      if (msg.size() >= 2)
      {
        m_currentframe.initiator   = msg.initiator();
        m_currentframe.destination = msg.destination();
        m_currentframe.ack         = msg.ack();
        m_currentframe.eom         = msg.eom();
      }
    }
    break;
  case MSGCODE_FRAME_DATA:
    {
      if (msg.size() >= 2)
      {
        m_currentframe.push_back(msg[1]);
        m_currentframe.eom = msg.eom();
      }
      bEom = msg.eom();
    }
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
  if (m_iLogicalAddress != CECDEVICE_UNKNOWN && m_busDevices[m_iLogicalAddress])
    return m_busDevices[m_iLogicalAddress]->GetPhysicalAddress();
  return false;
}

void CCECProcessor::SetCurrentButton(cec_user_control_code iButtonCode)
{
  m_controller->SetCurrentButton(iButtonCode);
}

void CCECProcessor::AddCommand(const cec_command &command)
{
  m_controller->AddCommand(command);
}

void CCECProcessor::AddKey(cec_keypress &key)
{
  m_controller->AddKey(key);
}

void CCECProcessor::AddKey(void)
{
  m_controller->AddKey();
}

void CCECProcessor::AddLog(cec_log_level level, const CStdString &strMessage)
{
  m_controller->AddLog(level, strMessage);
}

bool CCECProcessor::SetAckMask(uint16_t iMask)
{
  bool bReturn(false);
  CStdString strLog;
  strLog.Format("setting ackmask to %2x", iMask);
  m_controller->AddLog(CEC_LOG_DEBUG, strLog.c_str());

  CCECAdapterMessage *output = new CCECAdapterMessage;

  output->push_back(MSGSTART);
  output->push_escaped(MSGCODE_SET_ACK_MASK);
  output->push_escaped(iMask >> 8);
  output->push_escaped((uint8_t)iMask);
  output->push_back(MSGEND);

  if ((bReturn = Transmit(output)) == false)
    m_controller->AddLog(CEC_LOG_ERROR, "could not set the ackmask");

  delete output;

  return bReturn;
}
