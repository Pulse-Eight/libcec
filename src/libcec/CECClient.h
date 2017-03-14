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
#include "LibCEC.h"
#include "p8-platform/threads/threads.h"
#include "p8-platform/util/buffer.h"
#include "p8-platform/threads/mutex.h"
#include <string>
#include <memory>

namespace CEC
{
  class CCECProcessor;
  class CCECBusDevice;
  class CCECPlaybackDevice;
  class CCECClient;

  typedef std::shared_ptr<CCECClient> CECClientPtr;

  class CCallbackWrap
  {
  public:
    CCallbackWrap(const cec_command& command) :
      m_type(CEC_CB_COMMAND),
      m_command(command),
      m_alertType(CEC_ALERT_SERVICE_DEVICE),
      m_menuState(CEC_MENU_STATE_ACTIVATED),
      m_bActivated(false),
      m_logicalAddress(CECDEVICE_UNKNOWN),
      m_keepResult(false),
      m_result(0),
      m_bSucceeded(false) {}

    CCallbackWrap(const cec_keypress& key) :
      m_type(CEC_CB_KEY_PRESS),
      m_key(key),
      m_alertType(CEC_ALERT_SERVICE_DEVICE),
      m_menuState(CEC_MENU_STATE_ACTIVATED),
      m_bActivated(false),
      m_logicalAddress(CECDEVICE_UNKNOWN),
      m_keepResult(false),
      m_result(0),
      m_bSucceeded(false) {}

    CCallbackWrap(const cec_log_message_cpp& message) :
      m_type(CEC_CB_LOG_MESSAGE),
      m_message(message),
      m_alertType(CEC_ALERT_SERVICE_DEVICE),
      m_menuState(CEC_MENU_STATE_ACTIVATED),
      m_bActivated(false),
      m_logicalAddress(CECDEVICE_UNKNOWN),
      m_keepResult(false),
      m_result(0),
      m_bSucceeded(false) {}

    CCallbackWrap(const libcec_alert type, const libcec_parameter& param) :
      m_type(CEC_CB_ALERT),
      m_alertType(type),
      m_alertParam(param),
      m_menuState(CEC_MENU_STATE_ACTIVATED),
      m_bActivated(false),
      m_logicalAddress(CECDEVICE_UNKNOWN),
      m_keepResult(false),
      m_result(0),
      m_bSucceeded(false) {}

    CCallbackWrap(const libcec_configuration& config) :
      m_type(CEC_CB_CONFIGURATION),
      m_alertType(CEC_ALERT_SERVICE_DEVICE),
      m_config(config),
      m_menuState(CEC_MENU_STATE_ACTIVATED),
      m_bActivated(false),
      m_logicalAddress(CECDEVICE_UNKNOWN),
      m_keepResult(false),
      m_result(0),
      m_bSucceeded(false) {}

    CCallbackWrap(const cec_menu_state newState, const bool keepResult = false) :
      m_type(CEC_CB_MENU_STATE),
      m_alertType(CEC_ALERT_SERVICE_DEVICE),
      m_menuState(newState),
      m_bActivated(false),
      m_logicalAddress(CECDEVICE_UNKNOWN),
      m_keepResult(keepResult),
      m_result(0),
      m_bSucceeded(false) {}

    CCallbackWrap(bool bActivated, const cec_logical_address logicalAddress) :
      m_type(CEC_CB_SOURCE_ACTIVATED),
      m_alertType(CEC_ALERT_SERVICE_DEVICE),
      m_menuState(CEC_MENU_STATE_ACTIVATED),
      m_bActivated(bActivated),
      m_logicalAddress(logicalAddress),
      m_keepResult(false),
      m_result(0),
      m_bSucceeded(false) {}

    int Result(uint32_t iTimeout)
    {
      P8PLATFORM::CLockObject lock(m_mutex);

      bool bReturn = m_bSucceeded ? true : m_condition.Wait(m_mutex, m_bSucceeded, iTimeout);
      if (bReturn)
        return m_result;
      return 0;
    }

    void Report(int result)
    {
      P8PLATFORM::CLockObject lock(m_mutex);

      m_result = result;
      m_bSucceeded = true;
      m_condition.Signal();
    }

    enum callbackWrapType {
      CEC_CB_LOG_MESSAGE,
      CEC_CB_KEY_PRESS,
      CEC_CB_COMMAND,
      CEC_CB_ALERT,
      CEC_CB_CONFIGURATION,
      CEC_CB_MENU_STATE,
      CEC_CB_SOURCE_ACTIVATED,
    } m_type;

    cec_command                  m_command;
    cec_keypress                 m_key;
    cec_log_message_cpp          m_message;
    libcec_alert                 m_alertType;
    libcec_parameter             m_alertParam;
    libcec_configuration         m_config;
    cec_menu_state               m_menuState;
    bool                         m_bActivated;
    cec_logical_address          m_logicalAddress;
    bool                         m_keepResult;
    int                          m_result;
    P8PLATFORM::CCondition<bool> m_condition;
    P8PLATFORM::CMutex           m_mutex;
    bool                         m_bSucceeded;
  };

  class CCECClient : private P8PLATFORM::CThread
  {
    friend class CCECProcessor;

  public:
    CCECClient(CCECProcessor *processor, const libcec_configuration &configuration);
    virtual ~CCECClient(void);

    /*!
     * @return True when initialised and registered, false otherwise.
     */
    virtual bool IsInitialised(void);

    /*!
     * @return True when registered in the processor, false otherwise.
     */
    virtual bool IsRegistered(void);

    /*!
     * @return The primary logical address that this client is controlling.
     */
    virtual cec_logical_address GetPrimaryLogicalAddress(void);

    /*!
     * @return The primary device that this client is controlling, or NULL if none.
     */
    virtual CCECBusDevice *GetPrimaryDevice(void);

    /*!
     * @return Get the playback device or recording device that this client is controlling, or NULL if none.
     */
    virtual CCECPlaybackDevice *GetPlaybackDevice(void);

    /*!
     * @brief Change one of the device types that this client is controlling into another.
     * @param from The type to change.
     * @param to The new value.
     * @return True when changed, false otherwise.
     */
    virtual bool ChangeDeviceType(const cec_device_type from, const cec_device_type to);

    /*!
     * @brief Get a device that this client is controlling, given it's type.
     * @param type The type of the device to get.
     * @return The requested device, or NULL if not found.
     */
    virtual CCECBusDevice *GetDeviceByType(const cec_device_type type) const;

    /*!
     * @brief Reset the physical address from the configuration.
     */
    virtual void ResetPhysicalAddress(void);

    /*!
     * @return A string that describes this client.
     */
    virtual std::string GetConnectionInfo(void);

    /*!
     * @return The current value of the TV vendor override setting.
     */
    virtual cec_vendor_id GetTVVendorOverride(void);

    /*!
     * @return The current value of the OSD name setting.
     */
    virtual std::string GetOSDName(void);

    /*!
     * @return Get the current value of the wake device setting.
     */
    virtual cec_logical_addresses GetWakeDevices(void);

    /*!
     * @return The version of this client.
     */
    virtual uint32_t GetClientVersion(void);

    /*!
     * @return The device types that this client is controlling.
     */
    virtual cec_device_type_list GetDeviceTypes(void);

    // client-specific part of ICECAdapter
    virtual bool                  EnableCallbacks(void *cbParam, ICECCallbacks *callbacks);
    virtual bool                  PingAdapter(void);
    virtual bool                  Transmit(const cec_command &data, bool bIsReply);
    virtual bool                  SetLogicalAddress(const cec_logical_address iLogicalAddress);
    virtual bool                  SetPhysicalAddress(const uint16_t iPhysicalAddress);
    virtual bool                  SetHDMIPort(const cec_logical_address iBaseDevice, const uint8_t iPort, bool bForce = false);
    virtual bool                  SendPowerOnDevices(const cec_logical_address address = CECDEVICE_TV);
    virtual bool                  SendStandbyDevices(const cec_logical_address address = CECDEVICE_BROADCAST);
    virtual bool                  SendSetActiveSource(const cec_device_type type = CEC_DEVICE_TYPE_RESERVED);
    virtual bool                  SendSetDeckControlMode(const cec_deck_control_mode mode, bool bSendUpdate = true);
    virtual bool                  SendSetDeckInfo(const cec_deck_info info, bool bSendUpdate = true);
    virtual bool                  SendSetInactiveView(void);
    virtual bool                  SendSetMenuState(const cec_menu_state state, bool bSendUpdate = true);
    virtual bool                  SendSetOSDString(const cec_logical_address iLogicalAddress, const cec_display_control duration, const char *strMessage);
    virtual bool                  SwitchMonitoring(bool bEnable);
    virtual cec_version           GetDeviceCecVersion(const cec_logical_address iAddress);
    virtual std::string           GetDeviceMenuLanguage(const cec_logical_address iAddress);
    virtual uint32_t              GetDeviceVendorId(const cec_logical_address iAddress);
    virtual cec_power_status      GetDevicePowerStatus(const cec_logical_address iAddress);
    virtual uint16_t              GetDevicePhysicalAddress(const cec_logical_address iAddress);
    virtual bool                  PollDevice(const cec_logical_address iAddress);
    virtual cec_logical_addresses GetActiveDevices(void);
    virtual bool                  IsActiveDevice(const cec_logical_address iAddress);
    virtual bool                  IsActiveDeviceType(const cec_device_type type);
    virtual uint8_t               SendVolumeUp(bool bSendRelease = true);
    virtual uint8_t               SendVolumeDown(bool bSendRelease = true);
    virtual uint8_t               SendMuteAudio(void);
    virtual uint8_t               AudioToggleMute(void);
    virtual uint8_t               AudioMute(void);
    virtual uint8_t               AudioUnmute(void);
    virtual uint8_t               AudioStatus(void);
    virtual bool                  SendKeypress(const cec_logical_address iDestination, const cec_user_control_code key, bool bWait = true);
    virtual bool                  SendKeyRelease(const cec_logical_address iDestination, bool bWait = true);
    virtual std::string           GetDeviceOSDName(const cec_logical_address iAddress);
    virtual cec_logical_address   GetActiveSource(void);
    virtual bool                  IsActiveSource(const cec_logical_address iAddress);
    virtual bool                  SetStreamPath(const cec_logical_address iAddress);
    virtual bool                  SetStreamPath(const uint16_t iPhysicalAddress);
    virtual cec_logical_addresses GetLogicalAddresses(void);
    virtual void                  RescanActiveDevices(void);
    virtual bool                  IsLibCECActiveSource(void);
    bool                          AudioEnable(bool enable);

    // configuration
    virtual bool                  GetCurrentConfiguration(libcec_configuration &configuration);
    virtual bool                  SetConfiguration(const libcec_configuration &configuration);
    virtual bool                  CanPersistConfiguration(void);
    virtual bool                  PersistConfiguration(const libcec_configuration &configuration);
    virtual void                  SetPhysicalAddress(const libcec_configuration &configuration);

    void QueueAddCommand(const cec_command& command);
    void QueueAddKey(const cec_keypress& key);
    void QueueAddLog(const cec_log_message_cpp& message);
    void QueueAlert(const libcec_alert type, const libcec_parameter& param);
    void QueueConfigurationChanged(const libcec_configuration& config);
    int QueueMenuStateChanged(const cec_menu_state newState); //TODO
    void QueueSourceActivated(bool bActivated, const cec_logical_address logicalAddress);

    // callbacks
    virtual void                  Alert(const libcec_alert type, const libcec_parameter &param) { QueueAlert(type, param); }
    virtual void                  AddLog(const cec_log_message_cpp &message) { QueueAddLog(message); }
    virtual void                  AddKey(bool bSendComboKey = false, bool bButtonRelease = false);
    virtual void                  AddKey(const cec_keypress &key);
    virtual void                  SetCurrentButton(const cec_user_control_code iButtonCode);
    virtual uint16_t              CheckKeypressTimeout(void);
    virtual void                  SourceActivated(const cec_logical_address logicalAddress);
    virtual void                  SourceDeactivated(const cec_logical_address logicalAddress);

  protected:
    void* Process(void);

    /*!
     * @brief Register this client in the processor
     * @return True when registered, false otherwise.
     */
    virtual bool OnRegister(void);

    /*!
     * @brief Called by the processor when this client is unregistered
     */
    virtual void OnUnregister(void) { SetRegistered(false); SetInitialised(false); }

    /*!
     * @brief Set the registered state of this client.
     * @param bSetTo The new value.
     */
    virtual void SetRegistered(bool bSetTo);

    /*!
     * @brief Set the initialised state of this client.
     * @param bSetTo The new value
     */
    virtual void SetInitialised(bool bSetTo);

    /*!
     * @brief Change the TV vendor id override setting.
     * @param id The new value.
     */
    virtual void SetTVVendorOverride(const cec_vendor_id id);

    /*!
     * @brief Change the OSD name of the primary device that this client is controlling.
     * @param strDeviceName The new value.
     */
    virtual void SetOSDName(const std::string &strDeviceName);

    /*!
     * @brief Change the value of the devices to wake.
     * @param addresses The new value.
     */
    virtual void SetWakeDevices(const cec_logical_addresses &addresses);

    /*!
     * @brief Change the value of the client version setting.
     * @param version The new version setting.
     */
    virtual void SetClientVersion(uint32_t version);

    /*!
     * @brief Change the device types that this client is controlling.
     * @param deviceTypes The new types.
     * @return True when the client needs to be re-registered to pick up the new setting, false otherwise.
     */
    virtual bool SetDeviceTypes(const cec_device_type_list &deviceTypes);

    /*!
     * @return A pointer to the current configuration of this client.
     */
    virtual libcec_configuration *GetConfiguration(void) { return &m_configuration; }

    /*!
     * @brief Called by the processor when registering this client to allocate the logical addresses.
     * @return True when the addresses for all types were allocated, false otherwise.
     */
    virtual bool AllocateLogicalAddresses(void);

    /*!
     * @brief Try to allocate a logical address for a recording device controlled by this client.
     * @return The logical address that was allocated, or CECDEVICE_UNKNOWN if none could be allocated.
     */
    virtual cec_logical_address AllocateLogicalAddressRecordingDevice(void);

    /*!
     * @brief Try to allocate a logical address for a tuner controlled by this client.
     * @return The logical address that was allocated, or CECDEVICE_UNKNOWN if none could be allocated.
     */
    virtual cec_logical_address AllocateLogicalAddressTuner(void);

    /*!
     * @brief Try to allocate a logical address for a playback device controlled by this client.
     * @return The logical address that was allocated, or CECDEVICE_UNKNOWN if none could be allocated.
     */
    virtual cec_logical_address AllocateLogicalAddressPlaybackDevice(void);

    /*!
     * @brief Try to allocate a logical address for an audiosystem controlled by this client.
     * @return The logical address that was allocated, or CECDEVICE_UNKNOWN if none could be allocated.
     */
    virtual cec_logical_address AllocateLogicalAddressAudioSystem(void);

    /*!
     * @brief Change the physical address of the devices controlled by this client.
     * @param iPhysicalAddress The new physical address.
     * @return True when changed, false otherwise.
     */
    virtual bool SetDevicePhysicalAddress(const uint16_t iPhysicalAddress);

    /*!
     * @brief Try to autodetect the physical address.
     * @return True when autodetected (and set in m_configuration), false otherwise.
     */
    virtual bool AutodetectPhysicalAddress(void);

    /*!
     * @brief Replaces all device types in m_configuration by types that are supported by the command handler of the TV
     */
    virtual void SetSupportedDeviceTypes(void);

    void AddCommand(const cec_command &command);
    void CallbackAddCommand(const cec_command& command);
    void CallbackAddKey(const cec_keypress& key);
    void CallbackAddLog(const cec_log_message_cpp& message);
    void CallbackAlert(const libcec_alert type, const libcec_parameter& param);
    void CallbackConfigurationChanged(const libcec_configuration& config);
    int  CallbackMenuStateChanged(const cec_menu_state newState);
    void CallbackSourceActivated(bool bActivated, const cec_logical_address logicalAddress);

    uint32_t DoubleTapTimeoutMS(void);

    CCECProcessor *                          m_processor;                         /**< a pointer to the processor */
    libcec_configuration                     m_configuration;                     /**< the configuration of this client */
    bool                                     m_bInitialised;                      /**< true when initialised, false otherwise */
    bool                                     m_bRegistered;                       /**< true when registered in the processor, false otherwise */
    P8PLATFORM::CMutex                       m_mutex;                             /**< mutex for changes to this instance */
    P8PLATFORM::CMutex                       m_cbMutex;                           /**< mutex that is held when doing anything with callbacks */
    cec_user_control_code                    m_iCurrentButton;                    /**< the control code of the button that's currently held down (if any) */
    int64_t                                  m_initialButtontime;                 /**< the timestamp when the button was initially pressed (in seconds since epoch), or 0 if none was pressed. */
    int64_t                                  m_updateButtontime;                  /**< the timestamp when the button was updated (in seconds since epoch), or 0 if none was pressed. */
    int64_t                                  m_repeatButtontime;                  /**< the timestamp when the button will next repeat (in seconds since epoch), or 0 if repeat is disabled. */
    int64_t                                  m_releaseButtontime;                 /**< the timestamp when the button will be released (in seconds since epoch), or 0 if none was pressed. */
    int32_t                                  m_pressedButtoncount;                /**< the number of times a button released message has been seen for this press. */
    int32_t                                  m_releasedButtoncount;               /**< the number of times a button pressed message has been seen for this press. */
    int64_t                                  m_iPreventForwardingPowerOffCommand; /**< prevent forwarding standby commands until this time */
    P8PLATFORM::SyncedBuffer<CCallbackWrap*> m_callbackCalls;
  };
}
