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
      virtual ~CCECProcessor(void);

      virtual bool Start(void);
      void *Process(void);

      virtual bool SetActiveView(void);
      virtual bool SetInactiveView(void);
      virtual bool Transmit(const cec_command &data, bool bWaitForAck = true);
      virtual bool SetLogicalAddress(cec_logical_address iLogicalAddress);
      virtual bool SetPhysicalAddress(uint16_t iPhysicalAddress);
      virtual bool SwitchMonitoring(bool bEnable);

      virtual cec_logical_address GetLogicalAddress(void) const { return m_iLogicalAddress; }
      virtual uint16_t GetPhysicalAddress(void) const;
      virtual const std::string &GetDeviceName(void) { return m_strDeviceName; }

      virtual void SetCurrentButton(cec_user_control_code iButtonCode);
      virtual void AddCommand(const cec_command &command);
      virtual void AddKey(void);
      virtual void AddLog(cec_log_level level, const CStdString &strMessage);

      virtual bool TransmitFormatted(const cec_adapter_message &data, bool bWaitForAck = true);
      virtual void TransmitAbort(cec_logical_address address, cec_opcode opcode, ECecAbortReason reason = CEC_ABORT_REASON_UNRECOGNIZED_OPCODE);

      CCECBusDevice *m_busDevices[16];

    private:
      void LogOutput(const cec_command &data);
      bool WaitForAck(bool *bError, uint8_t iLength, uint32_t iTimeout = 1000);
      bool ParseMessage(cec_adapter_message &msg);
      void ParseCommand(cec_command &command);

      cec_command                    m_currentframe;
      cec_logical_address            m_iLogicalAddress;
      CecBuffer<cec_adapter_message> m_frameBuffer;
      std::string                    m_strDeviceName;
      CMutex                         m_mutex;
      CAdapterCommunication         *m_communication;
      CLibCEC                       *m_controller;
      bool                           m_bMonitor;
  };
};
