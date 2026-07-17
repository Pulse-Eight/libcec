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

#include <cstdarg>
#include <cstdio>
#include <string>

namespace CEC
{
  /*!
   * @brief printf-style formatting into a std::string.
   *
   * The one piece of string handling with no standard answer before C++20's
   * std::format. Everything else libcec needs is on std::string itself.
   */
  class StringUtils
  {
  public:
    static std::string FormatV(const char* fmt, va_list args)
    {
      if (!fmt)
        return "";

      va_list argsCopy;
      va_copy(argsCopy, args);
      int len = vsnprintf(nullptr, 0, fmt, argsCopy);
      va_end(argsCopy);

      if (len <= 0)
        return "";

      std::string str(static_cast<size_t>(len), '\0');
      vsnprintf(&str[0], static_cast<size_t>(len) + 1, fmt, args);
      return str;
    }

    static std::string Format(const char* fmt, ...)
    {
      va_list args;
      va_start(args, fmt);
      std::string str = FormatV(fmt, args);
      va_end(args);
      return str;
    }
  };
};
