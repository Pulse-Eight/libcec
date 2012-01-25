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

namespace CEC
{
  typedef enum cec_adapter_message_state
  {
    ADAPTER_MESSAGE_STATE_UNKNOWN = 0,
    ADAPTER_MESSAGE_STATE_WAITING_TO_BE_SENT,
    ADAPTER_MESSAGE_STATE_SENT,
    ADAPTER_MESSAGE_STATE_SENT_NOT_ACKED,
    ADAPTER_MESSAGE_STATE_SENT_ACKED,
    ADAPTER_MESSAGE_STATE_INCOMING,
    ADAPTER_MESSAGE_STATE_ERROR
  } cec_adapter_message_state;


  class CCECAdapterMessage
  {
  public:
    CCECAdapterMessage(void)
    {
      Clear();
    }

    CCECAdapterMessage(const cec_command &command)
    {
      Clear();

      //set ack polarity to high when transmitting to the broadcast address
      //set ack polarity low when transmitting to any other address
      PushBack(MSGSTART);
      PushEscaped(MSGCODE_TRANSMIT_ACK_POLARITY);
      if (command.destination == CECDEVICE_BROADCAST)
        PushEscaped(CEC_TRUE);
      else
        PushEscaped(CEC_FALSE);
      PushBack(MSGEND);

      // add source and destination
      PushBack(MSGSTART);
      PushEscaped(command.opcode_set == 0 ? (uint8_t)MSGCODE_TRANSMIT_EOM : (uint8_t)MSGCODE_TRANSMIT);
      PushBack(((uint8_t)command.initiator << 4) + (uint8_t)command.destination);
      PushBack(MSGEND);

      // add opcode
      if (command.opcode_set == 1)
      {
        PushBack(MSGSTART);
        PushEscaped(command.parameters.IsEmpty() ? (uint8_t)MSGCODE_TRANSMIT_EOM : (uint8_t)MSGCODE_TRANSMIT);
        PushBack((uint8_t) command.opcode);
        PushBack(MSGEND);

        // add parameters
        for (int8_t iPtr = 0; iPtr < command.parameters.size; iPtr++)
        {
          PushBack(MSGSTART);

          if (iPtr == command.parameters.size - 1)
            PushEscaped( MSGCODE_TRANSMIT_EOM);
          else
            PushEscaped(MSGCODE_TRANSMIT);

          PushEscaped(command.parameters[iPtr]);

          PushBack(MSGEND);
        }
      }

      // set timeout
      transmit_timeout = command.transmit_timeout;
      //TODO
    }

    CCECAdapterMessage &operator=(const CCECAdapterMessage &msg)
    {
      packet = msg.packet;
      state  = msg.state;
      return *this;
    }

    CStdString ToString(void) const
    {
      CStdString strMsg;
      if (Size() == 0)
      {
        strMsg = "empty message";
      }
      else
      {
        strMsg = MessageCodeAsString();

        switch (Message())
        {
        case MSGCODE_TIMEOUT_ERROR:
        case MSGCODE_HIGH_ERROR:
        case MSGCODE_LOW_ERROR:
          {
            uint32_t iLine = (Size() >= 3) ? (At(1) << 8) | At(2) : 0;
            uint32_t iTime = (Size() >= 7) ? (At(3) << 24) | (At(4) << 16) | (At(5) << 8) | At(6) : 0;
            strMsg.AppendFormat(" line:%u", iLine);
            strMsg.AppendFormat(" time:%u", iTime);
          }
          break;
        case MSGCODE_FRAME_START:
          if (Size() >= 2)
            strMsg.AppendFormat(" initiator:%1x destination:%1x ack:%s %s", Initiator(), Destination(), IsACK() ? "high" : "low", IsEOM() ? "eom" : "");
          break;
        case MSGCODE_FRAME_DATA:
          if (Size() >= 2)
            strMsg.AppendFormat(" %02x %s", At(1), IsEOM() ? "eom" : "");
          break;
        default:
          break;
        }
      }

      return strMsg;
    }

    CStdString MessageCodeAsString(void) const
    {
      CStdString strMsg;
      switch (Message())
      {
      case MSGCODE_NOTHING:
        strMsg = "NOTHING";
        break;
      case MSGCODE_PING:
        strMsg = "PING";
        break;
      case MSGCODE_TIMEOUT_ERROR:
        strMsg = "TIMEOUT";
        break;
      case MSGCODE_HIGH_ERROR:
        strMsg = "HIGH_ERROR";
        break;
      case MSGCODE_LOW_ERROR:
        strMsg = "LOW_ERROR";
        break;
      case MSGCODE_FRAME_START:
        strMsg = "FRAME_START";
        break;
      case MSGCODE_FRAME_DATA:
        strMsg = "FRAME_DATA";
        break;
      case MSGCODE_RECEIVE_FAILED:
        strMsg = "RECEIVE_FAILED";
        break;
      case MSGCODE_COMMAND_ACCEPTED:
        strMsg = "COMMAND_ACCEPTED";
        break;
      case MSGCODE_COMMAND_REJECTED:
        strMsg = "COMMAND_REJECTED";
        break;
      case MSGCODE_SET_ACK_MASK:
        strMsg = "SET_ACK_MASK";
        break;
      case MSGCODE_TRANSMIT:
        strMsg = "TRANSMIT";
        break;
      case MSGCODE_TRANSMIT_EOM:
        strMsg = "TRANSMIT_EOM";
        break;
      case MSGCODE_TRANSMIT_IDLETIME:
        strMsg = "TRANSMIT_IDLETIME";
        break;
      case MSGCODE_TRANSMIT_ACK_POLARITY:
        strMsg = "TRANSMIT_ACK_POLARITY";
        break;
      case MSGCODE_TRANSMIT_LINE_TIMEOUT:
        strMsg = "TRANSMIT_LINE_TIMEOUT";
        break;
      case MSGCODE_TRANSMIT_SUCCEEDED:
        strMsg = "TRANSMIT_SUCCEEDED";
        break;
      case MSGCODE_TRANSMIT_FAILED_LINE:
        strMsg = "TRANSMIT_FAILED_LINE";
        break;
      case MSGCODE_TRANSMIT_FAILED_ACK:
        strMsg = "TRANSMIT_FAILED_ACK";
        break;
      case MSGCODE_TRANSMIT_FAILED_TIMEOUT_DATA:
        strMsg = "TRANSMIT_FAILED_TIMEOUT_DATA";
        break;
      case MSGCODE_TRANSMIT_FAILED_TIMEOUT_LINE:
        strMsg = "TRANSMIT_FAILED_TIMEOUT_LINE";
        break;
      case MSGCODE_FIRMWARE_VERSION:
        strMsg = "FIRMWARE_VERSION";
        break;
      case MSGCODE_START_BOOTLOADER:
        strMsg = "START_BOOTLOADER";
        break;
      case MSGCODE_FRAME_EOM:
        strMsg = "FRAME_EOM";
        break;
      case MSGCODE_FRAME_ACK:
        strMsg = "FRAME_ACK";
        break;
      }

      return strMsg;
    }

    uint8_t operator[](uint8_t pos) const
    {
      return packet[pos];
    }

    uint8_t At(uint8_t pos) const
    {
      return packet[pos];
    }

    uint8_t Size(void) const
    {
      return packet.size;
    }

    bool IsEmpty(void) const
    {
      return packet.IsEmpty();
    }

    void Clear(void)
    {
      state               = ADAPTER_MESSAGE_STATE_UNKNOWN;
      transmit_timeout    = 0;
      packet.Clear();
      maxTries            = CEC_DEFAULT_TRANSMIT_RETRIES + 1;
      tries               = 0;
      reply               = MSGCODE_NOTHING;
      isTransmission      = true;
      expectControllerAck = true;
    }

    void Shift(uint8_t iShiftBy)
    {
      packet.Shift(iShiftBy);
    }

    void PushBack(uint8_t add)
    {
      packet.PushBack(add);
    }

    void PushEscaped(uint8_t byte)
    {
      if (byte >= MSGESC)
      {
        PushBack(MSGESC);
        PushBack(byte - ESCOFFSET);
      }
      else
      {
        PushBack(byte);
      }
    }

    cec_adapter_messagecode Message(void) const
    {
      return packet.size >= 1 ?
          (cec_adapter_messagecode) (packet.At(0) & ~(MSGCODE_FRAME_EOM | MSGCODE_FRAME_ACK)) :
          MSGCODE_NOTHING;
    }

    bool IsEOM(void) const
    {
      return packet.size >= 1 ?
          (packet.At(0) & MSGCODE_FRAME_EOM) != 0 :
          false;
    }

    bool IsACK(void) const
    {
      return packet.size >= 1 ?
          (packet.At(0) & MSGCODE_FRAME_ACK) != 0 :
          false;
    }

    bool IsError(void) const
    {
      cec_adapter_messagecode code = Message();
      return (code == MSGCODE_HIGH_ERROR ||
              code == MSGCODE_LOW_ERROR ||
              code == MSGCODE_RECEIVE_FAILED ||
              code == MSGCODE_COMMAND_REJECTED ||
              code == MSGCODE_TRANSMIT_LINE_TIMEOUT ||
              code == MSGCODE_TRANSMIT_FAILED_LINE ||
              code == MSGCODE_TRANSMIT_FAILED_ACK ||
              code == MSGCODE_TRANSMIT_FAILED_TIMEOUT_DATA ||
              code == MSGCODE_TRANSMIT_FAILED_TIMEOUT_LINE);
    }

    bool NeedsRetry(void) const
    {
      return reply == MSGCODE_NOTHING ||
             reply == MSGCODE_RECEIVE_FAILED ||
             reply == MSGCODE_TIMEOUT_ERROR ||
             reply == MSGCODE_TRANSMIT_FAILED_LINE ||
             reply == MSGCODE_TRANSMIT_FAILED_TIMEOUT_DATA ||
             reply == MSGCODE_TRANSMIT_FAILED_TIMEOUT_LINE ||
             reply == MSGCODE_TRANSMIT_LINE_TIMEOUT;
    }

    cec_logical_address Initiator(void) const
    {
      return packet.size >= 2 ?
          (cec_logical_address) (packet.At(1) >> 4) :
          CECDEVICE_UNKNOWN;
    }

    cec_logical_address Destination(void) const
    {
      return packet.size >= 2 ?
          (cec_logical_address) (packet.At(1) & 0xF) :
          CECDEVICE_UNKNOWN;
    }

    uint8_t                   maxTries;
    uint8_t                   tries;
    cec_adapter_messagecode   reply;
    cec_datapacket            packet;
    cec_adapter_message_state state;
    int32_t                   transmit_timeout;
    bool                      isTransmission;
    bool                      expectControllerAck;
    PLATFORM::CMutex          mutex;
    PLATFORM::CCondition      condition;
  };
}
