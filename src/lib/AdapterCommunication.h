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
#include <string>

namespace CEC
{
  class CCECAdapterMessage
  {
  public:
    CCECAdapterMessage(void) {}
    CCECAdapterMessage(const cec_command &command)
    {
      clear();

      //set ack polarity to high when transmitting to the broadcast address
      //set ack polarity low when transmitting to any other address
      push_back(MSGSTART);
      push_escaped(MSGCODE_TRANSMIT_ACK_POLARITY);
      if (command.destination == CECDEVICE_BROADCAST)
        push_escaped(CEC_TRUE);
      else
        push_escaped(CEC_FALSE);
      push_back(MSGEND);

      // add source and destination
      push_back(MSGSTART);
      push_escaped(MSGCODE_TRANSMIT);
      push_back(((uint8_t)command.initiator << 4) + (uint8_t)command.destination);
      push_back(MSGEND);

      // add opcode
      push_back(MSGSTART);
      push_escaped(command.parameters.empty() ? (uint8_t)MSGCODE_TRANSMIT_EOM : (uint8_t)MSGCODE_TRANSMIT);
      push_back((uint8_t) command.opcode);
      push_back(MSGEND);

      // add parameters
      for (int8_t iPtr = 0; iPtr < command.parameters.size; iPtr++)
      {
        push_back(MSGSTART);

        if (iPtr == command.parameters.size - 1)
          push_escaped( MSGCODE_TRANSMIT_EOM);
        else
          push_escaped(MSGCODE_TRANSMIT);

        push_escaped(command.parameters[iPtr]);

        push_back(MSGEND);
      }
    }

    CCECAdapterMessage &operator =(const CCECAdapterMessage &msg)
    {
      packet = msg.packet;
      return *this;
    }

    bool                    empty(void) const             { return packet.empty(); }
    uint8_t                 operator[](uint8_t pos) const { return packet[pos]; }
    uint8_t                 at(uint8_t pos) const         { return packet[pos]; }
    uint8_t                 size(void) const              { return packet.size; }
    void                    clear(void)                   { packet.clear(); }
    void                    shift(uint8_t iShiftBy)       { packet.shift(iShiftBy); }
    void                    push_back(uint8_t add)        { packet.push_back(add); }
    cec_adapter_messagecode message(void) const           { return packet.size >= 1 ? (cec_adapter_messagecode) (packet.at(0) & ~(MSGCODE_FRAME_EOM | MSGCODE_FRAME_ACK))  : MSGCODE_NOTHING; }
    bool                    eom(void) const               { return packet.size >= 1 ? (packet.at(0) & MSGCODE_FRAME_EOM) != 0 : false; }
    bool                    ack(void) const               { return packet.size >= 1 ? (packet.at(0) & MSGCODE_FRAME_ACK) != 0 : false; }
    cec_logical_address     initiator(void) const         { return packet.size >= 2 ? (cec_logical_address) (packet.at(1) >> 4)  : CECDEVICE_UNKNOWN; };
    cec_logical_address     destination(void) const       { return packet.size >= 2 ? (cec_logical_address) (packet.at(1) & 0xF) : CECDEVICE_UNKNOWN; };
    void                    push_escaped(int16_t byte)
    {
      if (byte >= MSGESC && byte != MSGSTART)
      {
        push_back(MSGESC);
        push_back(byte - ESCOFFSET);
      }
      else
         push_back(byte);
    }

    cec_datapacket packet;
  };

  class CSerialPort;
  class CLibCEC;

  class CAdapterCommunication : private CThread
  {
  public:
    CAdapterCommunication(CLibCEC *controller);
    virtual ~CAdapterCommunication();

    bool Open(const char *strPort, uint16_t iBaudRate = 38400, uint32_t iTimeoutMs = 10000);
    bool Read(CCECAdapterMessage &msg, uint32_t iTimeout = 1000);
    bool Write(const CCECAdapterMessage &frame);
    bool PingAdapter(void);
    void Close(void);
    bool IsOpen(void) const;
    std::string GetError(void) const;

    void *Process(void);

    bool StartBootloader(void);
    bool SetAckMask(uint16_t iMask);

  private:
    void WriteNextCommand(void);
    void AddData(uint8_t *data, uint8_t iLen);
    bool ReadFromDevice(uint32_t iTimeout);

    CSerialPort *                 m_port;
    CLibCEC *                     m_controller;
    CecBuffer<uint8_t>            m_inBuffer;
    CecBuffer<CCECAdapterMessage> m_outBuffer;
    CMutex                        m_bufferMutex;
    CCondition                    m_rcvCondition;
  };
};
