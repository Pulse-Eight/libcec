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

#include "../os.h"
#include "../sockets/socket.h"

namespace PLATFORM
{
  inline int TcpGetSocketError(socket_t socket)
  {
    int iReturn(0);
    socklen_t optLen = sizeof(socket_t);
    getsockopt(socket, SOL_SOCKET, SO_ERROR, (void *)&iReturn, &optLen);
    return iReturn;
  }

  inline bool TcpSetNoDelay(socket_t socket)
  {
    int iSetTo(1);
    setsockopt(socket, SOL_TCP, TCP_NODELAY, &iSetTo, sizeof(iSetTo));
    return true;
  }

  inline bool TcpConnectSocket(socket_t socket, struct addrinfo* addr, int *iError, uint64_t iTimeout = 0)
  {
    bool bConnected = (connect(socket, addr->ai_addr, addr->ai_addrlen) == 0);
    if (!bConnected && errno == EINPROGRESS)
    {
      struct pollfd pfd;
      pfd.fd = socket;
      pfd.events = POLLOUT;
      pfd.revents = 0;

      int iPollResult = poll(&pfd, 1, iTimeout);
      if (iPollResult == 0)
        *iError = ETIMEDOUT;
      else if (iPollResult == -1)
        *iError = errno;
      else
        *iError = TcpGetSocketError(socket);
    }

    return bConnected;
  }

  inline bool TcpResolveAddress(const char *strHost, uint16_t iPort, int *iError, struct addrinfo *info)
  {
    struct   addrinfo hints;
    char     service[33];
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    sprintf(service, "%d", iPort);

    return ((*iError = getaddrinfo(strHost, service, &hints, &info)) == 0);
  }
}
