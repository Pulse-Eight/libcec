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

#include "USBCECAdapterMessage.h"

namespace CEC
{
  class CUSBCECAdapterCommunication;

  class CCECAdapterMessageQueueEntry
  {
  public:
    CCECAdapterMessageQueueEntry(CCECAdapterMessage *message);
    virtual ~CCECAdapterMessageQueueEntry(void);

    /*!
     * @brief Signal waiting threads
     */
    void Broadcast(void);

    /*!
     * @brief Called when a message was received.
     * @param message The message that was received.
     * @return True when this message was handled by this entry, false otherwise.
     */
    bool MessageReceived(const CCECAdapterMessage &message);

    /*!
     * @brief Wait for a response to this command.
     * @param iTimeout The timeout to use while waiting.
     * @return True when a response was received before the timeout passed, false otherwise.
     */
    bool Wait(uint32_t iTimeout);

    /*!
     * @return True while a thread is waiting for a signal or isn't waiting yet, false otherwise.
     */
    bool IsWaiting(void);

    /*!
     * @return The msgcode of the command that was sent.
     */
    cec_adapter_messagecode MessageCode(void);

    /*!
     * @brief Check whether a message is a response to this command.
     * @param msg The message to check.
     * @return True when it's a response, false otherwise.
     */
    bool IsResponse(const CCECAdapterMessage &msg);

    /*!
     * @return The command that was sent in human readable form.
     */
    const char *ToString(void) const;

  private:
    /*!
     * @brief Called when a 'command accepted' message was received.
     * @param message The message that was received.
     * @return True when the waiting thread need to be signaled, false otherwise.
     */
    bool MessageReceivedCommandAccepted(const CCECAdapterMessage &message);

    /*!
     * @brief Called when a 'transmit succeeded' message was received.
     * @param message The message that was received.
     * @return True when the waiting thread need to be signaled, false otherwise.
     */
    bool MessageReceivedTransmitSucceeded(const CCECAdapterMessage &message);

    /*!
     * @brief Called when a message that's not a 'command accepted' or 'transmit succeeded' message was received.
     * @param message The message that was received.
     * @return True when the waiting thread need to be signaled, false otherwise.
     */
    bool MessageReceivedResponse(const CCECAdapterMessage &message);

    CCECAdapterMessage *       m_message;      /**< the message that was sent */
    uint8_t                    m_iPacketsLeft; /**< the amount of acks that we're waiting on */
    bool                       m_bSucceeded;   /**< true when the command received a response, false otherwise */
    bool                       m_bWaiting;     /**< true while a thread is waiting or when it hasn't started waiting yet */
    PLATFORM::CCondition<bool> m_condition;    /**< the condition to wait on */
    PLATFORM::CMutex           m_mutex;        /**< mutex for changes to this class */
  };

  class CCECAdapterMessageQueue
  {
    friend class CUSBCECAdapterCommunication;

  public:
    /*!
     * @brief Create a new message queue.
     * @param com The communication handler callback to use.
     * @param iQueueSize The outgoing message queue size.
     */
    CCECAdapterMessageQueue(CUSBCECAdapterCommunication *com, size_t iQueueSize = 64) :
      m_com(com),
      m_messages(iQueueSize) {}
    virtual ~CCECAdapterMessageQueue(void);

    /*!
     * @brief Signal and delete everything in the queue
     */
    void Clear(void);

    /*!
     * @brief Called when a message was received from the adapter.
     * @param msg The message that was received.
     */
    void MessageReceived(const CCECAdapterMessage &msg);

    /*!
     * @brief Adds received data to the current frame.
     * @param data The data to add.
     * @param iLen The length of the data to add.
     */
    void AddData(uint8_t *data, size_t iLen);

    /*!
     * @brief Transmit a command to the adapter and wait for a response.
     * @param msg The command to send.
     * @return True when written, false otherwise.
     */
    bool Write(CCECAdapterMessage *msg);

  private:
    /*!
     * @return The next message in the queue, or NULL if there is none.
     */
    CCECAdapterMessageQueueEntry *GetNextQueuedEntry(void);

    CUSBCECAdapterCommunication *                          m_com;                    /**< the communication handler */
    PLATFORM::CMutex                                       m_mutex;                  /**< mutex for changes to this class */
    PLATFORM::SyncedBuffer<CCECAdapterMessageQueueEntry *> m_messages;               /**< the outgoing message queue */
    CCECAdapterMessage                                     m_incomingAdapterMessage; /**< the current incoming message that's being assembled */
    cec_command                                            m_currentCECFrame;        /**< the current incoming CEC command that's being assembled */
  };
}
