#pragma once
/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2015 Pulse-Eight Limited.  All rights reserved.
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

#include "env.h"

namespace CEC
{
  #define CEC_VID  0x2548
  #define CEC_PID  0x1001
  #define CEC_PID2 0x1002
  
  class CUSBCECAdapterDetection
  {
  public:
    static uint8_t FindAdapters(cec_adapter_descriptor *deviceList, uint8_t iBufSize, const char *strDevicePath = NULL);
    static bool    CanAutodetect(void);

  private:
    static uint8_t FindAdaptersWindows(cec_adapter_descriptor *deviceList, uint8_t iBufSize, const char *strDevicePath = NULL);
    static uint8_t FindAdaptersApple(cec_adapter_descriptor *deviceList, uint8_t iBufSize, const char *strDevicePath = NULL);
    static uint8_t FindAdaptersUdev(cec_adapter_descriptor *deviceList, uint8_t iBufSize, const char *strDevicePath = NULL);
    static uint8_t FindAdaptersFreeBSD(cec_adapter_descriptor *deviceList, uint8_t iBufSize, const char *strDevicePath = NULL);
  };
};
