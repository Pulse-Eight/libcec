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

#include "lib/adapter/AdapterCommunication.h"

namespace CEC
{
  class CCECAdapterMessage
  {
  public:
    /*!
     * @brief Create an empty message.
     */
    CCECAdapterMessage(void);

    /*!
     * @brief Create a message with a command that is to be transmitted over the CEC line.
     * @param command The command to transmit.
     * @param iLineTimeout The line timeout to use when sending this message.
     */
    CCECAdapterMessage(const cec_command &command, uint8_t iLineTimeout = 3);

    /*!
     * @return the message as human readable string.
     */
    std::string ToString(void) const;

    /*!
     * @brief Translate the messagecode into a human readable string.
     * @param msgCode The messagecode to translate.
     * @return The messagecode as string.
     */
    static const char *ToString(cec_adapter_messagecode msgCode);

    /*!
     * @brief Get the byte at the given position.
     * @param pos The position to get.
     * @return The requested byte, or 0 when it's out of range.
     */
    uint8_t At(uint8_t pos) const;
    uint8_t operator[](uint8_t pos) const;

    /*!
     * @return The size of the packet in bytes.
     */
    uint8_t Size(void) const;

    /*!
     * @return True when empty, false otherwise.
     */
    bool IsEmpty(void) const;

    /*!
     * @brief Clear this message and reset everything to the initial values.
     */
    void Clear(void);

    /*!
     * @brief Shift the message by the given number of bytes.
     * @param iShiftBy The number of bytes to shift.
     */
    void Shift(uint8_t iShiftBy);

    /*!
     * @brief Append the given message to this message.
     * @param data The message to append.
     */
    void Append(CCECAdapterMessage &data);

    /*!
     * @brief Append the given datapacket to this message.
     * @param data The packet to add.
     */
    void Append(cec_datapacket &data);

    /*!
     * @brief Adds a byte to this message. Does not escape the byte.
     * @param byte The byte to add.
     */
    void PushBack(uint8_t byte);

    /*!
     * @brief Adds a byte to this message and escapes the byte if needed.
     * @param byte The byte to add.
     */
    void PushEscaped(uint8_t byte);

    /*!
     * @brief Adds a byte to this message.
     * @param byte The byte to add.
     * @return True when a full message was received, false otherwise.
     */
    bool PushReceivedByte(uint8_t byte);

    /*!
     * @return The messagecode inside this adapter message, or MSGCODE_NOTHING if there is none.
     */
    cec_adapter_messagecode Message(void) const;

    /*!
     * @return The messagecode (if provided) that this message is responding to
     */
    cec_adapter_messagecode ResponseTo(void) const;

    /*!
     * @return True when this message is a transmission, false otherwise.
     */
    bool IsTranmission(void) const;

    /*!
     * @return True when the EOM bit is set, false otherwise.
     */
    bool IsEOM(void) const;

    /*!
     * @return True when the ACK bit is set, false otherwise.
     */
    bool IsACK(void) const;

    /*!
     * @return True when this message has been replied with an error code, false otherwise.
     */
    bool IsError(void) const;

    /*!
     * @return True when this message has been replied with an error code and needs to be retried, false otherwise.
     */
    bool NeedsRetry(void) const;

    /*!
     * @return The logical address of the initiator, or CECDEVICE_UNKNOWN if unknown.
     */
    cec_logical_address Initiator(void) const;

    /*!
     * @return The logical address of the destination, or CECDEVICE_UNKNOWN if unknown.
     */
    cec_logical_address Destination(void) const;

    /*!
     * @return True when this message contains a start message, false otherwise.
     */
    bool HasStartMessage(void) const;

    /*!
     * @brief Push this adapter message to the end of the given cec_command.
     * @param command The command to push this message to.
     * @return True when a full CEC message was received, false otherwise.
     */
    bool PushToCecCommand(cec_command &command) const;

    /*!
     * @return The response messagecode.
     */
    cec_adapter_messagecode Reply(void) const;

    uint8_t                               maxTries;             /**< the maximum number of times to try to send this message */
    cec_datapacket                        response;             /**< the response to this message */
    cec_datapacket                        packet;               /**< the actual data */
    cec_adapter_message_state             state;                /**< the current state of this message */
    int32_t                               transmit_timeout;     /**< the timeout to use when sending this message */
    uint8_t                               lineTimeout;          /**< the default CEC line timeout to use when sending this message */

  private:
    bool                                  bNextByteIsEscaped;   /**< true when the next byte that is added will be escaped, false otherwise */
  };
}
