#pragma once

/*
 * boblight
 * Copyright (C) Bob  2009 
 * 
 * boblight is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * boblight is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "os-dependent.h"
#include "../../../include/CECExports.h"
#include <string>
#include <stdint.h>
#include "../platform/threads.h"

#ifndef __WINDOWS__
#include <termios.h>
#else
#include "../util/buffer.h"
#endif

namespace CEC
{
  #define PAR_NONE 0
  #define PAR_EVEN 1
  #define PAR_ODD  2

  class CSerialPort
  {
    public:
      CSerialPort();
      virtual ~CSerialPort();

      bool Open(std::string name, uint32_t baudrate, uint8_t databits = 8, uint8_t stopbits = 1, uint8_t parity = PAR_NONE);
      bool IsOpen();
      void Close();

      int8_t Write(const cec_adapter_message &data);
      int32_t Read(uint8_t* data, uint32_t len, uint64_t iTimeoutMs = 0);

      std::string GetError() { return m_error; }
      std::string GetName() { return m_name; }

  private:
      bool SetBaudRate(uint32_t baudrate);

      std::string     m_error;
      std::string     m_name;
      CMutex          m_mutex;

  #ifdef __WINDOWS__
      bool SetTimeouts(bool bBlocking);

      HANDLE             m_handle;
      bool               m_bIsOpen;
      uint32_t           m_iBaudrate;
      uint8_t            m_iDatabits;
      uint8_t            m_iStopbits;
      uint8_t            m_iParity;
      int64_t            m_iTimeout;
      CecBuffer<uint8_t> m_buffer;
      HANDLE             m_ovHandle;
  #else
      struct termios     m_options;
      int                m_fd;
  #endif
  };
};
