/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2015 Pulse-Eight Limited.  All rights reserved.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
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

#include "env.h"
#include "cec.h"
#include "cecc.h"
#include "LibCEC.h"
#include "CECTypeUtils.h"
#include <algorithm>

using namespace CEC;

/*!
 * C interface implementation
 */
//@{

libcec_connection_t libcec_initialise(libcec_configuration* configuration)
{
  return (ICECAdapter*) CECInitialise(configuration);
}

void libcec_destroy(libcec_connection_t connection)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  if (adapter)
  {
    libcec_close(connection);
    CECDestroy(adapter);
  }
}

int libcec_open(libcec_connection_t connection, const char* strPort, uint32_t iTimeout)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter && adapter->Open(strPort, iTimeout);
}

void libcec_close(libcec_connection_t connection)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  if (adapter)
    adapter->Close();
}

void libcec_clear_configuration(libcec_configuration* configuration)
{
  if (configuration)
    configuration->Clear();
}

int libcec_enable_callbacks(libcec_connection_t connection, void* cbParam, ICECCallbacks* callbacks)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  if (adapter)
    return adapter->EnableCallbacks(cbParam, callbacks) ? 1 : 0;
  return -1;
}

int8_t libcec_find_adapters(libcec_connection_t connection, cec_adapter* deviceList, uint8_t iBufSize, const char* strDevicePath)
{
  //TODO change to use DetectAdapters()
  CLibCEC* adapter = static_cast<CLibCEC*>(connection);
  return adapter ?
      adapter->FindAdapters(deviceList, iBufSize, strDevicePath) :
      -1;
}

int libcec_ping_adapters(libcec_connection_t connection)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
    (adapter->PingAdapter() ? 1 : 0) :
    -1;
}

int libcec_start_bootloader(libcec_connection_t connection)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->StartBootloader() ? 1 : 0) :
      -1;
}

int libcec_transmit(libcec_connection_t connection, const CEC::cec_command* data)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->Transmit(*data) ? 1 : 0) :
      -1;
}

int libcec_set_logical_address(libcec_connection_t connection, cec_logical_address iLogicalAddress)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->SetLogicalAddress(iLogicalAddress) ? 1 : 0) :
      -1;
}

int libcec_set_physical_address(libcec_connection_t connection, uint16_t iPhysicalAddress)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->SetPhysicalAddress(iPhysicalAddress) ? 1 : 0) :
      -1;
}

int libcec_power_on_devices(libcec_connection_t connection, cec_logical_address address)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->PowerOnDevices(address) ? 1 : 0) :
      -1;
}

int libcec_standby_devices(libcec_connection_t connection, cec_logical_address address)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->StandbyDevices(address) ? 1 : 0) :
      -1;
}

int libcec_set_active_source(libcec_connection_t connection, cec_device_type type)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->SetActiveSource(type) ? 1 : 0) :
      -1;
}

int libcec_set_deck_control_mode(libcec_connection_t connection, cec_deck_control_mode mode, int bSendUpdate) {
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->SetDeckControlMode(mode, bSendUpdate == 1) ? 1 : 0) :
      -1;
}

int libcec_set_deck_info(libcec_connection_t connection, cec_deck_info info, int bSendUpdate) {
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->SetDeckInfo(info, bSendUpdate == 1) ? 1 : 0) :
      -1;

}

int libcec_set_inactive_view(libcec_connection_t connection)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->SetInactiveView() ? 1 : 0) :
      -1;
}

int libcec_set_menu_state(libcec_connection_t connection, cec_menu_state state, int bSendUpdate) {
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->SetMenuState(state, bSendUpdate == 1) ? 1 : 0) :
      -1;
}

int libcec_set_osd_string(libcec_connection_t connection, cec_logical_address iLogicalAddress, cec_display_control duration, const char* strMessage)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->SetOSDString(iLogicalAddress, duration, strMessage) ? 1 : 0) :
      -1;
}

int libcec_switch_monitoring(libcec_connection_t connection, int bEnable)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->SwitchMonitoring(bEnable == 1) ? 1 : 0) :
      -1;
}

cec_version libcec_get_device_cec_version(libcec_connection_t connection, cec_logical_address iLogicalAddress)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      adapter->GetDeviceCecVersion(iLogicalAddress) :
      CEC_VERSION_UNKNOWN;
}

int libcec_get_device_menu_language(libcec_connection_t connection, cec_logical_address iLogicalAddress, cec_menu_language language)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  if (!!adapter)
  {
    std::string menuLang(adapter->GetDeviceMenuLanguage(iLogicalAddress));
    strncpy(language, menuLang.c_str(), 4);
    return 0;
  }
  return -1;
}

uint32_t libcec_get_device_vendor_id(libcec_connection_t connection, cec_logical_address iLogicalAddress)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      adapter->GetDeviceVendorId(iLogicalAddress) :
      0;
}

uint16_t libcec_get_device_physical_address(libcec_connection_t connection, cec_logical_address iLogicalAddress)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      adapter->GetDevicePhysicalAddress(iLogicalAddress) :
      0;
}

cec_logical_address libcec_get_active_source(libcec_connection_t connection)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      adapter->GetActiveSource() :
      CECDEVICE_UNKNOWN;
}

int libcec_is_active_source(libcec_connection_t connection, cec_logical_address iAddress)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      adapter->IsActiveSource(iAddress) :
      0;
}

cec_power_status libcec_get_device_power_status(libcec_connection_t connection, cec_logical_address iLogicalAddress)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      adapter->GetDevicePowerStatus(iLogicalAddress) :
      CEC_POWER_STATUS_UNKNOWN;
}

int libcec_poll_device(libcec_connection_t connection, cec_logical_address iLogicalAddress)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->PollDevice(iLogicalAddress) ? 1 : 0) :
      -1;
}

cec_logical_addresses libcec_get_active_devices(libcec_connection_t connection)
{
  cec_logical_addresses addresses;
  addresses.Clear();
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  if (adapter)
    addresses = adapter->GetActiveDevices();
  return addresses;
}

int libcec_is_active_device(libcec_connection_t connection, cec_logical_address iAddress)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->IsActiveDevice(iAddress) ? 1 : 0) :
      -1;
}

int libcec_is_active_device_type(libcec_connection_t connection, cec_device_type type)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->IsActiveDeviceType(type) ? 1 : 0) :
      -1;
}

int libcec_set_hdmi_port(libcec_connection_t connection, cec_logical_address iBaseDevice, uint8_t iPort)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->SetHDMIPort(iBaseDevice, iPort) ? 1 : 0) :
      -1;
}

int libcec_volume_up(libcec_connection_t connection, int bSendRelease)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      adapter->VolumeUp(bSendRelease == 1) :
      -1;
}

int libcec_volume_down(libcec_connection_t connection, int bSendRelease)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      adapter->VolumeDown(bSendRelease == 1) :
      -1;
}

int libcec_mute_audio(libcec_connection_t connection, int UNUSED(bSendRelease))
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      adapter->AudioToggleMute() :
      -1;
}

int libcec_send_keypress(libcec_connection_t connection, cec_logical_address iDestination, cec_user_control_code key, int bWait)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->SendKeypress(iDestination, key, bWait == 1) ? 1 : 0) :
      -1;
}

int libcec_send_key_release(libcec_connection_t connection, cec_logical_address iDestination, int bWait)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->SendKeyRelease(iDestination, bWait == 1) ? 1 : 0) :
      -1;
}

int libcec_get_device_osd_name(libcec_connection_t connection, cec_logical_address iAddress, cec_osd_name name)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  if (!!adapter)
  {
    std::string osdName(adapter->GetDeviceOSDName(iAddress));
    strncpy(name, osdName.c_str(), std::min(sizeof(cec_osd_name), osdName.size()));
    return 0;
  }
  return -1;
}

int libcec_set_stream_path_logical(libcec_connection_t connection, cec_logical_address iAddress)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->SetStreamPath(iAddress) ? 1 : 0) :
      -1;
}

int libcec_set_stream_path_physical(libcec_connection_t connection, uint16_t iPhysicalAddress)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->SetStreamPath(iPhysicalAddress) ? 1 : 0) :
      -1;
}

cec_logical_addresses libcec_get_logical_addresses(libcec_connection_t connection)
{
  cec_logical_addresses addr;
  addr.Clear();
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  if (adapter)
    addr = adapter->GetLogicalAddresses();
  return addr;
}

int libcec_get_current_configuration(libcec_connection_t connection, libcec_configuration* configuration)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->GetCurrentConfiguration(configuration) ? 1 : 0) :
      -1;
}

int libcec_can_persist_configuration(libcec_connection_t connection)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->CanPersistConfiguration() ? 1 : 0) :
      -1;
}

int libcec_persist_configuration(libcec_connection_t connection, libcec_configuration* configuration)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->PersistConfiguration(configuration) ? 1 : 0) :
      -1;
}

int libcec_set_configuration(libcec_connection_t connection, libcec_configuration* configuration)
{
  return libcec_set_configuration(connection, static_cast<const libcec_configuration*>(configuration));
}

int libcec_set_configuration(libcec_connection_t connection, const libcec_configuration* configuration)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->SetConfiguration(configuration) ? 1 : 0) :
      -1;
}

void libcec_rescan_devices(libcec_connection_t connection)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  if (adapter)
    adapter->RescanActiveDevices();
}

int libcec_is_libcec_active_source(libcec_connection_t connection)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->IsLibCECActiveSource() ? 1 : 0) :
      -1;
}

int libcec_get_device_information(libcec_connection_t connection, const char* strPort, CEC::libcec_configuration* config, uint32_t iTimeoutMs)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      (adapter->GetDeviceInformation(strPort, config, iTimeoutMs) ? 1 : 0) :
      -1;
}

const char* libcec_get_lib_info(libcec_connection_t connection)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      adapter->GetLibInfo() :
      NULL;
}

void libcec_init_video_standalone(libcec_connection_t connection)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  if (adapter)
      adapter->InitVideoStandalone();
}

uint16_t libcec_get_adapter_vendor_id(libcec_connection_t connection)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      adapter->GetAdapterVendorId() :
      0;
}

uint16_t libcec_get_adapter_product_id(libcec_connection_t connection)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      adapter->GetAdapterProductId() :
      0;
}

uint8_t libcec_audio_toggle_mute(libcec_connection_t connection)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      adapter->AudioToggleMute() :
      (uint8_t)CEC_AUDIO_VOLUME_STATUS_UNKNOWN;
}

uint8_t libcec_audio_mute(libcec_connection_t connection)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      adapter->AudioMute() :
      (uint8_t)CEC_AUDIO_VOLUME_STATUS_UNKNOWN;
}

uint8_t libcec_audio_unmute(libcec_connection_t connection)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      adapter->AudioUnmute() :
      (uint8_t)CEC_AUDIO_VOLUME_STATUS_UNKNOWN;
}

uint8_t libcec_audio_get_status(libcec_connection_t connection)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      adapter->AudioStatus() :
      (uint8_t)CEC_AUDIO_VOLUME_STATUS_UNKNOWN;
}

int8_t libcec_detect_adapters(libcec_connection_t connection, cec_adapter_descriptor* deviceList, uint8_t iBufSize, const char* strDevicePath, int bQuickScan)
{
  ICECAdapter* adapter = static_cast<ICECAdapter*>(connection);
  return adapter ?
      adapter->DetectAdapters(deviceList, iBufSize, strDevicePath, bQuickScan == 1) :
      -1;
}

void libcec_menu_state_to_string(const CEC_NAMESPACE cec_menu_state state, char* buf, size_t bufsize)
{
  std::string strBuf(CCECTypeUtils::ToString(state));
  strncpy(buf, strBuf.c_str(), bufsize);
}

void libcec_cec_version_to_string(const CEC_NAMESPACE cec_version version, char* buf, size_t bufsize)
{
  std::string strBuf(CCECTypeUtils::ToString(version));
  strncpy(buf, strBuf.c_str(), bufsize);
}

void libcec_power_status_to_string(const CEC_NAMESPACE cec_power_status status, char* buf, size_t bufsize)
{
  std::string strBuf(CCECTypeUtils::ToString(status));
  strncpy(buf, strBuf.c_str(), bufsize);
}

void libcec_logical_address_to_string(const CEC_NAMESPACE cec_logical_address address, char* buf, size_t bufsize)
{
  std::string strBuf(CCECTypeUtils::ToString(address));
  strncpy(buf, strBuf.c_str(), bufsize);
}

void libcec_deck_control_mode_to_string(const CEC_NAMESPACE cec_deck_control_mode mode, char* buf, size_t bufsize)
{
  std::string strBuf(CCECTypeUtils::ToString(mode));
  strncpy(buf, strBuf.c_str(), bufsize);
}

void libcec_deck_status_to_string(const CEC_NAMESPACE cec_deck_info status, char* buf, size_t bufsize)
{
  std::string strBuf(CCECTypeUtils::ToString(status));
  strncpy(buf, strBuf.c_str(), bufsize);
}

void libcec_opcode_to_string(const CEC_NAMESPACE cec_opcode opcode, char* buf, size_t bufsize)
{
  std::string strBuf(CCECTypeUtils::ToString(opcode));
  strncpy(buf, strBuf.c_str(), bufsize);
}

void libcec_system_audio_status_to_string(const CEC_NAMESPACE cec_system_audio_status mode, char* buf, size_t bufsize)
{
  std::string strBuf(CCECTypeUtils::ToString(mode));
  strncpy(buf, strBuf.c_str(), bufsize);
}

void libcec_audio_status_to_string(const CEC_NAMESPACE cec_audio_status status, char* buf, size_t bufsize)
{
  std::string strBuf(CCECTypeUtils::ToString(status));
  strncpy(buf, strBuf.c_str(), bufsize);
}

void libcec_vendor_id_to_string(const CEC_NAMESPACE cec_vendor_id vendor, char* buf, size_t bufsize)
{
  std::string strBuf(CCECTypeUtils::ToString(vendor));
  strncpy(buf, strBuf.c_str(), bufsize);
}

void libcec_user_control_key_to_string(const CEC_NAMESPACE cec_user_control_code key, char* buf, size_t bufsize)
{
  std::string strBuf(CCECTypeUtils::ToString(key));
  strncpy(buf, strBuf.c_str(), bufsize);
}

void libcec_adapter_type_to_string(const CEC_NAMESPACE cec_adapter_type type, char* buf, size_t bufsize)
{
  std::string strBuf(CCECTypeUtils::ToString(type));
  strncpy(buf, strBuf.c_str(), bufsize);
}

void libcec_version_to_string(uint32_t version, char* buf, size_t bufsize)
{
  std::string strBuf(CCECTypeUtils::VersionToString(version));
  strncpy(buf, strBuf.c_str(), bufsize);
}

//@}
