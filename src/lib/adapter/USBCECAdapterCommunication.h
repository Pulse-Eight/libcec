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

#include "../../../include/cectypes.h"
#include "../platform/threads/threads.h"
#include "../platform/util/buffer.h"
#include "AdapterCommunication.h"
#include "USBCECAdapterMessage.h"

namespace PLATFORM
{
  class ISocket;
}

namespace CEC
{
  class CCECProcessor;
  class CAdapterPingThread;

  class CUSBCECAdapterCommunication : public IAdapterCommunication, private PLATFORM::CThread
  {
  public:
    CUSBCECAdapterCommunication(CCECProcessor *processor, const char *strPort, uint16_t iBaudRate = 38400);
    virtual ~CUSBCECAdapterCommunication() {};

    virtual bool Open(IAdapterCommunicationCallback *cb, uint32_t iTimeoutMs = 10000, bool bSkipChecks = false, bool bStartListening = true);
    virtual void Close(void);
    virtual bool IsOpen(void);
    virtual CStdString GetError(void) const;

    bool Read(cec_command &command, uint32_t iTimeout);
    cec_adapter_message_state Write(const cec_command &data, uint8_t iMaxTries, uint8_t iLineTimeout = 3, uint8_t iRetryLineTimeout = 3);

    virtual bool SetLineTimeout(uint8_t iTimeout);
    virtual bool StartBootloader(void);
    virtual bool SetAckMask(uint16_t iMask);
    virtual bool PingAdapter(void);
    virtual uint16_t GetFirmwareVersion(void);
    virtual bool SetControlledMode(bool controlled);
    virtual bool PersistConfiguration(libcec_configuration *configuration);
    virtual bool GetConfiguration(libcec_configuration *configuration);
    virtual CStdString GetPortName(void);
    virtual uint16_t GetPhysicalAddress(void) { return 0; }

    void *Process(void);
  private:
    bool SendCommand(cec_adapter_messagecode msgCode, CCECAdapterMessage &params, bool bExpectAck = true, bool bIsTransmission = false, bool bSendDirectly = true, bool bIsRetry = false);
    cec_datapacket GetSetting(cec_adapter_messagecode msgCode, uint8_t iResponseLength);

    bool SetSettingAutoEnabled(bool enabled);
    bool GetSettingAutoEnabled(bool &enabled);

    bool SetSettingDeviceType(cec_device_type type);
    bool GetSettingDeviceType(cec_device_type &type);

    bool SetSettingDefaultLogicalAddress(cec_logical_address address);
    bool GetSettingDefaultLogicalAddress(cec_logical_address &address);

    bool SetSettingLogicalAddressMask(uint16_t iMask);
    bool GetSettingLogicalAddressMask(uint16_t &iMask);

    bool SetSettingPhysicalAddress(uint16_t iPhysicalAddress);
    bool GetSettingPhysicalAddress(uint16_t &iPhysicalAddress);

    bool SetSettingCECVersion(cec_version version);
    bool GetSettingCECVersion(cec_version &version);

    bool SetSettingOSDName(const char *strOSDName);
    bool GetSettingOSDName(CStdString &strOSDName);

    bool WriteEEPROM(void);

    bool SetAckMaskInternal(uint16_t iMask, bool bWriteDirectly = false);

    bool CheckAdapter(uint32_t iTimeoutMs = 10000);
    bool Write(CCECAdapterMessage *data);
    bool Read(CCECAdapterMessage &msg, uint32_t iTimeout = 1000);
    bool ParseMessage(const CCECAdapterMessage &msg);
    void SendMessageToAdapter(CCECAdapterMessage *msg);
    void AddData(uint8_t *data, size_t iLen);
    bool ReadFromDevice(uint32_t iTimeout, size_t iSize = 256);
    bool WaitForAck(CCECAdapterMessage &message);

    PLATFORM::ISocket *                          m_port;
    CCECProcessor *                              m_processor;
    PLATFORM::SyncedBuffer<CCECAdapterMessage *> m_inBuffer;
    PLATFORM::CMutex                             m_mutex;
    PLATFORM::CCondition<volatile bool>          m_rcvCondition;
    volatile bool                                m_bHasData;
    uint8_t                                      m_iLineTimeout;
    uint16_t                                     m_iFirmwareVersion;
    cec_command                                  m_currentframe;
    cec_logical_address                          m_lastDestination;
    CCECAdapterMessage                           m_currentAdapterMessage;
    bool                                         m_bNextIsEscaped;
    bool                                         m_bGotStart;
    IAdapterCommunicationCallback *              m_callback;
    bool                                         m_bInitialised;
    bool                                         m_bWaitingForAck[15];
    CAdapterPingThread *                         m_pingThread;
  };

  class CAdapterPingThread : public PLATFORM::CThread
  {
  public:
    CAdapterPingThread(CUSBCECAdapterCommunication *com, uint32_t iTimeout) :
        m_com(com),
        m_timeout(iTimeout){}
    virtual ~CAdapterPingThread(void) {}

    virtual void* Process(void);
  private:
    CUSBCECAdapterCommunication *m_com;
    PLATFORM::CTimeout           m_timeout;
  };
};
