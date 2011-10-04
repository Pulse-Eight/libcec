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

#include <stdint.h>
#include <time.h>

namespace CEC
{
  inline int64_t GetTimeMs()
  {
  #ifdef __WINDOWS__
    time_t rawtime;
    time(&rawtime);

    LARGE_INTEGER tickPerSecond;
    LARGE_INTEGER tick;
    if (QueryPerformanceFrequency(&tickPerSecond))
    {
      QueryPerformanceCounter(&tick);
      return (int64_t) (tick.QuadPart / 1000.);
    }
    return -1;
  #else
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);

    return ((int64_t)time.tv_sec * (int64_t)1000) + (int64_t)time.tv_nsec / (int64_t)1000;
  #endif
  }

  template <class T>
  inline T GetTimeSec()
  {
    return (T)GetTimeMs() / (T)1000.0;
  }
};
