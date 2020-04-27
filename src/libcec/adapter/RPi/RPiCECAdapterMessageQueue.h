#pragma once
/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2015 Pulse-Eight Limited.  All rights reserved.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
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

#include "env.h"
#include "p8-platform/util/buffer.h"
#include <map>
#include "adapter/AdapterCommunication.h"

extern "C" {
#include <interface/vmcs_host/vc_cecservice.h>
#include <interface/vchiq_arm/vchiq_if.h>
}

namespace CEC
{
  class CRPiCECAdapterCommunication;
  class CRPiCECAdapterMessageQueue;

  class CRPiCECAdapterMessageQueueEntry
  {
  public:
    CRPiCECAdapterMessageQueueEntry(CRPiCECAdapterMessageQueue *queue, const cec_command &command);
    virtual ~CRPiCECAdapterMessageQueueEntry(void) {}

    /*!
     * @brief Signal waiting threads
     */
    void Broadcast(void);

    bool MessageReceived(cec_opcode opcode, cec_logical_address initiator, cec_logical_address destination, uint32_t response);

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
     * @brief Query result from worker thread
     */
    uint32_t Result() const;

    /*!
     * @return The command that was sent in human readable form.
     */
    const char *ToString(void) const { return "CEC transmission"; }

    CRPiCECAdapterMessageQueue * m_queue;
    bool                         m_bWaiting;     /**< true while a thread is waiting or when it hasn't started waiting yet */
    P8PLATFORM::CCondition<bool> m_condition;    /**< the condition to wait on */
    P8PLATFORM::CMutex           m_mutex;        /**< mutex for changes to this class */
    cec_command                  m_command;
    uint32_t                     m_retval;
    bool                         m_bSucceeded;
  };

  class CRPiCECAdapterMessageQueue
  {
    friend class CRPiCECAdapterMessageQueueEntry;

  public:
    /*!
     * @brief Create a new message queue.
     * @param com The communication handler callback to use.
     * @param iQueueSize The outgoing message queue size.
     */
    CRPiCECAdapterMessageQueue(CRPiCECAdapterCommunication *com) :
      m_com(com),
      m_iNextMessage(0)
    {
    }

    virtual ~CRPiCECAdapterMessageQueue(void)
    {
      Clear();
    }

    /*!
     * @brief Signal and delete everything in the queue
     */
    void Clear(void);

    void MessageReceived(cec_opcode opcode, cec_logical_address initiator, cec_logical_address destination, uint32_t response);

    cec_adapter_message_state Write(const cec_command &command, bool &bRetry, uint32_t iLineTimeout, bool bIsReply, VC_CEC_ERROR_T &vcReply);

  private:
    CRPiCECAdapterCommunication *                             m_com;                    /**< the communication handler */
    P8PLATFORM::CMutex                                        m_mutex;                  /**< mutex for changes to this class */
    std::map<uint64_t, CRPiCECAdapterMessageQueueEntry *>     m_messages;               /**< the outgoing message queue */
    uint64_t                                                  m_iNextMessage;           /**< the index of the next message */
  };
};
