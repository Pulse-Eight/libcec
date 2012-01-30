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
#include "../util/buffer.h"

#include <string>
#include <stdint.h>

#if !defined(__WINDOWS__)
#include <termios.h>
#endif

#include "socket.h"

namespace PLATFORM
{
  #define PAR_NONE 0
  #define PAR_EVEN 1
  #define PAR_ODD  2

  class CSerialPort : public CSocket
  {
    public:
      CSerialPort(void);
      virtual ~CSerialPort(void) {}

      bool Open(std::string name, uint32_t baudrate, uint8_t databits = 8, uint8_t stopbits = 1, uint8_t parity = PAR_NONE);

      CStdString GetName(void) const
      {
        CStdString strName;
        strName = m_strName;
        return strName;
      }

    #ifdef __WINDOWS__
      virtual bool IsOpen(void);
      virtual void Close(void);
      virtual int64_t Write(uint8_t* data, uint32_t len);
      virtual int32_t Read(uint8_t* data, uint32_t len, uint64_t iTimeoutMs = 0);
    #endif

    private:
      bool SetBaudRate(uint32_t baudrate);

    private:
    #ifdef __WINDOWS__
      void FormatWindowsError(int iErrorCode, CStdString &strMessage);
      bool SetTimeouts(bool bBlocking);

      HANDLE                m_handle; 
      bool                  m_bIsOpen;
      uint32_t              m_iBaudrate;
      uint8_t               m_iDatabits;
      uint8_t               m_iStopbits;
      uint8_t               m_iParity;
      int64_t               m_iTimeout;
      SyncedBuffer<uint8_t> m_buffer;
      HANDLE                m_ovHandle;
  #else
      struct termios     m_options;
  #endif
      std::string  m_strName;
      bool         m_bToStdOut;
  };
};
