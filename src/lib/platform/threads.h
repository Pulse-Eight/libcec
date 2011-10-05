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

#include "os-dependent.h"
#include <stdint.h>

namespace CEC
{
  class CMutex;

  class CCondition
  {
  public:
    CCondition(void);
    virtual ~CCondition(void);

    void Broadcast(void);
    void Signal(void);
    bool Wait(CMutex *mutex, int64_t iTimeout);
    static void Sleep(int64_t iTimeout);

  private:
    pthread_cond_t  m_cond;
  };

  class CMutex
  {
  public:
    CMutex(void);
    virtual ~CMutex(void);

    bool TryLock(void);
    bool Lock(void);
    void Unlock(void);

    pthread_mutex_t m_mutex;
  };

  class CLockObject
  {
  public:
    CLockObject(CMutex *mutex);
    ~CLockObject(void);

    bool IsLocked(void) const { return m_bLocked; }
    void Leave(void);
    void Lock(void);

  private:
    CMutex *m_mutex;
    bool    m_bLocked;
  };

  class CThread
  {
  public:
    CThread(void);
    virtual ~CThread(void);

    virtual bool IsRunning(void) const { return m_bRunning; }
    virtual bool CreateThread(void);
    virtual bool StopThread(bool bWaitForExit = true);
    virtual bool Sleep(uint64_t iTimeout);

    static void *ThreadHandler(CThread *thread);
    virtual void *Process(void) = 0;

  protected:
    pthread_t  m_thread;
    CMutex     m_threadMutex;
    CCondition m_threadCondition;
    bool       m_bRunning;
    bool       m_bStop;
  };
};
