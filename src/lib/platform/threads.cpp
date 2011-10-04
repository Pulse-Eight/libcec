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

CLockObject::CLockObject(CMutex *mutex) :
  m_mutex(mutex)
{
  m_mutex->Lock();
}

CLockObject::~CLockObject(void)
{
  Leave();
  m_mutex = NULL;
}

void CLockObject::Leave(void)
{
  m_mutex->Unlock();
}

void CLockObject::Lock(void)
{
  m_mutex->Lock();
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

void CCondition::Signal(void)
{
  pthread_cond_broadcast(&m_cond);
}

bool CCondition::Wait(CMutex *mutex, int64_t iTimeout)
{
  bool bReturn(false);
  struct timespec abstime;
  struct timeval now;
  if (gettimeofday(&now, NULL) == 0)
  {
    iTimeout       += now.tv_usec / 1000;
    abstime.tv_sec  = now.tv_sec + (time_t)(iTimeout / 1000);
    abstime.tv_nsec = (long)((iTimeout % (unsigned long)1000) * (unsigned long)1000000);
    bReturn         = (pthread_cond_timedwait(&m_cond, &mutex->m_mutex, &abstime) == 0);
  }

  return bReturn;
}

void CCondition::Sleep(int iTimeout)
{
  sched_yield();
  CCondition w;
  CMutex m;
  CLockObject lock(&m);
  w.Wait(&m, int64_t(iTimeout));
}

CThread::CThread(void) :
    m_bRunning(false),
    m_bStop(false)
{
}

CThread::~CThread(void)
{
  m_bStop = true;
  pthread_join(m_thread, NULL);
}

bool CThread::CreateThread(void)
{
  bool bReturn(false);

  if (!m_bRunning && pthread_create(&m_thread, NULL, (void *(*) (void *))&CThread::ThreadHandler, (void *)this) == 0)
  {
    m_bRunning = true;
    pthread_detach(m_thread);
    bReturn = true;
  }

  return bReturn;
}

void *CThread::ThreadHandler(CThread *thread)
{
  if (thread)
    return thread->Process();
  return NULL;
}

void CThread::StopThread(bool bWaitForExit /* = true */)
{
  m_bStop = true;
  if (bWaitForExit)
    pthread_join(m_thread, NULL);
}
