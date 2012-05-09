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
    CCECClient(CCECProcessor *processor, const libcec_configuration *configuration);
    virtual ~CCECClient(void);

    // methods for registration in CCECProcessor
    bool                Initialise(void);
    void                OnUnregister(void) { SetRegistered(false); SetInitialised(false); }
    bool                IsInitialised(void);
    void                SetInitialised(bool bSetTo);
    bool                IsRegistered(void);
    void                SetRegistered(bool bSetTo);
    CCECBusDevice *     GetPrimaryDevice(void);
    CCECPlaybackDevice *GetPlaybackDevice(void);
    bool                FindLogicalAddresses(void);
    bool                ChangeDeviceType(cec_device_type from, cec_device_type to);
    CCECBusDevice *     GetDeviceByType(const cec_device_type type) const;

    // client-specific part of ICECAdapter
    bool                EnableCallbacks(void *cbParam, ICECCallbacks *callbacks);
    bool                GetNextLogMessage(cec_log_message *message);
    bool                GetNextKeypress(cec_keypress *key);
    bool                GetNextCommand(cec_command *command);
    bool                Transmit(const cec_command &data);
    bool                SetLogicalAddress(cec_logical_address iLogicalAddress);
    bool                SetPhysicalAddress(uint16_t iPhysicalAddress);
    bool                SetHDMIPort(cec_logical_address iBaseDevice, uint8_t iPort, bool bForce = false);
    bool                SendPowerOnDevices(cec_logical_address address = CECDEVICE_TV);
    bool                SendStandbyDevices(cec_logical_address address = CECDEVICE_BROADCAST);
    bool                SendSetActiveSource(cec_device_type type = CEC_DEVICE_TYPE_RESERVED);
    bool                SendSetDeckControlMode(cec_deck_control_mode mode, bool bSendUpdate = true);
    bool                SendSetDeckInfo(cec_deck_info info, bool bSendUpdate = true);
    bool                SendSetInactiveView(void);
    bool                SendSetMenuState(cec_menu_state state, bool bSendUpdate = true);
    bool                SendSetOSDString(cec_logical_address iLogicalAddress, cec_display_control duration, const char *strMessage);
    cec_version         GetDeviceCecVersion(cec_logical_address iAddress);
    bool                GetDeviceMenuLanguage(cec_logical_address iAddress, cec_menu_language *language);
    uint64_t            GetDeviceVendorId(cec_logical_address iAddress);
    cec_power_status    GetDevicePowerStatus(cec_logical_address iAddress);
    uint16_t            GetDevicePhysicalAddress(cec_logical_address iAddress);
    uint8_t             SendVolumeUp(bool bSendRelease = true);
    uint8_t             SendVolumeDown(bool bSendRelease = true);
    uint8_t             SendMuteAudio(void);
    bool                SendKeypress(cec_logical_address iDestination, cec_user_control_code key, bool bWait = true);
    bool                SendKeyRelease(cec_logical_address iDestination, bool bWait = true);
    cec_osd_name        GetDeviceOSDName(cec_logical_address iAddress);

    // configuration
    libcec_configuration *GetConfiguration(void) { return &m_configuration; }
    bool                  GetCurrentConfiguration(libcec_configuration *configuration);
    bool                  SetConfiguration(const libcec_configuration *configuration);

    // callbacks
    void AddCommand(const cec_command &command);
    int  MenuStateChanged(const cec_menu_state newState);
    void Alert(const libcec_alert type, const libcec_parameter &param);
    void AddLog(const cec_log_message &message);
    void AddKey(void);
    void AddKey(const cec_keypress &key);
    void SetCurrentButton(cec_user_control_code iButtonCode);
    void CheckKeypressTimeout(void);
    void ConfigurationChanged(const libcec_configuration &config);

  protected:
    cec_logical_address FindLogicalAddressRecordingDevice(void);
    cec_logical_address FindLogicalAddressTuner(void);
    cec_logical_address FindLogicalAddressPlaybackDevice(void);
    cec_logical_address FindLogicalAddressAudioSystem(void);

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
