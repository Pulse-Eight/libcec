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

#include "platform/os.h"
#include "platform/util/timeutils.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>

namespace CEC
{
  inline void SocketClose(socket_t socket)
  {
    if (socket != INVALID_SOCKET_VALUE)
      close(socket);
  }

  inline void SocketSetBlocking(socket_t socket, bool bSetTo)
  {
    if (socket != INVALID_SOCKET_VALUE)
    {
      if (bSetTo)
        fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) & ~O_NONBLOCK);
      else
        fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) | O_NONBLOCK);
    }
  }

  inline ssize_t SocketWrite(socket_t socket, int *iError, void* data, size_t len)
  {
    fd_set port;

    if (socket == INVALID_SOCKET_VALUE)
    {
      *iError = EINVAL;
      return -EINVAL;
    }

    ssize_t iBytesWritten(0);

    while (iBytesWritten < (ssize_t)len)
    {
      FD_ZERO(&port);
      FD_SET(socket, &port);
      int returnv = select(socket + 1, NULL, &port, NULL, NULL);
      if (returnv < 0)
      {
        *iError = errno;
        return -errno;
      }
      else if (returnv == 0)
      {
        *iError = ETIMEDOUT;
        return -ETIMEDOUT;
      }

      returnv = write(socket, (char*)data + iBytesWritten, len - iBytesWritten);
      if (returnv == -1)
      {
        *iError = errno;
        return -errno;
      }
      iBytesWritten += returnv;
    }

    return iBytesWritten;
  }

  inline ssize_t SocketRead(socket_t socket, int *iError, void* data, size_t len, uint64_t iTimeoutMs /*= 0*/)
  {
    fd_set port;
    struct timeval timeout, *tv;
    ssize_t iBytesRead(0);
    *iError = 0;
    CTimeout readTimeout(iTimeoutMs);

    if (socket == INVALID_SOCKET_VALUE)
    {
      *iError = EINVAL;
      return -EINVAL;
    }

    while (iBytesRead >= 0 && iBytesRead < (ssize_t)len && (iTimeoutMs == 0 || readTimeout.TimeLeft() > 0))
    {
      if (iTimeoutMs == 0)
      {
        tv = NULL;
      }
      else
      {
        long iTimeLeft = (long)readTimeout.TimeLeft();
        timeout.tv_sec  = iTimeLeft / 1000;
        timeout.tv_usec = (iTimeLeft % 1000) * 1000;
        tv = &timeout;
      }

      FD_ZERO(&port);
      FD_SET(socket, &port);
      int32_t returnv = select(socket + 1, &port, NULL, NULL, tv);

      if (returnv == -1)
      {
        *iError = errno;
        return -errno;
      }
      else if (returnv == 0)
      {
        break; //nothing to read
      }

      returnv = read(socket, (char*)data + iBytesRead, len - iBytesRead);
      if (returnv == -1)
      {
        *iError = errno;
        return -errno;
      }

      iBytesRead += returnv;
    }

    return iBytesRead;
  }

  inline int SocketIoctl(socket_t socket, int *iError, int request, void* data)
  {
    if (socket == INVALID_SOCKET_VALUE)
    {
      *iError = EINVAL;
      return -1;
    }

    int iReturn = ioctl(socket, request, data);
    if (iReturn < 0)
      *iError = errno;
    return iReturn;
  }
};
