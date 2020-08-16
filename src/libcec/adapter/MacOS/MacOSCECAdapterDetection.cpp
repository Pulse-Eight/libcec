/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC Exynos Code is Copyright (C) 2014 Valentin Manea
 * libCEC(R) is Copyright (C) 2011-2015 Pulse-Eight Limited.  All rights
 * reserved. libCEC(R) is an original work, containing original code.
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

#include <ApplicationServices/ApplicationServices.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <IOKit/i2c/IOI2CInterface.h>
#include <stdio.h>

#include "env.h"

#if defined(HAVE_MACOS_API)
#include "MacOSCEC.h"
#include "MacOSCECAdapterDetection.h"

using namespace CEC;

bool CMacOSCECAdapterDetection::FindAdapter(void) {
  kern_return_t kr;
  io_iterator_t io_objects;
  io_service_t io_service;

  kr = IOServiceGetMatchingServices(
      kIOMasterPortDefault, IOServiceNameMatching("IOFramebufferI2CInterface"),
      &io_objects);

  if (kr != KERN_SUCCESS) {
    printf("E: Fatal - No matching service! \n");
    return 0;
  }

  uint8_t hasDisplayPortNativeSupport(0);
  while ((io_service = IOIteratorNext(io_objects)) != MACH_PORT_NULL) {
    CFMutableDictionaryRef service_properties;
    CFIndex types = 0;
    CFNumberRef typesRef;

    kr = IORegistryEntryCreateCFProperties(io_service, &service_properties,
                                           kCFAllocatorDefault, kNilOptions);
    if (kr == KERN_SUCCESS) {
      if (CFDictionaryGetValueIfPresent(service_properties,
                                        CFSTR(kIOI2CTransactionTypesKey),
                                        (const void**)&typesRef)) {
        CFNumberGetValue(typesRef, kCFNumberCFIndexType, &types);
        hasDisplayPortNativeSupport |=
            types & (1 << kIOI2CDisplayPortNativeTransactionType);
      }
      CFRelease(service_properties);
    }
    IOObjectRelease(io_service);
  }
  return hasDisplayPortNativeSupport;
}

#endif
