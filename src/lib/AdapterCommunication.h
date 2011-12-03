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
#include "platform/threads.h"
#include "util/buffer.h"
#include "util/StdString.h"
#include <string>

namespace CEC
{
  typedef enum cec_adapter_message_state
  {
    ADAPTER_MESSAGE_STATE_UNKNOWN = 0,
    ADAPTER_MESSAGE_STATE_WAITING,
    ADAPTER_MESSAGE_STATE_SENT,
    ADAPTER_MESSAGE_STATE_RECEIVED,
    ADAPTER_MESSAGE_STATE_ERROR
  } cec_adapter_message_state;


  class CCECAdapterMessage
  {
  public:
    CCECAdapterMessage(void) { clear(); }
    CCECAdapterMessage(const cec_command &command);
    CCECAdapterMessage &operator =(const CCECAdapterMessage &msg);
    CStdString ToString(void) const;
    CStdString MessageCodeAsString(void) const;

    bool                    empty(void) const             { return packet.IsEmpty(); }
    uint8_t                 operator[](uint8_t pos) const { return packet[pos]; }
    uint8_t                 at(uint8_t pos) const         { return packet[pos]; }
    uint8_t                 size(void) const              { return packet.size; }
    void                    clear(void)                   { state = ADAPTER_MESSAGE_STATE_UNKNOWN; transmit_timeout = 0; packet.Clear(); maxTries = 5; tries = 0; reply = MSGCODE_NOTHING; }
    void                    shift(uint8_t iShiftBy)       { packet.Shift(iShiftBy); }
    void                    push_back(uint8_t add)        { packet.PushBack(add); }
    cec_adapter_messagecode message(void) const           { return packet.size >= 1 ? (cec_adapter_messagecode) (packet.At(0) & ~(MSGCODE_FRAME_EOM | MSGCODE_FRAME_ACK))  : MSGCODE_NOTHING; }
    bool                    eom(void) const               { return packet.size >= 1 ? (packet.At(0) & MSGCODE_FRAME_EOM) != 0 : false; }
    bool                    ack(void) const               { return packet.size >= 1 ? (packet.At(0) & MSGCODE_FRAME_ACK) != 0 : false; }
    cec_logical_address     initiator(void) const         { return packet.size >= 2 ? (cec_logical_address) (packet.At(1) >> 4)  : CECDEVICE_UNKNOWN; };
    cec_logical_address     destination(void) const       { return packet.size >= 2 ? (cec_logical_address) (packet.At(1) & 0xF) : CECDEVICE_UNKNOWN; };
    bool                    is_error(void) const;
    void                    push_escaped(uint8_t byte);
    bool                    needs_retry(void) const       { return reply == MSGCODE_NOTHING ||
                                                                   reply == MSGCODE_RECEIVE_FAILED ||
                                                                   reply == MSGCODE_TIMEOUT_ERROR ||
                                                                   reply == MSGCODE_TRANSMIT_FAILED_ACK ||
                                                                   reply == MSGCODE_TRANSMIT_FAILED_LINE ||
                                                                   reply == MSGCODE_TRANSMIT_FAILED_TIMEOUT_DATA ||
                                                                   reply == MSGCODE_TRANSMIT_FAILED_TIMEOUT_LINE ||
                                                                   reply == MSGCODE_TRANSMIT_LINE_TIMEOUT; }

    uint8_t                   maxTries;
    uint8_t                   tries;
    cec_adapter_messagecode   reply;
    cec_datapacket            packet;
    cec_adapter_message_state state;
    int32_t                   transmit_timeout;
    CMutex                    mutex;
    CCondition                condition;
  };

  class CSerialPort;
  class CCECProcessor;

  class CAdapterCommunication : private CThread
  {
  public:
    CAdapterCommunication(CCECProcessor *processor);
    virtual ~CAdapterCommunication();

    bool Open(const char *strPort, uint16_t iBaudRate = 38400, uint32_t iTimeoutMs = 10000);
    bool Read(CCECAdapterMessage &msg, uint32_t iTimeout = 1000);
    bool Write(CCECAdapterMessage *data);
    bool PingAdapter(void);
    void Close(void);
    bool IsOpen(void) const;
    std::string GetError(void) const;

    void *Process(void);

    bool StartBootloader(void);

  private:
    void SendMessageToAdapter(CCECAdapterMessage *msg);
    void WriteNextCommand(void);
    void AddData(uint8_t *data, uint8_t iLen);
    bool ReadFromDevice(uint32_t iTimeout);

    CSerialPort *                    m_port;
    CCECProcessor *                  m_processor;
    CecBuffer<uint8_t>               m_inBuffer;
    CecBuffer<CCECAdapterMessage *>  m_outBuffer;
    CMutex                           m_mutex;
    CCondition                       m_rcvCondition;
    CCondition                       m_startCondition;
  };
};
