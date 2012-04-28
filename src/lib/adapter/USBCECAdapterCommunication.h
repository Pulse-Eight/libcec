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
  class CUSBCECAdapterCommands;
  class CCECAdapterMessageQueue;

  class CUSBCECAdapterCommunication : public IAdapterCommunication, public PLATFORM::CThread
  {
    friend class CUSBCECAdapterCommands;
    friend class CCECAdapterMessageQueue;

  public:
    /*!
     * @brief Create a new USB-CEC communication handler.
     * @param callback The callback to use for incoming CEC commands.
     * @param strPort The name of the com port to use.
     * @param iBaudRate The baudrate to use on the com port connection.
     */
    CUSBCECAdapterCommunication(IAdapterCommunicationCallback *callback, const char *strPort, uint16_t iBaudRate = CEC_SERIAL_DEFAULT_BAUDRATE);
    virtual ~CUSBCECAdapterCommunication(void);

    /** @name IAdapterCommunication implementation */
    ///{
    bool Open(uint32_t iTimeoutMs = CEC_DEFAULT_CONNECT_TIMEOUT, bool bSkipChecks = false, bool bStartListening = true);
    void Close(void);
    bool IsOpen(void);
    CStdString GetError(void) const;
    cec_adapter_message_state Write(const cec_command &data, bool &bRetry, uint8_t iLineTimeout = 3);

    bool StartBootloader(void);
    bool SetAckMask(uint16_t iMask);
    bool PingAdapter(void);
    uint16_t GetFirmwareVersion(void);
    uint32_t GetFirmwareBuildDate(void);
    bool PersistConfiguration(libcec_configuration *configuration);
    bool GetConfiguration(libcec_configuration *configuration);
    CStdString GetPortName(void);
    uint16_t GetPhysicalAddress(void) { return 0; }
    bool SetControlledMode(bool controlled);
    ///}

    void *Process(void);

  private:
    /*!
     * @brief Clear all input bytes.
     * @param iTimeout Timeout when anything was received.
     */
    void ClearInputBytes(uint32_t iTimeout = CEC_CLEAR_INPUT_DEFAULT_WAIT);

    /*!
     * @brief Change the current CEC line timeout.
     * @param iTimeout The new timeout.
     * @return True when acked by the controller, false otherwise.
     */
    bool SetLineTimeout(uint8_t iTimeout);

    /*!
     * @brief Send a command to the controller and wait for an ack.
     * @param msgCode The command to send.
     * @param params The parameters to the command.
     * @param bIsRetry True when this command is being retried, false otherwise.
     * @return The message. Delete when done with it.
     */
    CCECAdapterMessage *SendCommand(cec_adapter_messagecode msgCode, CCECAdapterMessage &params, bool bIsRetry = false);

    /*!
     * @brief Change the "initialised" status.
     * @param bSetTo The new value.
     */
    void SetInitialised(bool bSetTo = true);

    /*!
     * @return True when initialised, false otherwise.
     */
    bool IsInitialised(void);

    /*!
     * @brief Pings the adapter, checks the firmware version and sets controlled mode.
     * @param iTimeoutMs The timeout after which this fails if no proper data was received.
     * @return True when the checks passed, false otherwise.
     */
    bool CheckAdapter(uint32_t iTimeoutMs = CEC_DEFAULT_CONNECT_TIMEOUT);

    /*!
     * @brief Handle a poll message inside the adapter message (checks if one is present).
     * @param msg The adapter message to parse.
     * @return True when the message resulted in a CEC error, false otherwise.
     */
    bool HandlePoll(const CCECAdapterMessage &msg);

    /*!
     * @brief Read data from the device.
     * @param iTimeout The read timeout to use.
     * @param iSize The maximum read size.
     * @return True when something was read, false otherwise.
     */
    bool ReadFromDevice(uint32_t iTimeout, size_t iSize = 256);

    /*!
     * @brief Writes a message to the serial port.
     * @param message The message to write.
     * @return True when written, false otherwise.
     */
    bool WriteToDevice(CCECAdapterMessage *message);

    /*!
     * @brief Called before sending a CEC command over the line, so we know we're expecting an ack.
     * @param dest The destination of the CEC command.
     */
    void MarkAsWaiting(const cec_logical_address dest);

    PLATFORM::ISocket *                          m_port;                 /**< the com port connection */
    PLATFORM::CMutex                             m_mutex;                /**< mutex for changes in this class */
    uint8_t                                      m_iLineTimeout;         /**< the current line timeout on the CEC line */
    cec_logical_address                          m_lastPollDestination;  /**< the destination of the last poll message that was received */
    bool                                         m_bInitialised;         /**< true when the connection is initialised, false otherwise */
    bool                                         m_bWaitingForAck[15];   /**< array in which we store from which devices we're expecting acks */
    CAdapterPingThread *                         m_pingThread;           /**< ping thread, that pings the adapter every 15 seconds */
    CUSBCECAdapterCommands *                     m_commands;             /**< commands that can be sent to the adapter */
    CCECAdapterMessageQueue *                    m_adapterMessageQueue;  /**< the incoming and outgoing message queue */
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
