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

#include "../../include/cectypes.h"
#include "platform/threads/mutex.h"
#include "platform/util/buffer.h"

#include "devices/CECBusDevice.h"

namespace CEC
{
  class CCECProcessor;

  class CCECClient
  {
  public:
    CCECClient(CCECProcessor *processor, const libcec_configuration &configuration);
    virtual ~CCECClient(void);

    // methods for registration in CCECProcessor
    bool                  OnRegister(void);
    void                  OnUnregister(void) { SetRegistered(false); SetInitialised(false); }
    bool                  IsInitialised(void);
    void                  SetInitialised(bool bSetTo);
    bool                  IsRegistered(void);
    void                  SetRegistered(bool bSetTo);
    cec_logical_address   GetPrimaryLogicalAdddress(void);

    // device specific methods
    CCECBusDevice *       GetPrimaryDevice(void);
    CCECPlaybackDevice *  GetPlaybackDevice(void);
    bool                  AllocateLogicalAddresses(void);
    bool                  ChangeDeviceType(const cec_device_type from, const cec_device_type to);
    CCECBusDevice *       GetDeviceByType(const cec_device_type type) const;
    void                  ResetPhysicalAddress(void);
    CStdString            GetConnectionInfo(void);
    void                  SetTVVendorOverride(const cec_vendor_id id);
    cec_vendor_id         GetTVVendorOverride(void);
    void                  SetOSDName(const CStdString &strDeviceName);
    CStdString            GetOSDName(void);
    void                  SetWakeDevices(const cec_logical_addresses &addresses);
    cec_logical_addresses GetWakeDevices(void);
    bool                  AutodetectPhysicalAddress(void);
    void                  SetClientVersion(const cec_client_version version);
    cec_client_version    GetClientVersion(void);
    bool                  SetDeviceTypes(const cec_device_type_list &deviceTypes);
    cec_device_type_list  GetDeviceTypes(void);

    // client-specific part of ICECAdapter
    bool                  EnableCallbacks(void *cbParam, ICECCallbacks *callbacks);
    bool                  PingAdapter(void);
    bool                  GetNextLogMessage(cec_log_message *message); /**< @deprecated will be removed in v2.0 */
    bool                  GetNextKeypress(cec_keypress *key);          /**< @deprecated will be removed in v2.0 */
    bool                  GetNextCommand(cec_command *command);        /**< @deprecated will be removed in v2.0 */
    bool                  Transmit(const cec_command &data);
    bool                  SetLogicalAddress(const cec_logical_address iLogicalAddress);
    bool                  SetPhysicalAddress(const uint16_t iPhysicalAddress);
    bool                  SetHDMIPort(const cec_logical_address iBaseDevice, const uint8_t iPort, bool bForce = false);
    bool                  SendPowerOnDevices(const cec_logical_address address = CECDEVICE_TV);
    bool                  SendStandbyDevices(const cec_logical_address address = CECDEVICE_BROADCAST);
    bool                  SendSetActiveSource(const cec_device_type type = CEC_DEVICE_TYPE_RESERVED);
    bool                  SendSetDeckControlMode(const cec_deck_control_mode mode, bool bSendUpdate = true);
    bool                  SendSetDeckInfo(const cec_deck_info info, bool bSendUpdate = true);
    bool                  SendSetInactiveView(void);
    bool                  SendSetMenuState(const cec_menu_state state, bool bSendUpdate = true);
    bool                  SendSetOSDString(const cec_logical_address iLogicalAddress, const cec_display_control duration, const char *strMessage);
    bool                  SwitchMonitoring(bool bEnable);
    cec_version           GetDeviceCecVersion(const cec_logical_address iAddress);
    bool                  GetDeviceMenuLanguage(const cec_logical_address iAddress, cec_menu_language &language);
    uint64_t              GetDeviceVendorId(const cec_logical_address iAddress);
    cec_power_status      GetDevicePowerStatus(const cec_logical_address iAddress);
    uint16_t              GetDevicePhysicalAddress(const cec_logical_address iAddress);
    bool                  PollDevice(const cec_logical_address iAddress);
    cec_logical_addresses GetActiveDevices(void);
    bool                  IsActiveDevice(const cec_logical_address iAddress);
    bool                  IsActiveDeviceType(const cec_device_type type);
    uint8_t               SendVolumeUp(bool bSendRelease = true);
    uint8_t               SendVolumeDown(bool bSendRelease = true);
    uint8_t               SendMuteAudio(void);
    bool                  SendKeypress(const cec_logical_address iDestination, const cec_user_control_code key, bool bWait = true);
    bool                  SendKeyRelease(const cec_logical_address iDestination, bool bWait = true);
    cec_osd_name          GetDeviceOSDName(const cec_logical_address iAddress);
    cec_logical_address   GetActiveSource(void);
    bool                  IsActiveSource(const cec_logical_address iAddress);
    bool                  SetStreamPath(const cec_logical_address iAddress);
    bool                  SetStreamPath(const uint16_t iPhysicalAddress);
    cec_logical_addresses GetLogicalAddresses(void);
    void                  RescanActiveDevices(void);
    bool                  IsLibCECActiveSource(void);

    // configuration
    libcec_configuration *GetConfiguration(void) { return &m_configuration; }
    bool                  GetCurrentConfiguration(libcec_configuration &configuration);
    bool                  SetConfiguration(const libcec_configuration &configuration);
    bool                  CanPersistConfiguration(void);
    bool                  PersistConfiguration(const libcec_configuration &configuration);
    void                  SetPhysicalAddress(const libcec_configuration &configuration);

    // callbacks
    void                  AddCommand(const cec_command &command);
    int                   MenuStateChanged(const cec_menu_state newState);
    void                  Alert(const libcec_alert type, const libcec_parameter &param);
    void                  AddLog(const cec_log_message &message);
    void                  AddKey(void);
    void                  AddKey(const cec_keypress &key);
    void                  SetCurrentButton(const cec_user_control_code iButtonCode);
    void                  CheckKeypressTimeout(void);
    void                  ConfigurationChanged(const libcec_configuration &config);

  protected:
    cec_logical_address   AllocateLogicalAddressRecordingDevice(void);
    cec_logical_address   AllocateLogicalAddressTuner(void);
    cec_logical_address   AllocateLogicalAddressPlaybackDevice(void);
    cec_logical_address   AllocateLogicalAddressAudioSystem(void);

    bool                  SetDevicePhysicalAddress(const uint16_t iPhysicalAddress);

    CCECProcessor *                         m_processor;
    libcec_configuration                    m_configuration;
    bool                                    m_bInitialised;
    bool                                    m_bRegistered;
    PLATFORM::CMutex                        m_mutex;
    PLATFORM::SyncedBuffer<cec_log_message> m_logBuffer;
    PLATFORM::CMutex                        m_logMutex;
    PLATFORM::SyncedBuffer<cec_keypress>    m_keyBuffer;
    PLATFORM::SyncedBuffer<cec_command>     m_commandBuffer;
    cec_user_control_code                   m_iCurrentButton;
    int64_t                                 m_buttontime;
  };
}
