#pragma once
/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#ifdef _MSC_VER
#include <windows.h>
#endif

///////////////////////////////////////////////////////////////////////////
// 32-bit atomic increment
// Returns new value of *pAddr
///////////////////////////////////////////////////////////////////////////
static inline long atomic_inc(volatile long* pAddr)
{
#if defined(HAS_BUILTIN_SYNC_ADD_AND_FETCH)
  return __sync_add_and_fetch(pAddr, 1);

#elif defined(__ppc__) || defined(__powerpc__) // PowerPC
  long val;
  __asm__ __volatile__ (
    "sync             \n"
    "1: lwarx  %0, 0, %1 \n"
    "addic  %0, %0, 1 \n"
    "stwcx. %0, 0, %1 \n"
    "bne-   1b        \n"
    "isync"
    : "=&r" (val)
    : "r" (pAddr)
    : "cc", "xer", "memory");
  return val;

#elif defined(__arm__) && !defined(__ARM_ARCH_5__)
  register long val;
  asm volatile (
    "dmb      ish            \n" // Memory barrier. Make sure all memory accesses appearing before this complete before any that appear after
    "1:                     \n"
    "ldrex   %0, [%1]       \n" // (val = *pAddr)
    "add     %0,  #1        \n" // (val += 1)
    "strex   r1,  %0, [%1]      \n"
    "cmp     r1,   #0       \n"
    "bne     1b             \n"
    "dmb     ish            \n" // Memory barrier.
    : "=&r" (val)
    : "r"(pAddr)
    : "r1"
    );
  return val;

#elif defined(__mips__)
// TODO:
  long val;
  #error AtomicIncrement undefined for mips
  return val;

#elif defined(WIN32)
  long val;
  __asm
  {
    mov eax, pAddr ;
    lock inc dword ptr [eax] ;
    mov eax, [eax] ;
    mov val, eax ;
  }
  return val;

#elif defined(__x86_64__)
  register long result;
  __asm__ __volatile__ (
    "lock/xaddq %q0, %1"
    : "=r" (result), "=m" (*pAddr)
    : "0" ((long) (1)), "m" (*pAddr));
  return *pAddr;

#else // Linux / OSX86 (GCC)
  register long reg __asm__ ("eax") = 1;
  __asm__ __volatile__ (
    "lock/xadd %0, %1 \n"
    "inc %%eax"
    : "+r" (reg)
    : "m" (*pAddr)
    : "memory" );
  return reg;

#endif
}
