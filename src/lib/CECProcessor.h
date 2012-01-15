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

#include <string>
#include <cectypes.h>
#include "AdapterCommunication.h"
#include "platform/os.h"

class CSerialPort;

namespace CEC
{
  class CLibCEC;
  class CAdapterCommunication;
  class CCECBusDevice;

  class CCECProcessor : public PLATFORM::CThread
  {
    public:
      CCECProcessor(CLibCEC *controller, const char *strDeviceName, cec_logical_address iLogicalAddress = CECDEVICE_PLAYBACKDEVICE1, uint16_t iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS);
      CCECProcessor(CLibCEC *controller, const char *strDeviceName, const cec_device_type_list &types);
      virtual ~CCECProcessor(void);

      virtual bool Start(const char *strPort, uint16_t iBaudRate = 38400, uint32_t iTimeoutMs = 10000);
      virtual void *Process(void);

      virtual bool                  IsMonitoring(void) const { return m_bMonitor; }
      virtual CCECBusDevice *       GetDeviceByPhysicalAddress(uint16_t iPhysicalAddress, bool bRefresh = false) const;
      virtual CCECBusDevice *       GetDeviceByType(cec_device_type type) const;
      virtual CCECBusDevice *       GetPrimaryDevice(void) const;
      virtual cec_version           GetDeviceCecVersion(cec_logical_address iAddress);
      virtual bool                  GetDeviceMenuLanguage(cec_logical_address iAddress, cec_menu_language *language);
      virtual const std::string &   GetDeviceName(void) { return m_strDeviceName; }
      virtual cec_osd_name          GetDeviceOSDName(cec_logical_address iAddress);
      virtual uint64_t              GetDeviceVendorId(cec_logical_address iAddress);
      virtual cec_power_status      GetDevicePowerStatus(cec_logical_address iAddress);
      virtual cec_logical_address   GetLogicalAddress(void) const { return m_logicalAddresses.primary; }
      virtual cec_logical_addresses GetLogicalAddresses(void) const { return m_logicalAddresses; }
      virtual cec_logical_addresses GetActiveDevices(void);
      virtual uint16_t              GetDevicePhysicalAddress(cec_logical_address iAddress);
      virtual bool                  HasLogicalAddress(cec_logical_address address) const { return m_logicalAddresses.IsSet(address); }
      virtual bool                  IsPresentDevice(cec_logical_address address);
      virtual bool                  IsPresentDeviceType(cec_device_type type);
      virtual uint16_t              GetPhysicalAddress(void) const;
      virtual uint64_t              GetLastTransmission(void) const { return m_iLastTransmission; }
      virtual bool                  IsStarted(void) const { return m_bStarted; }
      virtual cec_logical_address   GetActiveSource(void);
      virtual bool                  IsActiveSource(cec_logical_address iAddress);
      virtual bool                  IsInitialised(void) const { return m_bInitialised; }

      virtual bool SetActiveView(void);
      virtual bool SetActiveSource(cec_device_type type = CEC_DEVICE_TYPE_RESERVED);
      virtual bool SetDeckControlMode(cec_deck_control_mode mode, bool bSendUpdate = true);
      virtual bool SetDeckInfo(cec_deck_info info, bool bSendUpdate = true);
      virtual bool SetHDMIPort(cec_logical_address iBaseDevice, uint8_t iPort, bool bForce = false);
      virtual bool TransmitInactiveSource(void);
      virtual bool SetLogicalAddress(cec_logical_address iLogicalAddress);
      virtual bool SetMenuState(cec_menu_state state, bool bSendUpdate = true);
      virtual bool SetPhysicalAddress(uint16_t iPhysicalAddress, bool bSendUpdate = true);
      virtual bool SetActiveSource(uint16_t iStreamPath);
      virtual bool SwitchMonitoring(bool bEnable);
      virtual bool PollDevice(cec_logical_address iAddress);
      virtual uint8_t VolumeUp(bool bSendRelease = true);
      virtual uint8_t VolumeDown(bool bSendRelease = true);
      virtual uint8_t MuteAudio(bool bSendRelease = true);
      virtual bool TransmitKeypress(cec_logical_address iDestination, cec_user_control_code key, bool bWait = true);
      virtual bool TransmitKeyRelease(cec_logical_address iDestination, bool bWait = true);
      virtual bool EnablePhysicalAddressDetection(void) { return false; };
      void SetStandardLineTimeout(uint8_t iTimeout);
      void SetRetryLineTimeout(uint8_t iTimeout);

      bool SetLineTimeout(uint8_t iTimeout);

      const char *ToString(const cec_device_type type);
      const char *ToString(const cec_menu_state state);
      const char *ToString(const cec_version version);
      const char *ToString(const cec_power_status status);
      const char *ToString(const cec_logical_address address);
      const char *ToString(const cec_deck_control_mode mode);
      const char *ToString(const cec_deck_info status);
      const char *ToString(const cec_opcode opcode);
      const char *ToString(const cec_system_audio_status mode);
      const char *ToString(const cec_audio_status status);
      const char *ToString(const cec_vendor_id vendor);

      virtual bool Transmit(const cec_command &data);
      virtual bool Transmit(CCECAdapterMessage *output);
      virtual void TransmitAbort(cec_logical_address address, cec_opcode opcode, cec_abort_reason reason = CEC_ABORT_REASON_UNRECOGNIZED_OPCODE);

      virtual void SetCurrentButton(cec_user_control_code iButtonCode);
      virtual void AddCommand(const cec_command &command);
      virtual void AddKey(cec_keypress &key);
      virtual void AddKey(void);
      virtual void AddLog(cec_log_level level, const CStdString &strMessage);

      virtual bool ChangeDeviceType(cec_device_type from, cec_device_type to);
      virtual bool FindLogicalAddresses(void);
      virtual bool SetAckMask(uint16_t iMask);

      virtual bool StartBootloader(void);
      virtual bool PingAdapter(void);

      CCECBusDevice *  m_busDevices[16];
      PLATFORM::CMutex m_transmitMutex;

  private:
      void ReplaceHandlers(void);
      void ScanCECBus(void);
      bool PhysicalAddressInUse(uint16_t iPhysicalAddress);
      bool TryLogicalAddress(cec_logical_address address);
      bool FindLogicalAddressRecordingDevice(void);
      bool FindLogicalAddressTuner(void);
      bool FindLogicalAddressPlaybackDevice(void);
      bool FindLogicalAddressAudioSystem(void);

      void LogOutput(const cec_command &data);
      bool WaitForTransmitSucceeded(CCECAdapterMessage *message);
      bool ParseMessage(const CCECAdapterMessage &msg);
      void ParseCommand(cec_command &command);

      bool                                m_bStarted;
      bool                                m_bInitialised;
      uint8_t                             m_iHDMIPort;
      cec_logical_address                 m_iBaseDevice;
      cec_command                         m_currentframe;
      cec_logical_addresses               m_logicalAddresses;
      cec_logical_address                 m_lastInitiator;
      std::string                         m_strDeviceName;
      cec_device_type_list                m_types;
      PLATFORM::CMutex                    m_mutex;
      PLATFORM::CCondition                m_startCondition;
      CAdapterCommunication*              m_communication;
      CLibCEC*                            m_controller;
      bool                                m_bMonitor;
      PLATFORM::SyncedBuffer<cec_command> m_commandBuffer;
      cec_keypress                        m_previousKey;
      PLATFORM::CThread *                 m_busScan;
      uint8_t                             m_iLineTimeout;
      uint8_t                             m_iStandardLineTimeout;
      uint8_t                             m_iRetryLineTimeout;
      uint64_t                            m_iLastTransmission;
  };

  class CCECBusScan : public PLATFORM::CThread
  {
  public:
    CCECBusScan(CCECProcessor *processor) { m_processor = processor; }
    virtual ~CCECBusScan(void) { StopThread(true); }
    virtual void *Process(void);

  private:
    void WaitUntilIdle(void);

    CCECProcessor *m_processor;
  };
};
