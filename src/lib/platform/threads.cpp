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

#include "threads.h"
#include "timeutils.h"

using namespace CEC;

CMutex::CMutex(void)
{
  pthread_mutex_init(&m_mutex, NULL);
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

CLockObject::CLockObject(CMutex *mutex, bool bTryLock /* = false */) :
  m_mutex(mutex)
{
  if (m_mutex)
    m_bLocked = bTryLock ? m_mutex->TryLock() : m_mutex->Lock();
}

CLockObject::~CLockObject(void)
{
  Leave();
  m_mutex = NULL;
}

void CLockObject::Leave(void)
{
  if (m_mutex && m_bLocked)
  {
    m_bLocked = false;
    m_mutex->Unlock();
  }
}

void CLockObject::Lock(void)
{
  if (m_mutex)
    m_bLocked = m_mutex->Lock();
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

bool CCondition::Wait(CMutex *mutex, uint32_t iTimeout /* = 0 */)
{
  bool bReturn(false);
  sched_yield();
  if (mutex)
  {
    if (iTimeout > 0)
    {
      struct timespec abstime;
      struct timeval now;
      gettimeofday(&now, NULL);
      iTimeout       += now.tv_usec / 1000;
      abstime.tv_sec  = now.tv_sec + (time_t)(iTimeout / 1000);
      abstime.tv_nsec = (int32_t)((iTimeout % (uint32_t)1000) * (uint32_t)1000000);
      bReturn         = (pthread_cond_timedwait(&m_cond, &mutex->m_mutex, &abstime) == 0);
    }
    else
    {
      bReturn         = (pthread_cond_wait(&m_cond, &mutex->m_mutex) == 0);
    }
  }

  return bReturn;
}

void CCondition::Sleep(uint32_t iTimeout)
{
  CCondition w;
  CMutex m;
  CLockObject lock(&m);
  w.Wait(&m, iTimeout);
}

CThread::CThread(void) :
    m_bStop(false),
    m_bRunning(false)
{
}

CThread::~CThread(void)
{
  StopThread();
}

bool CThread::CreateThread(bool bWait /* = true */)
{
  bool bReturn(false);

  CLockObject lock(&m_threadMutex);
  m_bStop = false;
  if (!m_bRunning && pthread_create(&m_thread, NULL, (void *(*) (void *))&CThread::ThreadHandler, (void *)this) == 0)
  {
    if (bWait)
      m_threadCondition.Wait(&m_threadMutex);
    bReturn = true;
  }

  return bReturn;
}

void *CThread::ThreadHandler(CThread *thread)
{
  void *retVal = NULL;

  if (thread)
  {
    thread->m_bRunning = true;
    thread->m_threadCondition.Broadcast();
    retVal = thread->Process();
    thread->m_bRunning = false;
  }

  return retVal;
}

bool CThread::StopThread(bool bWaitForExit /* = true */)
{
  bool bReturn(true);
  m_bStop = true;

  m_threadCondition.Broadcast();

  void *retVal;
  if (bWaitForExit && m_bRunning)
    bReturn = (pthread_join(m_thread, &retVal) == 0);

  return bReturn;
}

bool CThread::Sleep(uint32_t iTimeout)
{
  CLockObject lock(&m_threadMutex);
  return m_bStop ? false : m_threadCondition.Wait(&m_threadMutex, iTimeout);
}
