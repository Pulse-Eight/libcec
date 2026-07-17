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

#include "platform/threads/mutex.h"

#include <stdint.h>
#include <thread>

namespace CEC
{
  class CThread
  {
  public:
    CThread(void) :
        m_bStop(false),
        m_bRunning(false),
        m_bStopped(false),
        m_bStarted(false) {}

    virtual ~CThread(void)
    {
      StopThread(0);
      // the thread signals m_bStopped and only then unwinds ThreadHandler, which
      // still touches this object. StopThread() returns as soon as it sees the
      // flag, so without joining here the thread can outlive what it is reading.
      JoinThread();
    }

    CThread(const CThread &) = delete;
    CThread &operator=(const CThread &) = delete;

    virtual bool IsRunning(void)
    {
      CLockObject lock(m_threadMutex);
      return m_bRunning;
    }

    virtual bool IsStopped(void)
    {
      CLockObject lock(m_threadMutex);
      return m_bStop;
    }

    virtual bool CreateThread(bool bWait = true)
    {
      CLockObject lock(m_threadMutex);
      if (m_bRunning)
        return false;

      // reap a previous run before starting another one
      if (m_thread.joinable())
        m_thread.join();

      m_bStop    = false;
      m_bStarted = false;
      m_thread   = std::thread(&CThread::ThreadHandler, this);

      // wait for the thread to have reached the handler, not for it to be running:
      // a thread that is asked to stop before it starts never runs at all
      if (bWait)
        m_threadCondition.Wait(lock, m_bStarted);

      return true;
    }

    /*!
     * @brief Stop the thread
     * @param iWaitMs negative = don't wait, 0 = infinite, or the amount of ms to wait
     */
    virtual bool StopThread(int iWaitMs = 5000)
    {
      bool bRunning(false);
      {
        CLockObject lock(m_threadMutex);
        bRunning = m_bRunning;
        m_bStop  = true;
      }

      if (!bRunning || iWaitMs < 0)
        return true;

      CLockObject lock(m_threadMutex);
      return m_threadCondition.Wait(lock, m_bStopped, (uint32_t)iWaitMs);
    }

    virtual bool Sleep(uint32_t iTimeout)
    {
      CLockObject lock(m_threadMutex);
      return m_bStop ? false : m_threadCondition.Wait(lock, m_bStopped, iTimeout);
    }

    virtual void *Process(void) = 0;

  private:
    static void ThreadHandler(CThread *thread)
    {
      {
        CLockObject lock(thread->m_threadMutex);
        thread->m_bStarted = true;

        // StopThread() got here first, so this object may already be part way
        // through being destroyed. Process() is pure virtual in ~CThread and the
        // derived half is gone by then: don't call it at all. both this and
        // StopThread() decide under the same lock, so one of them always wins
        // cleanly.
        if (thread->m_bStop)
        {
          thread->m_bRunning = false;
          thread->m_bStopped = true;
          thread->m_threadCondition.Broadcast();
          return;
        }

        thread->m_bRunning = true;
        thread->m_bStopped = false;
        thread->m_threadCondition.Broadcast();
      }

      thread->Process();

      {
        CLockObject lock(thread->m_threadMutex);
        thread->m_bRunning = false;
        thread->m_bStopped = true;
        thread->m_threadCondition.Broadcast();
      }
    }

    void JoinThread(void)
    {
      if (m_thread.joinable())
        m_thread.join();
    }

    bool             m_bStop;
    bool             m_bRunning;
    bool             m_bStopped;
    bool             m_bStarted;
    CCondition<bool> m_threadCondition;
    CMutex           m_threadMutex;
    std::thread      m_thread;
  };
};
