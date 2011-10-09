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
#if defined(__APPLE__)
#include <mach/mach_time.h>
#include <CoreVideo/CVHostTime.h>
#elif defined(__WINDOWS__)
#include <time.h>
#else
#include <sys/time.h>
#endif

namespace CEC
{
  inline int64_t GetTimeMs()
  {
  #if defined(__APPLE__)
    return (int64_t) (CVGetCurrentHostTime() * 1000 / CVGetHostClockFrequency());
  #elif defined(__WINDOWS__)
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
    timeval time;
    gettimeofday(&time, NULL);
    return (int64_t) (time.tv_sec * 1000 + time.tv_usec / 1000);
  #endif
  }

  template <class T>
  inline T GetTimeSec()
  {
    return (T)GetTimeMs() / (T)1000.0;
  }
};
