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
extern DECLSPEC bool cec_init(const char *strDeviceName, CEC::cec_logical_address iLogicalAddress = CEC::CECDEVICE_PLAYBACKDEVICE1, int iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS);
#else
extern DECLSPEC bool cec_init(const char *strDeviceName, cec_logical_address iLogicalAddress = CECDEVICE_PLAYBACKDEVICE1, int iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS);
#endif

/*!
 * @brief Open a connection to the CEC adapter.
 * @param strPort The path to the port.
 * @param iTimeout Connection timeout in ms.
 * @return True when connected, false otherwise.
 */
extern DECLSPEC bool cec_open(const char *strPort, int iTimeout);

/*!
 * @brief Close the connection to the CEC adapter.
 * @param iTimeout Timeout in ms
 */
extern DECLSPEC bool cec_close(int iTimeout);

/*!
 * @brief Ping the CEC adapter.
 * @return True when the ping was succesful, false otherwise.
 */
extern DECLSPEC bool cec_ping(void);

/*!
 * @brief Start the bootloader of the CEC adapter.
 * @return True when the command was sent succesfully, false otherwise.
 */
extern DECLSPEC bool cec_start_bootloader(void);

/*!
 * @depcrecated Use cec_standby_devices() instead
 */
#ifdef __cplusplus
extern DECLSPEC bool cec_power_off_devices(CEC::cec_logical_address address = CEC::CECDEVICE_BROADCAST);
#else
extern DECLSPEC bool cec_power_off_devices(cec_logical_address address = CECDEVICE_BROADCAST);
#endif

/*!
 * @brief Power on the connected CEC capable devices.
 * @param address The logical address to power on.
 * @return True when the command was sent succesfully, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC bool cec_power_on_devices(CEC::cec_logical_address address = CEC::CECDEVICE_TV);
#else
extern DECLSPEC bool cec_power_on_devices(cec_logical_address address = CECDEVICE_TV);
#endif

/*!
 * @brief Put connected CEC capable devices in standby mode.
 * @brief address The logical address of the device to put in standby.
 * @return True when the command was sent succesfully, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC bool cec_standby_devices(CEC::cec_logical_address address = CEC::CECDEVICE_BROADCAST);
#else
extern DECLSPEC bool cec_standby_devices(cec_logical_address address = CECDEVICE_BROADCAST);
#endif

/*!
 * @brief Broadcast a message that notifies connected CEC capable devices that this device is the active source.
 * @return True when the command was sent succesfully, false otherwise.
 */
extern DECLSPEC bool cec_set_active_view(void);

/*!
 * @brief Broadcast a message that notifies connected CEC capable devices that this device is no longer the active source.
 * @return True when the command was sent succesfully, false otherwise.
 */
extern DECLSPEC bool cec_set_inactive_view(void);

/*!
 * @brief Get the next log message in the queue, if there is one.
 * @param message The next message.
 * @return True if a message was passed, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC bool cec_get_next_log_message(CEC::cec_log_message *message);
#else
extern DECLSPEC bool cec_get_next_log_message(cec_log_message *message);
#endif

/*!
 * @brief Get the next keypress in the queue, if there is one.
 * @param key The next keypress.
 * @return True if a key was passed, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC bool cec_get_next_keypress(CEC::cec_keypress *key);
#else
extern DECLSPEC bool cec_get_next_keypress(cec_keypress *key);
#endif

/*!
 * @brief Get the next CEC command that was received by the adapter.
 * @param action The next command.
 * @return True when a command was passed, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC bool cec_get_next_command(CEC::cec_command *command);
#else
extern DECLSPEC bool cec_get_next_command(cec_command *command);
#endif

/*!
 * @brief Transmit a frame on the CEC line.
 * @param data The frame to send.
 * @param bWaitForAck Wait for an ACK message for 1 second after this frame has been sent.
 * @return True when the data was sent and acked, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC bool cec_transmit(const CEC::cec_frame &data, bool bWaitForAck = true);
#else
extern DECLSPEC bool cec_transmit(const cec_frame &data, bool bWaitForAck = true);
#endif

/*!
 * @brief Set the logical address of the CEC adapter.
 * @param iLogicalAddress The cec adapter's logical address.
 * @return True when the logical address was set succesfully, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC bool cec_set_logical_address(CEC::cec_logical_address iLogicalAddress);
#else
extern DECLSPEC bool cec_set_logical_address(cec_logical_address myAddress, cec_logical_address targetAddress);
#endif

/*!
 * @deprecated Use cec_set_logical_address() instead.
 * @brief Set the ack mask of the CEC adapter.
 * @param iMask The cec adapter's ack mask.
 * @return True when the ack mask was sent succesfully, false otherwise.
 */
extern DECLSPEC bool cec_set_ack_mask(uint16_t iMask);

/*!
 * @return Get the minimal version of libcec that this version of libcec can interface with.
 */
extern DECLSPEC int cec_get_min_version(void);

/*!
 * @return Get the version of libcec.
 */
extern DECLSPEC int cec_get_lib_version(void);

/*!
 * @brief Try to find all connected CEC adapters. Only implemented on Linux at the moment.
 * @param deviceList The vector to store device descriptors in.
 * @param strDevicePath Optional device path. Only adds device descriptors that match the given device path.
 * @return The number of devices that were found, or -1 when an error occured.
 */
#ifdef __cplusplus
extern DECLSPEC int cec_find_devices(std::vector<CEC::cec_device> &deviceList, const char *strDevicePath = NULL);
#else
extern DECLSPEC int cec_find_devices(std::vector<cec_device> &deviceList, const char *strDevicePath = NULL);
#endif

#ifdef __cplusplus
};
#endif

#endif /* CECEXPORTS_C_H_ */
