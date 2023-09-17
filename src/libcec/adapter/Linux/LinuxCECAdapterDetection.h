#pragma once
/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC Linux CEC Adapter is Copyright (C) 2017 Jonas Karlman
 * based heavily on:
 * libCEC AOCEC Code is Copyright (C) 2016 Gerald Dachs
 * libCEC Exynos Code is Copyright (C) 2014 Valentin Manea
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

#include "env.h"

#if defined(HAVE_LINUX_API)

namespace CEC
{
  class CLinuxCECAdapterDetection
  {
  public:
    static uint8_t FindAdapters(cec_adapter_descriptor *deviceList, uint8_t iBufSize, const char *strDevicePath = NULL);
    static bool IsAdapter(const char *strPort);

  private:
    static uint8_t FindAdaptersUdev(cec_adapter_descriptor *deviceList, uint8_t iBufSize, const char *strDevicePath = NULL);
    static uint8_t FindAdaptersLinux(cec_adapter_descriptor *deviceList, uint8_t iBufSize, const char *strDevicePath = NULL);
  };
};

#endif
