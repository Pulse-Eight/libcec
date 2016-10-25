#pragma once
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

#ifndef CECEXPORTS_C_H_
#define CECEXPORTS_C_H_

#include "cectypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
#define CEC_NAMESPACE CEC::
typedef CEC::ICECAdapter* libcec_connection_t;
#else
#define CEC_NAMESPACE
typedef void* libcec_connection_t;
#endif

extern DECLSPEC libcec_connection_t libcec_initialise(CEC_NAMESPACE libcec_configuration* configuration);
extern DECLSPEC void libcec_destroy(libcec_connection_t connection);
extern DECLSPEC int libcec_open(libcec_connection_t connection, const char* strPort, uint32_t iTimeout);
extern DECLSPEC void libcec_close(libcec_connection_t connection);
extern DECLSPEC void libcec_clear_configuration(CEC_NAMESPACE libcec_configuration* configuration);
extern DECLSPEC int libcec_enable_callbacks(libcec_connection_t connection, void* cbParam, CEC_NAMESPACE ICECCallbacks* callbacks);
extern DECLSPEC int8_t libcec_find_adapters(libcec_connection_t connection, CEC_NAMESPACE cec_adapter* deviceList, uint8_t iBufSize, const char* strDevicePath);
extern DECLSPEC int libcec_ping_adapters(libcec_connection_t connection);
extern DECLSPEC int libcec_start_bootloader(libcec_connection_t connection);
extern DECLSPEC int libcec_power_on_devices(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address address);
extern DECLSPEC int libcec_standby_devices(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address address);
extern DECLSPEC int libcec_set_active_source(libcec_connection_t connection, CEC_NAMESPACE cec_device_type type);
extern DECLSPEC int libcec_set_deck_control_mode(libcec_connection_t connection, CEC_NAMESPACE cec_deck_control_mode mode, int bSendUpdate);
extern DECLSPEC int libcec_set_deck_info(libcec_connection_t connection, CEC_NAMESPACE cec_deck_info info, int bSendUpdate);
extern DECLSPEC int libcec_set_inactive_view(libcec_connection_t connection);
extern DECLSPEC int libcec_set_menu_state(libcec_connection_t connection, CEC_NAMESPACE cec_menu_state state, int bSendUpdate);
extern DECLSPEC int libcec_transmit(libcec_connection_t connection, const CEC_NAMESPACE cec_command* data);
extern DECLSPEC int libcec_set_logical_address(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iLogicalAddress);
extern DECLSPEC int libcec_set_physical_address(libcec_connection_t connection, uint16_t iPhysicalAddress);
extern DECLSPEC int libcec_set_osd_string(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iLogicalAddress, CEC_NAMESPACE cec_display_control duration, const char* strMessage);
extern DECLSPEC int libcec_switch_monitoring(libcec_connection_t connection, int bEnable);
extern DECLSPEC CEC_NAMESPACE cec_version libcec_get_device_cec_version(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iLogicalAddress);
extern DECLSPEC int libcec_get_device_menu_language(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iLogicalAddress, CEC_NAMESPACE cec_menu_language language);
extern DECLSPEC uint32_t libcec_get_device_vendor_id(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iLogicalAddress);
extern DECLSPEC uint16_t libcec_get_device_physical_address(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iLogicalAddress);
extern DECLSPEC CEC_NAMESPACE cec_logical_address libcec_get_active_source(libcec_connection_t connection);
extern DECLSPEC int libcec_is_active_source(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iAddress);
extern DECLSPEC CEC_NAMESPACE cec_power_status libcec_get_device_power_status(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iLogicalAddress);
extern DECLSPEC int libcec_poll_device(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iLogicalAddress);
extern DECLSPEC CEC_NAMESPACE cec_logical_addresses libcec_get_active_devices(libcec_connection_t connection);
extern DECLSPEC int libcec_is_active_device(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address address);
extern DECLSPEC int libcec_is_active_device_type(libcec_connection_t connection, CEC_NAMESPACE cec_device_type type);
extern DECLSPEC int libcec_set_hdmi_port(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address baseDevice, uint8_t iPort);
extern DECLSPEC int libcec_volume_up(libcec_connection_t connection, int bSendRelease);
extern DECLSPEC int libcec_volume_down(libcec_connection_t connection, int bSendRelease);
extern DECLSPEC int libcec_mute_audio(libcec_connection_t connection, int bSendRelease);
extern DECLSPEC int libcec_send_keypress(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iDestination, CEC_NAMESPACE cec_user_control_code key, int bWait);
extern DECLSPEC int libcec_send_key_release(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iDestination, int bWait);
extern DECLSPEC int libcec_get_device_osd_name(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iAddress, CEC_NAMESPACE cec_osd_name name);
extern DECLSPEC int libcec_set_stream_path_logical(libcec_connection_t connection, CEC_NAMESPACE cec_logical_address iAddress);
extern DECLSPEC int libcec_set_stream_path_physical(libcec_connection_t connection, uint16_t iPhysicalAddress);
extern DECLSPEC CEC_NAMESPACE cec_logical_addresses libcec_get_logical_addresses(libcec_connection_t connection);
extern DECLSPEC int libcec_get_current_configuration(libcec_connection_t connection, CEC_NAMESPACE libcec_configuration* configuration);
extern DECLSPEC int libcec_can_persist_configuration(libcec_connection_t connection);
extern DECLSPEC int libcec_persist_configuration(libcec_connection_t connection, CEC_NAMESPACE libcec_configuration* configuration);
extern DECLSPEC int libcec_set_configuration(libcec_connection_t connection, const CEC_NAMESPACE libcec_configuration* configuration);
extern DECLSPEC void libcec_rescan_devices(libcec_connection_t connection);
extern DECLSPEC int libcec_is_libcec_active_source(libcec_connection_t connection);
extern DECLSPEC int libcec_get_device_information(libcec_connection_t connection, const char* strPort, CEC_NAMESPACE libcec_configuration* config, uint32_t iTimeoutMs);
extern DECLSPEC const char* libcec_get_lib_info(libcec_connection_t connection);
extern DECLSPEC void libcec_init_video_standalone(libcec_connection_t connection);
extern DECLSPEC uint16_t libcec_get_adapter_vendor_id(libcec_connection_t connection);
extern DECLSPEC uint16_t libcec_get_adapter_product_id(libcec_connection_t connection);
extern DECLSPEC uint8_t libcec_audio_toggle_mute(libcec_connection_t connection);
extern DECLSPEC uint8_t libcec_audio_mute(libcec_connection_t connection);
extern DECLSPEC uint8_t libcec_audio_unmute(libcec_connection_t connection);
extern DECLSPEC uint8_t libcec_audio_get_status(libcec_connection_t connection);
extern DECLSPEC int8_t libcec_detect_adapters(libcec_connection_t connection, CEC_NAMESPACE cec_adapter_descriptor* deviceList, uint8_t iBufSize, const char* strDevicePath, int bQuickScan);
#ifdef SWIG
%cstring_bounded_output(char* buf, 50);
#endif
extern DECLSPEC void libcec_menu_state_to_string(const CEC_NAMESPACE cec_menu_state state, char* buf, size_t bufsize);
extern DECLSPEC void libcec_cec_version_to_string(const CEC_NAMESPACE cec_version version, char* buf, size_t bufsize);
extern DECLSPEC void libcec_power_status_to_string(const CEC_NAMESPACE cec_power_status status, char* buf, size_t bufsize);
extern DECLSPEC void libcec_logical_address_to_string(const CEC_NAMESPACE cec_logical_address address, char* buf, size_t bufsize);
extern DECLSPEC void libcec_deck_control_mode_to_string(const CEC_NAMESPACE cec_deck_control_mode mode, char* buf, size_t bufsize);
extern DECLSPEC void libcec_deck_status_to_string(const CEC_NAMESPACE cec_deck_info status, char* buf, size_t bufsize);
extern DECLSPEC void libcec_opcode_to_string(const CEC_NAMESPACE cec_opcode opcode, char* buf, size_t bufsize);
extern DECLSPEC void libcec_system_audio_status_to_string(const CEC_NAMESPACE cec_system_audio_status mode, char* buf, size_t bufsize);
extern DECLSPEC void libcec_audio_status_to_string(const CEC_NAMESPACE cec_audio_status status, char* buf, size_t bufsize);
extern DECLSPEC void libcec_vendor_id_to_string(const CEC_NAMESPACE cec_vendor_id vendor, char* buf, size_t bufsize);
extern DECLSPEC void libcec_user_control_key_to_string(const CEC_NAMESPACE cec_user_control_code key, char* buf, size_t bufsize);
extern DECLSPEC void libcec_adapter_type_to_string(const CEC_NAMESPACE cec_adapter_type type, char* buf, size_t bufsize);
extern DECLSPEC void libcec_version_to_string(uint32_t version, char* buf, size_t bufsize);

#ifdef __cplusplus
};
#endif

#endif /* CECEXPORTS_C_H_ */
