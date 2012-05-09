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

#include "CECClient.h"
#include "CECProcessor.h"
#include "LibCEC.h"
#include "devices/CECPlaybackDevice.h"
#include "devices/CECAudioSystem.h"
#include "devices/CECTV.h"

using namespace CEC;
using namespace PLATFORM;

#define LIB_CEC     m_processor->GetLib()
#define ToString(x) LIB_CEC->ToString(x)

CCECClient::CCECClient(CCECProcessor *processor, const libcec_configuration *configuration) :
    m_processor(processor),
    m_bInitialised(false),
    m_bRegistered(false),
    m_iCurrentButton(CEC_USER_CONTROL_CODE_UNKNOWN),
    m_buttontime(0)
{
  SetConfiguration(configuration);
}

CCECClient::~CCECClient(void)
{
  if (m_processor)
    m_processor->UnregisterClient(this);
}

bool CCECClient::IsInitialised(void)
{
  CLockObject lock(m_mutex);
  return m_bInitialised && m_processor;
}

void CCECClient::SetInitialised(bool bSetTo)
{
  CLockObject lock(m_mutex);
  m_bInitialised = bSetTo;
}

bool CCECClient::IsRegistered(void)
{
  CLockObject lock(m_mutex);
  return m_bRegistered && m_processor;
}

void CCECClient::SetRegistered(bool bSetTo)
{
  CLockObject lock(m_mutex);
  m_bRegistered = bSetTo;
}

bool CCECClient::Initialise(void)
{
  if (IsInitialised())
    return true;

  //TODO do the same for the other devices
  CCECBusDevice *primary = m_processor->GetDevice(m_configuration.logicalAddresses.primary);
  if (!primary)
  {
    LIB_CEC->AddLog(CEC_LOG_WARNING, "cannot find the primary device (logical address %x)", m_configuration.logicalAddresses.primary);
    return false;
  }

  /* only set our OSD name for the primary device */
  primary->SetOSDName(m_configuration.strDeviceName);

  /* set the default menu language for devices we control */
  primary->SetMenuLanguage(m_configuration.strDeviceLanguage);

  if (CLibCEC::IsValidPhysicalAddress(m_configuration.iPhysicalAddress))
  {
    primary->SetPhysicalAddress(m_configuration.iPhysicalAddress);
    primary->TransmitPhysicalAddress();
  }
  else
  {
    SetHDMIPort(m_configuration.baseDevice, m_configuration.iHDMIPort, true);
  }

  /* make the primary device the active source if the option is set */
  if (m_configuration.bActivateSource == 1)
    primary->ActivateSource();

  SetInitialised(true);
  return true;
}

bool CCECClient::SetHDMIPort(cec_logical_address iBaseDevice, uint8_t iPort, bool bForce /* = false */)
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

  if (!m_processor->IsRunning() && !bForce)
    return true;

  LIB_CEC->AddLog(CEC_LOG_DEBUG, "setting HDMI port to %d on device %s (%d)", iPort, ToString(iBaseDevice), (int)iBaseDevice);

  uint16_t iPhysicalAddress(CEC_INVALID_PHYSICAL_ADDRESS);
  CCECBusDevice *baseDevice = m_processor->GetDevice(iBaseDevice);
  if (baseDevice)
    iPhysicalAddress = baseDevice->GetPhysicalAddress(m_configuration.logicalAddresses.primary);

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
    LIB_CEC->AddLog(CEC_LOG_WARNING, "failed to set the physical address to %04X, setting it to the default value %04X", iPhysicalAddress, CEC_DEFAULT_PHYSICAL_ADDRESS);
    iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS;
  }

  SetPhysicalAddress(iPhysicalAddress);

  return bReturn;
}

bool CCECClient::SetPhysicalAddress(uint16_t iPhysicalAddress)
{
  bool bSendActiveView(false);
  bool bReturn(false);
  bool bSendUpdate = m_processor->CECInitialised();

  CECDEVICEVEC sendUpdatesTo;
  {
    CLockObject lock(m_mutex);
    m_configuration.iPhysicalAddress = iPhysicalAddress;
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "setting physical address to '%04X'", iPhysicalAddress);

    bool bWasActiveSource(false);
    CECDEVICEVEC devices;
    // TODO
    m_processor->GetDevices()->GetLibCECControlled(devices);

    for (CECDEVICEVEC::iterator it = devices.begin(); it != devices.end(); it++)
    {
      bWasActiveSource |= (*it)->IsActiveSource();
      (*it)->MarkAsInactiveSource();
      (*it)->SetPhysicalAddress(iPhysicalAddress);
      if (bSendUpdate)
        sendUpdatesTo.push_back(*it);
    }

    bSendActiveView = bWasActiveSource && bSendUpdate;
    bReturn = true;
  }

  for (CECDEVICEVEC::iterator it = sendUpdatesTo.begin(); it != sendUpdatesTo.end(); it++)
  {
    (*it)->TransmitPhysicalAddress();
    if (bSendActiveView && m_configuration.logicalAddresses.primary == (*it)->GetLogicalAddress())
    {
      (*it)->MarkAsActiveSource();
      if ((*it)->HasValidPhysicalAddress())
        (*it)->ActivateSource();
    }
  }

  if (bReturn)
  {
    m_processor->PersistConfiguration(&m_configuration);
    ConfigurationChanged(m_configuration);
  }

  return bReturn;
}

bool CCECClient::FindLogicalAddresses(void)
{
  m_configuration.logicalAddresses.Clear();

  if (m_configuration.deviceTypes.IsEmpty())
  {
    LIB_CEC->AddLog(CEC_LOG_ERROR, "no device types given");
    return false;
  }

  for (uint8_t iPtr = 0; iPtr < 5; iPtr++)
  {
    if (m_configuration.deviceTypes.types[iPtr] == CEC_DEVICE_TYPE_RESERVED)
      continue;

    cec_logical_address address(CECDEVICE_UNKNOWN);
    if (m_configuration.deviceTypes.types[iPtr] == CEC_DEVICE_TYPE_RECORDING_DEVICE)
      address = FindLogicalAddressRecordingDevice();
    if (m_configuration.deviceTypes.types[iPtr] == CEC_DEVICE_TYPE_TUNER)
      address = FindLogicalAddressTuner();
    if (m_configuration.deviceTypes.types[iPtr] == CEC_DEVICE_TYPE_PLAYBACK_DEVICE)
      address = FindLogicalAddressPlaybackDevice();
    if (m_configuration.deviceTypes.types[iPtr] == CEC_DEVICE_TYPE_AUDIO_SYSTEM)
      address = FindLogicalAddressAudioSystem();

    if (address == CECDEVICE_UNKNOWN)
    {
      LIB_CEC->AddLog(CEC_LOG_ERROR, "%s - failed to allocate device '%d', type '%s'", __FUNCTION__, iPtr, ToString(m_configuration.deviceTypes.types[iPtr]));
      return false;
    }

    LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - device '%d', type '%s', LA '%X'", __FUNCTION__, iPtr, ToString(m_configuration.deviceTypes.types[iPtr]), address);
    m_configuration.logicalAddresses.Set(address);
  }

  return true;
}

cec_logical_address CCECClient::FindLogicalAddressRecordingDevice(void)
{
  cec_logical_address retVal(CECDEVICE_UNKNOWN);

  LIB_CEC->AddLog(CEC_LOG_DEBUG, "detecting logical address for type 'recording device'");
  if (m_processor->TryLogicalAddress(CECDEVICE_RECORDINGDEVICE1))
    retVal = CECDEVICE_RECORDINGDEVICE1;
  else if (m_processor->TryLogicalAddress(CECDEVICE_RECORDINGDEVICE2))
    retVal = CECDEVICE_RECORDINGDEVICE2;
  else if (m_processor->TryLogicalAddress(CECDEVICE_RECORDINGDEVICE3))
    retVal = CECDEVICE_RECORDINGDEVICE3;

  return retVal;
}

cec_logical_address CCECClient::FindLogicalAddressTuner(void)
{
  cec_logical_address retVal(CECDEVICE_UNKNOWN);

  LIB_CEC->AddLog(CEC_LOG_DEBUG, "detecting logical address for type 'tuner'");
  if (m_processor->TryLogicalAddress(CECDEVICE_TUNER1))
    retVal = CECDEVICE_TUNER1;
  else if (m_processor->TryLogicalAddress(CECDEVICE_TUNER2))
    retVal = CECDEVICE_TUNER2;
  else if (m_processor->TryLogicalAddress(CECDEVICE_TUNER3))
    retVal = CECDEVICE_TUNER3;
  else if (m_processor->TryLogicalAddress(CECDEVICE_TUNER4))
    retVal = CECDEVICE_TUNER4;

  return retVal;
}

cec_logical_address CCECClient::FindLogicalAddressPlaybackDevice(void)
{
  cec_logical_address retVal(CECDEVICE_UNKNOWN);

  LIB_CEC->AddLog(CEC_LOG_DEBUG, "detecting logical address for type 'playback device'");
  if (m_processor->TryLogicalAddress(CECDEVICE_PLAYBACKDEVICE1))
    retVal = CECDEVICE_PLAYBACKDEVICE1;
  else if (m_processor->TryLogicalAddress(CECDEVICE_PLAYBACKDEVICE2))
    retVal = CECDEVICE_PLAYBACKDEVICE2;
  else if (m_processor->TryLogicalAddress(CECDEVICE_PLAYBACKDEVICE3))
    retVal = CECDEVICE_PLAYBACKDEVICE3;

  return retVal;
}

cec_logical_address CCECClient::FindLogicalAddressAudioSystem(void)
{
  cec_logical_address retVal(CECDEVICE_UNKNOWN);

  LIB_CEC->AddLog(CEC_LOG_DEBUG, "detecting logical address for type 'audiosystem'");
  if (m_processor->TryLogicalAddress(CECDEVICE_AUDIOSYSTEM))
    retVal = CECDEVICE_AUDIOSYSTEM;

  return retVal;
}

CCECBusDevice *CCECClient::GetDeviceByType(const cec_device_type type) const
{
  // get all devices that match our logical addresses
  CECDEVICEVEC devices;
  m_processor->GetDevices()->GetByLogicalAddresses(devices, m_configuration.logicalAddresses);

  // filter the type we need
  CCECDeviceMap::FilterType(type, devices);

  return devices.empty() ?
      NULL :
      *devices.begin();
}

bool CCECClient::ChangeDeviceType(cec_device_type from, cec_device_type to)
{
  bool bChanged(false);

  LIB_CEC->AddLog(CEC_LOG_NOTICE, "changing device type '%s' into '%s'", ToString(from), ToString(to));

  CLockObject lock(m_mutex);

  CCECBusDevice *previousDevice = GetDeviceByType(from);
  if (!previousDevice)
    return false;

  m_processor->UnregisterClient(this);

  m_configuration.logicalAddresses.primary = CECDEVICE_UNREGISTERED;

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
    // re-register the client to set the new ackmask
    if (!m_processor->RegisterClient(this))
      return false;

    // copy the data from the previous device
    CCECBusDevice *newDevice = GetDeviceByType(to);
    if (previousDevice && newDevice)
    {
      newDevice->SetDeviceStatus(CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC);
      newDevice->SetCecVersion(previousDevice->GetCecVersion(m_configuration.logicalAddresses.primary, false));
      newDevice->SetMenuLanguage(previousDevice->GetMenuLanguage(m_configuration.logicalAddresses.primary, false));
      newDevice->SetMenuState(previousDevice->GetMenuState(m_configuration.logicalAddresses.primary));
      newDevice->SetOSDName(previousDevice->GetOSDName(m_configuration.logicalAddresses.primary, false));
      newDevice->SetPhysicalAddress(previousDevice->GetCurrentPhysicalAddress());
      newDevice->SetPowerStatus(previousDevice->GetPowerStatus(m_configuration.logicalAddresses.primary, false));
      newDevice->SetVendorId(previousDevice->GetVendorId(m_configuration.logicalAddresses.primary, false));

      if ((from == CEC_DEVICE_TYPE_PLAYBACK_DEVICE || from == CEC_DEVICE_TYPE_RECORDING_DEVICE) &&
          (to == CEC_DEVICE_TYPE_PLAYBACK_DEVICE || to == CEC_DEVICE_TYPE_RECORDING_DEVICE))
      {
        newDevice->AsPlaybackDevice()->SetDeckControlMode(previousDevice->AsPlaybackDevice()->GetDeckControlMode(m_configuration.logicalAddresses.primary));
        newDevice->AsPlaybackDevice()->SetDeckStatus(previousDevice->AsPlaybackDevice()->GetDeckStatus(m_configuration.logicalAddresses.primary));
      }
    }

    // and reset the previous device to the initial state
    if (previousDevice)
      previousDevice->ResetDeviceStatus();
  }

  return true;
}

bool CCECClient::SetLogicalAddress(cec_logical_address iLogicalAddress)
{
  CLockObject lock(m_mutex);
  if (m_configuration.logicalAddresses.primary != iLogicalAddress)
  {
    LIB_CEC->AddLog(CEC_LOG_NOTICE, "<< setting primary logical address to %1x", iLogicalAddress);
    m_configuration.logicalAddresses.primary = iLogicalAddress;
    m_configuration.logicalAddresses.Set(iLogicalAddress);
    return m_processor->RegisterClient(this);
  }

  return true;
}

bool CCECClient::Transmit(const cec_command &data)
{
  return m_processor ? m_processor->Transmit(data) : false;
}

bool CCECClient::SendPowerOnDevices(cec_logical_address address /* = CECDEVICE_TV */)
{
  if (address == CECDEVICE_BROADCAST && m_configuration.clientVersion >= CEC_CLIENT_VERSION_1_5_0)
  {
    CECDEVICEVEC devices;
    m_processor->GetDevices()->GetWakeDevices(m_configuration, devices);
    return m_processor->PowerOnDevices(m_configuration.logicalAddresses.primary, devices);
  }

  return m_processor->PowerOnDevice(m_configuration.logicalAddresses.primary, address);
}

bool CCECClient::SendStandbyDevices(cec_logical_address address /* = CECDEVICE_BROADCAST */)
{
  if (address == CECDEVICE_BROADCAST && m_configuration.clientVersion >= CEC_CLIENT_VERSION_1_5_0)
  {
    CECDEVICEVEC devices;
    m_processor->GetDevices()->GetPowerOffDevices(m_configuration, devices);
    return m_processor->StandbyDevices(m_configuration.logicalAddresses.primary, devices);
  }

  return m_processor->StandbyDevice(m_configuration.logicalAddresses.primary, address);
}

bool CCECClient::SendSetActiveSource(cec_device_type type /* = CEC_DEVICE_TYPE_RESERVED */)
{
  bool bReturn(false);

  CCECBusDevice *device(NULL);
  CECDEVICEVEC devices;
  m_processor->GetDevices()->GetByLogicalAddresses(devices, m_configuration.logicalAddresses);

  if (type != CEC_DEVICE_TYPE_RESERVED)
    CCECDeviceMap::FilterType(type, devices);

  // no devices left
  if (devices.empty())
    m_processor->GetDevices()->GetByLogicalAddresses(devices, m_configuration.logicalAddresses);

  if (!devices.empty())
    device = *devices.begin();

  if (device)
  {
    bReturn = true;
    if (m_processor->IsRunning() && device->HasValidPhysicalAddress())
      bReturn = device->ActivateSource();
  }

  return bReturn;
}

CCECPlaybackDevice *CCECClient::GetPlaybackDevice(void)
{
  CCECPlaybackDevice *device(NULL);
  CECDEVICEVEC devices;
  m_processor->GetDevices()->GetByLogicalAddresses(devices, m_configuration.logicalAddresses);
  CCECDeviceMap::FilterType(CEC_DEVICE_TYPE_PLAYBACK_DEVICE, devices);

  // no devices left
  if (devices.empty())
  {
    m_processor->GetDevices()->GetByLogicalAddresses(devices, m_configuration.logicalAddresses);
    CCECDeviceMap::FilterType(CEC_DEVICE_TYPE_RECORDING_DEVICE, devices);
  }

  if (!devices.empty())
    device = (*devices.begin())->AsPlaybackDevice();

  return device;
}

CCECBusDevice *CCECClient::GetPrimaryDevice(void)
{
  return m_processor->GetDevice(m_configuration.logicalAddresses.primary);
}

bool CCECClient::SendSetDeckControlMode(cec_deck_control_mode mode, bool bSendUpdate /* = true */)
{
  bool bReturn(false);

  CCECBusDevice *device = GetPlaybackDevice();
  if (device)
  {
    device->AsPlaybackDevice()->SetDeckControlMode(mode);
    if (bSendUpdate)
      bReturn = device->AsPlaybackDevice()->TransmitDeckStatus(CECDEVICE_TV);
    else
      bReturn = true;
  }

  return false;
}

bool CCECClient::SendSetDeckInfo(cec_deck_info info, bool bSendUpdate /* = true */)
{
  bool bReturn(false);

  CCECBusDevice *device = GetPlaybackDevice();
  if (device)
  {
    device->AsPlaybackDevice()->SetDeckStatus(info);
    if (bSendUpdate)
      bReturn = device->AsPlaybackDevice()->TransmitDeckStatus(CECDEVICE_TV);
    else
      bReturn = true;
  }

  return false;
}

bool CCECClient::SendSetMenuState(cec_menu_state state, bool bSendUpdate /* = true */)
{
  CECDEVICEVEC devices;
  m_processor->GetDevices()->GetByLogicalAddresses(devices, m_configuration.logicalAddresses);

  for (CECDEVICEVEC::iterator it = devices.begin(); it != devices.end(); it++)
  {
    (*it)->SetMenuState(state);
    if (bSendUpdate)
      (*it)->TransmitMenuState(CECDEVICE_TV);
  }

  return true;
}

bool CCECClient::SendSetInactiveView(void)
{
  CCECBusDevice *primary = GetPrimaryDevice();
  if (primary)
  {
    primary->MarkAsInactiveSource();
    return primary->TransmitInactiveSource();
  }
  return false;
}

bool CCECClient::SendSetOSDString(cec_logical_address iLogicalAddress, cec_display_control duration, const char *strMessage)
{
  CCECBusDevice *primary = GetPrimaryDevice();
  if (primary)
    return primary->TransmitOSDString(iLogicalAddress, duration, strMessage);

  return false;
}

cec_version CCECClient::GetDeviceCecVersion(cec_logical_address iAddress)
{
  CCECBusDevice *device = m_processor->GetDevice(iAddress);
  if (device)
    return device->GetCecVersion(m_configuration.logicalAddresses.primary);
  return CEC_VERSION_UNKNOWN;
}

bool CCECClient::GetDeviceMenuLanguage(cec_logical_address iAddress, cec_menu_language *language)
{
  CCECBusDevice *device = m_processor->GetDevice(iAddress);
  if (device)
  {
    *language = device->GetMenuLanguage(m_configuration.logicalAddresses.primary);
    return (strcmp(language->language, "???") != 0);
  }
  return false;
}

cec_osd_name CCECClient::GetDeviceOSDName(cec_logical_address iAddress)
{
  cec_osd_name retVal;
  retVal.device = iAddress;
  retVal.name[0] = 0;

  CCECBusDevice *device = m_processor->GetDevice(iAddress);
  if (device)
  {
    CStdString strOSDName = device->GetOSDName(m_configuration.logicalAddresses.primary);
    snprintf(retVal.name, sizeof(retVal.name), "%s", strOSDName.c_str());
    retVal.device = iAddress;
  }

  return retVal;
}

uint16_t CCECClient::GetDevicePhysicalAddress(cec_logical_address iAddress)
{
  CCECBusDevice *device = m_processor->GetDevice(iAddress);
  if (device)
    return device->GetPhysicalAddress(m_configuration.logicalAddresses.primary);
  return CEC_INVALID_PHYSICAL_ADDRESS;
}

cec_power_status CCECClient::GetDevicePowerStatus(cec_logical_address iAddress)
{
  CCECBusDevice *device = m_processor->GetDevice(iAddress);
  if (device)
    return device->GetPowerStatus(m_configuration.logicalAddresses.primary);
  return CEC_POWER_STATUS_UNKNOWN;
}

uint64_t CCECClient::GetDeviceVendorId(cec_logical_address iAddress)
{
  CCECBusDevice *device = m_processor->GetDevice(iAddress);
  if (device)
    return device->GetVendorId(m_configuration.logicalAddresses.primary);
  return CEC_VENDOR_UNKNOWN;
}

uint8_t CCECClient::SendVolumeUp(bool bSendRelease /* = true */)
{
  CCECBusDevice *device = GetPrimaryDevice();
  CCECAudioSystem *audio = m_processor->GetAudioSystem();

  return device && audio && audio->IsPresent() ?
      audio->VolumeUp(device->GetLogicalAddress(), bSendRelease) :
      (uint8_t)CEC_AUDIO_VOLUME_STATUS_UNKNOWN;
}

uint8_t CCECClient::SendVolumeDown(bool bSendRelease /* = true */)
{
  CCECBusDevice *device = GetPrimaryDevice();
  CCECAudioSystem *audio = m_processor->GetAudioSystem();

  return device && audio && audio->IsPresent() ?
      audio->VolumeDown(device->GetLogicalAddress(), bSendRelease) :
      (uint8_t)CEC_AUDIO_VOLUME_STATUS_UNKNOWN;
}

uint8_t CCECClient::SendMuteAudio(void)
{
  CCECBusDevice *device = GetPrimaryDevice();
  CCECAudioSystem *audio = m_processor->GetAudioSystem();

  return device && audio && audio->IsPresent() ?
      audio->MuteAudio(device->GetLogicalAddress()) :
      (uint8_t)CEC_AUDIO_VOLUME_STATUS_UNKNOWN;
}

bool CCECClient::SendKeypress(cec_logical_address iDestination, cec_user_control_code key, bool bWait /* = true */)
{
  CCECBusDevice *device = GetPrimaryDevice();
  CCECBusDevice *dest = m_processor->GetDevice(iDestination);

  return device && dest ?
      device->TransmitKeypress(m_configuration.logicalAddresses.primary, key, bWait) :
      false;
}

bool CCECClient::SendKeyRelease(cec_logical_address iDestination, bool bWait /* = true */)
{
  CCECBusDevice *device = GetPrimaryDevice();
  CCECBusDevice *dest = m_processor->GetDevice(iDestination);

  return device && dest ?
      device->TransmitKeyRelease(m_configuration.logicalAddresses.primary, bWait) :
      false;
}

bool CCECClient::GetCurrentConfiguration(libcec_configuration *configuration)
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

bool CCECClient::SetConfiguration(const libcec_configuration *configuration)
{
  bool bReinit(false);
  bool bIsRunning(m_processor && m_processor->IsRunning());

  if (configuration->callbacks)
  {
    m_configuration.callbacks     = configuration->callbacks;
    m_configuration.callbackParam = configuration->callbackParam;
  }

  //TODO
  CCECBusDevice *primary = bIsRunning ? GetPrimaryDevice() : NULL;
  cec_device_type oldPrimaryType = primary ? primary->GetType() : CEC_DEVICE_TYPE_RECORDING_DEVICE;

  m_configuration.serverVersion  = LIBCEC_VERSION_CURRENT;
  m_configuration.clientVersion  = configuration->clientVersion;
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - using client version '%s'", __FUNCTION__, ToString((cec_client_version)configuration->clientVersion));

  // client version 1.5.0

  // device types
  bool bDeviceTypeChanged = bIsRunning && m_configuration.deviceTypes != configuration->deviceTypes;
  m_configuration.deviceTypes = configuration->deviceTypes;
  if (bDeviceTypeChanged)
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - using primary device type '%s'", __FUNCTION__, ToString(configuration->deviceTypes[0]));

  bool bPhysicalAddressChanged(false);

  // autodetect address
  bool bPhysicalAutodetected(false);
  if (bIsRunning && configuration->bAutodetectAddress == 1)
  {
    uint16_t iPhysicalAddress = m_processor->GetDetectedPhysicalAddress();
    if (CLibCEC::IsValidPhysicalAddress(iPhysicalAddress))
    {
      if (bIsRunning)
        LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - autodetected physical address '%04X'", __FUNCTION__, iPhysicalAddress);
      else
        LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - using physical address '%04X'", __FUNCTION__, iPhysicalAddress);
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
    uint16_t iPhysicalAddress(CLibCEC::IsValidPhysicalAddress(configuration->iPhysicalAddress) ? configuration->iPhysicalAddress : CEC_PHYSICAL_ADDRESS_TV);
    bPhysicalAddressChanged = bIsRunning && m_configuration.iPhysicalAddress != iPhysicalAddress;
    if (bPhysicalAddressChanged)
    {
      LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - physical address '%04X'", __FUNCTION__, iPhysicalAddress);
      m_configuration.iPhysicalAddress = iPhysicalAddress;
    }
  }

  bool bHdmiPortChanged(false);
  if (!bPhysicalAutodetected && !CLibCEC::IsValidPhysicalAddress(configuration->iPhysicalAddress))
  {
    // base device
    bHdmiPortChanged = bIsRunning && m_configuration.baseDevice != configuration->baseDevice;
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - using base device '%x'", __FUNCTION__, (int)configuration->baseDevice);
    m_configuration.baseDevice = configuration->baseDevice;

    // hdmi port
    bHdmiPortChanged |= bIsRunning && m_configuration.iHDMIPort != configuration->iHDMIPort;
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - using HDMI port '%d'", __FUNCTION__, configuration->iHDMIPort);
    m_configuration.iHDMIPort = configuration->iHDMIPort;
  }
  else
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - resetting HDMI port and base device to defaults", __FUNCTION__);
    m_configuration.baseDevice = CECDEVICE_UNKNOWN;
    m_configuration.iHDMIPort  = CEC_HDMI_PORTNUMBER_NONE;
  }

  bReinit = bPhysicalAddressChanged || bHdmiPortChanged || bDeviceTypeChanged;

  // device name
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - using OSD name '%s'", __FUNCTION__, configuration->strDeviceName);
  snprintf(m_configuration.strDeviceName, 13, "%s", configuration->strDeviceName);
  if (primary && !primary->GetOSDName(m_configuration.logicalAddresses.primary, false).Equals(m_configuration.strDeviceName))
  {
    primary->SetOSDName(m_configuration.strDeviceName);
    if (!bReinit && bIsRunning)
      primary->TransmitOSDName(CECDEVICE_TV);
  }

  // tv vendor id override
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - vendor id '%s'", __FUNCTION__, ToString((cec_vendor_id)configuration->tvVendor));
  if (m_processor && m_configuration.tvVendor != configuration->tvVendor)
  {
    m_configuration.tvVendor= configuration->tvVendor;
    m_processor->GetTV()->SetVendorId((uint64_t)m_configuration.tvVendor);
  }

  // wake CEC devices
  if (m_configuration.wakeDevices != configuration->wakeDevices)
  {
    m_configuration.wakeDevices = configuration->wakeDevices;
    if (!bReinit && bIsRunning)
      SendPowerOnDevices();
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

  if (bIsRunning)
    m_processor->GetTV()->ReplaceHandler(false);

  bool bReturn(true);
  if (bReinit || m_configuration.logicalAddresses.IsEmpty())
  {
    if (bDeviceTypeChanged)
      bReturn = ChangeDeviceType(oldPrimaryType, m_configuration.deviceTypes[0]);
    else if (CLibCEC::IsValidPhysicalAddress(m_configuration.iPhysicalAddress))
      bReturn = SetPhysicalAddress(m_configuration.iPhysicalAddress);
    else if (m_configuration.baseDevice != CECDEVICE_UNKNOWN && m_configuration.iHDMIPort != CEC_HDMI_PORTNUMBER_NONE)
      bReturn = SetHDMIPort(m_configuration.baseDevice, m_configuration.iHDMIPort);
  }
  else if (m_configuration.bActivateSource == 1 && bIsRunning && !m_processor->IsActiveSource(m_configuration.logicalAddresses.primary))
  {
    // activate the source if we're not already the active source
    m_processor->SetActiveSource(m_configuration.deviceTypes.types[0]);
  }

  // persist the configuration
  if (bIsRunning)
    m_processor->PersistConfiguration(&m_configuration);

  return bReturn;
}

void CCECClient::AddCommand(const cec_command &command)
{
  CLockObject lock(m_mutex);

  LIB_CEC->AddLog(CEC_LOG_NOTICE, ">> %s (%X) -> %s (%X): %s (%2X)", ToString(command.initiator), command.initiator, ToString(command.destination), command.destination, ToString(command.opcode), command.opcode);

  if (m_configuration.callbacks && m_configuration.callbacks->CBCecCommand)
    m_configuration.callbacks->CBCecCommand(m_configuration.callbackParam, command);
  else if (!m_commandBuffer.Push(command))
    LIB_CEC->AddLog(CEC_LOG_WARNING, "command buffer is full");
}

int CCECClient::MenuStateChanged(const cec_menu_state newState)
{
  CLockObject lock(m_mutex);

  LIB_CEC->AddLog(CEC_LOG_NOTICE, ">> %s: %s", ToString(CEC_OPCODE_MENU_REQUEST), ToString(newState));

  if (m_configuration.callbacks &&
      m_configuration.clientVersion >= CEC_CLIENT_VERSION_1_6_2 &&
      m_configuration.callbacks->CBCecMenuStateChanged)
    return m_configuration.callbacks->CBCecMenuStateChanged(m_configuration.callbackParam, newState);

  return 0;
}

void CCECClient::Alert(const libcec_alert type, const libcec_parameter &param)
{
  CLockObject lock(m_mutex);

  if (m_configuration.callbacks &&
      m_configuration.clientVersion >= CEC_CLIENT_VERSION_1_6_0 &&
      m_configuration.callbacks->CBCecAlert)
    m_configuration.callbacks->CBCecAlert(m_configuration.callbackParam, type, param);
}

void CCECClient::AddLog(const cec_log_message &message)
{
  CLockObject lock(m_logMutex);
  if (m_configuration.callbacks && m_configuration.callbacks->CBCecLogMessage)
    m_configuration.callbacks->CBCecLogMessage(m_configuration.callbackParam, message);
  else
    m_logBuffer.Push(message);
}

void CCECClient::AddKey(void)
{
  CLockObject lock(m_mutex);

  if (m_iCurrentButton != CEC_USER_CONTROL_CODE_UNKNOWN)
  {
    cec_keypress key;

    key.duration = (unsigned int) (GetTimeMs() - m_buttontime);
    key.keycode = m_iCurrentButton;
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "key released: %1x", key.keycode);

    if (m_configuration.callbacks && m_configuration.callbacks->CBCecKeyPress)
      m_configuration.callbacks->CBCecKeyPress(m_configuration.callbackParam, key);
    else
      m_keyBuffer.Push(key);
    m_iCurrentButton = CEC_USER_CONTROL_CODE_UNKNOWN;
  }

  m_buttontime = 0;
}

void CCECClient::AddKey(const cec_keypress &key)
{
  CLockObject lock(m_mutex);

  LIB_CEC->AddLog(CEC_LOG_DEBUG, "key pressed: %1x", key.keycode);

  if (m_configuration.callbacks && m_configuration.callbacks->CBCecKeyPress)
    m_configuration.callbacks->CBCecKeyPress(m_configuration.callbackParam, key);
  else
    m_keyBuffer.Push(key);

  m_iCurrentButton = key.duration > 0 ? CEC_USER_CONTROL_CODE_UNKNOWN : key.keycode;
  m_buttontime = key.duration > 0 ? 0 : GetTimeMs();
}

void CCECClient::SetCurrentButton(cec_user_control_code iButtonCode)
{
  /* push keypress to the keybuffer with 0 duration.
     push another press to the keybuffer with the duration set when the button is released */
  cec_keypress key;
  key.duration = 0;
  key.keycode = iButtonCode;

  AddKey(key);
}

void CCECClient::CheckKeypressTimeout(void)
{
  if (m_iCurrentButton != CEC_USER_CONTROL_CODE_UNKNOWN && GetTimeMs() - m_buttontime > CEC_BUTTON_TIMEOUT)
  {
    AddKey();
    m_iCurrentButton = CEC_USER_CONTROL_CODE_UNKNOWN;
  }
}

void CCECClient::ConfigurationChanged(const libcec_configuration &config)
{
  CLockObject lock(m_mutex);

  if (m_configuration.callbacks &&
      m_configuration.clientVersion >= CEC_CLIENT_VERSION_1_5_0 &&
      m_configuration.callbacks->CBCecConfigurationChanged &&
      m_processor->CECInitialised())
    m_configuration.callbacks->CBCecConfigurationChanged(m_configuration.callbackParam, config);
}

bool CCECClient::EnableCallbacks(void *cbParam, ICECCallbacks *callbacks)
{
  CLockObject lock(m_mutex);
  m_configuration.callbackParam = cbParam;
  m_configuration.callbacks     = callbacks;
  return true;
}

bool CCECClient::GetNextLogMessage(cec_log_message *message)
{
  return (m_logBuffer.Pop(*message));
}

bool CCECClient::GetNextKeypress(cec_keypress *key)
{
  return m_keyBuffer.Pop(*key);
}

bool CCECClient::GetNextCommand(cec_command *command)
{
  return m_commandBuffer.Pop(*command);
}
