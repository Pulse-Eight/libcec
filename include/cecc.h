#pragma once
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

#ifndef CECEXPORTS_C_H_
#define CECEXPORTS_C_H_

#include <cectypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
extern DECLSPEC int cec_init(const char *strDeviceName, CEC::cec_logical_address iLogicalAddress = CEC::CECDEVICE_PLAYBACKDEVICE1, uint16_t iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS);
#else
extern DECLSPEC int cec_init(const char *strDeviceName, cec_logical_address iLogicalAddress = CECDEVICE_PLAYBACKDEVICE1, uint16_t iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS);
#endif

#ifdef __cplusplus
extern DECLSPEC int cec_init_typed(const char *strDeviceName, CEC::cec_device_type_list devicesTypes);
#else
extern DECLSPEC int cec_init_typed(const char *strDeviceName, cec_device_type_list devicesTypes);
#endif


extern DECLSPEC void cec_destroy(void);

extern DECLSPEC int cec_open(const char *strPort, uint32_t iTimeout);

extern DECLSPEC void cec_close(void);

#ifdef __cplusplus
extern DECLSPEC int8_t cec_find_adapters(CEC::cec_adapter *deviceList, uint8_t iBufSize, const char *strDevicePath = NULL);
#else
extern DECLSPEC int8_t cec_find_adapters(cec_adapter *deviceList, uint8_t iBufSize, const char *strDevicePath = NULL);
#endif

extern DECLSPEC int cec_ping_adapters(void);

extern DECLSPEC int cec_start_bootloader(void);

extern DECLSPEC int8_t cec_get_min_lib_version(void);

extern DECLSPEC int8_t cec_get_lib_version_major(void);

extern DECLSPEC int8_t cec_get_lib_version_minor(void);

#ifdef __cplusplus
extern DECLSPEC int cec_power_on_devices(CEC::cec_logical_address address = CEC::CECDEVICE_TV);
#else
extern DECLSPEC int cec_power_on_devices(cec_logical_address address = CECDEVICE_TV);
#endif

#ifdef __cplusplus
extern DECLSPEC int cec_standby_devices(CEC::cec_logical_address address = CEC::CECDEVICE_BROADCAST);
#else
extern DECLSPEC int cec_standby_devices(cec_logical_address address = CECDEVICE_BROADCAST);
#endif

extern DECLSPEC int cec_set_active_view(void);

#ifdef __cplusplus
extern DECLSPEC int cec_set_active_source(CEC::cec_device_type type = CEC::CEC_DEVICE_TYPE_RESERVED);
#else
extern DECLSPEC int cec_set_active_source(cec_device_type type = CEC_DEVICE_TYPE_RESERVED);
#endif

extern DECLSPEC int cec_set_inactive_view(void);

#ifdef __cplusplus
extern DECLSPEC int cec_get_next_log_message(CEC::cec_log_message *message);
#else
extern DECLSPEC int cec_get_next_log_message(cec_log_message *message);
#endif

#ifdef __cplusplus
extern DECLSPEC int cec_get_next_keypress(CEC::cec_keypress *key);
#else
extern DECLSPEC int cec_get_next_keypress(cec_keypress *key);
#endif

#ifdef __cplusplus
extern DECLSPEC int cec_get_next_command(CEC::cec_command *command);
#else
extern DECLSPEC int cec_get_next_command(cec_command *command);
#endif

#ifdef __cplusplus
extern DECLSPEC int cec_transmit(const CEC::cec_command &data);
#else
extern DECLSPEC int cec_transmit(const cec_command &data);
#endif

#ifdef __cplusplus
extern DECLSPEC int cec_set_logical_address(CEC::cec_logical_address iLogicalAddress = CEC::CECDEVICE_PLAYBACKDEVICE1);
#else
extern DECLSPEC int cec_set_logical_address(cec_logical_address myAddress, cec_logical_address targetAddress);
#endif

extern DECLSPEC int cec_set_physical_address(uint16_t iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS);

#ifdef __cplusplus
extern DECLSPEC int cec_set_osd_string(CEC::cec_logical_address iLogicalAddress, CEC::cec_display_control duration, const char *strMessage);
#else
extern DECLSPEC int cec_set_osd_string(cec_logical_address iLogicalAddress, cec_display_control duration, const char *strMessage);
#endif

extern DECLSPEC int cec_switch_monitoring(int bEnable);

#ifdef __cplusplus
extern DECLSPEC CEC::cec_version cec_get_device_cec_version(CEC::cec_logical_address iLogicalAddress);
#else
extern DECLSPEC cec_version cec_get_device_cec_version(cec_logical_address iLogicalAddress);
#endif

#ifdef __cplusplus
extern DECLSPEC int cec_get_device_menu_language(CEC::cec_logical_address iLogicalAddress, CEC::cec_menu_language *language);
#else
extern DECLSPEC int cec_get_device_menu_language(cec_logical_address iLogicalAddress, cec_menu_language *language);
#endif

#ifdef __cplusplus
extern DECLSPEC uint64_t cec_get_device_vendor_id(CEC::cec_logical_address iLogicalAddress);
#else
extern DECLSPEC uint64_t cec_get_device_vendor_id(cec_logical_address iLogicalAddress);
#endif

#ifdef __cplusplus
extern DECLSPEC CEC::cec_power_status cec_get_device_power_status(CEC::cec_logical_address iLogicalAddress);
#else
extern DECLSPEC cec_power_status cec_get_device_power_status(cec_logical_address iLogicalAddress);
#endif

#ifdef __cplusplus
extern DECLSPEC int cec_poll_device(CEC::cec_logical_address iLogicalAddress);
#else
extern DECLSPEC int cec_poll_device(cec_logical_address iLogicalAddress);
#endif

#ifdef __cplusplus
};
#endif

#endif /* CECEXPORTS_C_H_ */
