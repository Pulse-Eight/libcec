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
#include "USBCECAdapterDetection.h"

#if defined(__APPLE__)
#include <dirent.h>
#include <sys/param.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <CoreFoundation/CoreFoundation.h>
#elif defined(__WINDOWS__)
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "cfgmgr32.lib")
#include <setupapi.h>
#include <cfgmgr32.h>
#include <tchar.h>

// the virtual COM port only shows up when requesting devices with the raw device guid!
static GUID USB_RAW_GUID = { 0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } };
static GUID USB_CDC_GUID = { 0x4D36E978, 0xE325, 0x11CE, { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };

#elif defined(HAVE_LIBUDEV)
#include <dirent.h>
#include <poll.h>
extern "C" {
#include <libudev.h>
}
#elif defined(__FreeBSD__)
#include <sys/param.h>
#include <sys/sysctl.h>
#include <stdio.h>
#include <unistd.h>
#endif

#include <string>
#include <algorithm>
#include <stdio.h>
#include "p8-platform/util/StringUtils.h"

#define CEC_VID  0x2548
#define CEC_PID  0x1001
#define CEC_PID2 0x1002

using namespace CEC;

#if defined(HAVE_LIBUDEV)
bool TranslateComPort(std::string& strString)
{
  std::string strTmp(strString);
  std::reverse(strTmp.begin(), strTmp.end());
  const char* iSlash = strchr(strTmp.c_str(), '/');
  if (iSlash)
  {
    strTmp = StringUtils::Left(strTmp, iSlash - strTmp.c_str());
    std::reverse(strTmp.begin(), strTmp.end());
    strString = StringUtils::Format("%s/%s:1.0/tty", strString.c_str(), strTmp.c_str());
    return true;
  }

  return false;
}

bool FindComPort(std::string& strLocation)
{
  std::string strPort = strLocation;
  bool bReturn(!strPort.empty());
  std::string strConfigLocation(strLocation);
  if (TranslateComPort(strConfigLocation))
  {
    DIR *dir;
    struct dirent *dirent;
    if((dir = opendir(strConfigLocation.c_str())) == NULL)
      return bReturn;

    while ((dirent = readdir(dir)) != NULL)
    {
      if(strcmp((char*)dirent->d_name, "." ) != 0 && strcmp((char*)dirent->d_name, ".." ) != 0)
      {
        strPort = StringUtils::Format("/dev/%s", dirent->d_name);
        if (!strPort.empty())
        {
          strLocation = strPort;
          bReturn = true;
          break;
        }
      }
    }
    closedir(dir);
  }

  return bReturn;
}
#endif

bool CUSBCECAdapterDetection::CanAutodetect(void)
{
#if defined(__APPLE__) || defined(HAVE_LIBUDEV) || defined(__WINDOWS__) || defined(__FreeBSD__)
  return true;
#else
  return false;
#endif
}

#if defined(__WINDOWS__)
static bool GetComPortFromDevNode(DEVINST hDevInst, char* strPortName, unsigned int iSize)
{
  bool bReturn(false);
  TCHAR strRegPortName[256];
  strRegPortName[0] = _T('\0');
  DWORD dwSize = sizeof(strRegPortName);
  DWORD dwType = 0;
  HKEY hDeviceKey;

  // open the device node key
  if (CM_Open_DevNode_Key(hDevInst, KEY_QUERY_VALUE, 0, RegDisposition_OpenExisting, &hDeviceKey, CM_REGISTRY_HARDWARE) != CR_SUCCESS)
  {
    printf("reg key not found\n");
    return bReturn;
  }

  // locate the PortName entry. TODO this one doesn't seem to be available in universal
  if ((RegQueryValueEx(hDeviceKey, _T("PortName"), NULL, &dwType, reinterpret_cast<LPBYTE>(strRegPortName), &dwSize) == ERROR_SUCCESS) &&
    (dwType == REG_SZ) &&
    _tcslen(strRegPortName) > 3 &&
    _tcsnicmp(strRegPortName, _T("COM"), 3) == 0 &&
    _ttoi(&(strRegPortName[3])) > 0)
  {
    // return the port name
    snprintf(strPortName, iSize, "%s", strRegPortName);
    bReturn = true;
  }

  // TODO this one doesn't seem to be available in universal
  RegCloseKey(hDeviceKey);

  return bReturn;
}

static bool GetPidVidFromDeviceName(const std::string strDevName, int* vid, int* pid)
{
  std::string strDevNameUpper(strDevName);
  StringUtils::ToUpper(strDevNameUpper);
  size_t iPidPos = strDevNameUpper.find("PID_");
  size_t iVidPos = strDevNameUpper.find("VID_");
  if (iPidPos == std::string::npos || iVidPos == std::string::npos || (strDevNameUpper.find("&MI_") != std::string::npos && strDevNameUpper.find("&MI_00") == std::string::npos))
    return false;

  std::string strVendorId(strDevNameUpper.substr(iVidPos + 4, 4));
  std::string strProductId(strDevNameUpper.substr(iPidPos + 4, 4));

  sscanf(strVendorId.c_str(), "%x", vid);
  sscanf(strProductId.c_str(), "%x", pid);

  return true;
}
#endif

uint8_t CUSBCECAdapterDetection::FindAdaptersWindows(cec_adapter_descriptor* deviceList, uint8_t iBufSize, const char* strDevicePath /* = NULL */)
{
  uint8_t iFound(0);

#if defined(__WINDOWS__)
  ULONG len;
  PCHAR buffer;

  CM_Get_Device_ID_List_Size(&len, 0, CM_GETIDLIST_FILTER_NONE);
  buffer = (PCHAR)malloc(sizeof(CHAR) * len);
  if (buffer)
  {
    CM_Get_Device_ID_List(0, buffer, len, CM_GETIDLIST_FILTER_NONE);

    for (CHAR* devId = buffer; *devId; devId += strlen(devId) + 1)
    {
      // check whether the path matches, if a path was given
      if (strDevicePath && strcmp(strDevicePath, devId) != 0)
        continue;

      // get the vid and pid
      int iVendor, iProduct;
      if (!GetPidVidFromDeviceName(devId, &iVendor, &iProduct))
        continue;

      // no match
      if (iVendor != CEC_VID || (iProduct != CEC_PID && iProduct != CEC_PID2))
        continue;

      // locate the device node
      DEVINST devInst = 0, childInst = 0;
      if (CM_Locate_DevNode(&devInst, devId, 0) != CR_SUCCESS)
        continue;

      // get the child node if this is a composite device
      if (iProduct == CEC_PID2)
      {
        if (CM_Get_Child(&childInst, devInst, 0) != CR_SUCCESS)
          continue;
        devInst = childInst;
      }

      // get the com port
      if (devInst != 0)
      {
        if (GetComPortFromDevNode(devInst, deviceList[iFound].strComName, sizeof(deviceList[iFound].strComName)))
        {
          snprintf(deviceList[iFound].strComPath, sizeof(deviceList[iFound].strComPath), "%s", devId);
          deviceList[iFound].iVendorId = (uint16_t)iVendor;
          deviceList[iFound].iProductId = (uint16_t)iProduct;
          deviceList[iFound].adapterType = ADAPTERTYPE_P8_EXTERNAL; // will be overridden when not doing a "quick scan" by the actual type
          iFound++;
        }
      }
    }

    free(buffer);
  }
#else
  (void)deviceList;
  (void)iBufSize;
  (void)strDevicePath;
#endif

  return iFound;
}

uint8_t CUSBCECAdapterDetection::FindAdaptersApple(cec_adapter_descriptor *deviceList, uint8_t iBufSize, const char *strDevicePath /* = NULL */)
{
  uint8_t iFound(0);

#if defined(__APPLE__)
  kern_return_t	kresult;
  char bsdPath[MAXPATHLEN] = { 0 };
  io_iterator_t	serialPortIterator;

  CFMutableDictionaryRef classesToMatch = IOServiceMatching(kIOSerialBSDServiceValue);
  if (classesToMatch)
  {
    CFDictionarySetValue(classesToMatch, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));
    kresult = IOServiceGetMatchingServices(kIOMasterPortDefault, classesToMatch, &serialPortIterator);
    if (kresult == KERN_SUCCESS)
    {
      io_object_t serialService;
      while ((serialService = IOIteratorNext(serialPortIterator)))
      {
        int iVendor = 0, iProduct = 0;
        CFTypeRef	bsdPathAsCFString;

        // fetch the device path.
        bsdPathAsCFString = IORegistryEntryCreateCFProperty(serialService,
          CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, 0);
        if (bsdPathAsCFString)
        {
          // convert the path from a CFString to a C (NUL-terminated) string.
          CFStringGetCString((CFStringRef)bsdPathAsCFString, bsdPath, MAXPATHLEN - 1, kCFStringEncodingUTF8);
          CFRelease(bsdPathAsCFString);

          // now walk up the hierarchy until we find the entry with vendor/product IDs
          io_registry_entry_t parent;
          CFTypeRef vendorIdAsCFNumber = NULL;
          CFTypeRef productIdAsCFNumber = NULL;
          kern_return_t kresult = IORegistryEntryGetParentEntry(serialService, kIOServicePlane, &parent);
          while (kresult == KERN_SUCCESS)
          {
            vendorIdAsCFNumber = IORegistryEntrySearchCFProperty(parent,
              kIOServicePlane, CFSTR(kUSBVendorID), kCFAllocatorDefault, 0);
            productIdAsCFNumber = IORegistryEntrySearchCFProperty(parent,
              kIOServicePlane, CFSTR(kUSBProductID), kCFAllocatorDefault, 0);
            if (vendorIdAsCFNumber && productIdAsCFNumber)
            {
              CFNumberGetValue((CFNumberRef)vendorIdAsCFNumber, kCFNumberIntType, &iVendor);
              CFRelease(vendorIdAsCFNumber);
              CFNumberGetValue((CFNumberRef)productIdAsCFNumber, kCFNumberIntType, &iProduct);
              CFRelease(productIdAsCFNumber);
              IOObjectRelease(parent);
              break;
            }
            io_registry_entry_t oldparent = parent;
            kresult = IORegistryEntryGetParentEntry(parent, kIOServicePlane, &parent);
            IOObjectRelease(oldparent);
          }
          if (strlen(bsdPath) && iVendor == CEC_VID && (iProduct == CEC_PID || iProduct == CEC_PID2))
          {
            if (!strDevicePath || !strcmp(bsdPath, strDevicePath))
            {
              // on darwin, the device path is the same as the comm path.
              if (iFound == 0 || strcmp(deviceList[iFound - 1].strComName, bsdPath))
              {
                snprintf(deviceList[iFound].strComPath, sizeof(deviceList[iFound].strComPath), "%s", bsdPath);
                snprintf(deviceList[iFound].strComName, sizeof(deviceList[iFound].strComName), "%s", bsdPath);
                deviceList[iFound].iVendorId = iVendor;
                deviceList[iFound].iProductId = iProduct;
                deviceList[iFound].adapterType = ADAPTERTYPE_P8_EXTERNAL; // will be overridden when not doing a "quick scan" by the actual type
                iFound++;
              }
            }
          }
        }
        IOObjectRelease(serialService);
      }
    }
    IOObjectRelease(serialPortIterator);
  }
#else
  (void)deviceList;
  (void)iBufSize;
  (void)strDevicePath;
#endif
  return iFound;
}

uint8_t CUSBCECAdapterDetection::FindAdaptersUdev(cec_adapter_descriptor *deviceList, uint8_t iBufSize, const char *strDevicePath /* = NULL */)
{
  uint8_t iFound(0);

#if defined(HAVE_LIBUDEV)
  struct udev *udev;
  if (!(udev = udev_new()))
    return -1;

  struct udev_enumerate *enumerate;
  struct udev_list_entry *devices, *dev_list_entry;
  struct udev_device *dev, *pdev;
  enumerate = udev_enumerate_new(udev);
  udev_enumerate_scan_devices(enumerate);
  devices = udev_enumerate_get_list_entry(enumerate);
  udev_list_entry_foreach(dev_list_entry, devices)
  {
    const char *strPath;
    strPath = udev_list_entry_get_name(dev_list_entry);

    dev = udev_device_new_from_syspath(udev, strPath);
    if (!dev)
      continue;

    pdev = udev_device_get_parent(udev_device_get_parent(dev));
    if (!pdev || !udev_device_get_sysattr_value(pdev, "idVendor") || !udev_device_get_sysattr_value(pdev, "idProduct"))
    {
      udev_device_unref(dev);
      continue;
    }

    int iVendor, iProduct;
    sscanf(udev_device_get_sysattr_value(pdev, "idVendor"), "%x", &iVendor);
    sscanf(udev_device_get_sysattr_value(pdev, "idProduct"), "%x", &iProduct);
    if (iVendor == CEC_VID && (iProduct == CEC_PID || iProduct == CEC_PID2))
    {
      std::string strPath(udev_device_get_syspath(pdev));
      if (!strDevicePath || !strcmp(strPath.c_str(), strDevicePath))
      {
        std::string strComm(strPath);
        if (FindComPort(strComm) && (iFound == 0 || strcmp(deviceList[iFound - 1].strComName, strComm.c_str())))
        {
          snprintf(deviceList[iFound].strComPath, sizeof(deviceList[iFound].strComPath), "%s", strPath.c_str());
          snprintf(deviceList[iFound].strComName, sizeof(deviceList[iFound].strComName), "%s", strComm.c_str());
          deviceList[iFound].iVendorId = iVendor;
          deviceList[iFound].iProductId = iProduct;
          deviceList[iFound].adapterType = ADAPTERTYPE_P8_EXTERNAL; // will be overridden when not doing a "quick scan" by the actual type
          iFound++;
        }
      }
    }
    udev_device_unref(dev);

    if (iFound >= iBufSize)
      break;
  }

  udev_enumerate_unref(enumerate);
  udev_unref(udev);
#else
  (void)deviceList;
  (void)iBufSize;
  (void)strDevicePath;
#endif

  return iFound;
}

uint8_t CUSBCECAdapterDetection::FindAdaptersFreeBSD(cec_adapter_descriptor *deviceList, uint8_t iBufSize, const char *strDevicePath /* = NULL */)
{
  uint8_t iFound(0);

#if defined(__FreeBSD__)
  char devicePath[PATH_MAX + 1];
  char infos[512];
  char sysctlname[32];
  char ttyname[8];
  char *pos;
  size_t infos_size = sizeof(infos);
  int i;

  for (i = 0;; ++i)
  {
    unsigned int iVendor, iProduct;
    memset(infos, 0, sizeof(infos));
    (void)snprintf(sysctlname, sizeof(sysctlname),
      "dev.umodem.%d.%%pnpinfo", i);
    if (sysctlbyname(sysctlname, infos, &infos_size,
      NULL, 0) != 0)
      break;
    pos = strstr(infos, "vendor=");
    if (pos == NULL)
      continue;
    sscanf(pos, "vendor=%x ", &iVendor);

    pos = strstr(infos, "product=");
    if (pos == NULL)
      continue;
    sscanf(pos, "product=%x ", &iProduct);

    if (iVendor != CEC_VID || (iProduct != CEC_PID && iProduct != CEC_PID2))
      continue;

    pos = strstr(infos, "ttyname=");
    if (pos == NULL)
      continue;
    sscanf(pos, "ttyname=%s ", ttyname);

    (void)snprintf(devicePath, sizeof(devicePath),
      "/dev/tty%s", ttyname);

    if (strDevicePath) {
      char currStrDevicePath[512];
      int port = 0;
      int devaddr = 0;
      memset(currStrDevicePath, 0, sizeof(currStrDevicePath));
      memset(infos, 0, sizeof(infos));
      (void)snprintf(sysctlname, sizeof(sysctlname),
        "dev.umodem.%d.%%location", i);
      if (sysctlbyname(sysctlname, infos, &infos_size,
        NULL, 0) != 0)
        break;

      pos = strstr(infos, "port=");
      if (pos == NULL)
        continue;
      sscanf(pos, "port=%d ", &port);

      pos = strstr(infos, "devaddr=");
      if (pos == NULL)
        continue;
      sscanf(pos, "devaddr=%d ", &devaddr);

      (void)snprintf(currStrDevicePath, sizeof(currStrDevicePath),
        "/dev/ugen%d.%d", port, devaddr);

      if (strcmp(currStrDevicePath, strDevicePath) != 0)
        continue;
    }
    snprintf(deviceList[iFound].strComPath, sizeof(deviceList[iFound].strComPath), "%s", devicePath);
    snprintf(deviceList[iFound].strComName, sizeof(deviceList[iFound].strComName), "%s", devicePath);
    deviceList[iFound].iVendorId = iVendor;
    deviceList[iFound].iProductId = iProduct;
    deviceList[iFound].adapterType = ADAPTERTYPE_P8_EXTERNAL; // will be overridden when not doing a "quick scan" by the actual type
    iFound++;
  }
#else
  (void)deviceList;
  (void)iBufSize;
  (void)strDevicePath;
#endif

  return iFound;
}

uint8_t CUSBCECAdapterDetection::FindAdapters(cec_adapter_descriptor *deviceList, uint8_t iBufSize, const char *strDevicePath /* = NULL */)
{
  uint8_t iFound(0);
  iFound = FindAdaptersApple(deviceList, iBufSize, strDevicePath);
  if (iFound == 0)
    iFound = FindAdaptersFreeBSD(deviceList, iBufSize, strDevicePath);
  if (iFound == 0)
    iFound = FindAdaptersUdev(deviceList, iBufSize, strDevicePath);
  if (iFound == 0)
    iFound = FindAdaptersWindows(deviceList, iBufSize, strDevicePath);
  return iFound;
}
