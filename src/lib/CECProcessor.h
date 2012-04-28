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

#include <string>
#include "../../include/cectypes.h"
#include "platform/threads/threads.h"
#include "platform/util/buffer.h"
#include "adapter/AdapterCommunication.h"

namespace CEC
{
  class CLibCEC;
  class IAdapterCommunication;
  class CCECBusDevice;

  // a buffer that priotises the input from the TV.
  // if we need more than this, we'll have to change it into a priority_queue
  class CCECInputBuffer
  {
  public:
    CCECInputBuffer(void) : m_bHasData(false) {}
    virtual ~CCECInputBuffer(void)
    {
      m_condition.Broadcast();
    }

    bool Push(const cec_command &command)
    {
      bool bReturn(false);
      PLATFORM::CLockObject lock(m_mutex);
      if (command.initiator == CECDEVICE_TV)
        bReturn = m_tvInBuffer.Push(command);
      else
        bReturn = m_inBuffer.Push(command);

      m_bHasData |= bReturn;
      if (m_bHasData)
        m_condition.Signal();

      return bReturn;
    }

    bool Pop(cec_command &command, uint16_t iTimeout)
    {
      bool bReturn(false);
      PLATFORM::CLockObject lock(m_mutex);
      if (m_tvInBuffer.IsEmpty() && m_inBuffer.IsEmpty() &&
          !m_condition.Wait(m_mutex, m_bHasData, iTimeout))
        return bReturn;

      if (m_tvInBuffer.Pop(command))
        bReturn = true;
      else if (m_inBuffer.Pop(command))
        bReturn = true;

      m_bHasData = !m_tvInBuffer.IsEmpty() || !m_inBuffer.IsEmpty();
      return bReturn;
    }

  private:
    PLATFORM::CMutex                    m_mutex;
    PLATFORM::CCondition<volatile bool> m_condition;
    volatile bool                       m_bHasData;
    PLATFORM::SyncedBuffer<cec_command> m_tvInBuffer;
    PLATFORM::SyncedBuffer<cec_command> m_inBuffer;
  };

  class CCECProcessor : public PLATFORM::CThread, public IAdapterCommunicationCallback
  {
    public:
      CCECProcessor(CLibCEC *controller, const char *strDeviceName, const cec_device_type_list &types, uint16_t iPhysicalAddress);
      CCECProcessor(CLibCEC *controller, libcec_configuration *configuration);
      virtual ~CCECProcessor(void);

      bool Start(const char *strPort, uint16_t iBaudRate = CEC_SERIAL_DEFAULT_BAUDRATE, uint32_t iTimeoutMs = CEC_DEFAULT_CONNECT_TIMEOUT);
      void *Process(void);
      void Close(void);

      bool                  OnCommandReceived(const cec_command &command);

      bool                  IsMonitoring(void) const { return m_bMonitor; }
      CCECBusDevice *       GetDeviceByPhysicalAddress(uint16_t iPhysicalAddress, bool bSuppressUpdate = true);
      CCECBusDevice *       GetDeviceByType(cec_device_type type) const;
      CCECBusDevice *       GetPrimaryDevice(void) const;
      cec_version           GetDeviceCecVersion(cec_logical_address iAddress);
      bool                  GetDeviceMenuLanguage(cec_logical_address iAddress, cec_menu_language *language);
      CStdString            GetDeviceName(void) const;
      cec_osd_name          GetDeviceOSDName(cec_logical_address iAddress);
      uint64_t              GetDeviceVendorId(cec_logical_address iAddress);
      cec_power_status      GetDevicePowerStatus(cec_logical_address iAddress);
      cec_logical_address   GetLogicalAddress(void) const { return m_configuration.logicalAddresses.primary; }
      cec_logical_addresses GetLogicalAddresses(void) const { return m_configuration.logicalAddresses; }
      cec_logical_addresses GetActiveDevices(void);
      uint16_t              GetDevicePhysicalAddress(cec_logical_address iAddress);
      bool                  HasLogicalAddress(cec_logical_address address) const { return m_configuration.logicalAddresses.IsSet(address); }
      bool                  IsPresentDevice(cec_logical_address address);
      bool                  IsPresentDeviceType(cec_device_type type);
      uint16_t              GetPhysicalAddress(void) const;
      uint64_t              GetLastTransmission(void) const { return m_iLastTransmission; }
      cec_logical_address   GetActiveSource(bool bRequestActiveSource = true);
      bool                  IsActiveSource(cec_logical_address iAddress);
      bool                  IsInitialised(void);
      bool                  SetStreamPath(uint16_t iPhysicalAddress);
      cec_client_version    GetClientVersion(void) const { return (cec_client_version)m_configuration.clientVersion; };
      bool                  StandbyDevices(cec_logical_address address = CECDEVICE_BROADCAST);
      bool                  PowerOnDevices(cec_logical_address address = CECDEVICE_BROADCAST);

      bool SetActiveView(void);
      bool SetActiveSource(cec_device_type type = CEC_DEVICE_TYPE_RESERVED);
      bool SetDeckControlMode(cec_deck_control_mode mode, bool bSendUpdate = true);
      bool SetDeckInfo(cec_deck_info info, bool bSendUpdate = true);
      bool SetHDMIPort(cec_logical_address iBaseDevice, uint8_t iPort, bool bForce = false);
      bool TransmitInactiveSource(void);
      bool SetLogicalAddress(cec_logical_address iLogicalAddress);
      bool SetMenuState(cec_menu_state state, bool bSendUpdate = true);
      bool SetPhysicalAddress(uint16_t iPhysicalAddress, bool bSendUpdate = true);
      bool SetActiveSource(uint16_t iStreamPath);
      bool SwitchMonitoring(bool bEnable);
      bool PollDevice(cec_logical_address iAddress);
      uint8_t VolumeUp(bool bSendRelease = true);
      uint8_t VolumeDown(bool bSendRelease = true);
      uint8_t MuteAudio(bool bSendRelease = true);
      bool TransmitKeypress(cec_logical_address iDestination, cec_user_control_code key, bool bWait = true);
      bool TransmitKeyRelease(cec_logical_address iDestination, bool bWait = true);
      bool EnablePhysicalAddressDetection(void);
      void SetStandardLineTimeout(uint8_t iTimeout);
      void SetRetryLineTimeout(uint8_t iTimeout);
      bool GetCurrentConfiguration(libcec_configuration *configuration);
      bool SetConfiguration(const libcec_configuration *configuration);
      bool CanPersistConfiguration(void);
      bool PersistConfiguration(libcec_configuration *configuration);
      void RescanActiveDevices(void);

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
      const char *ToString(const cec_client_version version);
      const char *ToString(const cec_server_version version);

      static bool IsValidPhysicalAddress(uint16_t iPhysicalAddress);

      bool Transmit(const cec_command &data);
      void TransmitAbort(cec_logical_address address, cec_opcode opcode, cec_abort_reason reason = CEC_ABORT_REASON_UNRECOGNIZED_OPCODE);

      bool ChangeDeviceType(cec_device_type from, cec_device_type to);
      bool FindLogicalAddresses(void);
      bool SetAckMask(uint16_t iMask);

      bool StartBootloader(const char *strPort = NULL);
      bool PingAdapter(void);
      void HandlePoll(cec_logical_address initiator, cec_logical_address destination);
      bool HandleReceiveFailed(cec_logical_address initiator);

      bool GetDeviceInformation(const char *strPort, libcec_configuration *config, uint32_t iTimeoutMs = CEC_DEFAULT_CONNECT_TIMEOUT);

      bool TransmitPendingActiveSourceCommands(void);

      CCECBusDevice *  m_busDevices[16];

  private:
      bool OpenConnection(const char *strPort, uint16_t iBaudRate, uint32_t iTimeoutMs, bool bStartListening = true);
      bool Initialise(void);
      void SetInitialised(bool bSetTo = true);
      void CreateBusDevices(void);

      void ReplaceHandlers(void);
      bool PhysicalAddressInUse(uint16_t iPhysicalAddress);
      bool TryLogicalAddress(cec_logical_address address);
      bool FindLogicalAddressRecordingDevice(void);
      bool FindLogicalAddressTuner(void);
      bool FindLogicalAddressPlaybackDevice(void);
      bool FindLogicalAddressAudioSystem(void);

      void LogOutput(const cec_command &data);
      void ParseCommand(const cec_command &command);

      bool                                m_bConnectionOpened;
      bool                                m_bInitialised;
      PLATFORM::CMutex                    m_mutex;
      IAdapterCommunication *             m_communication;
      CLibCEC*                            m_controller;
      bool                                m_bMonitor;
      cec_keypress                        m_previousKey;
      PLATFORM::CThread *                 m_busScan;
      uint8_t                             m_iStandardLineTimeout;
      uint8_t                             m_iRetryLineTimeout;
      uint64_t                            m_iLastTransmission;
      CCECInputBuffer                     m_inBuffer;
      libcec_configuration                m_configuration;
  };
};
