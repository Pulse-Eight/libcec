#pragma once
/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2012 Pulse-Eight Limited.  All rights reserved.
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

#include "socket.h"

#if defined(__WINDOWS__)
#include "../windows/os-tcp.h"
#else
#include "../posix/os-tcp.h"
#endif

using namespace std;

namespace PLATFORM
{
  class CTcpSocket : public CSocket
  {
    public:
      CTcpSocket(void) {};
      virtual ~CTcpSocket(void) {}

      virtual bool Open(const CStdString &strHostname, uint16_t iPort, uint64_t nTimeout)
      {
        bool bReturn(false);
        struct addrinfo *address(NULL), *addr(NULL);
        CLockObject lock(m_mutex);
        if (!TcpResolveAddress(strHostname.c_str(), iPort, &m_iError, &address))
        {
          m_strError = strerror(m_iError);
          return bReturn;
        }

        for(addr = address; !bReturn && addr; addr = addr->ai_next)
        {
          m_socket = TcpCreateSocket(addr, &m_iError, nTimeout);
          if (m_socket != INVALID_SOCKET && m_socket != SOCKET_ERROR)
            bReturn = true;
          else
            m_strError = strerror(m_iError);
        }

        freeaddrinfo(address);
        return bReturn;
      }

      virtual void Shutdown(void)
      {
        CLockObject lock(m_mutex);
        if (m_socket != INVALID_SOCKET && m_socket != SOCKET_ERROR)
          TcpShutdownSocket(m_socket);
        m_socket = INVALID_SOCKET;
        m_strError = "";
      }

  protected:
    virtual socket_t TcpCreateSocket(struct addrinfo* addr, int* iError, uint64_t iTimeout)
    {
      socket_t fdSock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
      if (fdSock == INVALID_SOCKET || fdSock == SOCKET_ERROR)
      {
        *iError = errno;
        return (socket_t)SOCKET_ERROR;
      }

      if (!TcpConnectSocket(fdSock, addr, iError, iTimeout))
      {
        SocketClose(fdSock);
        return (socket_t)SOCKET_ERROR;
      }

      TcpSetNoDelay(fdSock);

      return fdSock;
    }
  };
};
