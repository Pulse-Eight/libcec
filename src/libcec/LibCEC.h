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
#include <string>
#include "cec.h"
#include "p8-platform/util/buffer.h"
#include "CECTypeUtils.h"
#include <memory>

#define CEC_PROCESSOR_SIGNAL_WAIT_TIME 1000

namespace CEC
{
  class CAdapterCommunication;
  class CCECProcessor;
  class CCECClient;
  typedef std::shared_ptr<CCECClient> CECClientPtr;

  typedef struct cec_log_message_cpp
  {
    std::string   message; /**< the actual message, valid until returning from the log callback */
    cec_log_level level;   /**< log level of the message */
    int64_t       time;    /**< the timestamp of this message */
  } cec_log_message_cpp;

  class CLibCEC : public ICECAdapter
  {
    public:
      CLibCEC(void);
      virtual ~CLibCEC(void);

      bool Open(const char *strPort, uint32_t iTimeout = CEC_DEFAULT_CONNECT_TIMEOUT);
      void Close(void);
#if CEC_LIB_VERSION_MAJOR >= 5
      bool SetCallbacks(ICECCallbacks *callbacks, void *cbParam);
      bool DisableCallbacks(void);
#endif
      bool EnableCallbacks(void *cbParam, ICECCallbacks *callbacks);
      int8_t FindAdapters(cec_adapter *deviceList, uint8_t iBufSize, const char *strDevicePath = NULL);
      int8_t DetectAdapters(cec_adapter_descriptor *deviceList, uint8_t iBufSize, const char *strDevicePath = NULL, bool bQuickScan = false);
      bool PingAdapter(void);
      bool StartBootloader(void);

      bool Transmit(const cec_command &data);
      bool SetLogicalAddress(cec_logical_address iLogicalAddress = CECDEVICE_PLAYBACKDEVICE1);
      bool SetPhysicalAddress(uint16_t iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS);

      bool PowerOnDevices(cec_logical_address address = CECDEVICE_TV);
      bool StandbyDevices(cec_logical_address address = CECDEVICE_BROADCAST);
      bool SetActiveSource(cec_device_type type = CEC_DEVICE_TYPE_RESERVED);
      bool SetDeckControlMode(cec_deck_control_mode mode, bool bSendUpdate = true);
      bool SetDeckInfo(cec_deck_info info, bool bSendUpdate = true);
      bool SetInactiveView(void);
      bool SetMenuState(cec_menu_state state, bool bSendUpdate = true);
      bool SetOSDString(cec_logical_address iLogicalAddress, cec_display_control duration, const char *strMessage);
      bool SwitchMonitoring(bool bEnable);
      cec_version GetDeviceCecVersion(cec_logical_address iAddress);
      std::string GetDeviceMenuLanguage(cec_logical_address iAddress);
      uint32_t GetDeviceVendorId(cec_logical_address iAddress);
      uint16_t GetDevicePhysicalAddress(cec_logical_address iAddress);
      cec_power_status GetDevicePowerStatus(cec_logical_address iAddress);
      bool PollDevice(cec_logical_address iAddress);
      cec_logical_addresses GetActiveDevices(void);
      bool IsActiveDevice(cec_logical_address iAddress);
      bool IsActiveDeviceType(cec_device_type type);
      bool SetHDMIPort(cec_logical_address iBaseDevice, uint8_t iPort = CEC_DEFAULT_HDMI_PORT);
      uint8_t VolumeUp(bool bSendRelease = true);
      uint8_t VolumeDown(bool bSendRelease = true);
#if CEC_LIB_VERSION_MAJOR >= 5
      uint8_t MuteAudio(void);
#endif
      bool SendKeypress(cec_logical_address iDestination, cec_user_control_code key, bool bWait = true);
      bool SendKeyRelease(cec_logical_address iDestination, bool bWait = true);
      std::string GetDeviceOSDName(cec_logical_address iAddress);
      cec_logical_address GetActiveSource(void);
      bool IsActiveSource(cec_logical_address iAddress);
      bool SetStreamPath(cec_logical_address iAddress);
      bool SetStreamPath(uint16_t iPhysicalAddress);
      cec_logical_addresses GetLogicalAddresses(void);
      bool GetCurrentConfiguration(libcec_configuration *configuration);
      bool SetConfiguration(const libcec_configuration *configuration);
#if CEC_LIB_VERSION_MAJOR >= 5
      bool CanSaveConfiguration(void);
#else
      bool CanPersistConfiguration(void);
      bool PersistConfiguration(libcec_configuration *configuration);
#endif
      void RescanActiveDevices(void);
      bool IsLibCECActiveSource(void);

      const char* ToString(const cec_menu_state state)         { return CCECTypeUtils::ToString(state); }
      const char* ToString(const cec_version version)          { return CCECTypeUtils::ToString(version); }
      const char* ToString(const cec_power_status status)      { return CCECTypeUtils::ToString(status); }
      const char* ToString(const cec_logical_address address)  { return CCECTypeUtils::ToString(address); }
      const char* ToString(const cec_deck_control_mode mode)   { return CCECTypeUtils::ToString(mode); }
      const char* ToString(const cec_deck_info status)         { return CCECTypeUtils::ToString(status); }
      const char* ToString(const cec_opcode opcode)            { return CCECTypeUtils::ToString(opcode); }
      const char* ToString(const cec_system_audio_status mode) { return CCECTypeUtils::ToString(mode); }
      const char* ToString(const cec_audio_status status)      { return CCECTypeUtils::ToString(status); }
      const char* ToString(const cec_device_type type)         { return CCECTypeUtils::ToString(type); }
      const char* ToString(const cec_user_control_code key)    { return CCECTypeUtils::ToString(key); }
      const char* ToString(const cec_adapter_type type)        { return CCECTypeUtils::ToString(type); }
      std::string VersionToString(uint32_t version)            { return CCECTypeUtils::VersionToString(version); }
      void PrintVersion(uint32_t version, char* buf, size_t bufSize);
      const char* VendorIdToString(uint32_t vendor)            { return CCECTypeUtils::ToString((cec_vendor_id)vendor); }

      static cec_device_type GetType(cec_logical_address address);
      static uint16_t GetMaskForType(cec_logical_address address);
      static uint16_t GetMaskForType(cec_device_type type);

      bool GetDeviceInformation(const char *strPort, libcec_configuration *config, uint32_t iTimeoutMs = CEC_DEFAULT_CONNECT_TIMEOUT);

      void AddLog(const cec_log_level level, const char *strFormat, ...);
      void AddCommand(const cec_command &command);
      bool CommandHandlerCB(const cec_command &command);
      uint16_t CheckKeypressTimeout(void);
      void Alert(const libcec_alert type, const libcec_parameter &param);

      static bool IsValidPhysicalAddress(uint16_t iPhysicalAddress);
      CECClientPtr RegisterClient(libcec_configuration &configuration);
      std::vector<CECClientPtr> GetClients(void) { return m_clients; };
      const char *GetLibInfo(void);
      void InitVideoStandalone(void);
      uint16_t GetAdapterVendorId(void) const;
      uint16_t GetAdapterProductId(void) const;

      uint8_t AudioToggleMute(void);
      uint8_t AudioMute(void);
      uint8_t AudioUnmute(void);
      uint8_t AudioStatus(void);

      cec_command CommandFromString(const char* strCommand);

      bool AudioEnable(bool enable);
      uint8_t SystemAudioModeStatus(void);
      bool GetStats(struct cec_adapter_stats* stats);

      CCECProcessor *           m_cec;

    protected:
      int64_t                   m_iStartTime;
      CECClientPtr              m_client;
      std::vector<CECClientPtr> m_clients;
  };
};
