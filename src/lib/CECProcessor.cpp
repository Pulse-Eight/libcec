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
#include "devices/CECAudioSystem.h"
#include "devices/CECPlaybackDevice.h"
#include "devices/CECRecordingDevice.h"
#include "devices/CECTuner.h"
#include "devices/CECTV.h"
#include "implementations/CECCommandHandler.h"
#include "LibCEC.h"
#include "util/StdString.h"
#include "platform/timeutils.h"

using namespace CEC;
using namespace std;

CCECProcessor::CCECProcessor(CLibCEC *controller, const char *strDeviceName, cec_logical_address iLogicalAddress /* = CECDEVICE_PLAYBACKDEVICE1 */, uint16_t iPhysicalAddress /* = CEC_DEFAULT_PHYSICAL_ADDRESS*/) :
    m_bStarted(false),
    m_iHDMIPort(CEC_DEFAULT_HDMI_PORT),
    m_iBaseDevice((cec_logical_address)CEC_DEFAULT_BASE_DEVICE),
    m_lastInitiator(CECDEVICE_UNKNOWN),
    m_strDeviceName(strDeviceName),
    m_controller(controller),
    m_bMonitor(false),
    m_busScan(NULL)
{
  m_communication = new CAdapterCommunication(this);
  m_logicalAddresses.Clear();
  m_logicalAddresses.Set(iLogicalAddress);
  m_types.clear();
  for (int iPtr = 0; iPtr <= 16; iPtr++)
    m_busDevices[iPtr] = new CCECBusDevice(this, (cec_logical_address) iPtr, iPtr == iLogicalAddress ? iPhysicalAddress : 0);
}

CCECProcessor::CCECProcessor(CLibCEC *controller, const char *strDeviceName, const cec_device_type_list &types) :
    m_bStarted(false),
    m_iHDMIPort(CEC_DEFAULT_HDMI_PORT),
    m_iBaseDevice((cec_logical_address)CEC_DEFAULT_BASE_DEVICE),
    m_strDeviceName(strDeviceName),
    m_types(types),
    m_controller(controller),
    m_bMonitor(false)
{
  m_communication = new CAdapterCommunication(this);
  m_logicalAddresses.Clear();
  for (int iPtr = 0; iPtr < 16; iPtr++)
  {
    switch(iPtr)
    {
    case CECDEVICE_AUDIOSYSTEM:
      m_busDevices[iPtr] = new CCECAudioSystem(this, (cec_logical_address) iPtr, 0xFFFF);
      break;
    case CECDEVICE_PLAYBACKDEVICE1:
    case CECDEVICE_PLAYBACKDEVICE2:
    case CECDEVICE_PLAYBACKDEVICE3:
      m_busDevices[iPtr] = new CCECPlaybackDevice(this, (cec_logical_address) iPtr, 0xFFFF);
      break;
    case CECDEVICE_RECORDINGDEVICE1:
    case CECDEVICE_RECORDINGDEVICE2:
    case CECDEVICE_RECORDINGDEVICE3:
      m_busDevices[iPtr] = new CCECRecordingDevice(this, (cec_logical_address) iPtr, 0xFFFF);
      break;
    case CECDEVICE_TUNER1:
    case CECDEVICE_TUNER2:
    case CECDEVICE_TUNER3:
    case CECDEVICE_TUNER4:
      m_busDevices[iPtr] = new CCECTuner(this, (cec_logical_address) iPtr, 0xFFFF);
      break;
    case CECDEVICE_TV:
      m_busDevices[iPtr] = new CCECTV(this, (cec_logical_address) iPtr, 0);
      break;
    default:
      m_busDevices[iPtr] = new CCECBusDevice(this, (cec_logical_address) iPtr, 0xFFFF);
      break;
    }
  }
}

CCECProcessor::~CCECProcessor(void)
{
  if (m_busScan)
  {
    m_busScan->StopThread();
    delete m_busScan;
  }

  m_startCondition.Broadcast();
  StopThread();

  delete m_communication;
  m_communication = NULL;
  m_controller = NULL;
  for (unsigned int iPtr = 0; iPtr < 16; iPtr++)
    delete m_busDevices[iPtr];
}

bool CCECProcessor::Start(const char *strPort, uint16_t iBaudRate /* = 38400 */, uint32_t iTimeoutMs /* = 10000 */)
{
  CLockObject lock(&m_mutex);
  if (!m_communication || m_communication->IsOpen())
  {
    m_controller->AddLog(CEC_LOG_ERROR, "connection already opened");
    return false;
  }

  if (!m_communication->Open(strPort, iBaudRate, iTimeoutMs))
  {
    m_controller->AddLog(CEC_LOG_ERROR, "could not open a connection");
    return false;
  }

  if (CreateThread())
  {
    if (!m_startCondition.Wait(&m_mutex) || !m_bStarted)
    {
      m_controller->AddLog(CEC_LOG_ERROR, "could not create a processor thread");
      return false;
    }

    lock.Leave();
    if (SetAckMask(m_logicalAddresses.AckMask()) &&
        SetHDMIPort(m_iBaseDevice, m_iHDMIPort, true))
    {
      m_busScan = new CCECBusScan(this);
      m_busScan->CreateThread(true);
      return true;
    }
  }
  else
    m_controller->AddLog(CEC_LOG_ERROR, "could not create a processor thread");

  return false;
}

bool CCECProcessor::TryLogicalAddress(cec_logical_address address)
{
  if (m_busDevices[address]->TryLogicalAddress())
  {
    /* only set our OSD name and active source for the primary device */
    if (m_logicalAddresses.IsEmpty())
    {
      m_busDevices[address]->m_strDeviceName = m_strDeviceName;
      m_busDevices[address]->m_bActiveSource = true;
    }
    m_logicalAddresses.Set(address);
    return true;
  }

  return false;
}

bool CCECProcessor::FindLogicalAddressRecordingDevice(void)
{
  AddLog(CEC_LOG_DEBUG, "detecting logical address for type 'recording device'");
  return TryLogicalAddress(CECDEVICE_RECORDINGDEVICE1) ||
      TryLogicalAddress(CECDEVICE_RECORDINGDEVICE2) ||
      TryLogicalAddress(CECDEVICE_RECORDINGDEVICE3);
}

bool CCECProcessor::FindLogicalAddressTuner(void)
{
  AddLog(CEC_LOG_DEBUG, "detecting logical address for type 'tuner'");
  return TryLogicalAddress(CECDEVICE_TUNER1) ||
      TryLogicalAddress(CECDEVICE_TUNER2) ||
      TryLogicalAddress(CECDEVICE_TUNER3) ||
      TryLogicalAddress(CECDEVICE_TUNER4);
}

bool CCECProcessor::FindLogicalAddressPlaybackDevice(void)
{
  AddLog(CEC_LOG_DEBUG, "detecting logical address for type 'playback device'");
  return TryLogicalAddress(CECDEVICE_PLAYBACKDEVICE1) ||
      TryLogicalAddress(CECDEVICE_PLAYBACKDEVICE2) ||
      TryLogicalAddress(CECDEVICE_PLAYBACKDEVICE3);
}

bool CCECProcessor::FindLogicalAddressAudioSystem(void)
{
  AddLog(CEC_LOG_DEBUG, "detecting logical address for type 'audio'");
  return TryLogicalAddress(CECDEVICE_AUDIOSYSTEM);
}

bool CCECProcessor::FindLogicalAddresses(void)
{
  bool bReturn(true);
  m_logicalAddresses.Clear();
  CStdString strLog;

  for (unsigned int iPtr = 0; iPtr < 5; iPtr++)
  {
    if (m_types.types[iPtr] == CEC_DEVICE_TYPE_RESERVED)
      continue;

    strLog.Format("%s - device %d: type %d", __FUNCTION__, iPtr, m_types.types[iPtr]);
    AddLog(CEC_LOG_DEBUG, strLog);

    if (m_types.types[iPtr] == CEC_DEVICE_TYPE_RECORDING_DEVICE)
      bReturn &= FindLogicalAddressRecordingDevice();
    if (m_types.types[iPtr] == CEC_DEVICE_TYPE_TUNER)
      bReturn &= FindLogicalAddressTuner();
    if (m_types.types[iPtr] == CEC_DEVICE_TYPE_PLAYBACK_DEVICE)
      bReturn &= FindLogicalAddressPlaybackDevice();
    if (m_types.types[iPtr] == CEC_DEVICE_TYPE_AUDIO_SYSTEM)
      bReturn &= FindLogicalAddressAudioSystem();
  }

  return bReturn;
}

bool CCECProcessor::SetLineTimeout(uint8_t iTimeout)
{
  bool bReturn(false);
  CCECAdapterMessage *output = new CCECAdapterMessage;

  output->push_back(MSGSTART);
  output->push_escaped(MSGCODE_TRANSMIT_IDLETIME);
  output->push_escaped(iTimeout);
  output->push_back(MSGEND);

  if ((bReturn = Transmit(output)) == false)
    m_controller->AddLog(CEC_LOG_ERROR, "could not set the idletime");
  delete output;
  return bReturn;
}

void *CCECProcessor::Process(void)
{
  bool                  bParseFrame(false);
  cec_command           command;
  CCECAdapterMessage    msg;

  if (m_logicalAddresses.IsEmpty() && !FindLogicalAddresses())
  {
    CLockObject lock(&m_mutex);
    m_controller->AddLog(CEC_LOG_ERROR, "could not detect our logical addresses");
    m_startCondition.Signal();
    return NULL;
  }
  else
  {
    CLockObject lock(&m_mutex);
    m_bStarted = true;
    m_controller->AddLog(CEC_LOG_DEBUG, "processor thread started");
    m_startCondition.Signal();
  }

  while (!IsStopped())
  {
    command.Clear();
    msg.clear();

    {
      CLockObject lock(&m_mutex);
      if (m_commandBuffer.Pop(command))
      {
        bParseFrame = true;
      }
      else if (m_communication->IsOpen() && m_communication->Read(msg, 50))
      {
        if ((bParseFrame = (ParseMessage(msg) && !IsStopped())) == true)
          command = m_currentframe;
      }
    }

    if (bParseFrame)
      ParseCommand(command);
    bParseFrame = false;

    Sleep(5);

    m_controller->CheckKeypressTimeout();
  }

  if (m_communication)
    m_communication->Close();

  return NULL;
}

bool CCECProcessor::SetActiveSource(cec_device_type type /* = CEC_DEVICE_TYPE_RESERVED */)
{
  bool bReturn(false);

  if (!IsRunning())
    return bReturn;

  cec_logical_address addr = m_logicalAddresses.primary;

  if (type != CEC_DEVICE_TYPE_RESERVED)
  {
    for (uint8_t iPtr = 0; iPtr < 16; iPtr++)
    {
      if (m_logicalAddresses[iPtr] && m_busDevices[iPtr]->m_type == type)
      {
        addr = (cec_logical_address) iPtr;
        break;
      }
    }
  }

  return SetStreamPath(m_busDevices[addr]->GetPhysicalAddress(false)) &&
      m_busDevices[addr]->TransmitActiveSource();
}

bool CCECProcessor::SetActiveSource(cec_logical_address iAddress)
{
  return SetStreamPath(m_busDevices[iAddress]->GetPhysicalAddress(false));
}

bool CCECProcessor::SetActiveView(void)
{
  return SetActiveSource(m_types.IsEmpty() ? CEC_DEVICE_TYPE_RESERVED : m_types[0]);
}

bool CCECProcessor::SetDeckControlMode(cec_deck_control_mode mode, bool bSendUpdate /* = true */)
{
  bool bReturn(false);

  CCECBusDevice *device = GetDeviceByType(CEC_DEVICE_TYPE_PLAYBACK_DEVICE);
  if (device)
  {
    ((CCECPlaybackDevice *) device)->SetDeckControlMode(mode);
    if (bSendUpdate)
      ((CCECPlaybackDevice *) device)->TransmitDeckStatus(CECDEVICE_TV);
    bReturn = true;
  }

  return bReturn;
}

bool CCECProcessor::SetDeckInfo(cec_deck_info info, bool bSendUpdate /* = true */)
{
  bool bReturn(false);

  CCECBusDevice *device = GetDeviceByType(CEC_DEVICE_TYPE_PLAYBACK_DEVICE);
  if (device)
  {
    ((CCECPlaybackDevice *) device)->SetDeckStatus(info);
    if (bSendUpdate)
      ((CCECPlaybackDevice *) device)->TransmitDeckStatus(CECDEVICE_TV);
    bReturn = true;
  }

  return bReturn;
}

bool CCECProcessor::SetHDMIPort(cec_logical_address iBaseDevice, uint8_t iPort, bool bForce /* = false */)
{
  bool bReturn(false);

  CStdString strLog;
  strLog.Format("setting HDMI port to %d on device %s (%d)", iPort, ToString(iBaseDevice), (int)iBaseDevice);
  AddLog(CEC_LOG_DEBUG, strLog);

  m_iBaseDevice = iBaseDevice;
  m_iHDMIPort = iPort;
  if (!m_bStarted && !bForce)
    return true;

  uint16_t iPhysicalAddress(0);
  iPhysicalAddress = m_busDevices[iBaseDevice]->GetPhysicalAddress();
  uint16_t iPos = 0;
  if (iPhysicalAddress == 0)
    iPos = 0x1000;
  else if (iPhysicalAddress % 0x1000 == 0)
    iPos = 0x100;
  else if (iPhysicalAddress % 0x100 == 0)
    iPos = 0x10;
  else if (iPhysicalAddress % 0x10 == 0)
    iPos = 0x1;

  while(!bReturn && iPos > 0)
  {
    iPhysicalAddress += (uint16_t)(iPort * iPos);
    strLog.Format("checking physical address %4x", iPhysicalAddress);
    AddLog(CEC_LOG_DEBUG, strLog);
    if (CheckPhysicalAddress(iPhysicalAddress))
    {
      strLog.Format("physical address %4x is in use", iPhysicalAddress);
      AddLog(CEC_LOG_DEBUG, strLog);
      iPos = (iPos == 1) ? 0 : iPos / 0x10;
    }
    else
    {
      SetPhysicalAddress(iPhysicalAddress);
      bReturn = true;
    }
  }

  return bReturn;
}

bool CCECProcessor::CheckPhysicalAddress(uint16_t iPhysicalAddress)
{
  for (unsigned int iPtr = 0; iPtr < 15; iPtr++)
  {
    if (m_busDevices[iPtr]->GetPhysicalAddress(false) == iPhysicalAddress)
      return true;
  }
  return false;
}

bool CCECProcessor::SetStreamPath(uint16_t iStreamPath)
{
  bool bReturn(false);

  CCECBusDevice *device = GetDeviceByPhysicalAddress(iStreamPath);
  if (device)
  {
    device->SetActiveDevice();
    bReturn = true;
  }

  return bReturn;
}

bool CCECProcessor::TransmitInactiveSource(void)
{
  if (!IsRunning())
    return false;

  if (!m_logicalAddresses.IsEmpty() && m_busDevices[m_logicalAddresses.primary])
    return m_busDevices[m_logicalAddresses.primary]->TransmitInactiveSource();
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
  if (m_logicalAddresses.primary != iLogicalAddress)
  {
    CStdString strLog;
    strLog.Format("<< setting primary logical address to %1x", iLogicalAddress);
    m_controller->AddLog(CEC_LOG_NOTICE, strLog.c_str());
    m_logicalAddresses.primary = iLogicalAddress;
    m_logicalAddresses.Set(iLogicalAddress);
    return SetAckMask(m_logicalAddresses.AckMask());
  }

  return true;
}

bool CCECProcessor::SetMenuState(cec_menu_state state, bool bSendUpdate /* = true */)
{
  for (uint8_t iPtr = 0; iPtr < 16; iPtr++)
  {
    if (m_logicalAddresses[iPtr])
      m_busDevices[iPtr]->SetMenuState(state);
  }

  if (bSendUpdate)
    m_busDevices[m_logicalAddresses.primary]->TransmitMenuState(CECDEVICE_TV);

  return true;
}

bool CCECProcessor::SetPhysicalAddress(uint16_t iPhysicalAddress)
{
  if (!m_logicalAddresses.IsEmpty())
  {
    for (uint8_t iPtr = 0; iPtr < 15; iPtr++)
      if (m_logicalAddresses[iPtr])
        m_busDevices[iPtr]->SetPhysicalAddress(iPhysicalAddress);
    return SetActiveView();
  }
  return false;
}

bool CCECProcessor::SwitchMonitoring(bool bEnable)
{
  CStdString strLog;
  strLog.Format("== %s monitoring mode ==", bEnable ? "enabling" : "disabling");
  m_controller->AddLog(CEC_LOG_NOTICE, strLog.c_str());

  {
    CLockObject lock(&m_mutex);
    m_bMonitor = bEnable;

    if (bEnable)
    {
      if (!m_busScan)
      {
        m_busScan = new CCECBusScan(this);
        m_busScan->CreateThread(true);
      }
    }
    else
    {
      if (m_busScan)
      {
        m_busScan->StopThread();
        delete m_busScan;
        m_busScan = NULL;
      }
    }
  }

  if (bEnable)
    return SetAckMask(0);
  else
    return SetAckMask(m_logicalAddresses.AckMask());
}

bool CCECProcessor::PollDevice(cec_logical_address iAddress)
{
  if (iAddress != CECDEVICE_UNKNOWN && m_busDevices[iAddress])
  {
    return m_logicalAddresses.primary == CECDEVICE_UNKNOWN ?
        m_busDevices[iAddress]->TransmitPoll(iAddress) :
        m_busDevices[m_logicalAddresses.primary]->TransmitPoll(iAddress);
  }
  return false;
}

uint8_t CCECProcessor::VolumeUp(bool bWait /* = true */)
{
  uint8_t status = 0;
  if (IsActiveDevice(CECDEVICE_AUDIOSYSTEM))
    status = ((CCECAudioSystem *)m_busDevices[CECDEVICE_AUDIOSYSTEM])->VolumeUp(bWait);

  return status;
}

uint8_t CCECProcessor::VolumeDown(bool bWait /* = true */)
{
  uint8_t status = 0;
  if (IsActiveDevice(CECDEVICE_AUDIOSYSTEM))
    status = ((CCECAudioSystem *)m_busDevices[CECDEVICE_AUDIOSYSTEM])->VolumeDown(bWait);

  return status;
}

uint8_t CCECProcessor::MuteAudio(bool bWait /* = true */)
{
  uint8_t status = 0;
  if (IsActiveDevice(CECDEVICE_AUDIOSYSTEM))
    status = ((CCECAudioSystem *)m_busDevices[CECDEVICE_AUDIOSYSTEM])->MuteAudio(bWait);

  return status;
}

CCECBusDevice *CCECProcessor::GetDeviceByPhysicalAddress(uint16_t iPhysicalAddress, bool bRefresh /* = false */) const
{
  if (m_busDevices[m_logicalAddresses.primary]->GetPhysicalAddress(false) == iPhysicalAddress)
    return m_busDevices[m_logicalAddresses.primary];

  CCECBusDevice *device = NULL;
  for (unsigned int iPtr = 0; iPtr < 16; iPtr++)
  {
    if (m_busDevices[iPtr]->GetPhysicalAddress(bRefresh) == iPhysicalAddress)
    {
      device = m_busDevices[iPtr];
      break;
    }
  }

  return device;
}

CCECBusDevice *CCECProcessor::GetDeviceByType(cec_device_type type) const
{
  CCECBusDevice *device = NULL;

  for (unsigned int iPtr = 0; iPtr < 16; iPtr++)
  {
    if (m_busDevices[iPtr]->m_type == type)
    {
      device = m_busDevices[iPtr];
      break;
    }
  }

  return device;
}

cec_version CCECProcessor::GetDeviceCecVersion(cec_logical_address iAddress)
{
  return m_busDevices[iAddress]->GetCecVersion();
}

cec_osd_name CCECProcessor::GetDeviceOSDName(cec_logical_address iAddress)
{
  CStdString strOSDName = m_busDevices[iAddress]->GetOSDName();
  cec_osd_name retVal;

  snprintf(retVal.name, sizeof(retVal.name), "%s", strOSDName.c_str());
  retVal.device = iAddress;

  return retVal;
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
    do
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
        if ((bReturn = WaitForTransmitSucceeded(output)) == false)
          m_controller->AddLog(CEC_LOG_DEBUG, "did not receive ack");
      }
      else
        bReturn = true;
    }while (output->transmit_timeout > 0 && output->needs_retry() && ++output->tries <= output->maxTries);
  }

  return bReturn;
}

void CCECProcessor::TransmitAbort(cec_logical_address address, cec_opcode opcode, cec_abort_reason reason /* = CEC_ABORT_REASON_UNRECOGNIZED_OPCODE */)
{
  m_controller->AddLog(CEC_LOG_DEBUG, "<< transmitting abort message");

  cec_command command;
  // TODO
  cec_command::Format(command, m_logicalAddresses.primary, address, CEC_OPCODE_FEATURE_ABORT);
  command.parameters.PushBack((uint8_t)opcode);
  command.parameters.PushBack((uint8_t)reason);

  Transmit(command);
}

bool CCECProcessor::WaitForTransmitSucceeded(CCECAdapterMessage *message)
{
  bool bError(false);
  bool bTransmitSucceeded(false);
  uint8_t iPacketsLeft(message->size() / 4);

  int64_t iNow = GetTimeMs();
  int64_t iTargetTime = iNow + message->transmit_timeout;

  while (!bTransmitSucceeded && !bError && (message->transmit_timeout == 0 || iNow < iTargetTime))
  {
    CCECAdapterMessage msg;

    if (!m_communication->Read(msg, message->transmit_timeout > 0 ? (int32_t)(iTargetTime - iNow) : 1000))
    {
      iNow = GetTimeMs();
      continue;
    }

    if (msg.message() == MSGCODE_FRAME_START && msg.ack())
    {
      m_busDevices[msg.initiator()]->GetHandler()->HandlePoll(msg.initiator(), msg.destination());
      m_lastInitiator = msg.initiator();
      iNow = GetTimeMs();
      continue;
    }

    bError = msg.is_error();
    if (msg.message() == MSGCODE_RECEIVE_FAILED &&
        m_lastInitiator != CECDEVICE_UNKNOWN &&
        !m_busDevices[m_lastInitiator]->GetHandler()->HandleReceiveFailed())
    {
      iNow = GetTimeMs();
      continue;
    }

    if (bError)
    {
      message->reply = msg.message();
      m_controller->AddLog(CEC_LOG_DEBUG, msg.ToString());
    }
    else
    {
      switch(msg.message())
      {
      case MSGCODE_COMMAND_ACCEPTED:
        m_controller->AddLog(CEC_LOG_DEBUG, msg.ToString());
        if (iPacketsLeft > 0)
          iPacketsLeft--;
        break;
      case MSGCODE_TRANSMIT_SUCCEEDED:
        m_controller->AddLog(CEC_LOG_DEBUG, msg.ToString());
        bTransmitSucceeded = (iPacketsLeft == 0);
        bError = !bTransmitSucceeded;
        message->reply = MSGCODE_TRANSMIT_SUCCEEDED;
        break;
      default:
        // ignore other data while waiting
        break;
      }

      iNow = GetTimeMs();
    }
  }

  return bTransmitSucceeded && !bError;
}

bool CCECProcessor::ParseMessage(const CCECAdapterMessage &msg)
{
  bool bEom(false);
  bool bIsError(msg.is_error());

  if (msg.empty())
    return bEom;

  switch(msg.message())
  {
  case MSGCODE_FRAME_START:
    {
      m_currentframe.Clear();
      if (msg.size() >= 2)
      {
        m_currentframe.initiator   = msg.initiator();
        m_currentframe.destination = msg.destination();
        m_currentframe.ack         = msg.ack();
        m_currentframe.eom         = msg.eom();
      }
      if (m_currentframe.ack == true)
      {
        m_lastInitiator = m_currentframe.initiator;
        m_busDevices[m_lastInitiator]->GetHandler()->HandlePoll(m_currentframe.initiator, m_currentframe.destination);
      }
    }
    break;
  case MSGCODE_RECEIVE_FAILED:
    {
      if (m_lastInitiator != CECDEVICE_UNKNOWN)
        bIsError = m_busDevices[m_lastInitiator]->GetHandler()->HandleReceiveFailed();
    }
    break;
  case MSGCODE_FRAME_DATA:
    {
      if (msg.size() >= 2)
      {
        m_currentframe.PushBack(msg[1]);
        m_currentframe.eom = msg.eom();
      }
      bEom = msg.eom();
    }
    break;
  default:
    break;
  }

  m_controller->AddLog(bIsError ? CEC_LOG_WARNING : CEC_LOG_DEBUG, msg.ToString());
  return bEom;
}

void CCECProcessor::ParseCommand(cec_command &command)
{
  CStdString dataStr;
  dataStr.Format(">> %1x%1x:%02x", command.initiator, command.destination, command.opcode);
  for (uint8_t iPtr = 0; iPtr < command.parameters.size; iPtr++)
    dataStr.AppendFormat(":%02x", (unsigned int)command.parameters[iPtr]);
  m_controller->AddLog(CEC_LOG_TRAFFIC, dataStr.c_str());

  if (!m_bMonitor && command.initiator >= CECDEVICE_TV && command.initiator <= CECDEVICE_BROADCAST)
    m_busDevices[(uint8_t)command.initiator]->HandleCommand(command);
}

cec_logical_addresses CCECProcessor::GetActiveDevices(void)
{
  cec_logical_addresses addresses;
  addresses.Clear();
  for (unsigned int iPtr = 0; iPtr < 15; iPtr++)
  {
    if (m_busDevices[iPtr]->GetStatus() == CEC_DEVICE_STATUS_PRESENT)
      addresses.Set((cec_logical_address) iPtr);
  }
  return addresses;
}

bool CCECProcessor::IsActiveDevice(cec_logical_address address)
{
  return m_busDevices[address]->GetStatus() == CEC_DEVICE_STATUS_PRESENT;
}

bool CCECProcessor::IsActiveDeviceType(cec_device_type type)
{
  for (unsigned int iPtr = 0; iPtr < 15; iPtr++)
  {
    if (m_busDevices[iPtr]->GetType() == type && m_busDevices[iPtr]->GetStatus() == CEC_DEVICE_STATUS_PRESENT)
      return true;
  }

  return false;
}

uint16_t CCECProcessor::GetPhysicalAddress(void) const
{
  if (!m_logicalAddresses.IsEmpty() && m_busDevices[m_logicalAddresses.primary])
    return m_busDevices[m_logicalAddresses.primary]->GetPhysicalAddress(false);
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

bool CCECProcessor::SendKeypress(cec_logical_address iDestination, cec_user_control_code key, bool bWait /* = false */)
{
  return m_busDevices[iDestination]->SendKeypress(key, bWait);
}

bool CCECProcessor::SendKeyRelease(cec_logical_address iDestination, bool bWait /* = false */)
{
  return m_busDevices[iDestination]->SendKeyRelease(bWait);
}

const char *CCECProcessor::ToString(const cec_menu_state state)
{
  switch (state)
  {
  case CEC_MENU_STATE_ACTIVATED:
    return "activated";
  case CEC_MENU_STATE_DEACTIVATED:
    return "deactivated";
  default:
    return "unknown";
  }
}

const char *CCECProcessor::ToString(const cec_version version)
{
  switch (version)
  {
  case CEC_VERSION_1_2:
    return "1.2";
  case CEC_VERSION_1_2A:
    return "1.2a";
  case CEC_VERSION_1_3:
    return "1.3";
  case CEC_VERSION_1_3A:
    return "1.3a";
  case CEC_VERSION_1_4:
    return "1.4";
  default:
    return "unknown";
  }
}

const char *CCECProcessor::ToString(const cec_power_status status)
{
  switch (status)
  {
  case CEC_POWER_STATUS_ON:
    return "on";
  case CEC_POWER_STATUS_STANDBY:
    return "standby";
  case CEC_POWER_STATUS_IN_TRANSITION_ON_TO_STANDBY:
    return "in transition from on to standby";
  case CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON:
    return "in transition from standby to on";
  default:
    return "unknown";
  }
}

const char *CCECProcessor::ToString(const cec_logical_address address)
{
  switch(address)
  {
  case CECDEVICE_AUDIOSYSTEM:
    return "Audio";
  case CECDEVICE_BROADCAST:
    return "Broadcast";
  case CECDEVICE_FREEUSE:
    return "Free use";
  case CECDEVICE_PLAYBACKDEVICE1:
    return "Playback 1";
  case CECDEVICE_PLAYBACKDEVICE2:
    return "Playback 2";
  case CECDEVICE_PLAYBACKDEVICE3:
    return "Playback 3";
  case CECDEVICE_RECORDINGDEVICE1:
    return "Recorder 1";
  case CECDEVICE_RECORDINGDEVICE2:
    return "Recorder 2";
  case CECDEVICE_RECORDINGDEVICE3:
    return "Recorder 3";
  case CECDEVICE_RESERVED1:
    return "Reserved 1";
  case CECDEVICE_RESERVED2:
    return "Reserved 2";
  case CECDEVICE_TUNER1:
    return "Tuner 1";
  case CECDEVICE_TUNER2:
    return "Tuner 2";
  case CECDEVICE_TUNER3:
    return "Tuner 3";
  case CECDEVICE_TUNER4:
    return "Tuner 4";
  case CECDEVICE_TV:
    return "TV";
  default:
    return "unknown";
  }
}

const char *CCECProcessor::ToString(const cec_deck_control_mode mode)
{
  switch (mode)
  {
  case CEC_DECK_CONTROL_MODE_SKIP_FORWARD_WIND:
    return "skip forward wind";
  case CEC_DECK_CONTROL_MODE_EJECT:
    return "eject";
  case CEC_DECK_CONTROL_MODE_SKIP_REVERSE_REWIND:
    return "reverse rewind";
  case CEC_DECK_CONTROL_MODE_STOP:
    return "stop";
  default:
    return "unknown";
  }
}

const char *CCECProcessor::ToString(const cec_deck_info status)
{
  switch (status)
  {
  case CEC_DECK_INFO_PLAY:
    return "play";
  case CEC_DECK_INFO_RECORD:
    return "record";
  case CEC_DECK_INFO_PLAY_REVERSE:
    return "play reverse";
  case CEC_DECK_INFO_STILL:
    return "still";
  case CEC_DECK_INFO_SLOW:
    return "slow";
  case CEC_DECK_INFO_SLOW_REVERSE:
    return "slow reverse";
  case CEC_DECK_INFO_FAST_FORWARD:
    return "fast forward";
  case CEC_DECK_INFO_FAST_REVERSE:
    return "fast reverse";
  case CEC_DECK_INFO_NO_MEDIA:
    return "no media";
  case CEC_DECK_INFO_STOP:
    return "stop";
  case CEC_DECK_INFO_SKIP_FORWARD_WIND:
    return "info skip forward wind";
  case CEC_DECK_INFO_SKIP_REVERSE_REWIND:
    return "info skip reverse rewind";
  case CEC_DECK_INFO_INDEX_SEARCH_FORWARD:
    return "info index search forward";
  case CEC_DECK_INFO_INDEX_SEARCH_REVERSE:
    return "info index search reverse";
  case CEC_DECK_INFO_OTHER_STATUS:
    return "other";
  default:
    return "unknown";
  }
}

const char *CCECProcessor::ToString(const cec_opcode opcode)
{
  switch (opcode)
  {
  case CEC_OPCODE_ACTIVE_SOURCE:
    return "active source";
  case CEC_OPCODE_IMAGE_VIEW_ON:
    return "image view on";
  case CEC_OPCODE_TEXT_VIEW_ON:
    return "text view on";
  case CEC_OPCODE_INACTIVE_SOURCE:
    return "inactive source";
  case CEC_OPCODE_REQUEST_ACTIVE_SOURCE:
    return "request active source";
  case CEC_OPCODE_ROUTING_CHANGE:
    return "routing change";
  case CEC_OPCODE_ROUTING_INFORMATION:
    return "routing information";
  case CEC_OPCODE_SET_STREAM_PATH:
    return "set stream path";
  case CEC_OPCODE_STANDBY:
    return "standby";
  case CEC_OPCODE_RECORD_OFF:
    return "record off";
  case CEC_OPCODE_RECORD_ON:
    return "record on";
  case CEC_OPCODE_RECORD_STATUS:
    return "record status";
  case CEC_OPCODE_RECORD_TV_SCREEN:
    return "record tv screen";
  case CEC_OPCODE_CLEAR_ANALOGUE_TIMER:
    return "clear analogue timer";
  case CEC_OPCODE_CLEAR_DIGITAL_TIMER:
    return "clear digital timer";
  case CEC_OPCODE_CLEAR_EXTERNAL_TIMER:
    return "clear external timer";
  case CEC_OPCODE_SET_ANALOGUE_TIMER:
    return "set analogue timer";
  case CEC_OPCODE_SET_DIGITAL_TIMER:
    return "set digital timer";
  case CEC_OPCODE_SET_EXTERNAL_TIMER:
    return "set external timer";
  case CEC_OPCODE_SET_TIMER_PROGRAM_TITLE:
    return "set timer program title";
  case CEC_OPCODE_TIMER_CLEARED_STATUS:
    return "timer cleared status";
  case CEC_OPCODE_TIMER_STATUS:
    return "timer status";
  case CEC_OPCODE_CEC_VERSION:
    return "cec version";
  case CEC_OPCODE_GET_CEC_VERSION:
    return "get cec version";
  case CEC_OPCODE_GIVE_PHYSICAL_ADDRESS:
    return "give physical address";
  case CEC_OPCODE_GET_MENU_LANGUAGE:
    return "get menu language";
  case CEC_OPCODE_REPORT_PHYSICAL_ADDRESS:
    return "report physical address";
  case CEC_OPCODE_SET_MENU_LANGUAGE:
    return "set menu language";
  case CEC_OPCODE_DECK_CONTROL:
    return "deck control";
  case CEC_OPCODE_DECK_STATUS:
    return "deck status";
  case CEC_OPCODE_GIVE_DECK_STATUS:
    return "give deck status";
  case CEC_OPCODE_PLAY:
    return "play";
  case CEC_OPCODE_GIVE_TUNER_DEVICE_STATUS:
    return "give tuner status";
  case CEC_OPCODE_SELECT_ANALOGUE_SERVICE:
    return "select analogue service";
  case CEC_OPCODE_SELECT_DIGITAL_SERVICE:
    return "set digital service";
  case CEC_OPCODE_TUNER_DEVICE_STATUS:
    return "tuner device status";
  case CEC_OPCODE_TUNER_STEP_DECREMENT:
    return "tuner step decrement";
  case CEC_OPCODE_TUNER_STEP_INCREMENT:
    return "tuner step increment";
  case CEC_OPCODE_DEVICE_VENDOR_ID:
    return "device vendor id";
  case CEC_OPCODE_GIVE_DEVICE_VENDOR_ID:
    return "give device vendor id";
  case CEC_OPCODE_VENDOR_COMMAND:
    return "vendor command";
  case CEC_OPCODE_VENDOR_COMMAND_WITH_ID:
    return "vendor command with id";
  case CEC_OPCODE_VENDOR_REMOTE_BUTTON_DOWN:
    return "vendor remote button down";
  case CEC_OPCODE_VENDOR_REMOTE_BUTTON_UP:
    return "vendor remote button up";
  case CEC_OPCODE_SET_OSD_STRING:
    return "set osd string";
  case CEC_OPCODE_GIVE_OSD_NAME:
    return "give osd name";
  case CEC_OPCODE_SET_OSD_NAME:
    return "set osd name";
  case CEC_OPCODE_MENU_REQUEST:
    return "menu request";
  case CEC_OPCODE_MENU_STATUS:
    return "menu status";
  case CEC_OPCODE_USER_CONTROL_PRESSED:
    return "user control pressed";
  case CEC_OPCODE_USER_CONTROL_RELEASE:
    return "user control release";
  case CEC_OPCODE_GIVE_DEVICE_POWER_STATUS:
    return "give device power status";
  case CEC_OPCODE_REPORT_POWER_STATUS:
    return "report power status";
  case CEC_OPCODE_FEATURE_ABORT:
    return "feature abort";
  case CEC_OPCODE_ABORT:
    return "abort";
  case CEC_OPCODE_GIVE_AUDIO_STATUS:
    return "give audio status";
  case CEC_OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS:
    return "give audio mode status";
  case CEC_OPCODE_REPORT_AUDIO_STATUS:
    return "report audio status";
  case CEC_OPCODE_SET_SYSTEM_AUDIO_MODE:
    return "set system audio mode";
  case CEC_OPCODE_SYSTEM_AUDIO_MODE_REQUEST:
    return "system audio mode request";
  case CEC_OPCODE_SYSTEM_AUDIO_MODE_STATUS:
    return "system audio mode status";
  case CEC_OPCODE_SET_AUDIO_RATE:
    return "set audio rate";
  default:
    return "UNKNOWN";
  }
}

const char *CCECProcessor::ToString(const cec_system_audio_status mode)
{
  switch(mode)
  {
  case CEC_SYSTEM_AUDIO_STATUS_ON:
    return "on";
  case CEC_SYSTEM_AUDIO_STATUS_OFF:
    return "off";
  default:
    return "unknown";
  }
}

const char *CCECProcessor::ToString(const cec_audio_status status)
{
  // TODO this is a mask
  return "TODO";
}

const char *CCECProcessor::ToString(const cec_vendor_id vendor)
{
  switch (vendor)
  {
  case CEC_VENDOR_SAMSUNG:
    return "Samsung";
  case CEC_VENDOR_LG:
    return "LG";
  case CEC_VENDOR_PANASONIC:
    return "Panasonic";
  case CEC_VENDOR_PIONEER:
    return "Pioneer";
  case CEC_VENDOR_ONKYO:
    return "Onkyo";
  case CEC_VENDOR_YAMAHA:
    return "Yamaha";
  case CEC_VENDOR_PHILIPS:
    return "Philips";
  default:
    return "Unknown";
  }
}

void *CCECBusScan::Process(void)
{
  CCECBusDevice *device(NULL);
  while (!IsStopped())
  {
    for (unsigned int iPtr = 0; iPtr < 15 && !IsStopped(); iPtr++)
    {
      device = m_processor->m_busDevices[iPtr];
      if (device && device->GetStatus() == CEC_DEVICE_STATUS_PRESENT)
      {
        if (!IsStopped())
          device->GetVendorId();
        Sleep(5);
      }
    }
    Sleep(5000);
  }
  return NULL;
}

bool CCECProcessor::StartBootloader(void)
{
  return m_communication->StartBootloader();
}

bool CCECProcessor::PingAdapter(void)
{
  return m_communication->PingAdapter();
}
