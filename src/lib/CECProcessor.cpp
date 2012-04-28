/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2012 Pulse-Eight Limited.  All rights reserved.
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

#include "adapter/USBCECAdapterCommunication.h"
#include "devices/CECBusDevice.h"
#include "devices/CECAudioSystem.h"
#include "devices/CECPlaybackDevice.h"
#include "devices/CECRecordingDevice.h"
#include "devices/CECTuner.h"
#include "devices/CECTV.h"
#include "implementations/CECCommandHandler.h"
#include "LibCEC.h"
#include "platform/util/timeutils.h"

using namespace CEC;
using namespace std;
using namespace PLATFORM;

#define CEC_PROCESSOR_SIGNAL_WAIT_TIME 1000

CCECProcessor::CCECProcessor(CLibCEC *controller, libcec_configuration *configuration) :
    m_bConnectionOpened(false),
    m_bInitialised(false),
    m_communication(NULL),
    m_controller(controller),
    m_bMonitor(false),
    m_iStandardLineTimeout(3),
    m_iRetryLineTimeout(3),
    m_iLastTransmission(0)
{
  CreateBusDevices();
  m_configuration.Clear();
  m_configuration.serverVersion = LIBCEC_VERSION_CURRENT;
  SetConfiguration(configuration);

  if (m_configuration.tvVendor != CEC_VENDOR_UNKNOWN)
    m_busDevices[CECDEVICE_TV]->ReplaceHandler(false);
}

CCECProcessor::CCECProcessor(CLibCEC *controller, const char *strDeviceName, const cec_device_type_list &types, uint16_t iPhysicalAddress) :
    m_bConnectionOpened(false),
    m_bInitialised(false),
    m_communication(NULL),
    m_controller(controller),
    m_bMonitor(false),
    m_iStandardLineTimeout(3),
    m_iRetryLineTimeout(3),
    m_iLastTransmission(0)
{
  m_configuration.Clear();
  m_configuration.serverVersion    = LIBCEC_VERSION_CURRENT;

  // client version < 1.5.0
  m_configuration.clientVersion    = (uint32_t)CEC_CLIENT_VERSION_PRE_1_5;
  snprintf(m_configuration.strDeviceName, 13, "%s", strDeviceName);
  m_configuration.deviceTypes      = types;
  m_configuration.iPhysicalAddress = iPhysicalAddress;
  m_configuration.baseDevice       = (cec_logical_address)CEC_DEFAULT_BASE_DEVICE;
  m_configuration.iHDMIPort        = CEC_DEFAULT_HDMI_PORT;

  if (m_configuration.deviceTypes.IsEmpty())
    m_configuration.deviceTypes.Add(CEC_DEVICE_TYPE_RECORDING_DEVICE);
  CreateBusDevices();
}

void CCECProcessor::CreateBusDevices(void)
{
  for (uint8_t iPtr = CECDEVICE_TV; iPtr <= CECDEVICE_BROADCAST; iPtr++)
  {
    switch(iPtr)
    {
    case CECDEVICE_AUDIOSYSTEM:
      m_busDevices[iPtr] = new CCECAudioSystem(this, (cec_logical_address) iPtr);
      break;
    case CECDEVICE_PLAYBACKDEVICE1:
    case CECDEVICE_PLAYBACKDEVICE2:
    case CECDEVICE_PLAYBACKDEVICE3:
      m_busDevices[iPtr] = new CCECPlaybackDevice(this, (cec_logical_address) iPtr);
      break;
    case CECDEVICE_RECORDINGDEVICE1:
    case CECDEVICE_RECORDINGDEVICE2:
    case CECDEVICE_RECORDINGDEVICE3:
      m_busDevices[iPtr] = new CCECRecordingDevice(this, (cec_logical_address) iPtr);
      break;
    case CECDEVICE_TUNER1:
    case CECDEVICE_TUNER2:
    case CECDEVICE_TUNER3:
    case CECDEVICE_TUNER4:
      m_busDevices[iPtr] = new CCECTuner(this, (cec_logical_address) iPtr);
      break;
    case CECDEVICE_TV:
      m_busDevices[iPtr] = new CCECTV(this, (cec_logical_address) iPtr);
      break;
    default:
      m_busDevices[iPtr] = new CCECBusDevice(this, (cec_logical_address) iPtr);
      break;
    }
  }
}

CCECProcessor::~CCECProcessor(void)
{
  Close();

  for (uint8_t iPtr = CECDEVICE_TV; iPtr <= CECDEVICE_BROADCAST; iPtr++)
  {
    delete m_busDevices[iPtr];
    m_busDevices[iPtr] = NULL;
  }
}

void CCECProcessor::Close(void)
{
  StopThread(false);
  SetInitialised(false);
  StopThread();

  bool bClose(false);
  {
    CLockObject lock(m_mutex);
    bClose = m_bConnectionOpened;
    m_bConnectionOpened = false;
  }

  if (bClose && m_communication)
  {
    delete m_communication;
    m_communication = NULL;
  }
}

bool CCECProcessor::OpenConnection(const char *strPort, uint16_t iBaudRate, uint32_t iTimeoutMs, bool bStartListening /* = true */)
{
  bool bReturn(false);
  Close();

  {
    CLockObject lock(m_mutex);
    if (m_bConnectionOpened)
    {
      CLibCEC::AddLog(CEC_LOG_ERROR, "connection already opened");
      return false;
    }
    m_communication = new CUSBCECAdapterCommunication(this, strPort, iBaudRate);
    m_bConnectionOpened = (m_communication != NULL);
  }

  CTimeout timeout(iTimeoutMs > 0 ? iTimeoutMs : CEC_DEFAULT_TRANSMIT_WAIT);

  /* open a new connection */
  unsigned iConnectTry(0);
  while (timeout.TimeLeft() > 0 && (bReturn = m_communication->Open((timeout.TimeLeft() / CEC_CONNECT_TRIES), false, bStartListening)) == false)
  {
    CLibCEC::AddLog(CEC_LOG_ERROR, "could not open a connection (try %d)", ++iConnectTry);
    m_communication->Close();
    CEvent::Sleep(CEC_DEFAULT_CONNECT_RETRY_WAIT);
  }

  if (bReturn)
  {
    m_configuration.iFirmwareVersion = m_communication->GetFirmwareVersion();
    m_configuration.iFirmwareBuildDate = m_communication->GetFirmwareBuildDate();
    CStdString strLog;
    strLog.Format("connected to the CEC adapter. libCEC version = %s, client version = %s, firmware version = %d", ToString((cec_server_version)m_configuration.serverVersion), ToString((cec_client_version)m_configuration.clientVersion), m_configuration.iFirmwareVersion);
    if (m_configuration.iFirmwareBuildDate != CEC_FW_BUILD_UNKNOWN)
    {
      time_t buildTime = (time_t)m_configuration.iFirmwareBuildDate;
      strLog.AppendFormat(", firmware build date: %s", asctime(gmtime(&buildTime)));
      strLog = strLog.Left((int)strLog.length() - 1); // strip \n added by asctime
      strLog.append(" +0000");
    }
    CLibCEC::AddLog(CEC_LOG_NOTICE, strLog);
  }

  if (m_configuration.bGetSettingsFromROM == 1)
  {
    libcec_configuration config;
    config.Clear();
    m_communication->GetConfiguration(&config);

    CLockObject lock(m_mutex);
    if (!config.deviceTypes.IsEmpty())
      m_configuration.deviceTypes = config.deviceTypes;
    if (IsValidPhysicalAddress(config.iPhysicalAddress))
      m_configuration.iPhysicalAddress = config.iPhysicalAddress;
    snprintf(m_configuration.strDeviceName, 13, "%s", config.strDeviceName);
  }

  return bReturn;
}

bool CCECProcessor::IsInitialised(void)
{
  CLockObject lock(m_threadMutex);
  return m_bInitialised;
}

void CCECProcessor::SetInitialised(bool bSetTo /* = true */)
{
  CLockObject lock(m_mutex);
  m_bInitialised = bSetTo;
}

bool CCECProcessor::Initialise(void)
{
  bool bReturn(false);
  {
    CLockObject lock(m_mutex);
    if (!m_configuration.logicalAddresses.IsEmpty())
      m_configuration.logicalAddresses.Clear();

    if (!FindLogicalAddresses())
    {
      CLibCEC::AddLog(CEC_LOG_ERROR, "could not detect our logical addresses");
      return bReturn;
    }

    /* only set our OSD name for the primary device */
    m_busDevices[m_configuration.logicalAddresses.primary]->m_strDeviceName = m_configuration.strDeviceName;

    /* make the primary device the active source if the option is set */
    if (m_configuration.bActivateSource == 1)
      m_busDevices[m_configuration.logicalAddresses.primary]->m_bActiveSource = true;

    /* set the default menu language for devices we control */
    cec_menu_language language;
    language.device = m_configuration.logicalAddresses.primary;
    memcpy(language.language, m_configuration.strDeviceLanguage, 3);
    language.language[3] = 0;

    for (uint8_t iPtr = CECDEVICE_TV; iPtr < CECDEVICE_BROADCAST; iPtr++)
    {
      if (m_configuration.logicalAddresses[iPtr])
      {
        language.device = (cec_logical_address) iPtr;
        m_busDevices[iPtr]->SetMenuLanguage(language);
      }
    }
  }

  /* get the vendor id from the TV, so we are using the correct handler */
  m_busDevices[CECDEVICE_TV]->GetVendorId();

  if (IsValidPhysicalAddress(m_configuration.iPhysicalAddress))
  {
    CLibCEC::AddLog(CEC_LOG_NOTICE, "setting the physical address to %04X", m_configuration.iPhysicalAddress);
    m_busDevices[m_configuration.logicalAddresses.primary]->m_iPhysicalAddress = m_configuration.iPhysicalAddress;
    if ((bReturn = m_busDevices[m_configuration.logicalAddresses.primary]->TransmitPhysicalAddress()) == false)
      CLibCEC::AddLog(CEC_LOG_ERROR, "unable to set the physical address to %04X", m_configuration.iPhysicalAddress);
  }
  else
  {
    if (!SetHDMIPort(m_configuration.baseDevice, m_configuration.iHDMIPort, true))
      CLibCEC::AddLog(CEC_LOG_ERROR, "unable to set HDMI port %d on %s (%x)", m_configuration.iHDMIPort, ToString(m_configuration.baseDevice), (uint8_t)m_configuration.baseDevice);
    bReturn = true;
  }

  if (bReturn && m_configuration.bActivateSource == 1)
    m_busDevices[m_configuration.logicalAddresses.primary]->ActivateSource();

  SetInitialised(bReturn);
  if (bReturn)
    CLibCEC::ConfigurationChanged(m_configuration);

  return bReturn;
}

bool CCECProcessor::Start(const char *strPort, uint16_t iBaudRate /* = CEC_SERIAL_DEFAULT_BAUDRATE */, uint32_t iTimeoutMs /* = CEC_DEFAULT_CONNECT_TIMEOUT */)
{
  bool bReturn(false);

  {
    CLockObject lock(m_mutex);
    if (!OpenConnection(strPort, iBaudRate, iTimeoutMs))
      return bReturn;

    /* create the processor thread */
    if (!CreateThread())
    {
      CLibCEC::AddLog(CEC_LOG_ERROR, "could not create a processor thread");
      return bReturn;
    }
  }

  if ((bReturn = Initialise()) == false)
  {
    CLibCEC::AddLog(CEC_LOG_ERROR, "could not create a processor thread");
    StopThread(true);
  }
  else
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "processor thread started");
  }

  return bReturn;
}

bool CCECProcessor::TryLogicalAddress(cec_logical_address address)
{
  if (m_busDevices[address]->TryLogicalAddress())
  {
    m_configuration.logicalAddresses.Set(address);
    return true;
  }

  return false;
}

bool CCECProcessor::FindLogicalAddressRecordingDevice(void)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "detecting logical address for type 'recording device'");
  return TryLogicalAddress(CECDEVICE_RECORDINGDEVICE1) ||
      TryLogicalAddress(CECDEVICE_RECORDINGDEVICE2) ||
      TryLogicalAddress(CECDEVICE_RECORDINGDEVICE3);
}

bool CCECProcessor::FindLogicalAddressTuner(void)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "detecting logical address for type 'tuner'");
  return TryLogicalAddress(CECDEVICE_TUNER1) ||
      TryLogicalAddress(CECDEVICE_TUNER2) ||
      TryLogicalAddress(CECDEVICE_TUNER3) ||
      TryLogicalAddress(CECDEVICE_TUNER4);
}

bool CCECProcessor::FindLogicalAddressPlaybackDevice(void)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "detecting logical address for type 'playback device'");
  return TryLogicalAddress(CECDEVICE_PLAYBACKDEVICE1) ||
      TryLogicalAddress(CECDEVICE_PLAYBACKDEVICE2) ||
      TryLogicalAddress(CECDEVICE_PLAYBACKDEVICE3);
}

bool CCECProcessor::FindLogicalAddressAudioSystem(void)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "detecting logical address for type 'audio'");
  return TryLogicalAddress(CECDEVICE_AUDIOSYSTEM);
}

bool CCECProcessor::ChangeDeviceType(cec_device_type from, cec_device_type to)
{
  bool bChanged(false);

  CLibCEC::AddLog(CEC_LOG_NOTICE, "changing device type '%s' into '%s'", ToString(from), ToString(to));

  CLockObject lock(m_mutex);
  CCECBusDevice *previousDevice = GetDeviceByType(from);
  m_configuration.logicalAddresses.primary = CECDEVICE_UNKNOWN;

  for (uint8_t iPtr = 0; iPtr < 5; iPtr++)
  {
    if (m_configuration.deviceTypes.types[iPtr] == CEC_DEVICE_TYPE_RESERVED)
      continue;

    if (m_configuration.deviceTypes.types[iPtr] == from)
    {
      bChanged = true;
      m_configuration.deviceTypes.types[iPtr] = to;
    }
    else if (m_configuration.deviceTypes.types[iPtr] == to && bChanged)
    {
      m_configuration.deviceTypes.types[iPtr] = CEC_DEVICE_TYPE_RESERVED;
    }
  }

  if (bChanged)
  {
    FindLogicalAddresses();

    CCECBusDevice *newDevice = GetDeviceByType(to);
    if (previousDevice && newDevice)
    {
      newDevice->SetDeviceStatus(CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC);
      previousDevice->SetDeviceStatus(CEC_DEVICE_STATUS_NOT_PRESENT);

      newDevice->SetCecVersion(previousDevice->GetCecVersion(false));
      previousDevice->SetCecVersion(CEC_VERSION_UNKNOWN);

      newDevice->SetMenuLanguage(previousDevice->GetMenuLanguage(false));

      newDevice->SetMenuState(previousDevice->GetMenuState());
      previousDevice->SetMenuState(CEC_MENU_STATE_DEACTIVATED);

      newDevice->SetOSDName(previousDevice->GetOSDName(false));
      previousDevice->SetOSDName(ToString(previousDevice->GetLogicalAddress()));

      newDevice->SetPhysicalAddress(previousDevice->GetPhysicalAddress());
      previousDevice->SetPhysicalAddress(CEC_INVALID_PHYSICAL_ADDRESS);

      newDevice->SetPowerStatus(previousDevice->GetPowerStatus(false));
      previousDevice->SetPowerStatus(CEC_POWER_STATUS_UNKNOWN);

      newDevice->SetVendorId(previousDevice->GetVendorId(false));
      previousDevice->SetVendorId(CEC_VENDOR_UNKNOWN);

      if ((from == CEC_DEVICE_TYPE_PLAYBACK_DEVICE || from == CEC_DEVICE_TYPE_RECORDING_DEVICE) &&
          (to == CEC_DEVICE_TYPE_PLAYBACK_DEVICE || to == CEC_DEVICE_TYPE_RECORDING_DEVICE))
      {
        ((CCECPlaybackDevice *) newDevice)->SetDeckControlMode(((CCECPlaybackDevice *) previousDevice)->GetDeckControlMode());
        ((CCECPlaybackDevice *) previousDevice)->SetDeckControlMode(CEC_DECK_CONTROL_MODE_STOP);

        ((CCECPlaybackDevice *) newDevice)->SetDeckStatus(((CCECPlaybackDevice *) previousDevice)->GetDeckStatus());
        ((CCECPlaybackDevice *) previousDevice)->SetDeckStatus(CEC_DECK_INFO_STOP);
      }
    }
  }

  return true;
}

bool CCECProcessor::FindLogicalAddresses(void)
{
  bool bReturn(true);
  m_configuration.logicalAddresses.Clear();

  if (m_configuration.deviceTypes.IsEmpty())
  {
    CLibCEC::AddLog(CEC_LOG_ERROR, "no device types set");
    return false;
  }

  for (uint8_t iPtr = 0; iPtr < 5; iPtr++)
  {
    if (m_configuration.deviceTypes.types[iPtr] == CEC_DEVICE_TYPE_RESERVED)
      continue;

    CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - device %d: type %d", __FUNCTION__, iPtr, m_configuration.deviceTypes.types[iPtr]);

    if (m_configuration.deviceTypes.types[iPtr] == CEC_DEVICE_TYPE_RECORDING_DEVICE)
      bReturn &= FindLogicalAddressRecordingDevice();
    if (m_configuration.deviceTypes.types[iPtr] == CEC_DEVICE_TYPE_TUNER)
      bReturn &= FindLogicalAddressTuner();
    if (m_configuration.deviceTypes.types[iPtr] == CEC_DEVICE_TYPE_PLAYBACK_DEVICE)
      bReturn &= FindLogicalAddressPlaybackDevice();
    if (m_configuration.deviceTypes.types[iPtr] == CEC_DEVICE_TYPE_AUDIO_SYSTEM)
      bReturn &= FindLogicalAddressAudioSystem();
  }

  if (bReturn)
    SetAckMask(m_configuration.logicalAddresses.AckMask());

  return bReturn;
}

void CCECProcessor::ReplaceHandlers(void)
{
  if (!IsInitialised())
    return;
  for (uint8_t iPtr = CECDEVICE_TV; iPtr <= CECDEVICE_PLAYBACKDEVICE3; iPtr++)
    m_busDevices[iPtr]->ReplaceHandler(m_bInitialised);
}

bool CCECProcessor::OnCommandReceived(const cec_command &command)
{
  return m_inBuffer.Push(command);
}

void *CCECProcessor::Process(void)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "processor thread started");

  cec_command command;
  command.Clear();

  while (!IsStopped() && m_communication->IsOpen())
  {
    if (m_inBuffer.Pop(command, CEC_PROCESSOR_SIGNAL_WAIT_TIME))
      ParseCommand(command);

    if (IsInitialised())
    {
      ReplaceHandlers();

      m_controller->CheckKeypressTimeout();
    }
  }

  return NULL;
}

bool CCECProcessor::SetActiveSource(cec_device_type type /* = CEC_DEVICE_TYPE_RESERVED */)
{
  bool bReturn(false);

  if (!IsRunning())
    return bReturn;

  cec_logical_address addr = m_configuration.logicalAddresses.primary;

  if (type != CEC_DEVICE_TYPE_RESERVED)
  {
    for (uint8_t iPtr = CECDEVICE_TV; iPtr <= CECDEVICE_PLAYBACKDEVICE3; iPtr++)
    {
      if (m_configuration.logicalAddresses[iPtr] && m_busDevices[iPtr]->m_type == type)
      {
        addr = (cec_logical_address) iPtr;
        break;
      }
    }
  }

  m_busDevices[addr]->SetActiveSource();
  if (IsValidPhysicalAddress(m_busDevices[addr]->GetPhysicalAddress()))
    bReturn = m_busDevices[addr]->ActivateSource();

  return bReturn;
}

bool CCECProcessor::SetActiveSource(uint16_t iStreamPath)
{
  bool bReturn(false);

  // suppress polls when searching for a device
  CCECBusDevice *device = GetDeviceByPhysicalAddress(iStreamPath);
  if (device)
  {
    device->SetActiveSource();
    bReturn = true;
  }
  else
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "device with PA '%04x' not found", iStreamPath);
  }

  return bReturn;
}

void CCECProcessor::SetStandardLineTimeout(uint8_t iTimeout)
{
  CLockObject lock(m_mutex);
  m_iStandardLineTimeout = iTimeout;
}

void CCECProcessor::SetRetryLineTimeout(uint8_t iTimeout)
{
  CLockObject lock(m_mutex);
  m_iRetryLineTimeout = iTimeout;
}

bool CCECProcessor::SetActiveView(void)
{
  CLibCEC::AddLog(CEC_LOG_WARNING, "deprecated method %s called", __FUNCTION__);
  return SetActiveSource(m_configuration.deviceTypes.IsEmpty() ? CEC_DEVICE_TYPE_RESERVED : m_configuration.deviceTypes[0]);
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

  // limit the HDMI port range to 1-15
  if (iPort < CEC_MIN_HDMI_PORTNUMBER ||
      iPort > CEC_MAX_HDMI_PORTNUMBER)
    return bReturn;

  {
    CLockObject lock(m_mutex);
    m_configuration.baseDevice = iBaseDevice;
    m_configuration.iHDMIPort  = iPort;
  }

  if (!IsRunning() && !bForce)
    return true;

  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting HDMI port to %d on device %s (%d)", iPort, ToString(iBaseDevice), (int)iBaseDevice);

  uint16_t iPhysicalAddress(0);
  if (iBaseDevice > CECDEVICE_TV)
    iPhysicalAddress = m_busDevices[iBaseDevice]->GetPhysicalAddress(false);

  if (iPhysicalAddress <= CEC_MAX_PHYSICAL_ADDRESS)
  {
    if (iPhysicalAddress == 0)
      iPhysicalAddress += 0x1000 * iPort;
    else if (iPhysicalAddress % 0x1000 == 0)
      iPhysicalAddress += 0x100 * iPort;
    else if (iPhysicalAddress % 0x100 == 0)
      iPhysicalAddress += 0x10 * iPort;
    else if (iPhysicalAddress % 0x10 == 0)
      iPhysicalAddress += iPort;

    bReturn = true;
  }

  if (!bReturn)
  {
    CLibCEC::AddLog(CEC_LOG_WARNING, "failed to set the physical address to %04X, setting it to the default value %04X", iPhysicalAddress, CEC_DEFAULT_PHYSICAL_ADDRESS);
    iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS;
  }

  SetPhysicalAddress(iPhysicalAddress);

  return bReturn;
}

bool CCECProcessor::PhysicalAddressInUse(uint16_t iPhysicalAddress)
{
  for (uint8_t iPtr = CECDEVICE_TV; iPtr < CECDEVICE_BROADCAST; iPtr++)
  {
    if (m_busDevices[iPtr]->GetPhysicalAddress() == iPhysicalAddress)
      return true;
  }
  return false;
}

bool CCECProcessor::TransmitInactiveSource(void)
{
  if (!IsRunning())
    return false;

  if (!m_configuration.logicalAddresses.IsEmpty() && m_busDevices[m_configuration.logicalAddresses.primary])
    return m_busDevices[m_configuration.logicalAddresses.primary]->TransmitInactiveSource();
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
  CLibCEC::AddLog(CEC_LOG_TRAFFIC, strTx.c_str());
}

bool CCECProcessor::SetLogicalAddress(cec_logical_address iLogicalAddress)
{
  CLockObject lock(m_mutex);
  if (m_configuration.logicalAddresses.primary != iLogicalAddress)
  {
    CLibCEC::AddLog(CEC_LOG_NOTICE, "<< setting primary logical address to %1x", iLogicalAddress);
    m_configuration.logicalAddresses.primary = iLogicalAddress;
    m_configuration.logicalAddresses.Set(iLogicalAddress);
    return SetAckMask(m_configuration.logicalAddresses.AckMask());
  }

  return true;
}

bool CCECProcessor::SetMenuState(cec_menu_state state, bool bSendUpdate /* = true */)
{
  for (uint8_t iPtr = CECDEVICE_TV; iPtr < CECDEVICE_BROADCAST; iPtr++)
  {
    if (m_configuration.logicalAddresses[iPtr])
      m_busDevices[iPtr]->SetMenuState(state);
  }

  if (bSendUpdate)
    m_busDevices[m_configuration.logicalAddresses.primary]->TransmitMenuState(CECDEVICE_TV);

  return true;
}

bool CCECProcessor::SetPhysicalAddress(uint16_t iPhysicalAddress, bool bSendUpdate /* = true */)
{
  bool bSendActiveView(false);
  bool bReturn(false);
  cec_logical_addresses sendUpdatesTo;
  sendUpdatesTo.Clear();

  {
    CLockObject lock(m_mutex);
    m_configuration.iPhysicalAddress = iPhysicalAddress;
    CLibCEC::AddLog(CEC_LOG_DEBUG, "setting physical address to '%04X'", iPhysicalAddress);

    if (!m_configuration.logicalAddresses.IsEmpty())
    {
      bool bWasActiveSource(false);
      for (uint8_t iPtr = CECDEVICE_TV; iPtr < CECDEVICE_BROADCAST; iPtr++)
        if (m_configuration.logicalAddresses[iPtr])
        {
          bWasActiveSource |= m_busDevices[iPtr]->IsActiveSource();
          m_busDevices[iPtr]->SetInactiveSource();
          m_busDevices[iPtr]->SetPhysicalAddress(iPhysicalAddress);
          if (bSendUpdate)
            sendUpdatesTo.Set((cec_logical_address)iPtr);
        }

      bSendActiveView = bWasActiveSource && bSendUpdate;
      bReturn = true;
    }
  }

  for (uint8_t iPtr = CECDEVICE_TV; iPtr < CECDEVICE_BROADCAST; iPtr++)
    if (sendUpdatesTo[iPtr])
      m_busDevices[iPtr]->TransmitPhysicalAddress();

  if (bSendActiveView)
    SetActiveView();

  if (bReturn)
  {
    libcec_configuration config;
    {
      CLockObject lock(m_mutex);
      config = m_configuration;
    }

    PersistConfiguration(&config);
    CLibCEC::ConfigurationChanged(config);
  }

  return bReturn;
}

bool CCECProcessor::SwitchMonitoring(bool bEnable)
{
  CLibCEC::AddLog(CEC_LOG_NOTICE, "== %s monitoring mode ==", bEnable ? "enabling" : "disabling");

  {
    CLockObject lock(m_mutex);
    m_bMonitor = bEnable;
  }

  if (bEnable)
    return SetAckMask(0);
  else
    return SetAckMask(m_configuration.logicalAddresses.AckMask());
}

bool CCECProcessor::PollDevice(cec_logical_address iAddress)
{
  if (iAddress != CECDEVICE_UNKNOWN && m_busDevices[iAddress])
  {
    return m_configuration.logicalAddresses.primary == CECDEVICE_UNKNOWN ?
        m_busDevices[iAddress]->TransmitPoll(iAddress) :
        m_busDevices[m_configuration.logicalAddresses.primary]->TransmitPoll(iAddress);
  }
  return false;
}

uint8_t CCECProcessor::VolumeUp(bool bSendRelease /* = true */)
{
  uint8_t status(CEC_AUDIO_VOLUME_STATUS_UNKNOWN);
  if (IsPresentDevice(CECDEVICE_AUDIOSYSTEM))
    status = ((CCECAudioSystem *)m_busDevices[CECDEVICE_AUDIOSYSTEM])->VolumeUp(bSendRelease);

  return status;
}

uint8_t CCECProcessor::VolumeDown(bool bSendRelease /* = true */)
{
  uint8_t status(CEC_AUDIO_VOLUME_STATUS_UNKNOWN);
  if (IsPresentDevice(CECDEVICE_AUDIOSYSTEM))
    status = ((CCECAudioSystem *)m_busDevices[CECDEVICE_AUDIOSYSTEM])->VolumeDown(bSendRelease);

  return status;
}

uint8_t CCECProcessor::MuteAudio(bool bSendRelease /* = true */)
{
  uint8_t status(CEC_AUDIO_VOLUME_STATUS_UNKNOWN);
  if (IsPresentDevice(CECDEVICE_AUDIOSYSTEM))
    status = ((CCECAudioSystem *)m_busDevices[CECDEVICE_AUDIOSYSTEM])->MuteAudio(bSendRelease);

  return status;
}

CCECBusDevice *CCECProcessor::GetDeviceByPhysicalAddress(uint16_t iPhysicalAddress, bool bSuppressUpdate /* = true */)
{
  CCECBusDevice *device(NULL);

  // check each device until we found a match
  for (uint8_t iPtr = CECDEVICE_TV; !device && iPtr < CECDEVICE_BROADCAST; iPtr++)
  {
    if (m_busDevices[iPtr]->GetPhysicalAddress(bSuppressUpdate) == iPhysicalAddress)
      device = m_busDevices[iPtr];
  }

  return device;
}

CCECBusDevice *CCECProcessor::GetDeviceByType(cec_device_type type) const
{
  CCECBusDevice *device = NULL;

  for (uint8_t iPtr = CECDEVICE_TV; iPtr < CECDEVICE_BROADCAST; iPtr++)
  {
    if (m_busDevices[iPtr]->m_type == type && m_configuration.logicalAddresses[iPtr])
    {
      device = m_busDevices[iPtr];
      break;
    }
  }

  return device;
}

CCECBusDevice *CCECProcessor::GetPrimaryDevice(void) const
{
  CCECBusDevice *device(NULL);
  cec_logical_address primary = m_configuration.logicalAddresses.primary;
  if (primary != CECDEVICE_UNKNOWN)
    device = m_busDevices[primary];
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

CStdString CCECProcessor::GetDeviceName(void) const
{
  CStdString strName;
  strName = m_configuration.strDeviceName;
  return strName;
}

uint64_t CCECProcessor::GetDeviceVendorId(cec_logical_address iAddress)
{
  if (m_busDevices[iAddress])
    return m_busDevices[iAddress]->GetVendorId();
  return false;
}

uint16_t CCECProcessor::GetDevicePhysicalAddress(cec_logical_address iAddress)
{
  if (m_busDevices[iAddress])
    return m_busDevices[iAddress]->GetPhysicalAddress(false);
  return false;
}

cec_power_status CCECProcessor::GetDevicePowerStatus(cec_logical_address iAddress)
{
  if (m_busDevices[iAddress])
    return m_busDevices[iAddress]->GetPowerStatus();
  return CEC_POWER_STATUS_UNKNOWN;
}

cec_logical_address CCECProcessor::GetActiveSource(bool bRequestActiveSource /* = true */)
{
  for (uint8_t iPtr = CECDEVICE_TV; iPtr <= CECDEVICE_PLAYBACKDEVICE3; iPtr++)
  {
    if (m_busDevices[iPtr]->IsActiveSource())
      return (cec_logical_address)iPtr;
  }

  if (bRequestActiveSource && m_configuration.logicalAddresses.primary != CECDEVICE_UNKNOWN)
  {
    CCECBusDevice *primary = m_busDevices[m_configuration.logicalAddresses.primary];
    if (primary)
      primary->RequestActiveSource();

    return GetActiveSource(false);
  }

  return CECDEVICE_UNKNOWN;
}

bool CCECProcessor::IsActiveSource(cec_logical_address iAddress)
{
  return iAddress >= CECDEVICE_TV && iAddress < CECDEVICE_BROADCAST ?
    m_busDevices[iAddress]->IsActiveSource() :
    false;
}

bool CCECProcessor::Transmit(const cec_command &data)
{
  if (m_configuration.logicalAddresses[(uint8_t)data.destination])
  {
    CLibCEC::AddLog(CEC_LOG_WARNING, "not sending data to myself!");
    return false;
  }

  uint8_t iMaxTries(0);
  {
    CLockObject lock(m_mutex);
    if (IsStopped())
      return false;
    LogOutput(data);
    m_iLastTransmission = GetTimeMs();
    if (!m_communication || !m_communication->IsOpen())
    {
      CLibCEC::AddLog(CEC_LOG_ERROR, "cannot transmit command: connection closed");
      return false;
    }
    iMaxTries = m_busDevices[data.initiator]->GetHandler()->GetTransmitRetries() + 1;
  }

  bool bRetry(true);
  uint8_t iTries(0);
  uint8_t iLineTimeout = m_iStandardLineTimeout;
  cec_adapter_message_state adapterState = ADAPTER_MESSAGE_STATE_UNKNOWN;

  while (bRetry && ++iTries < iMaxTries)
  {
    if (m_busDevices[data.initiator]->IsUnsupportedFeature(data.opcode))
      return false;

    adapterState = m_communication->Write(data, bRetry, iLineTimeout);
    iLineTimeout = m_iRetryLineTimeout;
  }

  return adapterState == ADAPTER_MESSAGE_STATE_SENT_ACKED;
}

void CCECProcessor::TransmitAbort(cec_logical_address address, cec_opcode opcode, cec_abort_reason reason /* = CEC_ABORT_REASON_UNRECOGNIZED_OPCODE */)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "<< transmitting abort message");

  cec_command command;
  // TODO
  cec_command::Format(command, m_configuration.logicalAddresses.primary, address, CEC_OPCODE_FEATURE_ABORT);
  command.parameters.PushBack((uint8_t)opcode);
  command.parameters.PushBack((uint8_t)reason);

  Transmit(command);
}

void CCECProcessor::ParseCommand(const cec_command &command)
{
  CStdString dataStr;
  dataStr.Format(">> %1x%1x", command.initiator, command.destination);
  if (command.opcode_set == 1)
    dataStr.AppendFormat(":%02x", command.opcode);
  for (uint8_t iPtr = 0; iPtr < command.parameters.size; iPtr++)
    dataStr.AppendFormat(":%02x", (unsigned int)command.parameters[iPtr]);
  CLibCEC::AddLog(CEC_LOG_TRAFFIC, dataStr.c_str());

  if (!m_bMonitor && command.initiator >= CECDEVICE_TV && command.initiator <= CECDEVICE_BROADCAST)
    m_busDevices[(uint8_t)command.initiator]->HandleCommand(command);
}

cec_logical_addresses CCECProcessor::GetActiveDevices(void)
{
  cec_logical_addresses addresses;
  addresses.Clear();
  for (uint8_t iPtr = CECDEVICE_TV; iPtr < CECDEVICE_BROADCAST; iPtr++)
  {
    if (m_busDevices[iPtr]->GetStatus() == CEC_DEVICE_STATUS_PRESENT)
      addresses.Set((cec_logical_address) iPtr);
  }
  return addresses;
}

bool CCECProcessor::IsPresentDevice(cec_logical_address address)
{
  return m_busDevices[address]->GetStatus() == CEC_DEVICE_STATUS_PRESENT;
}

bool CCECProcessor::IsPresentDeviceType(cec_device_type type)
{
  for (uint8_t iPtr = CECDEVICE_TV; iPtr < CECDEVICE_BROADCAST; iPtr++)
  {
    if (m_busDevices[iPtr]->GetType() == type && m_busDevices[iPtr]->GetStatus() == CEC_DEVICE_STATUS_PRESENT)
      return true;
  }

  return false;
}

uint16_t CCECProcessor::GetPhysicalAddress(void) const
{
  if (!m_configuration.logicalAddresses.IsEmpty() && m_busDevices[m_configuration.logicalAddresses.primary])
    return m_busDevices[m_configuration.logicalAddresses.primary]->GetPhysicalAddress();
  return false;
}

bool CCECProcessor::SetAckMask(uint16_t iMask)
{
  return m_communication ? m_communication->SetAckMask(iMask) : false;
}

bool CCECProcessor::TransmitKeypress(cec_logical_address iDestination, cec_user_control_code key, bool bWait /* = true */)
{
  return m_busDevices[iDestination]->TransmitKeypress(key, bWait);
}

bool CCECProcessor::TransmitKeyRelease(cec_logical_address iDestination, bool bWait /* = true */)
{
  return m_busDevices[iDestination]->TransmitKeyRelease(bWait);
}

bool CCECProcessor::EnablePhysicalAddressDetection(void)
{
  CLibCEC::AddLog(CEC_LOG_WARNING, "deprecated method %s called", __FUNCTION__);
  uint16_t iPhysicalAddress = m_communication->GetPhysicalAddress();
  if (IsValidPhysicalAddress(iPhysicalAddress))
  {
    m_configuration.bAutodetectAddress = 1;
    m_configuration.iPhysicalAddress   = iPhysicalAddress;
    m_configuration.baseDevice         = CECDEVICE_UNKNOWN;
    m_configuration.iHDMIPort          = CEC_HDMI_PORTNUMBER_NONE;
    return SetPhysicalAddress(iPhysicalAddress);
  }
  return false;
}

bool CCECProcessor::StandbyDevices(cec_logical_address address /* = CECDEVICE_BROADCAST */)
{
  if (address == CECDEVICE_BROADCAST && m_configuration.clientVersion >= CEC_CLIENT_VERSION_1_5_0)
  {
    bool bReturn(true);
    for (uint8_t iPtr = CECDEVICE_TV; iPtr <= CECDEVICE_BROADCAST; iPtr++)
    {
      if (m_configuration.powerOffDevices[iPtr])
      {
        CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - putting '%s' in standby mode", __FUNCTION__, ToString((cec_logical_address)iPtr));
        bReturn &= m_busDevices[iPtr]->Standby();
      }
    }
    return bReturn;
  }

  return m_busDevices[address]->Standby();
}

bool CCECProcessor::PowerOnDevices(cec_logical_address address /* = CECDEVICE_BROADCAST */)
{
  if (address == CECDEVICE_BROADCAST && m_configuration.clientVersion >= CEC_CLIENT_VERSION_1_5_0)
  {
    bool bReturn(true);
    for (uint8_t iPtr = CECDEVICE_TV; iPtr <= CECDEVICE_BROADCAST; iPtr++)
    {
      if (m_configuration.wakeDevices[iPtr])
      {
        CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - powering on '%s'", __FUNCTION__, ToString((cec_logical_address)iPtr));
        bReturn &= m_busDevices[iPtr]->PowerOn();
      }
    }
    return bReturn;
  }

  return m_busDevices[address]->PowerOn();
}

const char *CCECProcessor::ToString(const cec_device_type type)
{
  switch (type)
  {
  case CEC_DEVICE_TYPE_AUDIO_SYSTEM:
    return "audio system";
  case CEC_DEVICE_TYPE_PLAYBACK_DEVICE:
    return "playback device";
  case CEC_DEVICE_TYPE_RECORDING_DEVICE:
      return "recording device";
  case CEC_DEVICE_TYPE_RESERVED:
      return "reserved";
  case CEC_DEVICE_TYPE_TUNER:
      return "tuner";
  case CEC_DEVICE_TYPE_TV:
      return "TV";
  default:
    return "unknown";
  }
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
  case CEC_DECK_INFO_OTHER_STATUS_LG:
    return "LG other";
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
  case CEC_OPCODE_NONE:
    return "poll";
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

const char *CCECProcessor::ToString(const cec_audio_status UNUSED(status))
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
  case CEC_VENDOR_SONY:
    return "Sony";
  case CEC_VENDOR_TOSHIBA:
    return "Toshiba";
  default:
    return "Unknown";
  }
}

const char *CCECProcessor::ToString(const cec_client_version version)
{
  switch (version)
  {
  case CEC_CLIENT_VERSION_PRE_1_5:
    return "pre-1.5";
  case CEC_CLIENT_VERSION_1_5_0:
    return "1.5.0";
  case CEC_CLIENT_VERSION_1_5_1:
    return "1.5.1";
  case CEC_CLIENT_VERSION_1_5_2:
    return "1.5.2";
  case CEC_CLIENT_VERSION_1_5_3:
    return "1.5.3";
  case CEC_CLIENT_VERSION_1_6_0:
    return "1.6.0";
  case CEC_CLIENT_VERSION_1_6_1:
    return "1.6.1";
  case CEC_CLIENT_VERSION_1_6_2:
    return "1.6.2";
  default:
    return "Unknown";
  }
}

const char *CCECProcessor::ToString(const cec_server_version version)
{
  switch (version)
  {
  case CEC_SERVER_VERSION_PRE_1_5:
    return "pre-1.5";
  case CEC_SERVER_VERSION_1_5_0:
    return "1.5.0";
  case CEC_SERVER_VERSION_1_5_1:
    return "1.5.1";
  case CEC_SERVER_VERSION_1_5_2:
    return "1.5.2";
  case CEC_SERVER_VERSION_1_5_3:
    return "1.5.3";
  case CEC_SERVER_VERSION_1_6_0:
    return "1.6.0";
  case CEC_SERVER_VERSION_1_6_1:
    return "1.6.1";
  case CEC_SERVER_VERSION_1_6_2:
    return "1.6.2";
  default:
    return "Unknown";
  }
}

bool CCECProcessor::StartBootloader(const char *strPort /* = NULL */)
{
  bool bReturn(false);
  if (!m_communication && strPort)
  {
    IAdapterCommunication *comm = new CUSBCECAdapterCommunication(this, strPort);
    CTimeout timeout(CEC_DEFAULT_CONNECT_TIMEOUT);
    int iConnectTry(0);
    while (timeout.TimeLeft() > 0 && (bReturn = comm->Open(timeout.TimeLeft() / CEC_CONNECT_TRIES, true)) == false)
    {
      CLibCEC::AddLog(CEC_LOG_ERROR, "could not open a connection (try %d)", ++iConnectTry);
      comm->Close();
      Sleep(CEC_DEFAULT_TRANSMIT_RETRY_WAIT);
    }
    if (comm->IsOpen())
    {
      bReturn = comm->StartBootloader();
      delete comm;
    }
    return bReturn;
  }
  else
  {
    m_communication->StartBootloader();
    Close();
    bReturn = true;
  }

  return bReturn;
}

bool CCECProcessor::PingAdapter(void)
{
  return m_communication->PingAdapter();
}

void CCECProcessor::HandlePoll(cec_logical_address initiator, cec_logical_address destination)
{
  if (destination < CECDEVICE_BROADCAST)
    m_busDevices[destination]->HandlePollFrom(initiator);
}

bool CCECProcessor::HandleReceiveFailed(cec_logical_address initiator)
{
  return !m_busDevices[initiator]->HandleReceiveFailed();
}

bool CCECProcessor::SetStreamPath(uint16_t iPhysicalAddress)
{
  // stream path changes are sent by the TV
  return m_busDevices[CECDEVICE_TV]->GetHandler()->TransmitSetStreamPath(iPhysicalAddress);
}

bool CCECProcessor::SetConfiguration(const libcec_configuration *configuration)
{
  bool bReinit(false);
  CCECBusDevice *primary = IsRunning() ? GetPrimaryDevice() : NULL;
  cec_device_type oldPrimaryType = primary ? primary->GetType() : CEC_DEVICE_TYPE_RECORDING_DEVICE;
  m_configuration.clientVersion  = configuration->clientVersion;
  CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - using client version '%s'", __FUNCTION__, ToString((cec_client_version)configuration->clientVersion));

  // client version 1.5.0

  // device types
  bool bDeviceTypeChanged = IsRunning () && m_configuration.deviceTypes != configuration->deviceTypes;
  m_configuration.deviceTypes = configuration->deviceTypes;
  if (bDeviceTypeChanged)
    CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - using primary device type '%s'", __FUNCTION__, ToString(configuration->deviceTypes[0]));

  bool bPhysicalAddressChanged(false);

  // autodetect address
  bool bPhysicalAutodetected(false);
  if (IsRunning() && configuration->bAutodetectAddress == 1)
  {
    uint16_t iPhysicalAddress = m_communication->GetPhysicalAddress();
    if (IsValidPhysicalAddress(iPhysicalAddress))
    {
      if (IsRunning())
        CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - autodetected physical address '%04X'", __FUNCTION__, iPhysicalAddress);
      else
        CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - using physical address '%04X'", __FUNCTION__, iPhysicalAddress);
      bPhysicalAddressChanged = (m_configuration.iPhysicalAddress != iPhysicalAddress);
      m_configuration.iPhysicalAddress = iPhysicalAddress;
      m_configuration.iHDMIPort        = CEC_HDMI_PORTNUMBER_NONE;
      m_configuration.baseDevice       = CECDEVICE_UNKNOWN;
      bPhysicalAutodetected            = true;
    }
  }

  // physical address
  if (!bPhysicalAutodetected)
  {
    uint16_t iPhysicalAddress(IsValidPhysicalAddress(configuration->iPhysicalAddress) ? configuration->iPhysicalAddress : CEC_PHYSICAL_ADDRESS_TV);
    bPhysicalAddressChanged = IsRunning() && m_configuration.iPhysicalAddress != iPhysicalAddress;
    if (bPhysicalAddressChanged)
    {
      CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - physical address '%04X'", __FUNCTION__, iPhysicalAddress);
      m_configuration.iPhysicalAddress = iPhysicalAddress;
    }
  }

  bool bHdmiPortChanged(false);
  if (!bPhysicalAutodetected && !IsValidPhysicalAddress(configuration->iPhysicalAddress))
  {
    // base device
    bHdmiPortChanged = IsRunning() && m_configuration.baseDevice != configuration->baseDevice;
    CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - using base device '%x'", __FUNCTION__, (int)configuration->baseDevice);
    m_configuration.baseDevice = configuration->baseDevice;

    // hdmi port
    bHdmiPortChanged |= IsRunning() && m_configuration.iHDMIPort != configuration->iHDMIPort;
    CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - using HDMI port '%d'", __FUNCTION__, configuration->iHDMIPort);
    m_configuration.iHDMIPort = configuration->iHDMIPort;
  }
  else
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - resetting HDMI port and base device to defaults", __FUNCTION__);
    m_configuration.baseDevice = CECDEVICE_UNKNOWN;
    m_configuration.iHDMIPort  = CEC_HDMI_PORTNUMBER_NONE;
  }

  bReinit = bPhysicalAddressChanged || bHdmiPortChanged || bDeviceTypeChanged;

  // device name
  CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - using OSD name '%s'", __FUNCTION__, configuration->strDeviceName);
  snprintf(m_configuration.strDeviceName, 13, "%s", configuration->strDeviceName);
  if (primary && !primary->GetOSDName().Equals(m_configuration.strDeviceName))
  {
    primary->SetOSDName(m_configuration.strDeviceName);
    if (!bReinit && IsRunning())
      primary->TransmitOSDName(CECDEVICE_TV);
  }

  // tv vendor id override
  CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - vendor id '%s'", __FUNCTION__, ToString((cec_vendor_id)configuration->tvVendor));
  if (m_configuration.tvVendor != configuration->tvVendor)
  {
    m_configuration.tvVendor= configuration->tvVendor;
    m_busDevices[CECDEVICE_TV]->SetVendorId((uint64_t)m_configuration.tvVendor);
  }

  // wake CEC devices
  if (m_configuration.wakeDevices != configuration->wakeDevices)
  {
    m_configuration.wakeDevices = configuration->wakeDevices;
    if (!bReinit && IsRunning())
      PowerOnDevices();
  }

  // just copy these
  m_configuration.bUseTVMenuLanguage   = configuration->bUseTVMenuLanguage;
  m_configuration.bActivateSource      = configuration->bActivateSource;
  m_configuration.bGetSettingsFromROM  = configuration->bGetSettingsFromROM;
  m_configuration.powerOffDevices      = configuration->powerOffDevices;
  m_configuration.bPowerOffScreensaver = configuration->bPowerOffScreensaver;
  m_configuration.bPowerOffOnStandby   = configuration->bPowerOffOnStandby;

  // client version 1.5.1
  if (configuration->clientVersion >= CEC_CLIENT_VERSION_1_5_1)
    m_configuration.bSendInactiveSource = configuration->bSendInactiveSource;

  // client version 1.6.0
  if (configuration->clientVersion >= CEC_CLIENT_VERSION_1_6_0)
  {
    m_configuration.bPowerOffDevicesOnStandby = configuration->bPowerOffDevicesOnStandby;
    m_configuration.bShutdownOnStandby        = configuration->bShutdownOnStandby;
  }

  // client version 1.6.2
  if (configuration->clientVersion >= CEC_CLIENT_VERSION_1_6_2)
  {
    memcpy(m_configuration.strDeviceLanguage, configuration->strDeviceLanguage, 3);
  }

  // ensure that there is at least 1 device type set
  if (m_configuration.deviceTypes.IsEmpty())
    m_configuration.deviceTypes.Add(CEC_DEVICE_TYPE_RECORDING_DEVICE);

  bool bReturn(true);
  if (bReinit || m_configuration.logicalAddresses.IsEmpty())
  {
    if (bDeviceTypeChanged)
      bReturn = ChangeDeviceType(oldPrimaryType, m_configuration.deviceTypes[0]);
    else if (IsValidPhysicalAddress(m_configuration.iPhysicalAddress))
      bReturn = SetPhysicalAddress(m_configuration.iPhysicalAddress);
    else if (m_configuration.baseDevice != CECDEVICE_UNKNOWN && m_configuration.iHDMIPort != CEC_HDMI_PORTNUMBER_NONE)
      bReturn = SetHDMIPort(m_configuration.baseDevice, m_configuration.iHDMIPort);
  }
  else if (m_configuration.bActivateSource == 1 && IsRunning() && !IsActiveSource(m_configuration.logicalAddresses.primary))
  {
    // activate the source if we're not already the active source
    SetActiveSource(m_configuration.deviceTypes.types[0]);
  }

  // persist the configuration
  if (IsRunning())
    m_communication->PersistConfiguration(&m_configuration);

  return bReturn;
}

bool CCECProcessor::GetCurrentConfiguration(libcec_configuration *configuration)
{
  // client version 1.5.0
  snprintf(configuration->strDeviceName, 13, "%s", m_configuration.strDeviceName);
  configuration->deviceTypes          = m_configuration.deviceTypes;
  configuration->bAutodetectAddress   = m_configuration.bAutodetectAddress;
  configuration->iPhysicalAddress     = m_configuration.iPhysicalAddress;
  configuration->baseDevice           = m_configuration.baseDevice;
  configuration->iHDMIPort            = m_configuration.iHDMIPort;
  configuration->clientVersion        = m_configuration.clientVersion;
  configuration->serverVersion        = m_configuration.serverVersion;
  configuration->tvVendor             = m_configuration.tvVendor;

  configuration->bGetSettingsFromROM  = m_configuration.bGetSettingsFromROM;
  configuration->bUseTVMenuLanguage   = m_configuration.bUseTVMenuLanguage;
  configuration->bActivateSource      = m_configuration.bActivateSource;
  configuration->wakeDevices          = m_configuration.wakeDevices;
  configuration->powerOffDevices      = m_configuration.powerOffDevices;
  configuration->bPowerOffScreensaver = m_configuration.bPowerOffScreensaver;
  configuration->bPowerOffOnStandby   = m_configuration.bPowerOffOnStandby;

  // client version 1.5.1
  if (configuration->clientVersion >= CEC_CLIENT_VERSION_1_5_1)
    configuration->bSendInactiveSource = m_configuration.bSendInactiveSource;

  // client version 1.5.3
  if (configuration->clientVersion >= CEC_CLIENT_VERSION_1_5_3)
    configuration->logicalAddresses    = m_configuration.logicalAddresses;

  // client version 1.6.0
  if (configuration->clientVersion >= CEC_CLIENT_VERSION_1_6_0)
  {
    configuration->iFirmwareVersion          = m_configuration.iFirmwareVersion;
    configuration->bPowerOffDevicesOnStandby = m_configuration.bPowerOffDevicesOnStandby;
    configuration->bShutdownOnStandby        = m_configuration.bShutdownOnStandby;
  }

  // client version 1.6.2
  if (configuration->clientVersion >= CEC_CLIENT_VERSION_1_6_2)
  {
    memcpy(configuration->strDeviceLanguage, m_configuration.strDeviceLanguage, 3);
    configuration->iFirmwareBuildDate      = m_configuration.iFirmwareBuildDate;
  }
  return true;
}

bool CCECProcessor::CanPersistConfiguration(void)
{
  return m_communication ? m_communication->GetFirmwareVersion() >= 2 : false;
}

bool CCECProcessor::PersistConfiguration(libcec_configuration *configuration)
{
  return m_communication ? m_communication->PersistConfiguration(configuration) : false;
}

void CCECProcessor::RescanActiveDevices(void)
{
  for (uint8_t iPtr = CECDEVICE_TV; iPtr < CECDEVICE_BROADCAST; iPtr++)
    m_busDevices[iPtr]->GetStatus(true);
}

bool CCECProcessor::GetDeviceInformation(const char *strPort, libcec_configuration *config, uint32_t iTimeoutMs /* = CEC_DEFAULT_CONNECT_TIMEOUT */)
{
  if (!OpenConnection(strPort, CEC_SERIAL_DEFAULT_BAUDRATE, iTimeoutMs, false))
    return false;

  config->iFirmwareVersion   = m_communication->GetFirmwareVersion();
  config->iPhysicalAddress   = m_communication->GetPhysicalAddress();
  config->iFirmwareBuildDate = m_communication->GetFirmwareBuildDate();

  return true;
}

bool CCECProcessor::TransmitPendingActiveSourceCommands(void)
{
  bool bReturn(true);
  for (uint8_t iPtr = CECDEVICE_TV; iPtr < CECDEVICE_BROADCAST; iPtr++)
    bReturn &= m_busDevices[iPtr]->TransmitPendingActiveSourceCommands();
  return bReturn;
}

bool CCECProcessor::IsValidPhysicalAddress(uint16_t iPhysicalAddress)
{
  return iPhysicalAddress >= CEC_MIN_PHYSICAL_ADDRESS &&
         iPhysicalAddress <= CEC_MAX_PHYSICAL_ADDRESS;
}
