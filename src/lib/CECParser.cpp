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

#include "CECParser.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include "util/StdString.h"
#include "libPlatform/serialport.h"
#include "util/threads.h"
#include "util/timeutils.h"
#include "CECDetect.h"

using namespace CEC;
using namespace std;

#define CEC_MAX_RETRY 5

/*!
 * ICECDevice implementation
 */
//@{
CCECParser::CCECParser(const char *strDeviceName, cec_logical_address iLogicalAddress /* = CECDEVICE_PLAYBACKDEVICE1 */, int iPhysicalAddress /* = CEC_DEFAULT_PHYSICAL_ADDRESS*/) :
    m_inbuf(NULL),
    m_iInbufSize(0),
    m_iInbufUsed(0),
    m_iCurrentButton(CEC_USER_CONTROL_CODE_UNKNOWN),
    m_physicaladdress(iPhysicalAddress),
    m_iLogicalAddress(iLogicalAddress),
    m_strDeviceName(strDeviceName),
    m_bRunning(false)
{
  m_serialport = new CSerialPort;
}

CCECParser::~CCECParser(void)
{
  Close(0);
  m_serialport->Close();
  delete m_serialport;
}

bool CCECParser::Open(const char *strPort, int iTimeoutMs /* = 10000 */)
{
  bool bReturn(false);

  if (!(bReturn = m_serialport->Open(strPort, 38400)))
  {
    CStdString strError;
    strError.Format("error opening serial port '%s': %s", strPort, m_serialport->GetError().c_str());
    AddLog(CEC_LOG_ERROR, strError);
    return bReturn;
  }

  //clear any input bytes
  uint8_t buff[1024];
  m_serialport->Read(buff, sizeof(buff), CEC_SETTLE_DOWN_TIME);

  if (bReturn)
    bReturn = SetLogicalAddress(m_iLogicalAddress);

  if (!bReturn)
  {
    CStdString strError;
    strError.Format("error opening serial port '%s': %s", strPort, m_serialport->GetError().c_str());
    AddLog(CEC_LOG_ERROR, strError);
    return bReturn;
  }

  if (bReturn)
  {
    m_bRunning = true;
    if (pthread_create(&m_thread, NULL, (void *(*) (void *))&CCECParser::ThreadHandler, (void *)this) == 0)
      pthread_detach(m_thread);
    else
      m_bRunning = false;
  }

  return bReturn;
}

bool CCECParser::Close(int iTimeoutMs /* = 2000 */)
{
  m_bRunning = false;
  bool bExit(false);
  if (iTimeoutMs > 0)
  {
    bExit = m_exitCondition.Wait(&m_mutex, iTimeoutMs);
    m_mutex.Unlock();
  }
  else
  {
    pthread_join(m_thread, NULL);
    bExit = true;
  }

  return bExit;
}

void *CCECParser::ThreadHandler(CCECParser *parser)
{
  if (parser)
    parser->Process();
  return 0;
}

bool CCECParser::Process(void)
{
  int64_t now = GetTimeMs();
  while (m_bRunning)
  {
    {
      CLockObject lock(&m_mutex, 1000);
      if (lock.IsLocked())
      {
        if (!ReadFromDevice(100))
        {
          m_bRunning = false;
          return false;
        }
      }
    }

    //AddLog(CEC_LOG_DEBUG, "processing messages");
    ProcessMessages();
    now = GetTimeMs();
    CheckKeypressTimeout(now);
    CCondition::Sleep(50);
  }

  AddLog(CEC_LOG_DEBUG, "reader thread terminated");
  m_bRunning = false;
  m_exitCondition.Signal();
  return true;
}

bool CCECParser::Ping(void)
{
  if (!m_bRunning)
    return false;

  AddLog(CEC_LOG_DEBUG, "sending ping");
  cec_frame output;
  output.push_back(MSGSTART);
  PushEscaped(output, MSGCODE_PING);
  output.push_back(MSGEND);

  if (!TransmitFormatted(output, false, (int64_t) 5000))
  {
    AddLog(CEC_LOG_ERROR, "could not send ping command");
    return false;
  }

  AddLog(CEC_LOG_DEBUG, "ping tranmitted");

  // TODO check for pong
  return true;
}

bool CCECParser::StartBootloader(void)
{
  if (!m_bRunning)
    return false;

  AddLog(CEC_LOG_DEBUG, "starting the bootloader");
  cec_frame output;
  output.push_back(MSGSTART);
  PushEscaped(output, MSGCODE_START_BOOTLOADER);
  output.push_back(MSGEND);

  if (!TransmitFormatted(output, false, (int64_t) 5000))
  {
    AddLog(CEC_LOG_ERROR, "could not start the bootloader");
    return false;
  }

  AddLog(CEC_LOG_DEBUG, "bootloader start command transmitted");
  return true;
}

uint8_t CCECParser::GetSourceDestination(cec_logical_address destination /* = CECDEVICE_BROADCAST */)
{
  return ((uint8_t)m_iLogicalAddress << 4) + (uint8_t)destination;
}

bool CCECParser::PowerOffDevices(cec_logical_address address /* = CECDEVICE_BROADCAST */)
{
  if (!m_bRunning)
    return false;

  CStdString strLog;
  strLog.Format("powering off devices with logical address %d", (int8_t)address);
  AddLog(CEC_LOG_DEBUG, strLog.c_str());
  cec_frame frame;
  frame.push_back(GetSourceDestination(address));
  frame.push_back(CEC_OPCODE_STANDBY);
  return Transmit(frame);
}

bool CCECParser::PowerOnDevices(cec_logical_address address /* = CECDEVICE_BROADCAST */)
{
  if (!m_bRunning)
    return false;

  CStdString strLog;
  strLog.Format("powering on devices with logical address %d", (int8_t)address);
  AddLog(CEC_LOG_DEBUG, strLog.c_str());
  cec_frame frame;
  frame.push_back(GetSourceDestination(address));
  frame.push_back(CEC_OPCODE_TEXT_VIEW_ON);
  return Transmit(frame);
}

bool CCECParser::StandbyDevices(cec_logical_address address /* = CECDEVICE_BROADCAST */)
{
  if (!m_bRunning)
    return false;

  CStdString strLog;
  strLog.Format("putting all devices with logical address %d in standby mode", (int8_t)address);
  AddLog(CEC_LOG_DEBUG, strLog.c_str());
  cec_frame frame;
  frame.push_back(GetSourceDestination(address));
  frame.push_back(CEC_OPCODE_STANDBY);
  return Transmit(frame);
}

bool CCECParser::SetActiveView(void)
{
  if (!m_bRunning)
    return false;

  AddLog(CEC_LOG_DEBUG, "setting active view");
  cec_frame frame;
  frame.push_back(GetSourceDestination(CECDEVICE_BROADCAST));
  frame.push_back(CEC_OPCODE_ACTIVE_SOURCE);
  frame.push_back((m_physicaladdress >> 8) & 0xFF);
  frame.push_back(m_physicaladdress & 0xFF);
  return Transmit(frame);
}

bool CCECParser::SetInactiveView(void)
{
  if (!m_bRunning)
    return false;

  AddLog(CEC_LOG_DEBUG, "setting inactive view");
  cec_frame frame;
  frame.push_back(GetSourceDestination(CECDEVICE_BROADCAST));
  frame.push_back(CEC_OPCODE_INACTIVE_SOURCE);
  frame.push_back((m_physicaladdress >> 8) & 0xFF);
  frame.push_back(m_physicaladdress & 0xFF);
  return Transmit(frame);
}

bool CCECParser::GetNextLogMessage(cec_log_message *message)
{
  return m_bRunning ? m_logBuffer.Pop(*message) : false;
}

bool CCECParser::GetNextKeypress(cec_keypress *key)
{
  return m_bRunning ? m_keyBuffer.Pop(*key) : false;
}

bool CCECParser::GetNextCommand(cec_command *command)
{
  return m_bRunning ? m_commandBuffer.Pop(*command) : false;
}
//@}

void CCECParser::TransmitAbort(cec_logical_address address, cec_opcode opcode, ECecAbortReason reason /* = CEC_ABORT_REASON_UNRECOGNIZED_OPCODE */)
{
  AddLog(CEC_LOG_DEBUG, "transmitting abort message");
  cec_frame frame;
  frame.push_back(GetSourceDestination(address));
  frame.push_back(CEC_OPCODE_FEATURE_ABORT);
  frame.push_back(opcode);
  frame.push_back(reason);
  Transmit(frame);
}

void CCECParser::ReportCECVersion(cec_logical_address address /* = CECDEVICE_TV */)
{
  cec_frame frame;
  AddLog(CEC_LOG_NOTICE, "reporting CEC version as 1.3a");
  frame.push_back(GetSourceDestination(address));
  frame.push_back(CEC_OPCODE_CEC_VERSION);
  frame.push_back(CEC_VERSION_1_3A);
  Transmit(frame);
}

void CCECParser::ReportPowerState(cec_logical_address address /*= CECDEVICE_TV */, bool bOn /* = true */)
{
  cec_frame frame;
  if (bOn)
    AddLog(CEC_LOG_NOTICE, "reporting \"On\" power status");
  else
    AddLog(CEC_LOG_NOTICE, "reporting \"Off\" power status");

  frame.push_back(GetSourceDestination(address));
  frame.push_back(CEC_OPCODE_REPORT_POWER_STATUS);
  frame.push_back(bOn ? CEC_POWER_STATUS_ON : CEC_POWER_STATUS_STANDBY);
  Transmit(frame);
}

void CCECParser::ReportMenuState(cec_logical_address address /* = CECDEVICE_TV */, bool bActive /* = true */)
{
  cec_frame frame;
  if (bActive)
    AddLog(CEC_LOG_NOTICE, "reporting menu state as active");
  else
    AddLog(CEC_LOG_NOTICE, "reporting menu state as inactive");

  frame.push_back(GetSourceDestination(address));
  frame.push_back(CEC_OPCODE_MENU_STATUS);
  frame.push_back(bActive ? CEC_MENU_STATE_ACTIVATED : CEC_MENU_STATE_DEACTIVATED);
  Transmit(frame);
}

void CCECParser::ReportVendorID(cec_logical_address address /* = CECDEVICE_TV */)
{
  AddLog(CEC_LOG_NOTICE, "vendor ID requested, feature abort");
  TransmitAbort(address, CEC_OPCODE_GIVE_DEVICE_VENDOR_ID);
}

void CCECParser::ReportOSDName(cec_logical_address address /* = CECDEVICE_TV */)
{
  cec_frame frame;
  const char *osdname = m_strDeviceName.c_str();
  CStdString strLog;
  strLog.Format("reporting OSD name as %s", osdname);
  AddLog(CEC_LOG_NOTICE, strLog.c_str());
  frame.push_back(GetSourceDestination(address));
  frame.push_back(CEC_OPCODE_SET_OSD_NAME);

  for (unsigned int i = 0; i < strlen(osdname); i++)
    frame.push_back(osdname[i]);

  Transmit(frame);
}

void CCECParser::ReportPhysicalAddress(void)
{
  cec_frame frame;
  CStdString strLog;
  strLog.Format("reporting physical address as %04x", m_physicaladdress);
  AddLog(CEC_LOG_NOTICE, strLog.c_str());
  frame.push_back(GetSourceDestination(CECDEVICE_BROADCAST));
  frame.push_back(CEC_OPCODE_REPORT_PHYSICAL_ADDRESS);
  frame.push_back((m_physicaladdress >> 8) & 0xFF);
  frame.push_back(m_physicaladdress & 0xFF);
  frame.push_back(CEC_DEVICE_TYPE_PLAYBACK_DEVICE);
  Transmit(frame);
}

void CCECParser::BroadcastActiveSource(void)
{
  cec_frame frame;
  AddLog(CEC_LOG_NOTICE, "broadcasting active source");
  frame.push_back(GetSourceDestination(CECDEVICE_BROADCAST));
  frame.push_back(CEC_OPCODE_ACTIVE_SOURCE);
  frame.push_back((m_physicaladdress >> 8) & 0xFF);
  frame.push_back(m_physicaladdress & 0xFF);
  Transmit(frame);
}

bool CCECParser::TransmitFormatted(const cec_frame &data, bool bWaitForAck /* = true */, int64_t iTimeout /* = 2000 */)
{
  CLockObject lock(&m_mutex, iTimeout);
  if (!lock.IsLocked())
  {
    AddLog(CEC_LOG_ERROR, "could not get a write lock");
    return false;
  }

  if (m_serialport->Write(data) != data.size())
  {
    CStdString strError;
    strError.Format("error writing to serial port: %s", m_serialport->GetError().c_str());
    AddLog(CEC_LOG_ERROR, strError);
    return false;
  }
  AddLog(CEC_LOG_DEBUG, "command sent");

  CCondition::Sleep((int) data.size() * 24 /*data*/ + 5 /*start bit (4.5 ms)*/ + 50 /* to be on the safe side */);
  if (bWaitForAck && !WaitForAck())
  {
    AddLog(CEC_LOG_DEBUG, "did not receive ACK");
    return false;
  }

  return true;
}

bool CCECParser::Transmit(const cec_frame &data, bool bWaitForAck /* = true */, int64_t iTimeout /* = 5000 */)
{
  CStdString txStr = "transmit ";
  for (unsigned int i = 0; i < data.size(); i++)
    txStr.AppendFormat(" %02x", data[i]);
  AddLog(CEC_LOG_DEBUG, txStr.c_str());

  if (data.empty())
  {
    AddLog(CEC_LOG_WARNING, "transmit buffer is empty");
    return false;
  }

  cec_frame output;

  //set ack polarity to high when transmitting to the broadcast address
  //set ack polarity low when transmitting to any other address
  output.push_back(MSGSTART);
  PushEscaped(output, MSGCODE_TRANSMIT_ACK_POLARITY);

  if ((data[0] & 0xF) == 0xF)
    PushEscaped(output, CEC_TRUE);
  else
    PushEscaped(output, CEC_FALSE);

  output.push_back(MSGEND);

  for (unsigned int i = 0; i < data.size(); i++)
  {
    output.push_back(MSGSTART);

    if (i == data.size() - 1)
      PushEscaped(output, MSGCODE_TRANSMIT_EOM);
    else
      PushEscaped(output, MSGCODE_TRANSMIT);

    PushEscaped(output, data[i]);

    output.push_back(MSGEND);
  }

  return TransmitFormatted(output, bWaitForAck, iTimeout);
}

bool CCECParser::WaitForAck(int64_t iTimeout /* = 1000 */)
{
  bool bGotAck(false);
  bool bError(false);

  int64_t iNow = GetTimeMs();
  int64_t iTargetTime = iNow + iTimeout;

  while (!bGotAck && !bError && (iTimeout <= 0 || iNow < iTargetTime))
  {
    if (!ReadFromDevice((int) iTimeout))
    {
      AddLog(CEC_LOG_ERROR, "failed to read from device");
      return false;
    }

    cec_frame msg;
    while (!bGotAck && !bError && GetMessage(msg, false))
    {
      uint8_t iCode = msg[0] & ~(MSGCODE_FRAME_EOM | MSGCODE_FRAME_ACK);

      switch (iCode)
      {
      case MSGCODE_COMMAND_ACCEPTED:
        AddLog(CEC_LOG_DEBUG, "MSGCODE_COMMAND_ACCEPTED");
        break;
      case MSGCODE_TRANSMIT_SUCCEEDED:
        AddLog(CEC_LOG_DEBUG, "MSGCODE_TRANSMIT_SUCCEEDED");
        // TODO
        bGotAck = true;
        break;
      case MSGCODE_RECEIVE_FAILED:
        AddLog(CEC_LOG_WARNING, "MSGCODE_RECEIVE_FAILED");
        bError = true;
        break;
      case MSGCODE_COMMAND_REJECTED:
        AddLog(CEC_LOG_WARNING, "MSGCODE_COMMAND_REJECTED");
        bError = true;
        break;
      case MSGCODE_TRANSMIT_FAILED_LINE:
        AddLog(CEC_LOG_WARNING, "MSGCODE_TRANSMIT_FAILED_LINE");
        bError = true;
        break;
      case MSGCODE_TRANSMIT_FAILED_ACK:
        AddLog(CEC_LOG_WARNING, "MSGCODE_TRANSMIT_FAILED_ACK");
        bError = true;
        break;
      case MSGCODE_TRANSMIT_FAILED_TIMEOUT_DATA:
        AddLog(CEC_LOG_WARNING, "MSGCODE_TRANSMIT_FAILED_TIMEOUT_DATA");
        bError = true;
        break;
      case MSGCODE_TRANSMIT_FAILED_TIMEOUT_LINE:
        AddLog(CEC_LOG_WARNING, "MSGCODE_TRANSMIT_FAILED_TIMEOUT_LINE");
        bError = true;
        break;
      default:
        m_frameBuffer.Push(msg);
        bGotAck = (msg[0] & MSGCODE_FRAME_ACK) != 0;
        break;
      }
      iNow = GetTimeMs();
    }
  }

  return bGotAck && !bError;
}

bool CCECParser::ReadFromDevice(int iTimeout)
{
  uint8_t buff[1024];
  int iBytesRead = m_serialport->Read(buff, sizeof(buff), iTimeout);
  if (iBytesRead < 0)
  {
    CStdString strError;
    strError.Format("error reading from serial port: %s", m_serialport->GetError().c_str());
    AddLog(CEC_LOG_ERROR, strError);
    return false;
  }
  else if (iBytesRead > 0)
    AddData(buff, iBytesRead);

  return true;
}

void CCECParser::ProcessMessages(void)
{
  cec_frame msg;
  while (m_bRunning && GetMessage(msg))
    ParseMessage(msg);
}

bool CCECParser::GetMessage(cec_frame &msg, bool bFromBuffer /* = true */)
{
  if (bFromBuffer && m_frameBuffer.Pop(msg))
    return true;

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
    AddLog(CEC_LOG_ERROR, "received MSGSTART before MSGEND");
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

void CCECParser::ParseMessage(cec_frame &msg)
{
  if (msg.empty())
    return;

  CStdString logStr;
  uint8_t iCode = msg[0] & ~(MSGCODE_FRAME_EOM | MSGCODE_FRAME_ACK);
  bool    bEom  = (msg[0] & MSGCODE_FRAME_EOM) != 0;
  bool    bAck  = (msg[0] & MSGCODE_FRAME_ACK) != 0;

  switch(iCode)
  {
  case MSGCODE_NOTHING:
    AddLog(CEC_LOG_DEBUG, "MSGCODE_NOTHING");
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

      int iLine      = (msg.size() >= 3) ? (msg[1] << 8) | (msg[2]) : 0;
      uint32_t iTime = (msg.size() >= 7) ? (msg[3] << 24) | (msg[4] << 16) | (msg[5] << 8) | (msg[6]) : 0;
      logStr.AppendFormat(" line:%i", iLine);
      logStr.AppendFormat(" time:%u", iTime);
      AddLog(CEC_LOG_WARNING, logStr.c_str());
    }
    break;
  case MSGCODE_FRAME_START:
    {
      logStr = "MSGCODE_FRAME_START";
      m_currentframe.clear();
      if (msg.size() >= 2)
      {
        int iInitiator = msg[1] >> 4;
        int iDestination = msg[1] & 0xF;
        logStr.AppendFormat(" initiator:%u destination:%u ack:%s %s", iInitiator, iDestination, bAck ? "high" : "low", bEom ? "eom" : "");

        m_currentframe.push_back(msg[1]);
      }
      AddLog(CEC_LOG_DEBUG, logStr.c_str());
    }
    break;
  case MSGCODE_FRAME_DATA:
    {
      logStr = "MSGCODE_FRAME_DATA";
      if (msg.size() >= 2)
      {
        uint8_t iData = msg[1];
        logStr.AppendFormat(" %02x", iData);
        m_currentframe.push_back(iData);
      }
      AddLog(CEC_LOG_DEBUG, logStr.c_str());
    }
    if (bEom)
      ParseCurrentFrame();
    break;
  default:
    break;
  }
}

void CCECParser::ParseCurrentFrame(void)
{
  uint8_t initiator = m_currentframe[0] >> 4;
  uint8_t destination = m_currentframe[0] & 0xF;

  CStdString dataStr;
  dataStr.Format("received frame: initiator: %u destination: %u", initiator, destination);

  if (m_currentframe.size() > 1)
  {
    dataStr += " data:";
    for (unsigned int i = 1; i < m_currentframe.size(); i++)
      dataStr.AppendFormat(" %02x", m_currentframe[i]);
  }
  AddLog(CEC_LOG_DEBUG, dataStr.c_str());

  if (m_currentframe.size() <= 1)
    return;

  vector<uint8_t> tx;
  cec_opcode opCode = (cec_opcode) m_currentframe[1];
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
      if (m_currentframe.size() > 2)
      {
        AddKey();

        if (m_currentframe[2] <= CEC_USER_CONTROL_CODE_MAX)
        {
          m_iCurrentButton = (cec_user_control_code) m_currentframe[2];
          m_buttontime = GetTimeMs();
        }
      }
      break;
    case CEC_OPCODE_USER_CONTROL_RELEASE:
      AddKey();
      break;
    default:
      cec_frame params = m_currentframe;
      params.erase(params.begin(), params.begin() + 2);
      AddCommand((cec_logical_address) initiator, (cec_logical_address) destination, opCode, &params);
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
      if (m_currentframe.size() >= 4)
      {
        int streamaddr = ((int)m_currentframe[2] << 8) | ((int)m_currentframe[3]);
        CStdString strLog;
        strLog.Format("%i requests stream path from physical address %04x", initiator, streamaddr);
        AddLog(CEC_LOG_DEBUG, strLog.c_str());
        if (streamaddr == m_physicaladdress)
          BroadcastActiveSource();
      }
    }
    else
    {
      cec_frame params = m_currentframe;
      params.erase(params.begin(), params.begin() + 2);
      AddCommand((cec_logical_address) initiator, (cec_logical_address) destination, opCode, &params);
    }
  }
  else
  {
    CStdString strLog;
    strLog.Format("ignoring frame: destination: %u != %u", destination, (uint16_t)m_iLogicalAddress);
    AddLog(CEC_LOG_DEBUG, strLog.c_str());
  }
}

void CCECParser::AddData(uint8_t *data, int iLen)
{
  if (iLen + m_iInbufUsed > m_iInbufSize)
  {
    m_iInbufSize = iLen + m_iInbufUsed;
    m_inbuf = (uint8_t*)realloc(m_inbuf, m_iInbufSize);
  }

  memcpy(m_inbuf + m_iInbufUsed, data, iLen);
  m_iInbufUsed += iLen;
}

void CCECParser::PushEscaped(cec_frame &vec, uint8_t byte)
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

void CCECParser::CheckKeypressTimeout(int64_t now)
{
  if (m_iCurrentButton != CEC_USER_CONTROL_CODE_UNKNOWN && now - m_buttontime > 500)
  {
    AddKey();
    m_iCurrentButton = CEC_USER_CONTROL_CODE_UNKNOWN;
  }
}

bool CCECParser::SetLogicalAddress(cec_logical_address iLogicalAddress)
{
  CStdString strLog;
  strLog.Format("setting logical address to %d", iLogicalAddress);
  AddLog(CEC_LOG_NOTICE, strLog.c_str());

  m_iLogicalAddress = iLogicalAddress;
  return SetAckMask(0x1 << (uint8_t)m_iLogicalAddress);
}

bool CCECParser::SetAckMask(uint16_t iMask)
{
  CStdString strLog;
  strLog.Format("setting ackmask to %2x", iMask);
  AddLog(CEC_LOG_DEBUG, strLog.c_str());

  cec_frame output;

  output.push_back(MSGSTART);
  PushEscaped(output, MSGCODE_SET_ACK_MASK);
  PushEscaped(output, iMask >> 8);
  PushEscaped(output, (uint8_t)iMask);
  output.push_back(MSGEND);

  if (m_serialport->Write(output) == -1)
  {
    strLog.Format("error writing to serial port: %s", m_serialport->GetError().c_str());
    AddLog(CEC_LOG_ERROR, strLog);
    return false;
  }

  return true;
}

void CCECParser::AddLog(cec_log_level level, const string &strMessage)
{
  cec_log_message message;
  message.level = level;
  message.message.assign(strMessage.c_str());
  m_logBuffer.Push(message);
}

void CCECParser::AddKey(void)
{
  if (m_iCurrentButton != CEC_USER_CONTROL_CODE_UNKNOWN)
  {
    cec_keypress key;
    key.duration = (unsigned int) (GetTimeMs() - m_buttontime);
    key.keycode = m_iCurrentButton;
    m_keyBuffer.Push(key);
    m_iCurrentButton = CEC_USER_CONTROL_CODE_UNKNOWN;
    m_buttontime = 0;
  }
}

void CCECParser::AddCommand(cec_logical_address source, cec_logical_address destination, cec_opcode opcode, cec_frame *parameters)
{
  cec_command command;
  command.source       = source;
  command.destination  = destination;
  command.opcode       = opcode;
  if (parameters)
    command.parameters = *parameters;
  if (m_commandBuffer.Push(command))
  {
    CStdString strDebug;
    strDebug.Format("stored command '%d' in the command buffer. buffer size = %d", opcode, m_commandBuffer.Size());
    AddLog(CEC_LOG_DEBUG, strDebug);
  }
  else
  {
    AddLog(CEC_LOG_WARNING, "command buffer is full");
  }
}

int CCECParser::GetMinVersion(void)
{
  return CEC_MIN_VERSION;
}

int CCECParser::GetLibVersion(void)
{
  return CEC_LIB_VERSION;
}

int CCECParser::FindDevices(std::vector<cec_device> &deviceList, const char *strDevicePath /* = NULL */)
{
  CStdString strDebug;
  if (strDevicePath)
    strDebug.Format("trying to autodetect the com port for device path '%s'", strDevicePath);
  else
    strDebug.Format("trying to autodetect all CEC adapters");
  AddLog(CEC_LOG_DEBUG, strDebug);

  return CCECDetect::FindDevices(deviceList, strDevicePath);
}

DECLSPEC void * CECCreate(const char *strDeviceName, CEC::cec_logical_address iLogicalAddress /*= CEC::CECDEVICE_PLAYBACKDEVICE1 */, int iPhysicalAddress /* = CEC_DEFAULT_PHYSICAL_ADDRESS */)
{
  return static_cast< void* > (new CCECParser(strDeviceName, iLogicalAddress, iPhysicalAddress));
}
