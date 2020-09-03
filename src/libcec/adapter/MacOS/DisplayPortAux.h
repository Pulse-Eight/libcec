/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC MacOS Code is Copyright (C) 2020 Adam Engström
 * libCEC(R) is Copyright (C) 2011-2015 Pulse-Eight Limited.  All rights
 * reserved. libCEC(R) is an original work, containing original code.
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

#pragma once

#include "env.h"

#if defined(HAVE_MACOS_API)

#include <IOKit/IOKitLib.h>

#include "p8-platform/threads/mutex.h"

class IOI2CRequest;
namespace CEC {
class CMacOSCECAdapterCommunication;
}

class DisplayPortAux {
 public:
  DisplayPortAux(CEC::CMacOSCECAdapterCommunication *);
  virtual ~DisplayPortAux();

  bool Write(uint16_t address, uint8_t data) {
    return Write(address, &data, 1);
  }
  bool Write(uint16_t address, uint8_t *data, uint32_t len);

  bool Read(uint16_t address, uint8_t *byte) { return Read(address, byte, 1); }
  bool Read(uint16_t address, uint8_t *data, uint32_t len);

  uint16_t GetPhysicalAddress();

  bool IsOpen() { return m_framebuffer; }

 private:
  bool DisplayRequest(IOI2CRequest *request);

  io_service_t m_framebuffer;
  mutable P8PLATFORM::CMutex m_mutex;
  CEC::CMacOSCECAdapterCommunication *m_com;
};
#endif
