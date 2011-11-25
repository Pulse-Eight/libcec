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
#include "platform/threads.h"
#include "util/buffer.h"
#include "util/StdString.h"

class CSerialPort;

namespace CEC
{
  class CLibCEC;
  class CAdapterCommunication;
  class CCECBusDevice;

  class CCECProcessor : public CThread
  {
    public:
      CCECProcessor(CLibCEC *controller, CAdapterCommunication *serComm, const char *strDeviceName, cec_logical_address iLogicalAddress = CECDEVICE_PLAYBACKDEVICE1, uint16_t iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS);
      CCECProcessor(CLibCEC *controller, CAdapterCommunication *serComm, const char *strDeviceName, const cec_device_type_list &types);
      virtual ~CCECProcessor(void);

      virtual bool Start(void);
      virtual void *Process(void);

      virtual bool                  IsMonitoring(void) const { return m_bMonitor; }
      virtual CCECBusDevice *       GetDeviceByPhysicalAddress(uint16_t iPhysicalAddress) const;
      virtual CCECBusDevice *       GetDeviceByType(cec_device_type type) const;
      virtual cec_version           GetDeviceCecVersion(cec_logical_address iAddress);
      virtual bool                  GetDeviceMenuLanguage(cec_logical_address iAddress, cec_menu_language *language);
      virtual const std::string &   GetDeviceName(void) { return m_strDeviceName; }
      virtual uint64_t              GetDeviceVendorId(cec_logical_address iAddress);
      virtual cec_power_status      GetDevicePowerStatus(cec_logical_address iAddress);
      virtual cec_logical_address   GetLogicalAddress(void) const { return m_logicalAddresses.primary; }
      virtual cec_logical_addresses GetLogicalAddresses(void) const { return m_logicalAddresses; }
      virtual bool                  HasLogicalAddress(cec_logical_address address) const { return m_logicalAddresses.IsSet(address); }
      virtual uint16_t              GetPhysicalAddress(void) const;

      virtual bool SetActiveView(void);
      virtual bool SetActiveSource(cec_device_type type = CEC_DEVICE_TYPE_RESERVED);
      virtual bool SetDeckControlMode(cec_deck_control_mode mode, bool bSendUpdate = true);
      virtual bool SetDeckInfo(cec_deck_info info, bool bSendUpdate = true);
      virtual bool SetInactiveView(void);
      virtual bool SetLogicalAddress(cec_logical_address iLogicalAddress);
      virtual bool SetMenuState(cec_menu_state state, bool bSendUpdate = true);
      virtual bool SetPhysicalAddress(uint16_t iPhysicalAddress);
      virtual bool SetStreamPath(uint16_t iStreamPath);
      virtual bool SwitchMonitoring(bool bEnable);
      virtual bool PollDevice(cec_logical_address iAddress);

      virtual bool Transmit(const cec_command &data);
      virtual bool Transmit(CCECAdapterMessage *output);
      virtual void TransmitAbort(cec_logical_address address, cec_opcode opcode, cec_abort_reason reason = CEC_ABORT_REASON_UNRECOGNIZED_OPCODE);

      virtual void SetCurrentButton(cec_user_control_code iButtonCode);
      virtual void AddCommand(const cec_command &command);
      virtual void AddKey(cec_keypress &key);
      virtual void AddKey(void);
      virtual void AddLog(cec_log_level level, const CStdString &strMessage);

      virtual bool FindLogicalAddresses(void);
      virtual bool SetAckMask(uint16_t iMask);

      CCECBusDevice *m_busDevices[16];

  private:
      bool TryLogicalAddress(cec_logical_address address);
      bool FindLogicalAddressRecordingDevice(void);
      bool FindLogicalAddressTuner(void);
      bool FindLogicalAddressPlaybackDevice(void);
      bool FindLogicalAddressAudioSystem(void);

      void LogOutput(const cec_command &data);
      bool WaitForTransmitSucceeded(uint8_t iLength, uint32_t iTimeout = 1000);
      bool ParseMessage(const CCECAdapterMessage &msg);
      void ParseCommand(cec_command &command);

      bool                   m_bStarted;
      cec_command            m_currentframe;
      cec_logical_addresses  m_logicalAddresses;
      std::string            m_strDeviceName;
      cec_device_type_list   m_types;
      CMutex                 m_mutex;
      CCondition             m_startCondition;
      CAdapterCommunication* m_communication;
      CLibCEC*               m_controller;
      bool                   m_bMonitor;
      CecBuffer<cec_command> m_commandBuffer;
  };
};
