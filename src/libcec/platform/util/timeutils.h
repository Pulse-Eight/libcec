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

#include <chrono>
#include <stdint.h>

namespace CEC
{
  /*!
   * @return the number of milliseconds on a monotonic clock. only meaningful when
   *         compared against another value from this function.
   */
  inline int64_t GetTimeMs(void)
  {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
  }

  class CTimeout
  {
  public:
    CTimeout(void) : m_bSet(false) {}
    CTimeout(uint32_t iTimeout) { Init(iTimeout); }

    bool IsSet(void) const { return m_bSet; }

    void Init(uint32_t iTimeout)
    {
      m_target = std::chrono::steady_clock::now() + std::chrono::milliseconds(iTimeout);
      m_bSet   = true;
    }

    uint32_t TimeLeft(void) const
    {
      if (!m_bSet)
        return 0;

      auto now = std::chrono::steady_clock::now();
      if (now >= m_target)
        return 0;

      return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(m_target - now).count();
    }

  private:
    std::chrono::steady_clock::time_point m_target;
    bool                                  m_bSet;
  };
};
