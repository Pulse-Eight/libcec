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
#include "../timeutils.h"

using namespace CEC;

CMutex::CMutex(bool bRecursive /* = true */) :
    IMutex(bRecursive)
{
  pthread_mutex_init(&m_mutex, bRecursive ? GetMutexAttribute() : NULL);
}

CMutex::~CMutex(void)
{
  pthread_mutex_destroy(&m_mutex);
}

bool CMutex::TryLock(void)
{
  return (pthread_mutex_trylock(&m_mutex) == 0);
}

bool CMutex::Lock(void)
{
  return (pthread_mutex_lock(&m_mutex) == 0);
}

void CMutex::Unlock(void)
{
  pthread_mutex_unlock(&m_mutex);
}

static pthread_mutexattr_t g_mutexAttr;
pthread_mutexattr_t *CMutex::GetMutexAttribute()
{
  static bool bAttributeInitialised = false;
  if (!bAttributeInitialised)
  {
    pthread_mutexattr_init(&g_mutexAttr);
    pthread_mutexattr_settype(&g_mutexAttr, PTHREAD_MUTEX_RECURSIVE);
    bAttributeInitialised = true;
  }
  return &g_mutexAttr;
}

CCondition::CCondition(void)
{
  pthread_cond_init(&m_cond, NULL);
}

CCondition::~CCondition(void)
{
  pthread_cond_broadcast(&m_cond);
  pthread_cond_destroy(&m_cond);
}

void CCondition::Broadcast(void)
{
  pthread_cond_broadcast(&m_cond);
}

void CCondition::Signal(void)
{
  pthread_cond_signal(&m_cond);
}

bool CCondition::Wait(IMutex *mutex, uint32_t iTimeout /* = 0 */)
{
  bool bReturn(false);
  sched_yield();
  CMutex *pmutex = static_cast<CMutex *>(mutex);
  if (pmutex)
  {
    if (iTimeout > 0)
    {
      struct timespec abstime;
      struct timeval now;
      gettimeofday(&now, NULL);
      iTimeout       += now.tv_usec / 1000;
      abstime.tv_sec  = now.tv_sec + (time_t)(iTimeout / 1000);
      abstime.tv_nsec = (int32_t)((iTimeout % (uint32_t)1000) * (uint32_t)1000000);
      bReturn         = (pthread_cond_timedwait(&m_cond, &pmutex->m_mutex, &abstime) == 0);
    }
    else
    {
      bReturn         = (pthread_cond_wait(&m_cond, &pmutex->m_mutex) == 0);
    }
  }
  return bReturn;
}

bool CThread::CreateThread(bool bWait /* = true */)
{
  bool bReturn(false);
  CLockObject lock(m_threadMutex);
  m_bStop = false;
  if (!m_bRunning && pthread_create(&m_thread, NULL, (void *(*) (void *))&CThread::ThreadHandler, (void *)this) == 0)
  {
    if (bWait)
      m_threadCondition->Wait(m_threadMutex);
    bReturn = true;
  }
  return bReturn;
}

bool CThread::StopThread(bool bWaitForExit /* = true */)
{
  bool bReturn = IThread::StopThread(bWaitForExit);

  void *retVal;
  if (bWaitForExit && m_bRunning)
    bReturn = (pthread_join(m_thread, &retVal) == 0);

  return bReturn;
}

void *CThread::ThreadHandler(CThread *thread)
{
  void *retVal = NULL;

  if (thread)
  {
    CLockObject lock(thread->m_threadMutex);
    thread->m_bRunning = true;
    lock.Leave();
    thread->m_threadCondition->Broadcast();

    retVal = thread->Process();

    lock.Lock();
    thread->m_bRunning = false;
    lock.Leave();
    thread->m_threadCondition->Broadcast();
  }

  return retVal;
}
