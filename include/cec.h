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

#ifndef CECEXPORTS_H_
#define CECEXPORTS_H_

#include <cectypes.h>

namespace CEC
{
  class ICECAdapter
  {
  public:
    virtual ~ICECAdapter() {};
    /*! @name Adapter methods */
    //@{

    /*!
     * @brief Open a connection to the CEC adapter.
     * @param strPort The path to the port.
     * @param iTimeout Connection timeout in ms.
     * @return True when connected, false otherwise.
     */
    virtual bool Open(const char *strPort, uint32_t iTimeoutMs = 10000) = 0;

    /*!
     * @brief Close the connection to the CEC adapter.
     */
    virtual void Close(void) = 0;

    /*!
     * @brief Try to find all connected CEC adapters. Only implemented on Linux at the moment.
     * @param deviceList The vector to store device descriptors in.
     * @param iBufSize The size of the deviceList buffer.
     * @param strDevicePath Optional device path. Only adds device descriptors that match the given device path.
     * @return The number of devices that were found, or -1 when an error occured.
     */
    virtual int8_t FindAdapters(cec_adapter *deviceList, uint8_t iBufSize, const char *strDevicePath = NULL) = 0;

    /*!
     * @brief Ping the CEC adapter.
     * @return True when the ping was succesful, false otherwise.
     */
    virtual bool PingAdapter(void) = 0;

    /*!
     * @brief Start the bootloader of the CEC adapter.
     * @return True when the command was sent succesfully, false otherwise.
     */
    virtual bool StartBootloader(void) = 0;
    //@}

    /*!
     * @return Get the minimal version of libcec that this version of libcec can interface with.
     */
    virtual int8_t GetMinLibVersion(void) const = 0;

    /*!
     * @return Get the major version of libcec.
     */
    virtual int8_t GetLibVersionMajor(void) const = 0;

    /*!
     * @return Get the minor version of libcec.
     */
    virtual int8_t GetLibVersionMinor(void) const = 0;

    /*!
     * @brief Get the next log message in the queue, if there is one.
     * @param message The next message.
     * @return True if a message was passed, false otherwise.
     */
    virtual bool GetNextLogMessage(cec_log_message *message) = 0;

    /*!
     * @brief Get the next keypress in the queue, if there is one.
     * @param key The next keypress.
     * @return True if a key was passed, false otherwise.
     */
    virtual bool GetNextKeypress(cec_keypress *key) = 0;

    /*!
     * @brief Get the next CEC command that was received by the adapter.
     * @param action The next command.
     * @return True when a command was passed, false otherwise.
     */
    virtual bool GetNextCommand(cec_command *command) = 0;

    /*!
     * @brief Transmit a command over the CEC line.
     * @param data The command to send.
     * @return True when the data was sent and acked, false otherwise.
     */
    virtual bool Transmit(const cec_command &data) = 0;

    /*!
     * @brief Change the logical address of the CEC adapter.
     * @param iLogicalAddress The CEC adapter's new logical address.
     * @return True when the logical address was set successfully, false otherwise.
     */
    virtual bool SetLogicalAddress(cec_logical_address iLogicalAddress = CECDEVICE_PLAYBACKDEVICE1) = 0;

    /*!
     * @brief Change the physical address (HDMI port) of the CEC adapter.
     * @param iPhysicalAddress The CEC adapter's new physical address.
     * @brief True when the physical address was set successfully, false otherwise.
     */
    virtual bool SetPhysicalAddress(uint16_t iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS) = 0;

    /*!
     * @brief Power on the connected CEC capable devices.
     * @param address The logical address to power on.
     * @return True when the command was sent succesfully, false otherwise.
     */
    virtual bool PowerOnDevices(cec_logical_address address = CECDEVICE_TV) = 0;

    /*!
     * @brief Put connected CEC capable devices in standby mode.
     * @brief address The logical address of the device to put in standby.
     * @return True when the command was sent succesfully, false otherwise.
     */
    virtual bool StandbyDevices(cec_logical_address address = CECDEVICE_BROADCAST) = 0;

    /*!
     * @brief Change the active source.
     * @param type The new active source. Leave empty to use the primary type
     * @return True when the command was sent succesfully, false otherwise.
     */
    virtual bool SetActiveSource(cec_device_type type = CEC_DEVICE_TYPE_RESERVED) = 0;

    /*!
     * @deprecated Use SetActiveSource() instead
     */
    virtual bool SetActiveView(void) = 0;

    /*!
     * @brief Change the deck control mode, if this adapter is registered as playback device.
     * @param mode The new control mode.
     * @param bSendUpdate True to send the status over the CEC line.
     * @return True if set, false otherwise.
     */
    virtual bool SetDeckControlMode(cec_deck_control_mode mode, bool bSendUpdate = true) = 0;

    /*!
     * @brief Change the deck info, if this adapter is a playback device.
     * @param info The new deck info.
     * @return True if set, false otherwise.
     */
    virtual bool SetDeckInfo(cec_deck_info info, bool bSendUpdate = true) = 0;

    /*!
     * @brief Broadcast a message that notifies connected CEC capable devices that this device is no longer the active source.
     * @return True when the command was sent succesfully, false otherwise.
     */
    virtual bool SetInactiveView(void) = 0;

    /*!
     * @brief Change the menu state.
     * @param state The new true.
     * @param bSendUpdate True to send the status over the CEC line.
     * @return True if set, false otherwise.
     */
    virtual bool SetMenuState(cec_menu_state state, bool bSendUpdate = true) = 0;

    /*!
     * @brief Display a message on the device with the given logical address.
     * @param iLogicalAddres The device to display the message on.
     * @param duration The duration of the message
     * @param strMessage The message to display.
     * @return True when the command was sent, false otherwise.
     */
    virtual bool SetOSDString(cec_logical_address iLogicalAddress, cec_display_control duration, const char *strMessage) = 0;

    /*!
     * @brief Enable or disable monitoring mode.
     * @param bEnable True to enable, false to disable.
     * @return True when switched successfully, false otherwise.
     */
    virtual bool SwitchMonitoring(bool bEnable) = 0;

    /*!
     * @brief Get the CEC version of the device with the given logical address
     * @param iLogicalAddress The device to get the CEC version for.
     * @return The version or CEC_VERSION_UNKNOWN when the version couldn't be fetched.
     */
    virtual cec_version GetDeviceCecVersion(cec_logical_address iAddress) = 0;

    /*!
     * @brief Get the menu language of the device with the given logical address
     * @param iLogicalAddress The device to get the menu language for.
     * @param language The requested menu language.
     * @return True when fetched succesfully, false otherwise.
     */
    virtual bool GetDeviceMenuLanguage(cec_logical_address iAddress, cec_menu_language *language) = 0;

    /*!
     * @brief Get the vendor ID of the device with the given logical address.
     * @param iLogicalAddress The device to get the vendor id for.
     * @return The vendor ID or 0 if it wasn't found.
     */
    virtual uint64_t GetDeviceVendorId(cec_logical_address iAddress) = 0;

    /*!
     * @brief Get the power status of the device with the given logical address.
     * @param iLogicalAddress The device to get the power status for.
     * @return The power status or CEC_POWER_STATUS_UNKNOWN if it wasn't found.
     */
    virtual cec_power_status GetDevicePowerStatus(cec_logical_address iAddress) = 0;

    /*!
     * @brief Sends a POLL message to a device.
     * @param iAddress The device to send the message to.
     * @return True if the POLL was acked, false otherwise.
     */
    virtual bool PollDevice(cec_logical_address iAddress) = 0;

    /*!
     * @return The devices that are active on the bus and not handled by libcec.
     */
    virtual cec_logical_addresses GetActiveDevices(void) = 0;

    /*!
     * @brief Check whether a device is active on the bus.
     * @param iAddress The address to check.
     * @return True when active, false otherwise.
     */
    virtual bool IsActiveDevice(cec_logical_address iAddress) = 0;

    /*!
     * @brief Check whether a device of the given type is active on the bus.
     * @param type The type to check.
     * @return True when active, false otherwise.
     */
    virtual bool IsActiveDeviceType(cec_device_type type) = 0;

    /*!
     * @brief Changes the active HDMI port.
     * @param iPort The new port number.
     * @return True when changed, false otherwise.
     */
    virtual bool SetHDMIPort(uint8_t iPort) = 0;

    /*!
     * @brief Sends a volume up keypress to an audiosystem if it's present.
     * @return The new audio status.
     */
    virtual uint8_t VolumeUp(void) = 0;

    /*!
     * @brief Sends a volume down keypress to an audiosystem if it's present.
     * @return The new audio status.
     */
    virtual uint8_t VolumeDown(void) = 0;

    /*!
     * @brief Sends a mute keypress to an audiosystem if it's present.
     * @return The new audio status.
     */
    virtual uint8_t MuteAudio(void) = 0;
  };
};

/*!
 * @brief Load the CEC adapter library.
 * @param strDeviceName How to present this device to other devices.
 * @param deviceTypes The device types to use on the CEC bus.
 * @return An instance of ICECAdapter or NULL on error.
 */
extern "C" DECLSPEC void * CECInit(const char *strDeviceName, CEC::cec_device_type_list devicesTypes);

/*!
 * @deprecated Please use CECInit() instead
 * @brief Load the CEC adapter library.
 * @param strDeviceName How to present this device to other devices.
 * @param iLogicalAddress The logical of this device. PLAYBACKDEVICE1 by default.
 * @param iPhysicalAddress The physical address of this device. 0x1000 by default.
 * @return An instance of ICECAdapter or NULL on error.
 */
extern "C" DECLSPEC void * CECCreate(const char *strDeviceName, CEC::cec_logical_address iLogicalAddress = CEC::CECDEVICE_PLAYBACKDEVICE1, uint16_t iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS);

/*!
 * @brief Unload the CEC adapter library.
 */
extern "C" DECLSPEC void CECDestroy(CEC::ICECAdapter *instance);

#endif /* CECEXPORTS_H_ */
