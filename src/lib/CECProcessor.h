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
      if (bReturn)
        m_condition.Signal();

      return bReturn;
    }

    bool Pop(cec_command &command, uint16_t iTimeout = 10000)
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

      virtual bool Start(const char *strPort, uint16_t iBaudRate = 38400, uint32_t iTimeoutMs = 10000);
      virtual void *Process(void);
      virtual void Close(void);

      virtual bool                  OnCommandReceived(const cec_command &command);

      virtual bool                  IsMonitoring(void) const { return m_bMonitor; }
      virtual CCECBusDevice *       GetDeviceByPhysicalAddress(uint16_t iPhysicalAddress, bool bRefresh = false) const;
      virtual CCECBusDevice *       GetDeviceByType(cec_device_type type) const;
      virtual CCECBusDevice *       GetPrimaryDevice(void) const;
      virtual cec_version           GetDeviceCecVersion(cec_logical_address iAddress);
      virtual bool                  GetDeviceMenuLanguage(cec_logical_address iAddress, cec_menu_language *language);
      virtual CStdString            GetDeviceName(void) const
      {
        CStdString strName;
        strName = m_configuration.strDeviceName;
        return strName;
      }
      virtual cec_osd_name          GetDeviceOSDName(cec_logical_address iAddress);
      virtual uint64_t              GetDeviceVendorId(cec_logical_address iAddress);
      virtual cec_power_status      GetDevicePowerStatus(cec_logical_address iAddress);
      virtual cec_logical_address   GetLogicalAddress(void) const { return m_configuration.logicalAddresses.primary; }
      virtual cec_logical_addresses GetLogicalAddresses(void) const { return m_configuration.logicalAddresses; }
      virtual cec_logical_addresses GetActiveDevices(void);
      virtual uint16_t              GetDevicePhysicalAddress(cec_logical_address iAddress);
      virtual bool                  HasLogicalAddress(cec_logical_address address) const { return m_configuration.logicalAddresses.IsSet(address); }
      virtual bool                  IsPresentDevice(cec_logical_address address);
      virtual bool                  IsPresentDeviceType(cec_device_type type);
      virtual uint16_t              GetPhysicalAddress(void) const;
      virtual uint64_t              GetLastTransmission(void) const { return m_iLastTransmission; }
      virtual cec_logical_address   GetActiveSource(void);
      virtual bool                  IsActiveSource(cec_logical_address iAddress);
      virtual bool                  IsInitialised(void);
      virtual bool                  SetStreamPath(uint16_t iPhysicalAddress);
      virtual cec_client_version    GetClientVersion(void) const { return (cec_client_version)m_configuration.clientVersion; };
      virtual bool                  StandbyDevices(cec_logical_address address = CECDEVICE_BROADCAST);
      virtual bool                  PowerOnDevices(cec_logical_address address = CECDEVICE_BROADCAST);

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
      virtual bool EnablePhysicalAddressDetection(void);
      void SetStandardLineTimeout(uint8_t iTimeout);
      void SetRetryLineTimeout(uint8_t iTimeout);
      virtual bool GetCurrentConfiguration(libcec_configuration *configuration);
      virtual bool SetConfiguration(const libcec_configuration *configuration);
      virtual bool CanPersistConfiguration(void);
      virtual bool PersistConfiguration(libcec_configuration *configuration);
      virtual void RescanActiveDevices(void);

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

      virtual bool Transmit(const cec_command &data);
      virtual void TransmitAbort(cec_logical_address address, cec_opcode opcode, cec_abort_reason reason = CEC_ABORT_REASON_UNRECOGNIZED_OPCODE);

      virtual bool ChangeDeviceType(cec_device_type from, cec_device_type to);
      virtual bool FindLogicalAddresses(void);
      virtual bool SetAckMask(uint16_t iMask);

      virtual bool StartBootloader(const char *strPort = NULL);
      virtual bool PingAdapter(void);
      virtual void HandlePoll(cec_logical_address initiator, cec_logical_address destination);
      virtual bool HandleReceiveFailed(cec_logical_address initiator);

      virtual bool GetDeviceInformation(const char *strPort, libcec_configuration *config, uint32_t iTimeoutMs = 10000);

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
