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
#include "LibCEC.h"
#include "util/StdString.h"
#include "platform/timeutils.h"

using namespace CEC;
using namespace std;

CCECProcessor::CCECProcessor(CLibCEC *controller, CAdapterCommunication *serComm, const char *strDeviceName, cec_logical_address iLogicalAddress /* = CECDEVICE_PLAYBACKDEVICE1 */, uint16_t iPhysicalAddress /* = CEC_DEFAULT_PHYSICAL_ADDRESS*/) :
    m_iPhysicalAddress(iPhysicalAddress),
    m_iLogicalAddress(iLogicalAddress),
    m_strDeviceName(strDeviceName),
    m_communication(serComm),
    m_controller(controller),
    m_bMonitor(false)
{
  for (uint8_t iPtr = 0; iPtr < 16; iPtr++)
    m_vendorIds[iPtr] = CEC_VENDOR_UNKNOWN;
  for (uint8_t iPtr = 0; iPtr < 16; iPtr++)
    m_vendorClasses[iPtr] = (uint8_t) 0;
}

CCECProcessor::~CCECProcessor(void)
{
  StopThread();
  m_communication = NULL;
  m_controller = NULL;
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

  cec_command command;
  cec_adapter_message msg;

  while (!IsStopped())
  {
    bool bParseFrame(false);
    bool bError(false);
    bool bTransmitSucceeded(false);
    command.clear();
    msg.clear();

    {
      CLockObject lock(&m_mutex);
      if (m_communication->IsOpen() && m_communication->Read(msg, 50))
        ParseMessage(msg, &bError, &bTransmitSucceeded, &bParseFrame);

      bParseFrame &= !IsStopped();
      if (bParseFrame)
        command = m_currentframe;
    }

    if (bParseFrame)
      ParseCommand(command);

    m_controller->CheckKeypressTimeout();

    if (!IsStopped())
      Sleep(5);
  }

  return NULL;
}

bool CCECProcessor::PowerOnDevices(cec_logical_address address /* = CECDEVICE_TV */)
{
  if (!IsRunning())
    return false;

  CStdString strLog;
  strLog.Format("<< powering on device with logical address %d", (int8_t)address);
  m_controller->AddLog(CEC_LOG_DEBUG, strLog.c_str());

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, address, CEC_OPCODE_IMAGE_VIEW_ON);

  return Transmit(command);
}

bool CCECProcessor::StandbyDevices(cec_logical_address address /* = CECDEVICE_BROADCAST */)
{
  if (!IsRunning())
    return false;

  CStdString strLog;
  strLog.Format("<< putting device with logical address %d in standby mode", (int8_t)address);
  m_controller->AddLog(CEC_LOG_DEBUG, strLog.c_str());

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, address, CEC_OPCODE_STANDBY);

  return Transmit(command);
}

bool CCECProcessor::SetActiveView(void)
{
  if (!IsRunning())
    return false;

  m_controller->AddLog(CEC_LOG_DEBUG, "<< setting active view");

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, CECDEVICE_BROADCAST, CEC_OPCODE_ACTIVE_SOURCE);
  command.parameters.push_back((m_iPhysicalAddress >> 8) & 0xFF);
  command.parameters.push_back(m_iPhysicalAddress & 0xFF);

  return Transmit(command);
}

bool CCECProcessor::SetInactiveView(void)
{
  if (!IsRunning())
    return false;

  m_controller->AddLog(CEC_LOG_DEBUG, "<< setting inactive view");

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, CECDEVICE_BROADCAST, CEC_OPCODE_INACTIVE_SOURCE);
  command.parameters.push_back((m_iPhysicalAddress >> 8) & 0xFF);
  command.parameters.push_back(m_iPhysicalAddress & 0xFF);

  return Transmit(command);
}

void CCECProcessor::LogOutput(const cec_command &data)
{
  CStdString txStr = "transmit ";
  txStr.AppendFormat(" %02x", ((uint8_t)data.initiator << 4) + (uint8_t)data.destination);
  txStr.AppendFormat(":%02x", (uint8_t)data.opcode);

  for (uint8_t iPtr = 0; iPtr < data.parameters.size; iPtr++)
    txStr.AppendFormat(":%02x", data.parameters[iPtr]);
  m_controller->AddLog(CEC_LOG_DEBUG, txStr.c_str());
}

bool CCECProcessor::Transmit(const cec_command &data, bool bWaitForAck /* = true */)
{
  LogOutput(data);

  cec_adapter_message output;
  output.clear();
  CAdapterCommunication::FormatAdapterMessage(data, output);

  return TransmitFormatted(output, bWaitForAck);
}

bool CCECProcessor::SetLogicalAddress(cec_logical_address iLogicalAddress)
{
  CStdString strLog;
  strLog.Format("<< setting logical address to %1x", iLogicalAddress);
  m_controller->AddLog(CEC_LOG_NOTICE, strLog.c_str());

  m_iLogicalAddress = iLogicalAddress;
  return m_communication && m_communication->SetAckMask(0x1 << (uint8_t)m_iLogicalAddress);
}

bool CCECProcessor::SetPhysicalAddress(uint16_t iPhysicalAddress)
{
  CStdString strLog;
  strLog.Format("<< setting physical address to %2x", iPhysicalAddress);
  m_controller->AddLog(CEC_LOG_NOTICE, strLog.c_str());

  m_iPhysicalAddress = iPhysicalAddress;
  return SetActiveView();
}

bool CCECProcessor::SetOSDString(cec_logical_address iLogicalAddress, cec_display_control duration, const char *strMessage)
{
  CStdString strLog;
  strLog.Format("<< display message '%s'", strMessage);
  m_controller->AddLog(CEC_LOG_NOTICE, strLog.c_str());

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, iLogicalAddress, CEC_OPCODE_SET_OSD_STRING);
  command.parameters.push_back((uint8_t)duration);

  for (unsigned int iPtr = 0; iPtr < strlen(strMessage); iPtr++)
    command.parameters.push_back(strMessage[iPtr]);

  return Transmit(command);
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

bool CCECProcessor::TransmitFormatted(const cec_adapter_message &data, bool bWaitForAck /* = true */)
{
  CLockObject lock(&m_mutex);
  if (!m_communication || !m_communication->Write(data))
    return false;

  if (bWaitForAck)
  {
    uint64_t now = GetTimeMs();
    uint64_t target = now + 1000;
    bool bError(false);
    bool bGotAck(false);

    while (!bGotAck && now < target)
    {
      bGotAck = WaitForAck(&bError, (uint32_t) (target - now));
      now = GetTimeMs();

      if (bError && now < target)
      {
        m_controller->AddLog(CEC_LOG_ERROR, "retransmitting previous frame");
        if (!m_communication->Write(data))
          return false;
      }
    }
  }

  return true;
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

void CCECProcessor::ReportCECVersion(cec_logical_address address /* = CECDEVICE_TV */)
{
  m_controller->AddLog(CEC_LOG_NOTICE, "<< reporting CEC version as 1.3a");

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, address, CEC_OPCODE_CEC_VERSION);
  command.parameters.push_back(CEC_VERSION_1_3A);

  Transmit(command);
}

void CCECProcessor::ReportPowerState(cec_logical_address address /*= CECDEVICE_TV */, bool bOn /* = true */)
{
  if (bOn)
    m_controller->AddLog(CEC_LOG_NOTICE, "<< reporting \"On\" power status");
  else
    m_controller->AddLog(CEC_LOG_NOTICE, "<< reporting \"Off\" power status");

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, address, CEC_OPCODE_REPORT_POWER_STATUS);
  command.parameters.push_back(bOn ? (uint8_t) CEC_POWER_STATUS_ON : (uint8_t) CEC_POWER_STATUS_STANDBY);

  Transmit(command);
}

void CCECProcessor::ReportMenuState(cec_logical_address address /* = CECDEVICE_TV */, bool bActive /* = true */)
{
  if (bActive)
    m_controller->AddLog(CEC_LOG_NOTICE, "<< reporting menu state as active");
  else
    m_controller->AddLog(CEC_LOG_NOTICE, "<< reporting menu state as inactive");

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, address, CEC_OPCODE_MENU_STATUS);
  command.parameters.push_back(bActive ? (uint8_t) CEC_MENU_STATE_ACTIVATED : (uint8_t) CEC_MENU_STATE_DEACTIVATED);

  Transmit(command);
}

void CCECProcessor::ReportVendorID(cec_logical_address address /* = CECDEVICE_TV */)
{
  m_controller->AddLog(CEC_LOG_NOTICE, "<< vendor ID requested, feature abort");
  TransmitAbort(address, CEC_OPCODE_GIVE_DEVICE_VENDOR_ID);
}

void CCECProcessor::ReportOSDName(cec_logical_address address /* = CECDEVICE_TV */)
{
  const char *osdname = m_strDeviceName.c_str();
  CStdString strLog;
  strLog.Format("<< reporting OSD name as %s", osdname);
  m_controller->AddLog(CEC_LOG_NOTICE, strLog.c_str());

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, address, CEC_OPCODE_SET_OSD_NAME);
  for (unsigned int iPtr = 0; iPtr < strlen(osdname); iPtr++)
    command.parameters.push_back(osdname[iPtr]);

  Transmit(command);
}

void CCECProcessor::ReportPhysicalAddress(void)
{
  CStdString strLog;
  strLog.Format("<< reporting physical address as %04x", m_iPhysicalAddress);
  m_controller->AddLog(CEC_LOG_NOTICE, strLog.c_str());

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, CECDEVICE_BROADCAST, CEC_OPCODE_REPORT_PHYSICAL_ADDRESS);
  command.parameters.push_back((uint8_t) ((m_iPhysicalAddress >> 8) & 0xFF));
  command.parameters.push_back((uint8_t) (m_iPhysicalAddress & 0xFF));
  command.parameters.push_back((uint8_t) (CEC_DEVICE_TYPE_PLAYBACK_DEVICE));

  Transmit(command);
}

void CCECProcessor::BroadcastActiveSource(void)
{
  m_controller->AddLog(CEC_LOG_NOTICE, "<< broadcasting active source");

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, CECDEVICE_BROADCAST, CEC_OPCODE_ACTIVE_SOURCE);
  command.parameters.push_back((uint8_t) ((m_iPhysicalAddress >> 8) & 0xFF));
  command.parameters.push_back((uint8_t) (m_iPhysicalAddress & 0xFF));

  Transmit(command);
}

bool CCECProcessor::WaitForAck(bool *bError, uint32_t iTimeout /* = 1000 */)
{
  bool bTransmitSucceeded = false, bEom = false;
  *bError = false;

  int64_t iNow = GetTimeMs();
  int64_t iTargetTime = iNow + (uint64_t) iTimeout;

  while (!bTransmitSucceeded && !*bError && (iTimeout == 0 || iNow < iTargetTime))
  {
    cec_adapter_message msg;
    msg.clear();

    if (!m_communication->Read(msg, iTimeout > 0 ? (int32_t)(iTargetTime - iNow) : 1000))
    {
      iNow = GetTimeMs();
      continue;
    }

    ParseMessage(msg, bError, &bTransmitSucceeded, &bEom, false);
    iNow = GetTimeMs();
  }

  return bTransmitSucceeded && !*bError;
}

void CCECProcessor::ParseMessage(cec_adapter_message &msg, bool *bError, bool *bTransmitSucceeded, bool *bEom, bool bProcessMessages /* = true */)
{
  *bError = false;
  *bTransmitSucceeded = false;
  *bEom = false;

  if (msg.empty())
    return;

  CStdString logStr;

  switch(msg.message())
  {
  case MSGCODE_NOTHING:
    m_controller->AddLog(CEC_LOG_DEBUG, "MSGCODE_NOTHING");
    break;
  case MSGCODE_TIMEOUT_ERROR:
  case MSGCODE_HIGH_ERROR:
  case MSGCODE_LOW_ERROR:
    {
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
  case MSGCODE_FRAME_START:
    {
      if (bProcessMessages)
      {
        logStr = "MSGCODE_FRAME_START";
        m_currentframe.clear();
        if (msg.size() >= 2)
        {
          logStr.AppendFormat(" initiator:%u destination:%u ack:%s %s", msg.initiator(), msg.destination(), msg.ack() ? "high" : "low", msg.eom() ? "eom" : "");
          m_currentframe.initiator   = msg.initiator();
          m_currentframe.destination = msg.destination();
          m_currentframe.ack         = msg.ack();
          m_currentframe.eom         = msg.eom();
        }
        m_controller->AddLog(CEC_LOG_DEBUG, logStr.c_str());
      }
      else
      {
        m_frameBuffer.Push(msg);
      }
    }
    break;
  case MSGCODE_FRAME_DATA:
    {
      if (bProcessMessages)
      {
        logStr = "MSGCODE_FRAME_DATA";
        if (msg.size() >= 2)
        {
          uint8_t iData = msg[1];
          logStr.AppendFormat(" %02x", iData);
          m_currentframe.push_back(iData);
          m_currentframe.eom = msg.eom();
        }
        m_controller->AddLog(CEC_LOG_DEBUG, logStr.c_str());
      }
      else
      {
        m_frameBuffer.Push(msg);
      }

      *bEom = msg.eom();
    }
    break;
  case MSGCODE_COMMAND_ACCEPTED:
    m_controller->AddLog(CEC_LOG_DEBUG, "MSGCODE_COMMAND_ACCEPTED");
    break;
  case MSGCODE_TRANSMIT_SUCCEEDED:
    m_controller->AddLog(CEC_LOG_DEBUG, "MSGCODE_TRANSMIT_SUCCEEDED");
    *bTransmitSucceeded = true;
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
    break;
  }
}

void CCECProcessor::ParseVendorId(cec_logical_address device, const cec_datapacket &data)
{
  if (data.size < 3)
  {
    m_controller->AddLog(CEC_LOG_WARNING, "invalid vendor ID received");
    return;
  }

  uint64_t iVendorId = ((uint64_t)data[0] << 3) +
                       ((uint64_t)data[1] << 2) +
                        (uint64_t)data[2];

  m_vendorIds[(uint8_t)device]     = iVendorId;
  m_vendorClasses[(uint8_t)device] = data.size >= 4 ? data[3] : 0;

  CStdString strLog;
  strLog.Format("device %d: vendor = %s (%04x) class = %2x", (uint8_t)device, CECVendorIdToString(m_vendorIds[(uint8_t)device]), iVendorId, m_vendorClasses[(uint8_t)device]);
  m_controller->AddLog(CEC_LOG_DEBUG, strLog.c_str());
}

bool CCECProcessor::HandleANCommand(cec_command &command)
{
  bool bHandled(true);
  if (command.destination == m_iLogicalAddress)
  {
    switch(command.opcode)
    {
    case CEC_OPCODE_VENDOR_REMOTE_BUTTON_DOWN:
      if (command.parameters.size > 0)
      {
        m_controller->AddKey();

        uint8_t iButton = 0;
        switch (command.parameters[0])
        {
        case CEC_AN_USER_CONTROL_CODE_RETURN:
          iButton = CEC_USER_CONTROL_CODE_PREVIOUS_CHANNEL;
          break;
        default:
          break;
        }

        if (iButton > 0 && iButton <= CEC_USER_CONTROL_CODE_MAX)
        {
          CStdString strLog;
          strLog.Format("key pressed: %1x", iButton);
          m_controller->AddLog(CEC_LOG_DEBUG, strLog.c_str());

          m_controller->SetCurrentButton((cec_user_control_code) command.parameters[0]);
        }
      }
      break;
    case CEC_OPCODE_VENDOR_REMOTE_BUTTON_UP:
      m_controller->AddKey();
      break;
    default:
      bHandled = false;
      break;
    }
  }
  else if (command.destination == CECDEVICE_BROADCAST)
  {
    switch(command.opcode)
    {
    // TODO
    default:
      bHandled = false;
      break;
    }
  }

  if (!bHandled)
    bHandled = HandleCecCommand(command);

  return bHandled;
}

bool CCECProcessor::HandleSLCommand(cec_command &command)
{
  bool bHandled(true);
  if (command.destination == m_iLogicalAddress)
  {
    switch(command.opcode)
    {
    // TODO
    default:
      bHandled = false;
      break;
    }
  }
  else if (command.destination == CECDEVICE_BROADCAST)
  {
    switch(command.opcode)
    {
    // TODO
    default:
      bHandled = false;
      break;
    }
  }

  if (!bHandled)
    bHandled = HandleCecCommand(command);

  return bHandled;
}

bool CCECProcessor::HandleCecCommand(cec_command &command)
{
  bool bHandled(true);

  if (command.destination == m_iLogicalAddress)
  {
    switch(command.opcode)
    {
    case CEC_OPCODE_GIVE_PHYSICAL_ADDRESS:
      ReportPhysicalAddress();
      break;
    case CEC_OPCODE_GIVE_OSD_NAME:
      ReportOSDName(command.initiator);
      break;
    case CEC_OPCODE_GIVE_DEVICE_VENDOR_ID:
      ReportVendorID(command.initiator);
      break;
    case CEC_OPCODE_DEVICE_VENDOR_ID:
      ParseVendorId(command.initiator, command.parameters);
      break;
    case CEC_OPCODE_VENDOR_COMMAND_WITH_ID:
      ParseVendorId(command.initiator, command.parameters);
      break;
    case CEC_OPCODE_GIVE_DECK_STATUS:
      // need to support opcodes play and deck control before doing anything with this
      TransmitAbort(command.initiator, CEC_OPCODE_GIVE_DECK_STATUS);
      break;
    case CEC_OPCODE_MENU_REQUEST:
      if (command.parameters[0] == CEC_MENU_REQUEST_TYPE_QUERY)
        ReportMenuState(command.initiator);
      break;
    case CEC_OPCODE_GIVE_DEVICE_POWER_STATUS:
      ReportPowerState(command.initiator);
      break;
    case CEC_OPCODE_GET_CEC_VERSION:
      ReportCECVersion(command.initiator);
      break;
    case CEC_OPCODE_USER_CONTROL_PRESSED:
      if (command.parameters.size > 0)
      {
        m_controller->AddKey();

        if (command.parameters[0] <= CEC_USER_CONTROL_CODE_MAX)
        {
          CStdString strLog;
          strLog.Format("key pressed: %1x", command.parameters[0]);
          m_controller->AddLog(CEC_LOG_DEBUG, strLog.c_str());

          m_controller->SetCurrentButton((cec_user_control_code) command.parameters[0]);
        }
      }
      break;
    case CEC_OPCODE_USER_CONTROL_RELEASE:
      m_controller->AddKey();
      break;
    default:
      m_controller->AddCommand(command);
      bHandled = false;
      break;
    }
  }
  else if (command.destination == CECDEVICE_BROADCAST)
  {
    CStdString strLog;
    switch (command.opcode)
    {
    case CEC_OPCODE_REQUEST_ACTIVE_SOURCE:
      strLog.Format(">> %i requests active source", (uint8_t) command.initiator);
      m_controller->AddLog(CEC_LOG_DEBUG, strLog.c_str());
      BroadcastActiveSource();
      break;
    case CEC_OPCODE_SET_STREAM_PATH:
      if (command.parameters.size >= 2)
      {
        int streamaddr = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
        strLog.Format(">> %i requests stream path from physical address %04x", command.initiator, streamaddr);
        m_controller->AddLog(CEC_LOG_DEBUG, strLog.c_str());
        if (streamaddr == m_iPhysicalAddress)
          BroadcastActiveSource();
      }
      break;
    case CEC_OPCODE_ROUTING_CHANGE:
      if (command.parameters.size == 4)
      {
        uint16_t iOldAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
        uint16_t iNewAddress = ((uint16_t)command.parameters[2] << 8) | ((uint16_t)command.parameters[3]);
        strLog.Format(">> %i changed physical address from %04x to %04x", command.initiator, iOldAddress, iNewAddress);
        m_controller->AddLog(CEC_LOG_DEBUG, strLog.c_str());

        m_controller->AddCommand(command);
      }
      break;
    default:
      m_controller->AddCommand(command);
      bHandled = false;
      break;
    }
  }
  else
  {
    CStdString strLog;
    strLog.Format("ignoring frame: destination: %u != %u", command.destination, (uint8_t)m_iLogicalAddress);
    m_controller->AddLog(CEC_LOG_DEBUG, strLog.c_str());
    bHandled = false;
  }

  return bHandled;
}

void CCECProcessor::ParseCommand(cec_command &command)
{
  CStdString dataStr;
  dataStr.Format(">> received frame: %1x%1x:%02x", command.initiator, command.destination, command.opcode);
  for (uint8_t iPtr = 0; iPtr < command.parameters.size; iPtr++)
    dataStr.AppendFormat(":%02x", (unsigned int)command.parameters[iPtr]);
  m_controller->AddLog(CEC_LOG_DEBUG, dataStr.c_str());

  if (!m_bMonitor)
  {
    switch(m_vendorIds[command.initiator])
    {
    case CEC_VENDOR_LG:
      HandleSLCommand(command);
      break;
    case CEC_VENDOR_SAMSUNG:
      HandleANCommand(command);
      break;
    default:
      HandleCecCommand(command);
      break;
    }
  }
}

const char *CCECProcessor::CECVendorIdToString(const uint64_t iVendorId)
{
  switch (iVendorId)
  {
  case CEC_VENDOR_SAMSUNG:
    return "Samsung";
  case CEC_VENDOR_LG:
      return "LG";
  default:
    return "Unknown";
  }
}
