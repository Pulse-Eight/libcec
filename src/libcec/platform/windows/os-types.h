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

// libcec branches on __WINDOWS__ in a handful of places. nothing but this
// header has ever defined it.
#if !defined(__WINDOWS__)
#define __WINDOWS__
#endif

// windows.h pulls in winsock 1 unless told not to, which then clashes with
// winsock2 below. this has to be defined before windows.h is reached.
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX                      // keep min()/max() macros away from std::min()/std::max()
#endif
#include <windows.h>

#pragma warning(disable:4005)         // 'macro redefinition' for _WINSOCKAPI_
#include <winsock2.h>
#pragma warning(default:4005)

#include <errno.h>
#include <io.h>
#include <process.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>

typedef SOCKET tcp_socket_t;
#define INVALID_SOCKET_VALUE        INVALID_SOCKET

typedef HANDLE serial_socket_t;
#define INVALID_SERIAL_SOCKET_VALUE INVALID_HANDLE_VALUE

#ifndef _SSIZE_T_DEFINED
#if defined(_WIN64) || defined(_M_ARM64)
typedef __int64    ssize_t;
#else
typedef _W64 int   ssize_t;
#endif
#define _SSIZE_T_DEFINED
#endif

#pragma warning(disable:4189)         // 'defined but not used'
#pragma warning(disable:4100)         // 'unreferenced formal parameter'
