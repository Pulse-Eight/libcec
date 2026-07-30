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

#include <string.h>

namespace CEC
{
  class CEDIDParser
  {
  public:
    static uint16_t GetPhysicalAddress(void);

    /**
     * @return The year the display that libCEC is connected to was made, or 0 when there is
     *         no EDID to read it from. Not every combination of hardware exposes one.
     */
    static uint16_t GetModelYear(void)
    {
      if (!EDIDRead())
      {
        EDIDRead() = true;
        GetPhysicalAddress();
      }
      return CachedModelYear();
    }

    static uint16_t GetPhysicalAddressFromEDID(unsigned char *data, size_t size)
    {
      return GetPhysicalAddressFromEDID((char *)data, size);
    }

    static uint16_t GetPhysicalAddressFromEDID(char *data, size_t size)
    {
      uint16_t iPA(0);

      for (size_t iPtr = 0; data && size >= 5 && iPtr + 4 < size; iPtr++)
      {
        if (data[iPtr]     == 0x03 &&
            data[iPtr + 1] == 0x0C &&
            data[iPtr + 2] == 0x0)
        {
          //found the hdmi marker
          iPA = ((uint8_t)data[iPtr + 3] << 8) | (uint8_t)data[iPtr + 4];
          break;
        }
      }

      /* keep the model year of the display that the address came from, so that GetModelYear()
         reports the one libCEC is connected to rather than any other display */
      if (iPA != 0)
        CachedModelYear() = GetModelYearFromEDID(data, size);

      return iPA;
    }

    static uint16_t GetModelYearFromEDID(char *data, size_t size)
    {
      /* the base block opens with a fixed header, and carries the year of manufacture - or
         the model year, when the week byte is 0xff - as an offset from 1990 */
      static const char header[] = { 0x00, (char)0xff, (char)0xff, (char)0xff, (char)0xff, (char)0xff, (char)0xff, 0x00 };

      if (!data || size < 18 || memcmp(data, header, sizeof(header)) != 0 || data[17] == 0)
        return 0;

      return (uint16_t)(1990 + (uint8_t)data[17]);
    }

  private:
    static uint16_t& CachedModelYear(void) { static uint16_t iModelYear(0); return iModelYear; }
    static bool&     EDIDRead(void)        { static bool bEDIDRead(false); return bEDIDRead; }
  };
}
