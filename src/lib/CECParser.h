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

#include <queue>
#include <stdio.h>
#include "../../include/CECExports.h"
#include "../../include/CECTypes.h"
#include "util/threads.h"
#include "util/buffer.h"

class CSerialPort;

namespace CEC
{
  class CCommunication;

  class CCECParser : public ICECDevice, public CThread
  {
    public:
    /*!
     * ICECDevice implementation
     */
    //@{
      CCECParser(const char *strDeviceName, cec_logical_address iLogicalAddress = CECDEVICE_PLAYBACKDEVICE1, int iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS);
      virtual ~CCECParser(void);

      virtual bool Open(const char *strPort, int iTimeout = 10000);
      virtual void Close(void);
      virtual int  FindDevices(std::vector<cec_device> &deviceList, const char *strDevicePath = NULL);
      virtual bool Ping(void);
      virtual bool StartBootloader(void);
      virtual bool PowerOffDevices(cec_logical_address address = CECDEVICE_BROADCAST);
      virtual bool PowerOnDevices(cec_logical_address address = CECDEVICE_TV);
      virtual bool StandbyDevices(cec_logical_address address = CECDEVICE_BROADCAST);
      virtual bool SetActiveView(void);
      virtual bool SetInactiveView(void);
      virtual bool GetNextLogMessage(cec_log_message *message);
      virtual bool GetNextKeypress(cec_keypress *key);
      virtual bool GetNextCommand(cec_command *command);
      virtual bool Transmit(const cec_frame &data, bool bWaitForAck = true);
      virtual bool SetLogicalAddress(cec_logical_address iLogicalAddress);
      virtual bool SetAckMask(uint16_t iMask);
      virtual int  GetMinVersion(void);
      virtual int  GetLibVersion(void);
    //@}

      void *Process(void);
      void AddLog(cec_log_level level, const std::string &strMessage);
    protected:
      virtual bool TransmitFormatted(const cec_frame &data, bool bWaitForAck = true);
      virtual void TransmitAbort(cec_logical_address address, cec_opcode opcode, ECecAbortReason reason = CEC_ABORT_REASON_UNRECOGNIZED_OPCODE);
      virtual void ReportCECVersion(cec_logical_address address = CECDEVICE_TV);
      virtual void ReportPowerState(cec_logical_address address = CECDEVICE_TV, bool bOn = true);
      virtual void ReportMenuState(cec_logical_address address = CECDEVICE_TV, bool bActive = true);
      virtual void ReportVendorID(cec_logical_address address = CECDEVICE_TV);
      virtual void ReportOSDName(cec_logical_address address = CECDEVICE_TV);
      virtual void ReportPhysicalAddress(void);
      virtual void BroadcastActiveSource(void);
      virtual uint8_t GetSourceDestination(cec_logical_address destination = CECDEVICE_BROADCAST);

    private:
      void AddKey(void);
      void AddCommand(cec_logical_address source, cec_logical_address destination, cec_opcode opcode, cec_frame *parameters);
      bool WaitForAck(int iTimeout = 1000);
      bool ReadFromDevice(int iTimeout);
      void ProcessMessages(void);
      bool GetMessage(cec_frame &msg, bool bFromBuffer = true);
      bool ParseMessage(cec_frame &msg);
      void ParseCurrentFrame(void);

      void AddData(uint8_t* data, int len);
      void PushEscaped(cec_frame &vec, uint8_t iByte);

      void CheckKeypressTimeout(int64_t now);

      cec_frame                  m_currentframe;
      cec_user_control_code      m_iCurrentButton;
      int64_t                    m_buttontime;
      int                        m_physicaladdress;
      cec_logical_address        m_iLogicalAddress;
      CecBuffer<cec_frame>       m_frameBuffer;
      CecBuffer<cec_log_message> m_logBuffer;
      CecBuffer<cec_keypress>    m_keyBuffer;
      CecBuffer<cec_command>     m_commandBuffer;
      std::string                m_strDeviceName;
      CMutex                     m_mutex;
      CCommunication            *m_communication;
  };
};
