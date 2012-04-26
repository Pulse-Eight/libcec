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

#include "../platform/util/StdString.h"
#include "USBCECAdapterMessage.h"

namespace CEC
{
  class IAdapterCommunicationCallback
  {
  public:
    IAdapterCommunicationCallback(void) {}
    virtual ~IAdapterCommunicationCallback(void) {}

    /*!
     * @brief Callback method for IAdapterCommunication, called when a new cec_command is received
     * @param command The command that has been received
     * @return True when it was handled by this listener, false otherwise.
     */
    virtual bool OnCommandReceived(const cec_command &command) = 0;

    /*!
     * @brief Callback method for IAdapterCommunication, called when a poll was received.
     * @param initiator The initiator that sent the poll.
     * @param destination The destination of the poll message.
     */
    virtual void HandlePoll(cec_logical_address initiator, cec_logical_address destination) = 0;

    /*!
     * @brief Callback method for IAdapterCommunication, called when a receive failed message was received.
     * @param initiator The initiator that sent the receive failed message.
     * @return True when this is an error, false otherwise.
     */
    virtual bool HandleReceiveFailed(cec_logical_address initiator) = 0;
  };

  class IAdapterCommunication
  {
  public:
    /*!
     * @param callback The callback struct. if set to NULL, the Read() method has to be used to read commands. if set, OnCommandReceived() will be called for each command that was received
     */
    IAdapterCommunication(IAdapterCommunicationCallback *callback) :
      m_callback(callback) {}
    virtual ~IAdapterCommunication(void) {}

    /*!
     * @brief Open a connection to the CEC adapter
     * @param iTimeoutMs Connection timeout in ms
     * @param bSkipChecks Skips all initial checks of the adapter, and starts the reader/writer threads directly after connecting.
     * @param bStartListening Start a listener thread when true. False to just open a connection, read the device info, and close the connection.
     * @return True when connected, false otherwise
     */
    virtual bool Open(uint32_t iTimeoutMs = CEC_DEFAULT_CONNECT_TIMEOUT, bool bSkipChecks = false, bool bStartListening = true) = 0;

    /*!
     * @brief Close an open connection
     */
    virtual void Close(void) = 0;

    /*!
     * @return True when the connection is open, false otherwise
     */
    virtual bool IsOpen(void) = 0;

    /*!
     * @return The last error message, or an empty string if there was none
     */
    virtual CStdString GetError(void) const = 0;

    /*!
     * @brief Write a cec_command to the adapter
     * @param data The command to write
     * @param bRetry The command can be retried
     * @param iLineTimeout The line timeout to be used
     * @return The last state of the transmitted command
     */
    virtual cec_adapter_message_state Write(const cec_command &data, bool &bRetry, uint8_t iLineTimeout = 3) = 0;

    /*!
     * @brief Change the current line timeout on the CEC bus
     * @param iTimeout The new timeout
     * @return True when set, false otherwise
     */
    virtual bool SetLineTimeout(uint8_t iTimeout) = 0;

    /*!
     * @brief Put the device in bootloader mode (which will disrupt CEC communication when it succeeds)
     * @return True when the bootloader command has been sent, false otherwise.
     */
    virtual bool StartBootloader(void) = 0;

    /*!
     * @brief Change the ACK-mask of the device, the mask for logical addresses to which the CEC device should ACK
     * @param iMask The new mask
     * @return True when set, false otherwise.
     */
    virtual bool SetAckMask(uint16_t iMask) = 0;

    /*!
     * @brief Check whether the CEC adapter responds
     * @return True when the ping was sent and acked, false otherwise.
     */
    virtual bool PingAdapter(void) = 0;

    /*!
     * @return The firmware version of this CEC adapter, or 0 if it's unknown.
     */
    virtual uint16_t GetFirmwareVersion(void) = 0;

    /*!
     * @return The build date in seconds since epoch, or 0 when no (valid) reply was received.
     */
    virtual uint32_t GetFirmwareBuildDate(void) = 0;

    /*!
     * @return True when the control mode has been set, false otherwise.
     */
    virtual bool SetControlledMode(bool controlled) = 0;

    /*!
     * @brief Persist the given configuration in adapter (if supported)
     * @brief The configuration to store.
     * @return True when the configuration was persisted, false otherwise.
     */
    virtual bool PersistConfiguration(libcec_configuration *configuration) = 0;

    /*!
     * @brief Get the persisted configuration from the adapter (if supported)
     * @param configuration The updated configuration.
     * @return True when the configuration was updated, false otherwise.
     */
    virtual bool GetConfiguration(libcec_configuration *configuration) = 0;

    /*!
     * @return The name of the port
     */
    virtual CStdString GetPortName(void) = 0;

    /*!
     * @return The physical address, if the adapter supports this. 0 otherwise.
     */
    virtual uint16_t GetPhysicalAddress(void) = 0;

  protected:
    IAdapterCommunicationCallback *m_callback;
  };
};
