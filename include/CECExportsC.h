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
 * @brief Initialise the cec device.
 * @param strDeviceName How to present this device to other devices.
 * @return True when initialised, false otherwise.
 */
extern DECLSPEC bool cec_init(const char *strDeviceName);

/*!
 * @brief Close the cec device.
 * @return True when the device was closed, false otherwise.
 */
extern DECLSPEC bool cec_close(void);

/*!
 * @brief Open a connection to the CEC adapter.
 * @param strPort The path to the port.
 * @param iTimeout Connection timeout in ms.
 * @return True when connected, false otherwise.
 */
extern DECLSPEC bool cec_open(const char *strPort, int iTimeout);

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
 * @brief Power off connected CEC capable devices.
 * @return True when the command was sent succesfully, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC bool cec_power_off_devices(CEC::cec_logical_address address = CEC::CECDEVICE_BROADCAST);
#else
extern DECLSPEC bool cec_power_off_devices(cec_logical_address address = CECDEVICE_BROADCAST);
#endif

/*!
 * @brief Power on the connected CEC capable devices.
 * @return True when the command was sent succesfully, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC bool cec_power_on_devices(CEC::cec_logical_address address = CEC::CECDEVICE_BROADCAST);
#else
extern DECLSPEC bool cec_power_on_devices(cec_logical_address address = CECDEVICE_BROADCAST);
#endif

/*!
 * @brief Put connected CEC capable devices in standby mode.
 * @return True when the command was sent succesfully, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC bool cec_standby_devices(CEC::cec_logical_address address = CEC::CECDEVICE_BROADCAST);
#else
extern DECLSPEC bool cec_standby_devices(cec_logical_address address = CECDEVICE_BROADCAST);
#endif

/*!
 * @brief Set this device as the active source on connected CEC capable devices.
 * @return True when the command was sent succesfully, false otherwise.
 */
extern DECLSPEC bool cec_set_active_view(void);

/*!
 * @brief Mark this device as inactive on connected CEC capable devices.
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
 * @param key The next keypress
 * @return True if a key was passed, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC bool cec_get_next_keypress(CEC::cec_keypress *key);
#else
extern DECLSPEC bool cec_get_next_keypress(cec_keypress *key);
#endif

/*!
 * @brief Transmit a frame and wait for ACK.
 * @param data The frame to send.
 * @return True when the data was sent and acked, false otherwise.
 */
#ifdef __cplusplus
extern DECLSPEC bool cec_transmit(const CEC::cec_frame &data, bool bWaitForAck = true, int64_t iTimeout = (int64_t) 5000);
#else
extern DECLSPEC bool cec_transmit(const cec_frame &data, bool bWaitForAck = true, int64_t iTimeout = (int64_t) 5000);
#endif

/*!
 * @brief Set the ack mask for the CEC adapter.
 * @param ackmask The new ack mask.
 * @return True when the ack mask was sent succesfully, false otherwise.
 */
extern DECLSPEC bool cec_set_ack_mask(uint16_t ackmask);

extern DECLSPEC int cec_get_min_version(void);
extern DECLSPEC int cec_get_lib_version(void);

#ifdef __cplusplus
extern DECLSPEC int cec_find_devices(std::vector<CEC::cec_device> &deviceList, const char *strDevicePath = NULL);
#else
extern DECLSPEC int cec_find_devices(std::vector<cec_device> &deviceList, const char *strDevicePath = NULL);
#endif

#ifdef __cplusplus
};
#endif

#endif /* CECEXPORTS_C_H_ */
