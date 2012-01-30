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
#include "../util/timeutils.h"

#pragma comment(lib, "Ws2_32.lib")
#include <ws2spi.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>

#define SHUT_RDWR SD_BOTH

#ifndef ETIMEDOUT
#define ETIMEDOUT 138
#endif

namespace PLATFORM
{
  #ifndef MSG_WAITALL
  #define MSG_WAITALL 0x8
  #endif

  inline int GetSocketError(void)
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

  inline void SocketClose(socket_t socket)
  {
    if (socket != SOCKET_ERROR && socket != INVALID_SOCKET)
      closesocket(socket);
  }

  inline void SocketSetBlocking(socket_t socket, bool bSetTo)
  {
    u_long nVal = bSetTo ? 1 : 0;
    ioctlsocket(socket, FIONBIO, &nVal);
  }

  inline int64_t SocketWrite(socket_t socket, int *iError, uint8_t* data, uint32_t len)
  {
    int64_t iReturn(-1);
    if (socket != SOCKET_ERROR && socket != INVALID_SOCKET)
    {
      iReturn = send(socket, (char*)data, len, 0);
      if (iReturn <= 0)
        *iError = GetSocketError();
    }
    return iReturn;
  }

  inline int SocketReadFixed(socket_t socket, char *buf, int iLength, int iFlags)
  {
    int iReadResult(1), iBytesRead(0);

    if ((iFlags & MSG_WAITALL) == 0)
      return recv(socket, buf, iLength, iFlags);

    iFlags &= ~MSG_WAITALL;
    while(iBytesRead < iLength && iReadResult > 0)
    {
      if ((iReadResult = recv(socket, buf + iBytesRead, iLength - iBytesRead, iFlags)) < 0)
        return iReadResult;
    }
    return iLength - iBytesRead;
  }

  inline int32_t SocketRead(socket_t socket, void *buf, uint32_t nLen)
  {
    int iReadResult = SocketReadFixed(socket, (char *)buf, nLen, MSG_WAITALL);

    if (iReadResult == -1)
      return GetSocketError();
    if (iReadResult < (int)nLen)
      return ECONNRESET;

    return 0;
  }

  inline int32_t SocketRead(socket_t socket, int *iError, uint8_t* data, uint32_t iLength, uint64_t iTimeoutMs)
  {
    int iReadResult(0);
    uint32_t iBytesRead(0);
    fd_set fd_read;
    struct timeval tv;

    if (iTimeoutMs <= 0)
      return EINVAL;

    uint64_t iNow = GetTimeMs();
    uint64_t iTarget = iNow + iTimeoutMs;

    while(iNow < iTarget && iBytesRead < iLength)
    {
      tv.tv_sec  = (long)(iTarget - iNow / 1000);
      tv.tv_usec = (long)(1000 * (iTarget - iNow % 1000));

      FD_ZERO(&fd_read);
      FD_SET(socket, &fd_read);

      if ((iReadResult = select(socket + 1, &fd_read, NULL, NULL, &tv)) == 0)
        return ETIMEDOUT;

      SocketSetBlocking(socket, false);

      iReadResult = SocketReadFixed(socket, (char *)data + iBytesRead, iLength - iBytesRead, 0);

      SocketSetBlocking(socket, true);

      if (iReadResult == -1)
      {
        int iError = GetSocketError();
        if (iError == EAGAIN)
          continue;
        return iError;
      }
      else if (iReadResult == 0)
        return ECONNRESET;

      iBytesRead += iReadResult;
    }

    return iBytesRead == iLength ? 0 : ECONNRESET;
  }
}
