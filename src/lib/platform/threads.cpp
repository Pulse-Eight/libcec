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
#include "os-dependent.h"

using namespace CEC;

CLockObject::CLockObject(IMutex *mutex, bool bTryLock /* = false */) :
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

void ICondition::Sleep(uint32_t iTimeout)
{
  CCondition w;
  CMutex m;
  CLockObject lock(&m);
  w.Wait(&m, iTimeout);
}

IThread::IThread(void) :
    m_bStop(false),
    m_bRunning(false)
{
  m_threadCondition = new CCondition();
  m_threadMutex     = new CMutex();
}

IThread::~IThread(void)
{
  StopThread();
  delete m_threadCondition;
  delete m_threadMutex;
}

bool IThread::StopThread(bool bWaitForExit /* = true */)
{
  m_bStop = true;
  m_threadCondition->Broadcast();
  bWaitForExit = true;

  return false;
}

bool IThread::Sleep(uint32_t iTimeout)
{
  CLockObject lock(m_threadMutex);
  return m_bStop ? false : m_threadCondition->Wait(m_threadMutex, iTimeout);
}
