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

#include "LibCEC.h"

#include "adapter/USBCECAdapterDetection.h"
#include "adapter/USBCECAdapterCommunication.h"
#include "CECProcessor.h"
#include "devices/CECBusDevice.h"
#include "platform/util/timeutils.h"
#include "platform/util/StdString.h"

using namespace std;
using namespace CEC;
using namespace PLATFORM;

CLibCEC::CLibCEC(const char *strDeviceName, cec_device_type_list types, uint16_t iPhysicalAddress /* = 0 */) :
    m_iStartTime(GetTimeMs()),
    m_iCurrentButton(CEC_USER_CONTROL_CODE_UNKNOWN),
    m_buttontime(0),
    m_callbacks(NULL),
    m_cbParam(NULL)
{
  m_cec = new CCECProcessor(this, strDeviceName, types, iPhysicalAddress);
}

CLibCEC::CLibCEC(libcec_configuration *configuration) :
    m_iStartTime(GetTimeMs()),
    m_iCurrentButton(CEC_USER_CONTROL_CODE_UNKNOWN),
    m_buttontime(0),
    m_callbacks(configuration->callbacks),
    m_cbParam(configuration->callbackParam)
{
  m_cec = new CCECProcessor(this, configuration);
}

CLibCEC::~CLibCEC(void)
{
  delete m_cec;
}

bool CLibCEC::Open(const char *strPort, uint32_t iTimeoutMs /* = CEC_DEFAULT_CONNECT_TIMEOUT */)
{
  if (m_cec->IsRunning())
  {
    AddLog(CEC_LOG_ERROR, "connection already open");
    return false;
  }

  if (!m_cec->Start(strPort, CEC_SERIAL_DEFAULT_BAUDRATE, iTimeoutMs))
  {
    AddLog(CEC_LOG_ERROR, "could not start CEC communications");
    return false;
  }

  return true;
}

void CLibCEC::Close(void)
{
  if (m_cec)
    m_cec->Close();
}

bool CLibCEC::EnableCallbacks(void *cbParam, ICECCallbacks *callbacks)
{
  CLockObject lock(m_mutex);
  if (m_cec)
  {
    m_cbParam   = cbParam;
    m_callbacks = callbacks;
  }
  return false;
}

int8_t CLibCEC::FindAdapters(cec_adapter *deviceList, uint8_t iBufSize, const char *strDevicePath /* = NULL */)
{
  CStdString strDebug;
  if (strDevicePath)
    strDebug.Format("trying to autodetect the com port for device path '%s'", strDevicePath);
  else
    strDebug.Format("trying to autodetect all CEC adapters");
  AddLog(CEC_LOG_DEBUG, strDebug);

  return CUSBCECAdapterDetection::FindAdapters(deviceList, iBufSize, strDevicePath);
}

bool CLibCEC::PingAdapter(void)
{
  return m_cec ? m_cec->PingAdapter() : false;
}

bool CLibCEC::StartBootloader(void)
{
  return m_cec ? m_cec->StartBootloader() : false;
}

bool CLibCEC::GetNextLogMessage(cec_log_message *message)
{
  return (m_logBuffer.Pop(*message));
}

bool CLibCEC::GetNextKeypress(cec_keypress *key)
{
  return m_keyBuffer.Pop(*key);
}

bool CLibCEC::GetNextCommand(cec_command *command)
{
  return m_commandBuffer.Pop(*command);
}

bool CLibCEC::Transmit(const cec_command &data)
{
  return m_cec ? m_cec->Transmit(data) : false;
}

bool CLibCEC::SetLogicalAddress(cec_logical_address iLogicalAddress)
{
  return m_cec ? m_cec->SetLogicalAddress(iLogicalAddress) : false;
}

bool CLibCEC::SetPhysicalAddress(uint16_t iPhysicalAddress /* = CEC_DEFAULT_PHYSICAL_ADDRESS */)
{
  return m_cec ? m_cec->SetPhysicalAddress(iPhysicalAddress) : false;
}

bool CLibCEC::SetHDMIPort(cec_logical_address iBaseDevice, uint8_t iPort /* = CEC_DEFAULT_HDMI_PORT */)
{
  return m_cec ? m_cec->SetHDMIPort(iBaseDevice, iPort) : false;
}

bool CLibCEC::EnablePhysicalAddressDetection(void)
{
  return m_cec ? m_cec->EnablePhysicalAddressDetection() : false;
}

bool CLibCEC::PowerOnDevices(cec_logical_address address /* = CECDEVICE_TV */)
{
  return m_cec && address >= CECDEVICE_TV && address <= CECDEVICE_BROADCAST ? m_cec->PowerOnDevices(address) : false;
}

bool CLibCEC::StandbyDevices(cec_logical_address address /* = CECDEVICE_BROADCAST */)
{
  return m_cec && address >= CECDEVICE_TV && address <= CECDEVICE_BROADCAST ? m_cec->StandbyDevices(address) : false;
}

bool CLibCEC::SetActiveSource(cec_device_type type /* = CEC_DEVICE_TYPE_RESERVED */)
{
  return m_cec ? m_cec->SetActiveSource(type) : false;
}

bool CLibCEC::SetActiveView(void)
{
  return m_cec ? m_cec->SetActiveView() : false;
}

bool CLibCEC::SetDeckControlMode(cec_deck_control_mode mode, bool bSendUpdate /* = true */)
{
  return m_cec ? m_cec->SetDeckControlMode(mode, bSendUpdate) : false;
}

bool CLibCEC::SetDeckInfo(cec_deck_info info, bool bSendUpdate /* = true */)
{
  return m_cec ? m_cec->SetDeckInfo(info, bSendUpdate) : false;
}

bool CLibCEC::SetInactiveView(void)
{
  return m_cec ? m_cec->TransmitInactiveSource() : false;
}

bool CLibCEC::SetMenuState(cec_menu_state state, bool bSendUpdate /* = true */)
{
  return m_cec ? m_cec->SetMenuState(state, bSendUpdate) : false;
}

bool CLibCEC::SetOSDString(cec_logical_address iLogicalAddress, cec_display_control duration, const char *strMessage)
{
  return m_cec && iLogicalAddress >= CECDEVICE_TV && iLogicalAddress <= CECDEVICE_BROADCAST ?
      m_cec->m_busDevices[m_cec->GetLogicalAddress()]->TransmitOSDString(iLogicalAddress, duration, strMessage) :
      false;
}

bool CLibCEC::SwitchMonitoring(bool bEnable)
{
  return m_cec ? m_cec->SwitchMonitoring(bEnable) : false;
}

cec_version CLibCEC::GetDeviceCecVersion(cec_logical_address iAddress)
{
  if (m_cec && iAddress >= CECDEVICE_TV && iAddress < CECDEVICE_BROADCAST)
    return m_cec->GetDeviceCecVersion(iAddress);
  return CEC_VERSION_UNKNOWN;
}

bool CLibCEC::GetDeviceMenuLanguage(cec_logical_address iAddress, cec_menu_language *language)
{
  if (m_cec && iAddress >= CECDEVICE_TV && iAddress < CECDEVICE_BROADCAST)
    return m_cec->GetDeviceMenuLanguage(iAddress, language);
  return false;
}

uint64_t CLibCEC::GetDeviceVendorId(cec_logical_address iAddress)
{
  if (m_cec && iAddress >= CECDEVICE_TV && iAddress < CECDEVICE_BROADCAST)
    return m_cec->GetDeviceVendorId(iAddress);
  return 0;
}

uint16_t CLibCEC::GetDevicePhysicalAddress(cec_logical_address iAddress)
{
  if (m_cec && iAddress >= CECDEVICE_TV && iAddress < CECDEVICE_BROADCAST)
    return m_cec->GetDevicePhysicalAddress(iAddress);
  return 0;
}

cec_logical_address CLibCEC::GetActiveSource(void)
{
  return m_cec ? m_cec->GetActiveSource() : CECDEVICE_UNKNOWN;
}

bool CLibCEC::IsActiveSource(cec_logical_address iAddress)
{
  if (m_cec && iAddress >= CECDEVICE_TV && iAddress < CECDEVICE_BROADCAST)
    return m_cec->IsActiveSource(iAddress);
  return false;
}

cec_power_status CLibCEC::GetDevicePowerStatus(cec_logical_address iAddress)
{
  if (m_cec && iAddress >= CECDEVICE_TV && iAddress < CECDEVICE_BROADCAST)
    return m_cec->GetDevicePowerStatus(iAddress);
  return CEC_POWER_STATUS_UNKNOWN;
}

bool CLibCEC::PollDevice(cec_logical_address iAddress)
{
  if (m_cec && iAddress >= CECDEVICE_TV && iAddress < CECDEVICE_BROADCAST)
    return m_cec->PollDevice(iAddress);
  return false;
}

cec_logical_addresses CLibCEC::GetActiveDevices(void)
{
  cec_logical_addresses addresses;
  addresses.Clear();
  if (m_cec)
    addresses = m_cec->GetActiveDevices();
  return addresses;
}

bool CLibCEC::IsActiveDevice(cec_logical_address iAddress)
{
  if (m_cec && iAddress >= CECDEVICE_TV && iAddress < CECDEVICE_BROADCAST)
    return m_cec->IsPresentDevice(iAddress);
  return false;
}

bool CLibCEC::IsActiveDeviceType(cec_device_type type)
{
  if (m_cec && type >= CEC_DEVICE_TYPE_TV && type <= CEC_DEVICE_TYPE_AUDIO_SYSTEM)
    return m_cec->IsPresentDeviceType(type);
  return false;
}

uint8_t CLibCEC::VolumeUp(bool bSendRelease /* = true */)
{
  if (m_cec)
    return m_cec->VolumeUp(bSendRelease);
  return 0;
}

uint8_t CLibCEC::VolumeDown(bool bSendRelease /* = true */)
{
  if (m_cec)
    return m_cec->VolumeDown(bSendRelease);
  return 0;
}


uint8_t CLibCEC::MuteAudio(bool bSendRelease /* = true */)
{
  if (m_cec)
    return m_cec->MuteAudio(bSendRelease);
  return 0;
}

bool CLibCEC::SendKeypress(cec_logical_address iDestination, cec_user_control_code key, bool bWait /* = true */)
{
  if (m_cec)
    return m_cec->TransmitKeypress(iDestination, key, bWait);
  return false;
}

bool CLibCEC::SendKeyRelease(cec_logical_address iDestination, bool bWait /* = true */)
{
  if (m_cec)
    return m_cec->TransmitKeyRelease(iDestination, bWait);
  return false;
}

cec_osd_name CLibCEC::GetDeviceOSDName(cec_logical_address iAddress)
{
  cec_osd_name retVal;
  retVal.device = iAddress;
  retVal.name[0] = 0;

  if (m_cec)
    retVal = m_cec->GetDeviceOSDName(iAddress);

  return retVal;
}

void CLibCEC::AddLog(const cec_log_level level, const char *strFormat, ...)
{
  CStdString strLog;

  va_list argList;
  va_start(argList, strFormat);
  strLog.FormatV(strFormat, argList);
  va_end(argList);

  CLibCEC *instance = CLibCEC::GetInstance();
  if (!instance)
    return;
  CLockObject lock(instance->m_logMutex);

  cec_log_message message;
  message.level = level;
  message.time = GetTimeMs() - instance->m_iStartTime;
  snprintf(message.message, sizeof(message.message), "%s", strLog.c_str());

  if (instance->m_callbacks && instance->m_callbacks->CBCecLogMessage)
    instance->m_callbacks->CBCecLogMessage(instance->m_cbParam, message);
  else
    instance->m_logBuffer.Push(message);
}

void CLibCEC::AddKey(const cec_keypress &key)
{
  CLibCEC *instance = CLibCEC::GetInstance();
  if (!instance)
    return;
  CLockObject lock(instance->m_mutex);

  AddLog(CEC_LOG_DEBUG, "key pressed: %1x", key.keycode);

  if (instance->m_callbacks && instance->m_callbacks->CBCecKeyPress)
    instance->m_callbacks->CBCecKeyPress(instance->m_cbParam, key);
  else
    instance->m_keyBuffer.Push(key);

  instance->m_iCurrentButton = key.duration > 0 ? CEC_USER_CONTROL_CODE_UNKNOWN : key.keycode;
  instance->m_buttontime = key.duration > 0 ? 0 : GetTimeMs();
}

void CLibCEC::ConfigurationChanged(const libcec_configuration &config)
{
  CLibCEC *instance = CLibCEC::GetInstance();
  CLockObject lock(instance->m_mutex);

  if (instance->m_callbacks &&
      config.clientVersion >= CEC_CLIENT_VERSION_1_5_0 &&
      instance->m_callbacks->CBCecConfigurationChanged &&
      instance->m_cec->IsInitialised())
    instance->m_callbacks->CBCecConfigurationChanged(instance->m_cbParam, config);
}

void CLibCEC::SetCurrentButton(cec_user_control_code iButtonCode)
{
  /* push keypress to the keybuffer with 0 duration.
     push another press to the keybuffer with the duration set when the button is released */
  cec_keypress key;
  key.duration = 0;
  key.keycode = iButtonCode;

  AddKey(key);
}

void CLibCEC::AddKey(void)
{
  CLibCEC *instance = CLibCEC::GetInstance();
  if (!instance)
    return;
  CLockObject lock(instance->m_mutex);

  if (instance->m_iCurrentButton != CEC_USER_CONTROL_CODE_UNKNOWN)
  {
    cec_keypress key;

    key.duration = (unsigned int) (GetTimeMs() - instance->m_buttontime);
    key.keycode = instance->m_iCurrentButton;
    AddLog(CEC_LOG_DEBUG, "key released: %1x", key.keycode);

    if (instance->m_callbacks && instance->m_callbacks->CBCecKeyPress)
      instance->m_callbacks->CBCecKeyPress(instance->m_cbParam, key);
    else
      instance->m_keyBuffer.Push(key);
    instance->m_iCurrentButton = CEC_USER_CONTROL_CODE_UNKNOWN;
  }
  instance->m_buttontime = 0;
}

void CLibCEC::AddCommand(const cec_command &command)
{
  CLibCEC *instance = CLibCEC::GetInstance();
  if (!instance)
    return;
  CLockObject lock(instance->m_mutex);

  AddLog(CEC_LOG_NOTICE, ">> %s (%X) -> %s (%X): %s (%2X)", instance->m_cec->ToString(command.initiator), command.initiator, instance->m_cec->ToString(command.destination), command.destination, instance->m_cec->ToString(command.opcode), command.opcode);

  if (instance->m_callbacks && instance->m_callbacks->CBCecCommand)
    instance->m_callbacks->CBCecCommand(instance->m_cbParam, command);
  else if (!instance->m_commandBuffer.Push(command))
    AddLog(CEC_LOG_WARNING, "command buffer is full");
}

void CLibCEC::Alert(const libcec_alert type, const libcec_parameter &param)
{
  CLibCEC *instance = CLibCEC::GetInstance();
  if (!instance)
    return;
  CLockObject lock(instance->m_mutex);

  libcec_configuration config;
  instance->GetCurrentConfiguration(&config);

  if (instance->m_callbacks &&
      config.clientVersion >= CEC_CLIENT_VERSION_1_6_0 &&
      instance->m_cec->IsInitialised() &&
      instance->m_callbacks->CBCecAlert)
    instance->m_callbacks->CBCecAlert(instance->m_cbParam, type, param);

  if (type == CEC_ALERT_CONNECTION_LOST)
    instance->Close();
}

void CLibCEC::CheckKeypressTimeout(void)
{
  if (m_iCurrentButton != CEC_USER_CONTROL_CODE_UNKNOWN && GetTimeMs() - m_buttontime > CEC_BUTTON_TIMEOUT)
  {
    AddKey();
    m_iCurrentButton = CEC_USER_CONTROL_CODE_UNKNOWN;
  }
}

int CLibCEC::MenuStateChanged(const cec_menu_state newState)
{
  int iReturn(0);

  CLibCEC *instance = CLibCEC::GetInstance();
  if (!instance)
    return iReturn;
  CLockObject lock(instance->m_mutex);

  AddLog(CEC_LOG_NOTICE, ">> %s: %s", instance->m_cec->ToString(CEC_OPCODE_MENU_REQUEST), instance->m_cec->ToString(newState));

  libcec_configuration config;
  instance->GetCurrentConfiguration(&config);

  if (instance->m_callbacks &&
      config.clientVersion >= CEC_CLIENT_VERSION_1_6_2 &&
      instance->m_callbacks->CBCecMenuStateChanged)
    iReturn = instance->m_callbacks->CBCecMenuStateChanged(instance->m_cbParam, newState);

  return iReturn;
}

bool CLibCEC::SetStreamPath(cec_logical_address iAddress)
{
  uint16_t iPhysicalAddress = GetDevicePhysicalAddress(iAddress);
  if (iPhysicalAddress != CEC_INVALID_PHYSICAL_ADDRESS)
    return SetStreamPath(iPhysicalAddress);
  return false;
}

bool CLibCEC::SetStreamPath(uint16_t iPhysicalAddress)
{
  return m_cec->SetStreamPath(iPhysicalAddress);
}

cec_logical_addresses CLibCEC::GetLogicalAddresses(void)
{
  cec_logical_addresses addr = m_cec->GetLogicalAddresses();
  return addr;
}

static CLibCEC *g_libCEC_instance(NULL);
CLibCEC *CLibCEC::GetInstance(void)
{
  return g_libCEC_instance;
}

void CLibCEC::SetInstance(CLibCEC *instance)
{
  if (g_libCEC_instance)
    delete g_libCEC_instance;
  g_libCEC_instance = instance;
}

void * CECInit(const char *strDeviceName, CEC::cec_device_type_list types, uint16_t UNUSED(iPhysicalAddress) /* = 0 */)
{
  CLibCEC *lib = new CLibCEC(strDeviceName, types);
  CLibCEC::SetInstance(lib);
  return static_cast< void* > (lib);
}

void * CECInitialise(libcec_configuration *configuration)
{
  CLibCEC *lib = new CLibCEC(configuration);
  CLibCEC::SetInstance(lib);
  lib->GetCurrentConfiguration(configuration);
  return static_cast< void* > (lib);
}

bool CECStartBootloader(void)
{
  bool bReturn(false);
  cec_adapter deviceList[1];
  if (CUSBCECAdapterDetection::FindAdapters(deviceList, 1) > 0)
  {
    CUSBCECAdapterCommunication comm(NULL, deviceList[0].comm);
    CTimeout timeout(CEC_DEFAULT_CONNECT_TIMEOUT);
    while (timeout.TimeLeft() > 0 && (bReturn = comm.Open(timeout.TimeLeft() / CEC_CONNECT_TRIES, true)) == false)
    {
      comm.Close();
      CEvent::Sleep(500);
    }
    if (comm.IsOpen())
      bReturn = comm.StartBootloader();
  }

  return bReturn;
}

void CECDestroy(CEC::ICECAdapter *UNUSED(instance))
{
  CLibCEC::SetInstance(NULL);
}

const char *CLibCEC::ToString(const cec_menu_state state)
{
  return m_cec->ToString(state);
}

const char *CLibCEC::ToString(const cec_version version)
{
  return m_cec->ToString(version);
}

const char *CLibCEC::ToString(const cec_power_status status)
{
  return m_cec->ToString(status);
}

const char *CLibCEC::ToString(const cec_logical_address address)
{
  return m_cec->ToString(address);
}

const char *CLibCEC::ToString(const cec_deck_control_mode mode)
{
  return m_cec->ToString(mode);
}

const char *CLibCEC::ToString(const cec_deck_info status)
{
  return m_cec->ToString(status);
}

const char *CLibCEC::ToString(const cec_opcode opcode)
{
  return m_cec->ToString(opcode);
}

const char *CLibCEC::ToString(const cec_system_audio_status mode)
{
  return m_cec->ToString(mode);
}

const char *CLibCEC::ToString(const cec_audio_status status)
{
  return m_cec->ToString(status);
}

const char *CLibCEC::ToString(const cec_vendor_id vendor)
{
  return m_cec->ToString(vendor);
}

const char *CLibCEC::ToString(const cec_client_version version)
{
  return m_cec->ToString(version);
}

const char *CLibCEC::ToString(const cec_server_version version)
{
  return m_cec->ToString(version);
}

const char *CLibCEC::ToString(const cec_device_type type)
{
  return m_cec->ToString(type);
}

bool CLibCEC::GetCurrentConfiguration(libcec_configuration *configuration)
{
  return m_cec->GetCurrentConfiguration(configuration);
}

bool CLibCEC::SetConfiguration(const libcec_configuration *configuration)
{
  return m_cec->SetConfiguration(configuration);
}

bool CLibCEC::CanPersistConfiguration(void)
{
  return m_cec->CanPersistConfiguration();
}

bool CLibCEC::PersistConfiguration(libcec_configuration *configuration)
{
  return m_cec->PersistConfiguration(configuration);
}

void CLibCEC::RescanActiveDevices(void)
{
  return m_cec->RescanActiveDevices();
}

bool CLibCEC::IsLibCECActiveSource(void)
{
  bool bReturn(false);
  if (m_cec)
  {
    cec_logical_address activeSource = m_cec->GetActiveSource();
    if (activeSource != CECDEVICE_UNKNOWN)
      bReturn = m_cec->m_busDevices[activeSource]->GetStatus(false) == CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC;
  }
  return bReturn;
}

cec_device_type CLibCEC::GetType(cec_logical_address address)
{
  switch (address)
  {
    case CECDEVICE_AUDIOSYSTEM:
      return CEC_DEVICE_TYPE_AUDIO_SYSTEM;
    case CECDEVICE_PLAYBACKDEVICE1:
    case CECDEVICE_PLAYBACKDEVICE2:
    case CECDEVICE_PLAYBACKDEVICE3:
      return CEC_DEVICE_TYPE_PLAYBACK_DEVICE;
    case CECDEVICE_RECORDINGDEVICE1:
    case CECDEVICE_RECORDINGDEVICE2:
    case CECDEVICE_RECORDINGDEVICE3:
      return CEC_DEVICE_TYPE_RECORDING_DEVICE;
    case CECDEVICE_TUNER1:
    case CECDEVICE_TUNER2:
    case CECDEVICE_TUNER3:
    case CECDEVICE_TUNER4:
      return CEC_DEVICE_TYPE_TUNER;
    case CECDEVICE_TV:
      return CEC_DEVICE_TYPE_TV;
    default:
      return CEC_DEVICE_TYPE_RESERVED;
  }
}

uint16_t CLibCEC::GetMaskForType(cec_logical_address address)
{
  return GetMaskForType(GetType(address));
}

uint16_t CLibCEC::GetMaskForType(cec_device_type type)
{
  switch (type)
  {
    case CEC_DEVICE_TYPE_AUDIO_SYSTEM:
    {
      cec_logical_addresses addr;
      addr.Clear();
      addr.Set(CECDEVICE_AUDIOSYSTEM);
      return addr.AckMask();
    }
    case CEC_DEVICE_TYPE_PLAYBACK_DEVICE:
    {
      cec_logical_addresses addr;
      addr.Clear();
      addr.Set(CECDEVICE_PLAYBACKDEVICE1);
      addr.Set(CECDEVICE_PLAYBACKDEVICE2);
      addr.Set(CECDEVICE_PLAYBACKDEVICE3);
      return addr.AckMask();
    }
    case CEC_DEVICE_TYPE_RECORDING_DEVICE:
    {
      cec_logical_addresses addr;
      addr.Clear();
      addr.Set(CECDEVICE_RECORDINGDEVICE1);
      addr.Set(CECDEVICE_RECORDINGDEVICE2);
      addr.Set(CECDEVICE_RECORDINGDEVICE3);
      return addr.AckMask();
    }
    case CEC_DEVICE_TYPE_TUNER:
    {
      cec_logical_addresses addr;
      addr.Clear();
      addr.Set(CECDEVICE_TUNER1);
      addr.Set(CECDEVICE_TUNER2);
      addr.Set(CECDEVICE_TUNER3);
      addr.Set(CECDEVICE_TUNER4);
      return addr.AckMask();
    }
    case CEC_DEVICE_TYPE_TV:
    {
      cec_logical_addresses addr;
      addr.Clear();
      addr.Set(CECDEVICE_TV);
      return addr.AckMask();
    }
    default:
      return 0;
  }
}

bool CLibCEC::GetDeviceInformation(const char *strPort, libcec_configuration *config, uint32_t iTimeoutMs /* = CEC_DEFAULT_CONNECT_TIMEOUT */)
{
  if (m_cec->IsRunning())
    return false;
  
  return m_cec->GetDeviceInformation(strPort, config, iTimeoutMs);
}
