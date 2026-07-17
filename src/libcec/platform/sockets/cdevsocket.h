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
#include "platform/sockets/socket.h"

#include <fcntl.h>
#include <string>

namespace CEC
{
  class CCDevSocket : public CCommonSocket<chardev_socket_t>
  {
    public:
      CCDevSocket(const std::string &strName) :
        CCommonSocket<chardev_socket_t>(INVALID_CHARDEV_SOCKET_VALUE, strName) {}

      ~CCDevSocket(void)
      {
        Close();
      }

      /*!
       * @note iTimeoutMs is ignored: the open() below blocks. the callers that care
       *       retry around this instead.
       */
      bool Open(uint64_t iTimeoutMs = 0)
      {
        (void)iTimeoutMs;

        if (IsOpen())
          return false;

        m_socket = open(m_strName.c_str(), O_RDWR);

        if (m_socket == INVALID_CHARDEV_SOCKET_VALUE)
        {
          m_strError = strerror(errno);
          return false;
        }

        return true;
      }

      void Close(void)
      {
        if (IsOpen())
        {
          SocketClose(m_socket);
          m_socket = INVALID_CHARDEV_SOCKET_VALUE;
        }
      }

      int Ioctl(int request, void* data)
      {
        return IsOpen() ? SocketIoctl(m_socket, &m_iError, request, data) : -1;
      }

      ssize_t Write(void* data, size_t len)
      {
        return IsOpen() ? SocketWrite(m_socket, &m_iError, data, len) : -1;
      }

      ssize_t Read(void* data, size_t len, uint64_t iTimeoutMs = 0)
      {
        return IsOpen() ? SocketRead(m_socket, &m_iError, data, len, iTimeoutMs) : -1;
      }

      bool IsOpen(void)
      {
        return m_socket != INVALID_CHARDEV_SOCKET_VALUE;
      }
  };
};
