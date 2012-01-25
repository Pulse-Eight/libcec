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

#include <cectypes.h>
#include "../platform/os.h"

namespace PLATFORM
{
  class CSerialPort;
}

namespace CEC
{
  class CCECProcessor;
  class CCECAdapterMessage;

  class CAdapterCommunication : private PLATFORM::CThread
  {
  public:
    CAdapterCommunication(CCECProcessor *processor);
    virtual ~CAdapterCommunication();

    bool Open(const char *strPort, uint16_t iBaudRate = 38400, uint32_t iTimeoutMs = 10000);
    bool Read(CCECAdapterMessage &msg, uint32_t iTimeout = 1000);
    bool Write(CCECAdapterMessage *data);
    void Close(void);
    bool IsOpen(void);
    std::string GetError(void) const;

    void *Process(void);

    bool SetLineTimeout(uint8_t iTimeout);
    bool StartBootloader(void);
    bool SetAckMask(uint16_t iMask);
    bool PingAdapter(void);
    uint16_t GetFirmwareVersion(void);

    bool WaitForTransmitSucceeded(CCECAdapterMessage *message);

  private:
    void SendMessageToAdapter(CCECAdapterMessage *msg);
    void WriteNextCommand(void);
    void AddData(uint8_t *data, uint8_t iLen);
    bool ReadFromDevice(uint32_t iTimeout);

    PLATFORM::CSerialPort *                      m_port;
    CCECProcessor *                              m_processor;
    PLATFORM::SyncedBuffer<uint8_t>              m_inBuffer;
    PLATFORM::SyncedBuffer<CCECAdapterMessage *> m_outBuffer;
    PLATFORM::CMutex                             m_mutex;
    PLATFORM::CCondition                         m_rcvCondition;
    uint8_t                                      m_iLineTimeout;
    uint16_t                                     m_iFirmwareVersion;
  };
};
