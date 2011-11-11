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
#include <cec.h>
#include "util/buffer.h"

namespace CEC
{
  class CAdapterCommunication;
  class CCECProcessor;

  class CLibCEC : public ICECAdapter
  {
    public:
    /*!
     * ICECAdapter implementation
     */
    //@{
      CLibCEC(const char *strDeviceName, cec_device_type_list types);
      CLibCEC(const char *strDeviceName, cec_logical_address iLogicalAddress = CECDEVICE_PLAYBACKDEVICE1, uint16_t iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS);
      virtual ~CLibCEC(void);

      virtual bool Open(const char *strPort, uint32_t iTimeout = 10000);
      virtual void Close(void);
      virtual int8_t FindAdapters(cec_adapter *deviceList, uint8_t iBufSize, const char *strDevicePath = NULL);
      virtual bool PingAdapter(void);
      virtual bool StartBootloader(void);

      virtual int8_t GetMinLibVersion(void) const{ return CEC_MIN_LIB_VERSION; };
      virtual int8_t GetLibVersionMajor(void) const { return CEC_LIB_VERSION_MAJOR; };
      virtual int8_t GetLibVersionMinor(void) const { return CEC_LIB_VERSION_MINOR; };

      virtual bool GetNextLogMessage(cec_log_message *message);
      virtual bool GetNextKeypress(cec_keypress *key);
      virtual bool GetNextCommand(cec_command *command);

      virtual bool Transmit(const cec_command &data);
      virtual bool SetLogicalAddress(cec_logical_address iLogicalAddress = CECDEVICE_PLAYBACKDEVICE1);
      virtual bool SetPhysicalAddress(uint16_t iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS);

      virtual bool PowerOnDevices(cec_logical_address address = CECDEVICE_TV);
      virtual bool StandbyDevices(cec_logical_address address = CECDEVICE_BROADCAST);
      virtual bool SetActiveView(void);
      virtual bool SetActiveSource(cec_device_type type = CEC_DEVICE_TYPE_RESERVED);
      virtual bool SetDeckControlMode(cec_deck_control_mode mode, bool bSendUpdate = true);
      virtual bool SetDeckInfo(cec_deck_info info, bool bSendUpdate = true);
      virtual bool SetInactiveView(void);
      virtual bool SetMenuState(cec_menu_state state, bool bSendUpdate = true);
      virtual bool SetOSDString(cec_logical_address iLogicalAddress, cec_display_control duration, const char *strMessage);
      virtual bool SwitchMonitoring(bool bEnable);
      virtual cec_version GetDeviceCecVersion(cec_logical_address iAddress);
      virtual bool GetDeviceMenuLanguage(cec_logical_address iAddress, cec_menu_language *language);
      virtual uint64_t GetDeviceVendorId(cec_logical_address iAddress);
      virtual cec_power_status GetDevicePowerStatus(cec_logical_address iAddress);
      virtual bool PollDevice(cec_logical_address iAddress);
    //@}

      virtual void AddLog(cec_log_level level, const std::string &strMessage);
      virtual void AddKey(void);
      virtual void AddKey(cec_keypress &key);
      virtual void AddCommand(const cec_command &command);
      virtual void CheckKeypressTimeout(void);
      virtual void SetCurrentButton(cec_user_control_code iButtonCode);

    protected:
      int64_t                    m_iStartTime;
      cec_user_control_code      m_iCurrentButton;
      int64_t                    m_buttontime;
      CCECProcessor             *m_cec;
      CAdapterCommunication     *m_comm;
      CecBuffer<cec_log_message> m_logBuffer;
      CecBuffer<cec_keypress>    m_keyBuffer;
      CecBuffer<cec_command>     m_commandBuffer;
  };
};
