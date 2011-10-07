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
    m_physicaladdress(iPhysicalAddress),
    m_iLogicalAddress(iLogicalAddress),
    m_strDeviceName(strDeviceName),
    m_communication(serComm),
    m_controller(controller)
{
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
    return false;

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

  while (!m_bStop)
  {
    bool bParseFrame(false);
    {
      CLockObject lock(&m_mutex);
      cec_frame msg;
      msg.clear();

      if (!m_bStop && m_communication->IsOpen() && m_communication->Read(msg, CEC_BUTTON_TIMEOUT))
        bParseFrame = ParseMessage(msg);
    }

    if (!m_bStop && bParseFrame)
      ParseCurrentFrame();

    if (!m_bStop)
    {
      m_controller->CheckKeypressTimeout();
      Sleep(50);
    }
  }

  return NULL;
}

bool CCECProcessor::PowerOnDevices(cec_logical_address address /* = CECDEVICE_TV */)
{
  if (!IsRunning())
    return false;

  CStdString strLog;
  strLog.Format("powering on devices with logical address %d", (int8_t)address);
  m_controller->AddLog(CEC_LOG_DEBUG, strLog.c_str());
  cec_frame frame;
  frame.clear();

  frame.push_back(GetSourceDestination(address));
  frame.push_back((uint8_t) CEC_OPCODE_TEXT_VIEW_ON);
  return Transmit(frame);
}

bool CCECProcessor::StandbyDevices(cec_logical_address address /* = CECDEVICE_BROADCAST */)
{
  if (!IsRunning())
    return false;

  CStdString strLog;
  strLog.Format("putting all devices with logical address %d in standby mode", (int8_t)address);
  m_controller->AddLog(CEC_LOG_DEBUG, strLog.c_str());
  cec_frame frame;
  frame.clear();

  frame.push_back(GetSourceDestination(address));
  frame.push_back((uint8_t) CEC_OPCODE_STANDBY);
  return Transmit(frame);
}

bool CCECProcessor::SetActiveView(void)
{
  if (!IsRunning())
    return false;

  m_controller->AddLog(CEC_LOG_DEBUG, "setting active view");
  cec_frame frame;
  frame.clear();

  frame.push_back(GetSourceDestination(CECDEVICE_BROADCAST));
  frame.push_back((uint8_t) CEC_OPCODE_ACTIVE_SOURCE);
  frame.push_back((m_physicaladdress >> 8) & 0xFF);
  frame.push_back(m_physicaladdress & 0xFF);
  return Transmit(frame);
}

bool CCECProcessor::SetInactiveView(void)
{
  if (!IsRunning())
    return false;

  m_controller->AddLog(CEC_LOG_DEBUG, "setting inactive view");
  cec_frame frame;
  frame.clear();

  frame.push_back(GetSourceDestination(CECDEVICE_BROADCAST));
  frame.push_back((uint8_t) CEC_OPCODE_INACTIVE_SOURCE);
  frame.push_back((m_physicaladdress >> 8) & 0xFF);
  frame.push_back(m_physicaladdress & 0xFF);
  return Transmit(frame);
}

bool CCECProcessor::Transmit(const cec_frame &data, bool bWaitForAck /* = true */)
{
  CStdString txStr = "transmit ";
  for (unsigned int i = 0; i < data.size; i++)
    txStr.AppendFormat(" %02x", data.data[i]);
  m_controller->AddLog(CEC_LOG_DEBUG, txStr.c_str());

  if (data.size == 0)
  {
    m_controller->AddLog(CEC_LOG_WARNING, "transmit buffer is empty");
    return false;
  }

  cec_frame output;
  output.clear();

  //set ack polarity to high when transmitting to the broadcast address
  //set ack polarity low when transmitting to any other address
  output.push_back(MSGSTART);
  CAdapterCommunication::PushEscaped(output, MSGCODE_TRANSMIT_ACK_POLARITY);

  if ((data.data[0] & 0xF) == 0xF)
    CAdapterCommunication::PushEscaped(output, CEC_TRUE);
  else
    CAdapterCommunication::PushEscaped(output, CEC_FALSE);

  output.push_back(MSGEND);

  for (int8_t i = 0; i < data.size; i++)
  {
    output.push_back(MSGSTART);

    if (i == (int8_t)data.size - 1)
      CAdapterCommunication::PushEscaped(output, MSGCODE_TRANSMIT_EOM);
    else
      CAdapterCommunication::PushEscaped(output, MSGCODE_TRANSMIT);

    CAdapterCommunication::PushEscaped(output, data.data[i]);

    output.push_back(MSGEND);
  }

  return TransmitFormatted(output, bWaitForAck);
}

bool CCECProcessor::SetLogicalAddress(cec_logical_address iLogicalAddress)
{
  CStdString strLog;
  strLog.Format("setting logical address to %d", iLogicalAddress);
  m_controller->AddLog(CEC_LOG_NOTICE, strLog.c_str());

  m_iLogicalAddress = iLogicalAddress;
  return m_communication && m_communication->SetAckMask(0x1 << (uint8_t)m_iLogicalAddress);
}

bool CCECProcessor::TransmitFormatted(const cec_frame &data, bool bWaitForAck /* = true */)
{
  CLockObject lock(&m_mutex);
  if (!m_communication || !m_communication->Write(data))
    return false;

  if (bWaitForAck && !WaitForAck())
  {
    m_controller->AddLog(CEC_LOG_DEBUG, "did not receive ACK");
    return false;
  }

  return true;
}

void CCECProcessor::TransmitAbort(cec_logical_address address, cec_opcode opcode, ECecAbortReason reason /* = CEC_ABORT_REASON_UNRECOGNIZED_OPCODE */)
{
  m_controller->AddLog(CEC_LOG_DEBUG, "transmitting abort message");
  cec_frame frame;
  frame.clear();

  frame.push_back(GetSourceDestination(address));
  frame.push_back((uint8_t) CEC_OPCODE_FEATURE_ABORT);
  frame.push_back((uint8_t) opcode);
  frame.push_back((uint8_t) reason);
  Transmit(frame);
}

void CCECProcessor::ReportCECVersion(cec_logical_address address /* = CECDEVICE_TV */)
{
  cec_frame frame;
  frame.clear();

  m_controller->AddLog(CEC_LOG_NOTICE, "reporting CEC version as 1.3a");
  frame.push_back(GetSourceDestination(address));
  frame.push_back((uint8_t) CEC_OPCODE_CEC_VERSION);
  frame.push_back((uint8_t) CEC_VERSION_1_3A);
  Transmit(frame);
}

void CCECProcessor::ReportPowerState(cec_logical_address address /*= CECDEVICE_TV */, bool bOn /* = true */)
{
  cec_frame frame;
  frame.clear();

  if (bOn)
    m_controller->AddLog(CEC_LOG_NOTICE, "reporting \"On\" power status");
  else
    m_controller->AddLog(CEC_LOG_NOTICE, "reporting \"Off\" power status");

  frame.push_back(GetSourceDestination(address));
  frame.push_back((uint8_t) CEC_OPCODE_REPORT_POWER_STATUS);
  frame.push_back(bOn ? (uint8_t) CEC_POWER_STATUS_ON : (uint8_t) CEC_POWER_STATUS_STANDBY);
  Transmit(frame);
}

void CCECProcessor::ReportMenuState(cec_logical_address address /* = CECDEVICE_TV */, bool bActive /* = true */)
{
  cec_frame frame;
  frame.clear();

  if (bActive)
    m_controller->AddLog(CEC_LOG_NOTICE, "reporting menu state as active");
  else
    m_controller->AddLog(CEC_LOG_NOTICE, "reporting menu state as inactive");

  frame.push_back(GetSourceDestination(address));
  frame.push_back((uint8_t) CEC_OPCODE_MENU_STATUS);
  frame.push_back(bActive ? (uint8_t) CEC_MENU_STATE_ACTIVATED : (uint8_t) CEC_MENU_STATE_DEACTIVATED);
  Transmit(frame);
}

void CCECProcessor::ReportVendorID(cec_logical_address address /* = CECDEVICE_TV */)
{
  m_controller->AddLog(CEC_LOG_NOTICE, "vendor ID requested, feature abort");
  TransmitAbort(address, CEC_OPCODE_GIVE_DEVICE_VENDOR_ID);
}

void CCECProcessor::ReportOSDName(cec_logical_address address /* = CECDEVICE_TV */)
{
  cec_frame frame;
  frame.clear();

  const char *osdname = m_strDeviceName.c_str();
  CStdString strLog;
  strLog.Format("reporting OSD name as %s", osdname);
  m_controller->AddLog(CEC_LOG_NOTICE, strLog.c_str());
  frame.push_back(GetSourceDestination(address));
  frame.push_back((uint8_t) CEC_OPCODE_SET_OSD_NAME);

  for (unsigned int i = 0; i < strlen(osdname); i++)
    frame.push_back(osdname[i]);

  Transmit(frame);
}

void CCECProcessor::ReportPhysicalAddress(void)
{
  cec_frame frame;
  frame.clear();

  CStdString strLog;
  strLog.Format("reporting physical address as %04x", m_physicaladdress);
  m_controller->AddLog(CEC_LOG_NOTICE, strLog.c_str());
  frame.push_back(GetSourceDestination(CECDEVICE_BROADCAST));
  frame.push_back((uint8_t) CEC_OPCODE_REPORT_PHYSICAL_ADDRESS);
  frame.push_back((uint8_t) ((m_physicaladdress >> 8) & 0xFF));
  frame.push_back((uint8_t) (m_physicaladdress & 0xFF));
  frame.push_back((uint8_t) CEC_DEVICE_TYPE_PLAYBACK_DEVICE);
  Transmit(frame);
}

void CCECProcessor::BroadcastActiveSource(void)
{
  cec_frame frame;
  frame.clear();

  m_controller->AddLog(CEC_LOG_NOTICE, "broadcasting active source");
  frame.push_back(GetSourceDestination(CECDEVICE_BROADCAST));
  frame.push_back((uint8_t) CEC_OPCODE_ACTIVE_SOURCE);
  frame.push_back((uint8_t) ((m_physicaladdress >> 8) & 0xFF));
  frame.push_back((uint8_t) (m_physicaladdress & 0xFF));
  Transmit(frame);
}

uint8_t CCECProcessor::GetSourceDestination(cec_logical_address destination /* = CECDEVICE_BROADCAST */) const
{
  return ((uint8_t)m_iLogicalAddress << 4) + (uint8_t)destination;
}

bool CCECProcessor::WaitForAck(uint32_t iTimeout /* = 1000 */)
{
  bool bGotAck(false);
  bool bError(false);

  int64_t iNow = GetTimeMs();
  int64_t iTargetTime = iNow + (uint64_t) iTimeout;

  while (!bGotAck && !bError && (iTimeout == 0 || iNow < iTargetTime))
  {
    cec_frame msg;
    msg.clear();

    while (!bGotAck && !bError && m_communication->Read(msg, iTimeout))
    {
      uint8_t iCode = msg.data[0] & ~(MSGCODE_FRAME_EOM | MSGCODE_FRAME_ACK);

      switch (iCode)
      {
      case MSGCODE_COMMAND_ACCEPTED:
        m_controller->AddLog(CEC_LOG_DEBUG, "MSGCODE_COMMAND_ACCEPTED");
        break;
      case MSGCODE_TRANSMIT_SUCCEEDED:
        m_controller->AddLog(CEC_LOG_DEBUG, "MSGCODE_TRANSMIT_SUCCEEDED");
        // TODO
        bGotAck = true;
        break;
      case MSGCODE_RECEIVE_FAILED:
        m_controller->AddLog(CEC_LOG_WARNING, "MSGCODE_RECEIVE_FAILED");
        bError = true;
        break;
      case MSGCODE_COMMAND_REJECTED:
        m_controller->AddLog(CEC_LOG_WARNING, "MSGCODE_COMMAND_REJECTED");
        bError = true;
        break;
      case MSGCODE_TRANSMIT_FAILED_LINE:
        m_controller->AddLog(CEC_LOG_WARNING, "MSGCODE_TRANSMIT_FAILED_LINE");
        bError = true;
        break;
      case MSGCODE_TRANSMIT_FAILED_ACK:
        m_controller->AddLog(CEC_LOG_WARNING, "MSGCODE_TRANSMIT_FAILED_ACK");
        bError = true;
        break;
      case MSGCODE_TRANSMIT_FAILED_TIMEOUT_DATA:
        m_controller->AddLog(CEC_LOG_WARNING, "MSGCODE_TRANSMIT_FAILED_TIMEOUT_DATA");
        bError = true;
        break;
      case MSGCODE_TRANSMIT_FAILED_TIMEOUT_LINE:
        m_controller->AddLog(CEC_LOG_WARNING, "MSGCODE_TRANSMIT_FAILED_TIMEOUT_LINE");
        bError = true;
        break;
      default:
        m_frameBuffer.Push(msg);
        bGotAck = (msg.data[0] & MSGCODE_FRAME_ACK) != 0;
        break;
      }
      iNow = GetTimeMs();
    }
  }

  return bGotAck && !bError;
}

bool CCECProcessor::ParseMessage(cec_frame &msg)
{
  bool bReturn(false);

  if (msg.size == 0)
    return bReturn;

  CStdString logStr;
  uint8_t iCode = msg.data[0] & ~(MSGCODE_FRAME_EOM | MSGCODE_FRAME_ACK);
  bool    bEom  = (msg.data[0] & MSGCODE_FRAME_EOM) != 0;
  bool    bAck  = (msg.data[0] & MSGCODE_FRAME_ACK) != 0;

  switch(iCode)
  {
  case MSGCODE_NOTHING:
    m_controller->AddLog(CEC_LOG_DEBUG, "MSGCODE_NOTHING");
    break;
  case MSGCODE_TIMEOUT_ERROR:
  case MSGCODE_HIGH_ERROR:
  case MSGCODE_LOW_ERROR:
    {
      if (iCode == MSGCODE_TIMEOUT_ERROR)
        logStr = "MSGCODE_TIMEOUT";
      else if (iCode == MSGCODE_HIGH_ERROR)
        logStr = "MSGCODE_HIGH_ERROR";
      else
        logStr = "MSGCODE_LOW_ERROR";

      int iLine      = (msg.size >= 3) ? (msg.data[1] << 8) | (msg.data[2]) : 0;
      uint32_t iTime = (msg.size >= 7) ? (msg.data[3] << 24) | (msg.data[4] << 16) | (msg.data[5] << 8) | (msg.data[6]) : 0;
      logStr.AppendFormat(" line:%i", iLine);
      logStr.AppendFormat(" time:%u", iTime);
      m_controller->AddLog(CEC_LOG_WARNING, logStr.c_str());
    }
    break;
  case MSGCODE_FRAME_START:
    {
      logStr = "MSGCODE_FRAME_START";
      m_currentframe.clear();
      if (msg.size >= 2)
      {
        int iInitiator = msg.data[1] >> 4;
        int iDestination = msg.data[1] & 0xF;
        logStr.AppendFormat(" initiator:%u destination:%u ack:%s %s", iInitiator, iDestination, bAck ? "high" : "low", bEom ? "eom" : "");

        m_currentframe.push_back(msg.data[1]);
      }
      m_controller->AddLog(CEC_LOG_DEBUG, logStr.c_str());
    }
    break;
  case MSGCODE_FRAME_DATA:
    {
      logStr = "MSGCODE_FRAME_DATA";
      if (msg.size >= 2)
      {
        uint8_t iData = msg.data[1];
        logStr.AppendFormat(" %02x", iData);
        m_currentframe.push_back(iData);
      }
      m_controller->AddLog(CEC_LOG_DEBUG, logStr.c_str());
    }
    if (bEom)
      bReturn = true;
    break;
  default:
    break;
  }

  return bReturn;
}

void CCECProcessor::ParseCurrentFrame(void)
{
  uint8_t initiator = m_currentframe.data[0] >> 4;
  uint8_t destination = m_currentframe.data[0] & 0xF;

  CStdString dataStr;
  dataStr.Format("received frame: initiator: %u destination: %u", initiator, destination);

  if (m_currentframe.size > 1)
  {
    dataStr += " data:";
    for (unsigned int i = 1; i < m_currentframe.size; i++)
      dataStr.AppendFormat(" %02x", m_currentframe.data[i]);
  }
  m_controller->AddLog(CEC_LOG_DEBUG, dataStr.c_str());

  if (m_currentframe.size <= 1)
    return;

  cec_opcode opCode = (cec_opcode) m_currentframe.data[1];
  if (destination == (uint16_t) m_iLogicalAddress)
  {
    switch(opCode)
    {
    case CEC_OPCODE_GIVE_PHYSICAL_ADDRESS:
      ReportPhysicalAddress();
      SetActiveView();
      break;
    case CEC_OPCODE_GIVE_OSD_NAME:
      ReportOSDName((cec_logical_address)initiator);
      break;
    case CEC_OPCODE_GIVE_DEVICE_VENDOR_ID:
      ReportVendorID((cec_logical_address)initiator);
      break;
    case CEC_OPCODE_MENU_REQUEST:
      ReportMenuState((cec_logical_address)initiator);
      break;
    case CEC_OPCODE_GIVE_DEVICE_POWER_STATUS:
      ReportPowerState((cec_logical_address)initiator);
      break;
    case CEC_OPCODE_GET_CEC_VERSION:
      ReportCECVersion((cec_logical_address)initiator);
      break;
    case CEC_OPCODE_USER_CONTROL_PRESSED:
      if (m_currentframe.size > 2)
      {
        m_controller->AddKey();

        if (m_currentframe.data[2] <= CEC_USER_CONTROL_CODE_MAX)
          m_controller->SetCurrentButton((cec_user_control_code) m_currentframe.data[2]);
      }
      break;
    case CEC_OPCODE_USER_CONTROL_RELEASE:
      m_controller->AddKey();
      break;
    default:
      cec_frame params = m_currentframe;
      params.shift(2);
      m_controller->AddCommand((cec_logical_address) initiator, (cec_logical_address) destination, opCode, &params);
      break;
    }
  }
  else if (destination == (uint8_t) CECDEVICE_BROADCAST)
  {
    if (opCode == CEC_OPCODE_REQUEST_ACTIVE_SOURCE)
    {
      BroadcastActiveSource();
    }
    else if (opCode == CEC_OPCODE_SET_STREAM_PATH)
    {
      if (m_currentframe.size >= 4)
      {
        int streamaddr = ((int)m_currentframe.data[2] << 8) | ((int)m_currentframe.data[3]);
        CStdString strLog;
        strLog.Format("%i requests stream path from physical address %04x", initiator, streamaddr);
        m_controller->AddLog(CEC_LOG_DEBUG, strLog.c_str());
        if (streamaddr == m_physicaladdress)
          BroadcastActiveSource();
      }
    }
    else
    {
      cec_frame params = m_currentframe;
      params.shift(2);
      m_controller->AddCommand((cec_logical_address) initiator, (cec_logical_address) destination, opCode, &params);
    }
  }
  else
  {
    CStdString strLog;
    strLog.Format("ignoring frame: destination: %u != %u", destination, (uint16_t)m_iLogicalAddress);
    m_controller->AddLog(CEC_LOG_DEBUG, strLog.c_str());
  }
}
