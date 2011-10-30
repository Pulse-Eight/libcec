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

/*!
 * @brief Load the CEC adapter library.
 * @param strDeviceName How to present this device to other devices.
 * @param iLogicalAddress The logical of this device. PLAYBACKDEVICE1 by default.
 * @param iPhysicalAddress The physical address of this device. 0x1000 by default.
 * @return True when initialised, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC int cec_init(const char *strDeviceName, CEC::cec_logical_address iLogicalAddress = CEC::CECDEVICE_PLAYBACKDEVICE1, uint16_t iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS);
#else
extern DECLSPEC int cec_init(const char *strDeviceName, cec_logical_address iLogicalAddress = CECDEVICE_PLAYBACKDEVICE1, uint16_t iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS);
#endif

/*!
 * @brief Unload the CEC adapter library.
 */
extern DECLSPEC void cec_destroy(void);

/*!
 * @brief Open a connection to the CEC adapter.
 * @param strPort The path to the port.
 * @param iTimeout Connection timeout in ms.
 * @return True when connected, false otherwise.
 */
extern DECLSPEC int cec_open(const char *strPort, uint32_t iTimeout);

/*!
 * @brief Close the connection to the CEC adapter.
 */
extern DECLSPEC void cec_close(void);

/*!
 * @brief Try to find all connected CEC adapters. Only implemented on Linux at the moment.
 * @param deviceList The vector to store device descriptors in.
 * @param strDevicePath Optional device path. Only adds device descriptors that match the given device path.
 * @return The number of devices that were found, or -1 when an error occured.
 */
#ifdef __cplusplus
extern DECLSPEC int8_t cec_find_adapters(CEC::cec_adapter *deviceList, uint8_t iBufSize, const char *strDevicePath = NULL);
#else
extern DECLSPEC int8_t cec_find_adapters(cec_adapter *deviceList, uint8_t iBufSize, const char *strDevicePath = NULL);
#endif

/*!
 * @brief Ping the CEC adapter.
 * @return True when the ping was succesful, false otherwise.
 */
extern DECLSPEC int cec_ping_adapters(void);

/*!
 * @brief Start the bootloader of the CEC adapter.
 * @return True when the command was sent succesfully, false otherwise.
 */
extern DECLSPEC int cec_start_bootloader(void);

/*!
 * @return Get the minimal version of libcec that this version of libcec can interface with.
 */
extern DECLSPEC int8_t cec_get_min_version(void);

/*!
 * @return Get the version of libcec.
 */
extern DECLSPEC int8_t cec_get_lib_version(void);

/*!
 * @brief Power on the connected CEC capable devices.
 * @param address The logical address to power on.
 * @return True when the command was sent succesfully, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC int cec_power_on_devices(CEC::cec_logical_address address = CEC::CECDEVICE_TV);
#else
extern DECLSPEC int cec_power_on_devices(cec_logical_address address = CECDEVICE_TV);
#endif

/*!
 * @brief Put connected CEC capable devices in standby mode.
 * @brief address The logical address of the device to put in standby.
 * @return True when the command was sent succesfully, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC int cec_standby_devices(CEC::cec_logical_address address = CEC::CECDEVICE_BROADCAST);
#else
extern DECLSPEC int cec_standby_devices(cec_logical_address address = CECDEVICE_BROADCAST);
#endif

/*!
 * @brief Broadcast a message that notifies connected CEC capable devices that this device is the active source.
 * @return True when the command was sent succesfully, false otherwise.
 */
extern DECLSPEC int cec_set_active_view(void);

/*!
 * @brief Broadcast a message that notifies connected CEC capable devices that this device is no longer the active source.
 * @return True when the command was sent succesfully, false otherwise.
 */
extern DECLSPEC int cec_set_inactive_view(void);

/*!
 * @brief Get the next log message in the queue, if there is one.
 * @param message The next message.
 * @return True if a message was passed, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC int cec_get_next_log_message(CEC::cec_log_message *message);
#else
extern DECLSPEC int cec_get_next_log_message(cec_log_message *message);
#endif

/*!
 * @brief Get the next keypress in the queue, if there is one.
 * @param key The next keypress.
 * @return True if a key was passed, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC int cec_get_next_keypress(CEC::cec_keypress *key);
#else
extern DECLSPEC int cec_get_next_keypress(cec_keypress *key);
#endif

/*!
 * @brief Get the next CEC command that was received by the adapter.
 * @param action The next command.
 * @return True when a command was passed, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC int cec_get_next_command(CEC::cec_command *command);
#else
extern DECLSPEC int cec_get_next_command(cec_command *command);
#endif

/*!
 * @brief Transmit a frame on the CEC line.
 * @param data The frame to send.
 * @return True when the data was sent and acked, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC int cec_transmit(const CEC::cec_command &data);
#else
extern DECLSPEC int cec_transmit(const cec_command &data);
#endif

/*!
 * @brief Change the logical address of the CEC adapter.
 * @param iLogicalAddress The CEC adapter's new logical address.
 * @return True when the logical address was set successfully, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC int cec_set_logical_address(CEC::cec_logical_address iLogicalAddress = CEC::CECDEVICE_PLAYBACKDEVICE1);
#else
extern DECLSPEC int cec_set_logical_address(cec_logical_address myAddress, cec_logical_address targetAddress);
#endif

/*!
 * @brief Change the physical address (HDMI port) of the CEC adapter.
 * @param iPhysicalAddress The CEC adapter's new physical address.
 * @brief True when the physical address was set successfully, false otherwise.
 */
extern DECLSPEC int cec_set_physical_address(uint16_t iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS);

/*!
 * @brief Display a message on the device with the given logical address.
 * @param iLogicalAddres The device to display the message on.
 * @param duration The duration of the message
 * @param strMessage The message to display.
 * @return True when the command was sent, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC int cec_set_osd_string(CEC::cec_logical_address iLogicalAddress, CEC::cec_display_control duration, const char *strMessage);
#else
extern DECLSPEC int cec_set_osd_string(cec_logical_address iLogicalAddress, cec_display_control duration, const char *strMessage);
#endif

/*!
 * @brief Enable or disable monitoring mode.
 * @param bEnable True to enable, false to disable.
 * @return True when switched successfully, false otherwise.
 */
extern DECLSPEC int cec_switch_monitoring(int bEnable);

/*!
 * @brief Get the CEC version of the device with the given logical address
 * @param iLogicalAddress The device to get the CEC version for.
 * @return The version or CEC_VERSION_UNKNOWN when the version couldn't be fetched.
 */
#ifdef __cplusplus
extern DECLSPEC CEC::cec_version cec_get_device_cec_version(CEC::cec_logical_address iLogicalAddress);
#else
extern DECLSPEC cec_version cec_get_device_cec_version(cec_logical_address iLogicalAddress);
#endif

/*!
 * @brief Get the menu language of the device with the given logical address
 * @param iLogicalAddress The device to get the menu language for.
 * @param language The requested menu language.
 * @return True when fetched succesfully, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC int cec_get_device_menu_language(CEC::cec_logical_address iLogicalAddress, CEC::cec_menu_language *language);
#else
extern DECLSPEC cec_version cec_get_device_menu_language(cec_logical_address iLogicalAddress, cec_menu_language *language);
#endif
#ifdef __cplusplus
};
#endif

#endif /* CECEXPORTS_C_H_ */
