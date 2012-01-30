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

#include "../os.h"
#include "../sockets/socket.h"

namespace PLATFORM
{
  inline int TcpResolveAddress(const CStdString &strHostname, uint16_t iPort, struct addrinfo *address)
  {
     struct addrinfo hints;
     char   service[33];

     memset(&hints, 0, sizeof(hints));
     hints.ai_family   = AF_UNSPEC;
     hints.ai_socktype = SOCK_STREAM;
     hints.ai_protocol = IPPROTO_TCP;
     sprintf(service, "%d", iPort);

     return getaddrinfo(strHostname.c_str(), service, &hints, &address);
   }

  inline int TcpGetSocketError()
  {
    int error = WSAGetLastError();
    switch(error)
    {
      case WSAEINPROGRESS: return EINPROGRESS;
      case WSAECONNRESET : return ECONNRESET;
      case WSAETIMEDOUT  : return ETIMEDOUT;
      case WSAEWOULDBLOCK: return EAGAIN;
      default            : return error;
    }
  }

  inline int TcpConnectSocket(struct addrinfo* addr, socket_t socket, int *iError, uint64_t iTimeout)
  {
    int nRes = 0;
    socklen_t errlen = sizeof(int);

    SocketSetBlocking(socket, false);

    /* connect to the other side */
    nRes = connect(socket, addr->ai_addr, addr->ai_addrlen);

    /* poll until a connection is established */
    if (nRes == -1)
    {
      if (TcpGetSocketError() == EINPROGRESS || TcpGetSocketError() == EAGAIN)
      {
        fd_set fd_write, fd_except;
        struct timeval tv;
        tv.tv_sec  = (long)(iTimeout / 1000);
        tv.tv_usec = (long)(1000 * (iTimeout % 1000));

        FD_ZERO(&fd_write);
        FD_ZERO(&fd_except);
        FD_SET(socket, &fd_write);
        FD_SET(socket, &fd_except);

        nRes = select(sizeof(socket)*8, NULL, &fd_write, &fd_except, &tv);
        if (nRes == 0)
        {
          *iError = ETIMEDOUT;
          return SOCKET_ERROR;
        }
        else if (nRes == -1)
        {
          *iError = TcpGetSocketError();
          return SOCKET_ERROR;
        }

        /* check for errors */
        getsockopt(socket, SOL_SOCKET, SO_ERROR, (char *)iError, &errlen);
      }
      else
      {
        *iError = TcpGetSocketError();
      }
    }

    SocketSetBlocking(socket, true);

    return 0;
  }

  inline bool TcpSetNoDelay(socket_t socket)
  {
    int nVal = 1;
    setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&nVal, sizeof(nVal));
    return true;
  }
}
