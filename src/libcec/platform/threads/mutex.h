#pragma once
/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2026 Pulse-Eight Limited.  All rights reserved.
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

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdint.h>

namespace CEC
{
  /*!
   * recursive because the code has always relied on it: a locked object calling
   * another of its own methods re-locks the same mutex.
   */
  using CMutex = std::recursive_mutex;

  /*!
   * unique_lock rather than lock_guard so that the lock can be handed to
   * CCondition::Wait(), and released and re-taken by hand where needed.
   */
  using CLockObject = std::unique_lock<std::recursive_mutex>;

  template <typename _Predicate>
    class CCondition
    {
    public:
      CCondition(void) {}
      ~CCondition(void)
      {
        m_condition.notify_all();
      }

      CCondition(const CCondition &) = delete;
      CCondition &operator=(const CCondition &) = delete;

      void Broadcast(void)
      {
        m_condition.notify_all();
      }

      void Signal(void)
      {
        m_condition.notify_one();
      }

      /*!
       * @brief Wait until the predicate is true, or the timeout expires.
       * @param lock a lock held on the mutex that guards the predicate. it is released
       *             while waiting and held again on return.
       * @param iTimeout 0 waits forever
       * @return the value of the predicate, so false means it timed out
       */
      bool Wait(CLockObject &lock, _Predicate &predicate, uint32_t iTimeout = 0)
      {
        if (iTimeout == 0)
        {
          m_condition.wait(lock, [&predicate] { return !!predicate; });
          return true;
        }

        return m_condition.wait_for(lock, std::chrono::milliseconds(iTimeout),
                                    [&predicate] { return !!predicate; });
      }

    private:
      // _any, because the mutex is recursive and plain condition_variable only
      // accepts unique_lock<std::mutex>
      std::condition_variable_any m_condition;
    };

  /*!
   * @brief One-shot broadcast: releases everyone waiting on it, and nobody after.
   *
   * The signal is cleared by the last waiter to leave rather than by the first, so
   * that a single Broadcast() releases every thread that was already waiting. More
   * than one can be: CWaitForResponse hands the same event to every thread waiting
   * on a given opcode.
   */
  class CEvent
  {
  public:
    CEvent(void) :
      m_bSignaled(false),
      m_iWaitingThreads(0) {}

    CEvent(const CEvent &) = delete;
    CEvent &operator=(const CEvent &) = delete;

    void Broadcast(void)
    {
      CLockObject lock(m_mutex);
      m_bSignaled = true;
      m_condition.Broadcast();
    }

    /*!
     * @param iTimeout 0 waits forever
     * @return false if it timed out
     */
    bool Wait(uint32_t iTimeout)
    {
      CLockObject lock(m_mutex);
      ++m_iWaitingThreads;
      m_condition.Wait(lock, m_bSignaled, iTimeout);

      --m_iWaitingThreads;
      const bool bSignaled(m_bSignaled);
      if (bSignaled && m_iWaitingThreads == 0)
        m_bSignaled = false;
      return bSignaled;
    }

  private:
    bool             m_bSignaled;
    unsigned int     m_iWaitingThreads;
    CCondition<bool> m_condition;
    CMutex           m_mutex;
  };
};
