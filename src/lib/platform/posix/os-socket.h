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
#include <stdio.h>
#include <fcntl.h>

namespace PLATFORM
{
  inline void SocketClose(socket_t socket)
  {
    if (socket != INVALID_SOCKET)
      close(socket);
  }

  inline void SocketSetBlocking(socket_t socket, bool bSetTo)
  {
//    return bSetTo ?
//            fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) & ~O_NONBLOCK) == 0 :
//            fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) | O_NONBLOCK) == 0;
    fcntl(socket, F_SETFL, bSetTo ? FNDELAY : 0);
  }

  inline int64_t SocketWrite(socket_t socket, int *iError, uint8_t* data, uint32_t len)
  {
    fd_set port;

    if (socket == -1)
    {
      *iError = EINVAL;
      return -1;
    }

    int64_t iBytesWritten = 0;
    struct timeval *tv(NULL);

    while (iBytesWritten < len)
    {
      FD_ZERO(&port);
      FD_SET(socket, &port);
      int returnv = select(socket + 1, NULL, &port, NULL, tv);
      if (returnv < 0)
      {
        *iError = errno;
        return -1;
      }
      else if (returnv == 0)
      {
        *iError = ETIMEDOUT;
        return -1;
      }

      returnv = write(socket, data + iBytesWritten, len - iBytesWritten);
      if (returnv == -1)
      {
        *iError = errno;
        return -1;
      }
      iBytesWritten += returnv;
    }

    return iBytesWritten;
  }

  inline int32_t SocketRead(socket_t socket, int *iError, uint8_t* data, uint32_t len, uint64_t iTimeoutMs /*= 0*/)
  {
    fd_set port;
    struct timeval timeout, *tv;
    int64_t iNow(0), iTarget(0);
    int32_t iBytesRead = 0;
    *iError = 0;

    if (socket == -1)
    {
      *iError = EINVAL;
      return -1;
    }

    if (iTimeoutMs > 0)
    {
      iNow    = GetTimeMs();
      iTarget = iNow + (int64_t) iTimeoutMs;
    }

    while (iBytesRead < (int32_t) len && (iTimeoutMs == 0 || iTarget > iNow))
    {
      if (iTimeoutMs == 0)
      {
        tv = NULL;
      }
      else
      {
        timeout.tv_sec  = ((long int)iTarget - (long int)iNow) / (long int)1000.;
        timeout.tv_usec = ((long int)iTarget - (long int)iNow) % (long int)1000.;
        tv = &timeout;
      }

      FD_ZERO(&port);
      FD_SET(socket, &port);
      int32_t returnv = select(socket + 1, &port, NULL, NULL, tv);

      if (returnv == -1)
      {
        *iError = errno;
        return -1;
      }
      else if (returnv == 0)
      {
        break; //nothing to read
      }

      returnv = read(socket, data + iBytesRead, len - iBytesRead);
      if (returnv == -1)
      {
        *iError = errno;
        return -1;
      }

      iBytesRead += returnv;

      if (iTimeoutMs > 0)
        iNow = GetTimeMs();
    }

    return iBytesRead;
  }
}
