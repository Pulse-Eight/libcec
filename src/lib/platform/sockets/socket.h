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

#include "../threads/mutex.h"
#include "../util/StdString.h"

#if defined(__WINDOWS__)
#include "../windows/os-socket.h"
#else
#include "../posix/os-socket.h"
#endif

// Common socket operations

namespace PLATFORM
{
  class CSocket : public PreventCopy
  {
    public:
      CSocket(void) :
        m_socket(INVALID_SOCKET) {};
      virtual ~CSocket(void)
      {
        Close();
      }

      virtual bool IsOpen(void)
      {
        CLockObject lock(m_mutex);
        return m_socket != INVALID_SOCKET &&
            m_socket != SOCKET_ERROR;
      }

      virtual void Close(void)
      {
        CLockObject lock(m_mutex);
        SocketClose(m_socket);
        m_socket = INVALID_SOCKET;
        m_strError = "";
      }

      virtual int64_t Write(uint8_t* data, uint32_t len)
      {
        CLockObject lock(m_mutex);
        int iError(0);
        int64_t iReturn = SocketWrite(m_socket, &iError, data, len);
        m_strError = strerror(iError);
        return iReturn;
      }

      virtual int32_t Read(uint8_t* data, uint32_t len, uint64_t iTimeoutMs = 0)
      {
        CLockObject lock(m_mutex);
        int iError(0);
        int32_t iReturn = SocketRead(m_socket, &iError, data, len, iTimeoutMs);
        m_strError = strerror(iError);
        return iReturn;
      }

      virtual CStdString GetError(void) const
      {
        CStdString strReturn;
        strReturn = m_strError;
        return strReturn;
      }

    protected:
      socket_t   m_socket;
      CStdString m_strError;
      CMutex     m_mutex;
  };
};
