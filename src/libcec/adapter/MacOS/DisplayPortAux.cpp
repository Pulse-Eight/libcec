#include "env.h"

#if defined(HAVE_MACOS_API)

#include "DisplayPortAux.h"
#include "platform/util/edid.h"

extern "C" {
#include <ApplicationServices/ApplicationServices.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <IOKit/i2c/IOI2CInterface.h>
}

namespace {
/* Iterate IOreg's device tree to find the IOFramebuffer mach service port
that corresponds to a given CGDisplayID replaces CGDisplayIOServicePort:
https://developer.apple.com/library/mac/documentation/GraphicsImaging/Reference/Quartz_Services_Ref/index.html#//apple_ref/c/func/CGDisplayIOServicePort
based on: https://github.com/glfw/glfw/pull/192/files
*/
io_service_t IOFramebufferPortFromCGDisplayID(CGDirectDisplayID displayID) {
  io_iterator_t iter;
  io_service_t serv, servicePort = 0;

  kern_return_t err = IOServiceGetMatchingServices(
      kIOMasterPortDefault, IOServiceMatching(IOFRAMEBUFFER_CONFORMSTO), &iter);

  if (err != KERN_SUCCESS) return 0;

  // now recurse the IOReg tree
  while ((serv = IOIteratorNext(iter)) != MACH_PORT_NULL) {
    CFDictionaryRef info;
    io_name_t name;
    CFIndex vendorID = 0, productID = 0, serialNumber = 0;
    CFNumberRef vendorIDRef, productIDRef, serialNumberRef;
    Boolean success = 0;

    // get metadata from IOreg node
    IORegistryEntryGetName(serv, name);
    info = IODisplayCreateInfoDictionary(serv, kIODisplayOnlyPreferredName);

    if (CFDictionaryGetValueIfPresent(info, CFSTR(kDisplayVendorID),
                                      (const void **)&vendorIDRef)) {
      success = CFNumberGetValue(vendorIDRef, kCFNumberCFIndexType, &vendorID);
    }

    if (CFDictionaryGetValueIfPresent(info, CFSTR(kDisplayProductID),
                                      (const void **)&productIDRef)) {
      success &=
          CFNumberGetValue(productIDRef, kCFNumberCFIndexType, &productID);
    }

    IOItemCount busCount;
    IOFBGetI2CInterfaceCount(serv, &busCount);

    if (!success || busCount < 1 || CGDisplayIsBuiltin(displayID)) {
      // this does not seem to be a DDC-enabled display, skip it
      CFRelease(info);
      continue;
    }

    if (CFDictionaryGetValueIfPresent(info, CFSTR(kDisplaySerialNumber),
                                      (const void **)&serialNumberRef)) {
      CFNumberGetValue(serialNumberRef, kCFNumberCFIndexType, &serialNumber);
    }

    // compare IOreg's metadata to CGDisplay's metadata to infer if the
    // IOReg's I2C monitor is the display for the given NSScreen.displayID
    if (CGDisplayVendorNumber(displayID) != vendorID ||
        CGDisplayModelNumber(displayID) != productID ||
        CGDisplaySerialNumber(displayID) !=
            serialNumber)  // SN is zero in lots of cases, so
                           // duplicate-monitors can confuse us :-/
    {
      CFRelease(info);
      continue;
    }

    servicePort = serv;
    CFRelease(info);
    break;
  }

  IOObjectRelease(iter);
  return servicePort;
}

}  // namespace

using CLockObject = P8PLATFORM::CLockObject;

DisplayPortAux::DisplayPortAux()
    : m_framebuffer(IOFramebufferPortFromCGDisplayID(CGMainDisplayID())) {}

DisplayPortAux::~DisplayPortAux() { IOObjectRelease(m_framebuffer); }

bool DisplayPortAux::Write(uint16_t address, uint8_t *data, uint32_t len) {
  IOI2CRequest request = {
      .sendTransactionType = kIOI2CDisplayPortNativeTransactionType,
      .sendAddress = address,
      .sendBytes = len,
      .sendBuffer = (vm_address_t)data,
  };
  return DisplayRequest(&request);
}

bool DisplayPortAux::Read(uint16_t address, uint8_t *data, uint32_t len) {
  IOI2CRequest request = {
      .sendTransactionType = kIOI2CNoTransactionType,

      .sendAddress = 0,
      .sendBytes = 0,
      .sendBuffer = (vm_address_t)data,

      .replyTransactionType = kIOI2CDisplayPortNativeTransactionType,
      .replyAddress = address,
      .replyBytes = len,
      .replyBuffer = (vm_address_t)data,
  };
  return DisplayRequest(&request);
}

uint16_t DisplayPortAux::GetPhysicalAddress() {
  auto info =
      IODisplayCreateInfoDictionary(m_framebuffer, kIODisplayOnlyPreferredName);

  uint16_t phys = 0;
  CFDataRef edidData = 0;
  if (CFDictionaryGetValueIfPresent(info, CFSTR(kIODisplayEDIDKey),
                                    (const void **)&edidData)) {
    phys = P8PLATFORM::CEDIDParser::GetPhysicalAddressFromEDID(
        CFDataGetBytePtr(edidData), CFDataGetLength(edidData));
  }

  CFRelease(info);
  return phys;
}

bool DisplayPortAux::DisplayRequest(IOI2CRequest *request) {
  CLockObject lock(m_mutex);
  bool result = false;
  IOItemCount busCount;
  if (IOFBGetI2CInterfaceCount(m_framebuffer, &busCount) == KERN_SUCCESS) {
    IOOptionBits bus = 0;
    while (bus < busCount) {
      io_service_t interface;
      if (IOFBCopyI2CInterfaceForBus(m_framebuffer, bus++, &interface) !=
          KERN_SUCCESS)
        continue;

      IOI2CConnectRef connect;
      if (IOI2CInterfaceOpen(interface, kNilOptions, &connect) ==
          KERN_SUCCESS) {
        result =
            IOI2CSendRequest(connect, kNilOptions, request) == KERN_SUCCESS;
        IOI2CInterfaceClose(connect, kNilOptions);
      }
      IOObjectRelease(interface);
      if (result) break;
    }
  }
  if (request->replyTransactionType == kIOI2CNoTransactionType) {
    usleep(10000);
  }
  return result && request->result == KERN_SUCCESS;
}
#endif
