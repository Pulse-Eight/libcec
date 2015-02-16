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

#include "../threads/mutex.h"
#include <queue>

namespace PLATFORM
{
  template<typename _BType>
    struct SyncedBuffer
    {
    public:
      SyncedBuffer(size_t iMaxSize = 100) :
          m_maxSize(iMaxSize),
          m_bHasData(false) {}

      virtual ~SyncedBuffer(void)
      {
        Clear();
      }

      void Clear(void)
      {
        CLockObject lock(m_mutex);
        while (!m_buffer.empty())
          m_buffer.pop();
        m_bHasData = false;
        m_condition.Broadcast();
      }

      size_t Size(void)
      {
        CLockObject lock(m_mutex);
        return m_buffer.size();
      }

      bool IsEmpty(void)
      {
        CLockObject lock(m_mutex);
        return !m_bHasData;
      }

      bool Push(_BType entry)
      {
        CLockObject lock(m_mutex);
        if (m_buffer.size() == m_maxSize)
          return false;

        m_buffer.push(entry);
        m_bHasData = true;
        m_condition.Signal();
        return true;
      }

      bool Pop(_BType &entry, int32_t iTimeoutMs = 0)
      {
        CLockObject lock(m_mutex);
        if (m_buffer.empty())
        {
          if (iTimeoutMs == 0)
            return false;
          if (!m_condition.Wait(m_mutex, m_bHasData, iTimeoutMs))
            return false;
        }

        entry = m_buffer.front();
        m_buffer.pop();
        m_bHasData = !m_buffer.empty();
        return true;
      }

    private:
      size_t             m_maxSize;
      std::queue<_BType> m_buffer;
      CMutex             m_mutex;
      bool               m_bHasData;
      CCondition<bool>   m_condition;
    };
};
