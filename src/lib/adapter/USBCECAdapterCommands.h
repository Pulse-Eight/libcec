#pragma once
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

#include "USBCECAdapterCommunication.h"

namespace CEC
{
  class CUSBCECAdapterCommands
  {
  public:
    CUSBCECAdapterCommands(CUSBCECAdapterCommunication *comm) :
      m_comm(comm),
      m_iFirmwareVersion(CEC_FW_VERSION_UNKNOWN) {}

    /*!
     * @brief Request the firmware version from the adapter.
     * @return The firmware version, or 1 (default) if it couldn't be retrieved.
     */
    uint16_t RequestFirmwareVersion(void);

    /*!
     * @brief Request a setting value from the adapter.
     * @param msgCode The setting to retrieve.
     * @return The response from the adapter.
     */
    cec_datapacket RequestSetting(cec_adapter_messagecode msgCode);

    /*!
     * @brief Change the value of the "auto enabled" setting.
     * @param enabled The new value.
     * @return True when set, false otherwise.
     */
    bool SetSettingAutoEnabled(bool enabled);

    /*!
     * @brief Request the value of the "auto enabled" setting from the adapter.
     * @param enabled The current value.
     * @return True when retrieved, false otherwise.
     */
    bool RequestSettingAutoEnabled(bool &enabled);

    /*!
     * @brief Change the value of the "device type" setting, used when the device is in autonomous mode.
     * @param type The new value.
     * @return True when set, false otherwise.
     */
    bool SetSettingDeviceType(cec_device_type type);

    /*!
     * @brief Request the value of the "device type" setting from the adapter.
     * @param type The current value.
     * @return True when retrieved, false otherwise.
     */
    bool RequestSettingDeviceType(cec_device_type &type);

    /*!
     * @brief Change the value of the "default logical address" setting, used when the device is in autonomous mode.
     * @param address The new value.
     * @return True when set, false otherwise.
     */
    bool SetSettingDefaultLogicalAddress(cec_logical_address address);

    /*!
     * @brief Request the value of the "default logical address" setting from the adapter.
     * @param address The current value.
     * @return True when retrieved, false otherwise.
     */
    bool RequestSettingDefaultLogicalAddress(cec_logical_address &address);

    /*!
     * @brief Change the value of the "logical address mask" setting, used when the device is in autonomous mode.
     * @param iMask The new value.
     * @return True when set, false otherwise.
     */
    bool SetSettingLogicalAddressMask(uint16_t iMask);

    /*!
     * @brief Request the value of the "logical address mask" setting from the adapter.
     * @param iMask The current value.
     * @return True when retrieved, false otherwise.
     */
    bool RequestSettingLogicalAddressMask(uint16_t &iMask);

    /*!
     * @brief Change the value of the "physical address" setting, used when the device is in autonomous mode.
     * @param iPhysicalAddress The new value.
     * @return True when set, false otherwise.
     */
    bool SetSettingPhysicalAddress(uint16_t iPhysicalAddress);

    /*!
     * @brief Request the value of the "physical address" setting from the adapter.
     * @param iPhysicalAddress The current value.
     * @return True when retrieved, false otherwise.
     */
    bool RequestSettingPhysicalAddress(uint16_t &iPhysicalAddress);

    /*!
     * @brief Change the value of the "CEC version" setting, used when the device is in autonomous mode.
     * @param version The new value.
     * @return True when set, false otherwise.
     */
    bool SetSettingCECVersion(cec_version version);

    /*!
     * @brief Request the value of the "CEC version" setting from the adapter.
     * @param version The current value.
     * @return True when retrieved, false otherwise.
     */
    bool RequestSettingCECVersion(cec_version &version);

    /*!
     * @brief Change the value of the "OSD name" setting, used when the device is in autonomous mode.
     * @param strOSDName The new value.
     * @return True when set, false otherwise.
     */
    bool SetSettingOSDName(const char *strOSDName);

    /*!
     * @brief Request the value of the "OSD name" setting from the adapter.
     * @param strOSDName The current value.
     * @return True when retrieved, false otherwise.
     */
    bool RequestSettingOSDName(CStdString &strOSDName);

    /*!
     * @brief Persist the current settings in the EEPROM
     * @return True when persisted, false otherwise.
     */
    bool WriteEEPROM(void);

    /*!
     * @return The firmware version of the adapter, retrieved when the connection is opened.
     */
    uint16_t GetFirmwareVersion(void) const { return m_iFirmwareVersion; };

    /*!
     * @brief Persist the current configuration in the EEPROM.
     * @attention Not all settings are persisted at this time.
     * @param configuration The configuration to persist.
     * @return True when persisted, false otherwise.
     */
    bool PersistConfiguration(libcec_configuration *configuration);

    /*!
     * @brief Get the persisted configuration from the EEPROM.
     * @param configuration The persisted configuration.
     * @return True when retrieved, false otherwise.
     */
    bool GetConfiguration(libcec_configuration *configuration);

    /*!
     * @brief Send a ping command to the adapter.
     * @return True when acked by the adapter, false otherwise.
     */
    bool PingAdapter(void);

    /*!
     * @brief Change the ackmask of the adapter.
     * @param iMask The new mask.
     * @return True when the change was acked by the adapter, false otherwise.
     */
    bool SetAckMask(uint16_t iMask);

    /*!
     * @brief Put the adapter in bootloader mode.
     * @attention The connection needs to be closed after this call, since the adapter will no longer be available.
     * @return True when the command was sent, false otherwise.
     */
    bool StartBootloader(void);

    /*!
     * @brief Change the current CEC line timeout.
     * @param iTimeout The new timeout.
     * @return True when the change was acked by the adapter, false otherwise.
     */
    bool SetLineTimeout(uint8_t iTimeout);

    /*!
     * @brief Put the adapter in controlled or autonomous mode.
     * @param controlled True to switch to controlled mode, false to switch to auto mode.
     * @return True when acked by the controller, false otherwise.
     */
    bool SetControlledMode(bool controlled);

  private:
    CUSBCECAdapterCommunication *m_comm;             /**< the communication handler */
    uint16_t                     m_iFirmwareVersion; /**< the firwmare version that was retrieved while opening the connection */
  };
}
