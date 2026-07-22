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

#if defined(__WINDOWS__)
#include "platform/windows/os-socket.h"
#else
#include "platform/posix/os-socket.h"
#endif

#include <string>

namespace CEC
{
  /*!
   * @brief The error and name bookkeeping shared by the socket types.
   *
   * @param _SType the OS handle: a file descriptor on posix, a HANDLE for a
   *               serial port on windows.
   */
  template <typename _SType>
  class CCommonSocket
  {
  public:
    CCommonSocket(_SType initialSocketValue, const std::string &strName) :
      m_socket(initialSocketValue),
      m_strName(strName),
      m_iError(0) {}

    CCommonSocket(const CCommonSocket &) = delete;
    CCommonSocket &operator=(const CCommonSocket &) = delete;

    std::string GetError(void)
    {
      return m_strError.empty() && m_iError != 0 ? strerror(m_iError) : m_strError;
    }

    int GetErrorNumber(void)
    {
      return m_iError;
    }

    std::string GetName(void)
    {
      return m_strName;
    }

  protected:
    _SType      m_socket;
    std::string m_strError;
    std::string m_strName;
    int         m_iError;
    CMutex      m_mutex;
  };

  /*!
   * @brief Serialises access to a socket: one operation at a time, in the order
   *        the callers arrive.
   *
   * The socket is reached through the template parameter, so nothing here needs
   * to be virtual.
   */
  template <typename _Socket>
  class CProtectedSocket
  {
  public:
    CProtectedSocket(_Socket *socket) :
      m_socket(socket),
      m_bIsIdle(true) {}

    ~CProtectedSocket(void)
    {
      delete m_socket;
    }

    CProtectedSocket(const CProtectedSocket &) = delete;
    CProtectedSocket &operator=(const CProtectedSocket &) = delete;

    bool Open(uint64_t iTimeoutMs = 0)
    {
      bool bReturn(false);
      if (m_socket && WaitReady())
      {
        bReturn = m_socket->Open(iTimeoutMs);
        MarkReady();
      }
      return bReturn;
    }

    void Close(void)
    {
      if (m_socket && WaitReady())
      {
        m_socket->Close();
        MarkReady();
      }
    }

    bool IsOpen(void)
    {
      CLockObject lock(m_mutex);
      return m_socket && m_socket->IsOpen();
    }

    ssize_t Write(void* data, size_t len)
    {
      if (!m_socket || !WaitReady())
        return -EINVAL;

      ssize_t iReturn = m_socket->Write(data, len);
      MarkReady();

      return iReturn;
    }

    ssize_t Read(void* data, size_t len, uint64_t iTimeoutMs = 0)
    {
      if (!m_socket || !WaitReady())
        return -EINVAL;

      ssize_t iReturn = m_socket->Read(data, len, iTimeoutMs);
      MarkReady();

      return iReturn;
    }

    std::string GetError(void)
    {
      CLockObject lock(m_mutex);
      return m_socket ? m_socket->GetError() : "";
    }

    int GetErrorNumber(void)
    {
      CLockObject lock(m_mutex);
      return m_socket ? m_socket->GetErrorNumber() : -EINVAL;
    }

    std::string GetName(void)
    {
      CLockObject lock(m_mutex);
      return m_socket ? m_socket->GetName() : "";
    }

  private:
    bool WaitReady(void)
    {
      CLockObject lock(m_mutex);
      m_condition.Wait(lock, m_bIsIdle);
      m_bIsIdle = false;
      return true;
    }

    void MarkReady(void)
    {
      CLockObject lock(m_mutex);
      m_bIsIdle = true;
      m_condition.Signal();
    }

    _Socket *        m_socket;
    CMutex           m_mutex;
    CCondition<bool> m_condition;
    bool             m_bIsIdle;
  };
};
