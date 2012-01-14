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

#include "../threads.h"

namespace CEC
{
  class CMutex : public IMutex
  {
  public:
    CMutex(bool bRecursive = true);
    virtual ~CMutex(void);

    virtual bool TryLock(void);
    virtual bool Lock(void);
    virtual void Unlock(void);

    pthread_mutex_t m_mutex;

  private:
    static pthread_mutexattr_t *GetMutexAttribute();
  };

  class CCondition : public ICondition
  {
  public:
    CCondition(void);
    virtual ~CCondition(void);

    virtual void Broadcast(void);
    virtual void Signal(void);
    virtual bool Wait(IMutex *mutex, uint32_t iTimeout = 0);

  private:
    pthread_cond_t  m_cond;
  };

  class CThread : public IThread
  {
  public:
    CThread(void) { };
    virtual ~CThread(void) { };

    virtual bool CreateThread(bool bWait = true);
    virtual bool StopThread(bool bWaitForExit = true);

    static void *ThreadHandler(CThread *thread);

  private:
    pthread_t  m_thread;
  };
};
