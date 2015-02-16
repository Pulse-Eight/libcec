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

#define _FILE_OFFSET_BITS 64
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#if !defined(__APPLE__) && !defined(__FreeBSD__)
#include <sys/prctl.h>
#endif
#include <pthread.h>
#include <poll.h>
#include <semaphore.h>
#include <stdint.h>

#include <inttypes.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>

#define LIBTYPE
#define DECLSPEC

typedef int socket_t;
typedef socket_t tcp_socket_t;
#define INVALID_SOCKET_VALUE        (-1)
typedef socket_t serial_socket_t;
#define INVALID_SERIAL_SOCKET_VALUE (-1)

typedef long LONG;
typedef LONG HRESULT;

#define _FILE_OFFSET_BITS 64
#define FILE_BEGIN              0
#define FILE_CURRENT            1
#define FILE_END                2

// Success codes
#define S_OK           0L
#define S_FALSE        1L
#define FAILED(Status) ((HRESULT)(Status)<0)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

// Error codes
#define ERROR_FILENAME_EXCED_RANGE 206L
#define ERROR_INVALID_NAME         123L
#define E_OUTOFMEMORY              0x8007000EL
#define E_FAIL                     0x8004005EL

#ifdef TARGET_LINUX
#include <limits.h>
#define MAX_PATH PATH_MAX
#elif defined TARGET_DARWIN || defined __FreeBSD__
#include <sys/syslimits.h>
#define MAX_PATH PATH_MAX
#else
#define MAX_PATH 256
#endif

#if defined(__APPLE__)
  #include <stdio.h> // for fpos_t
  #include <sched.h>
  #include <AvailabilityMacros.h>
  typedef int64_t   off64_t;
  typedef off_t     __off_t;
  typedef off64_t   __off64_t;
  typedef fpos_t fpos64_t;
  #define __stat64 stat
  #define stat64 stat
  #if defined(TARGET_DARWIN_IOS)
    #define statfs64 statfs
  #endif
  #define fstat64 fstat
#elif defined(__FreeBSD__)
  #include <stdio.h> // for fpos_t
  typedef int64_t   off64_t;
  typedef off_t     __off_t;
  typedef off64_t   __off64_t;
  typedef fpos_t fpos64_t;
  #define __stat64 stat
  #define stat64 stat
  #define statfs64 statfs
  #define fstat64 fstat
#else
  #define __stat64 stat64
#endif

#include <string.h>
#define strnicmp(X,Y,N) strncasecmp(X,Y,N)

typedef unsigned char byte;

/* Platform dependent path separator */
#ifndef PATH_SEPARATOR_CHAR
#define PATH_SEPARATOR_CHAR '/'
#define PATH_SEPARATOR_STRING "/"
#endif

#ifdef TARGET_LINUX
// Retrieve the number of milliseconds that have elapsed since the system was started
#include <time.h>
inline unsigned long GetTickCount(void)
{
  struct timespec ts;
  if(clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
  {
    return 0;
  }
  return (unsigned long)( (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000) );
};
#else
#include <time.h>
inline unsigned long GetTickCount(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (unsigned long)( (tv.tv_sec * 1000) + (tv.tv_usec / 1000) );
};
#endif /* TARGET_LINUX || TARGET_DARWIN */

/* Handling of 2-byte Windows wchar strings on non-Windows targets
 * Used by The MediaPortal and ForTheRecord pvr addons 
 */
typedef uint16_t Wchar_t; /* sizeof(wchar_t) = 4 bytes on Linux, but the MediaPortal buffer files have 2-byte wchars */

/* This is a replacement of the Windows wcslen() function which assumes that
 * wchar_t is a 2-byte character.
 * It is used for processing Windows wchar strings
 */
inline size_t WcsLen(const Wchar_t *str)
{
  const unsigned short *eos = (const unsigned short*)str;
  while( *eos++ ) ;
  return( (size_t)(eos - (const unsigned short*)str) -1);
};

/* This is a replacement of the Windows wcstombs() function which assumes that
 * wchar_t is a 2-byte character.
 * It is used for processing Windows wchar strings
 */
inline size_t WcsToMbs(char *s, const Wchar_t *w, size_t n)
{
  size_t i = 0;
  const unsigned short *wc = (const unsigned short*) w;
  while(wc[i] && (i < n))
  {
    s[i] = wc[i];
    ++i;
  }
  if (i < n) s[i] = '\0';

  return (i);
};
