/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2012 Pulse-Eight Limited.  All rights reserved.
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

#include "env.h"
#include "USBCECAdapterDetection.h"
#include "lib/platform/util/StdString.h"

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
#include <setupapi.h>

// the virtual COM port only shows up when requesting devices with the raw device guid!
static GUID USB_RAW_GUID =  { 0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } };
#elif defined(HAVE_LIBUDEV)
#include <dirent.h>
#include <poll.h>
extern "C" {
#include <libudev.h>
}
#elif defined(__FreeBSD__)
#include <stdio.h>
#include <unistd.h>
#endif

#define CEC_VID 0x2548
#define CEC_PID 0x1001

using namespace CEC;
using namespace std;

#if defined(HAVE_LIBUDEV)
bool TranslateComPort(CStdString &strString)
{
  CStdString strTmp(strString);
  strTmp.MakeReverse();
  int iSlash = strTmp.Find('/');
  if (iSlash >= 0)
  {
    strTmp = strTmp.Left(iSlash);
    strTmp.MakeReverse();
    strString.Format("%s/%s:1.0/tty", strString.c_str(), strTmp.c_str());
    return true;
  }

  return false;
}

bool FindComPort(CStdString &strLocation)
{
  CStdString strPort = strLocation;
  bool bReturn(!strPort.IsEmpty());
  CStdString strConfigLocation(strLocation);
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
        strPort.Format("/dev/%s", dirent->d_name);
        if (!strPort.IsEmpty())
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

uint8_t CUSBCECAdapterDetection::FindAdapters(cec_adapter *deviceList, uint8_t iBufSize, const char *strDevicePath /* = NULL */)
{
  uint8_t iFound(0);

#if defined(__APPLE__)
  kern_return_t	kresult;
  char bsdPath[MAXPATHLEN] = {0};
  io_iterator_t	serialPortIterator;

  CFMutableDictionaryRef classesToMatch = IOServiceMatching(kIOSerialBSDServiceValue);
  if (classesToMatch)
  {
    CFDictionarySetValue(classesToMatch, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDModemType));
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
          CFTypeRef vendorIdAsCFNumber  = NULL;
          CFTypeRef productIdAsCFNumber = NULL;
          kern_return_t kresult = IORegistryEntryGetParentEntry(serialService, kIOServicePlane, &parent);
          while (kresult == KERN_SUCCESS)
          {
            vendorIdAsCFNumber  = IORegistryEntrySearchCFProperty(parent,
              kIOServicePlane, CFSTR(kUSBVendorID),  kCFAllocatorDefault, 0);
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
          if (strlen(bsdPath) && iVendor == CEC_VID && iProduct == CEC_PID)
          {
            if (!strDevicePath || !strcmp(bsdPath, strDevicePath))
            {
              // on darwin, the device path is the same as the comm path.
              snprintf(deviceList[iFound  ].path, sizeof(deviceList[iFound].path), "%s", bsdPath);
              snprintf(deviceList[iFound++].comm, sizeof(deviceList[iFound].path), "%s", bsdPath);
            }
          }
        }
	      IOObjectRelease(serialService);
      }
    }
    IOObjectRelease(serialPortIterator);
  }
#elif defined(HAVE_LIBUDEV)
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
    if (!pdev || !udev_device_get_sysattr_value(pdev,"idVendor") || !udev_device_get_sysattr_value(pdev, "idProduct"))
    {
      udev_device_unref(dev);
      continue;
    }

    int iVendor, iProduct;
    sscanf(udev_device_get_sysattr_value(pdev, "idVendor"), "%x", &iVendor);
    sscanf(udev_device_get_sysattr_value(pdev, "idProduct"), "%x", &iProduct);
    if (iVendor == CEC_VID && iProduct == CEC_PID)
    {
      CStdString strPath(udev_device_get_syspath(pdev));
      if (!strDevicePath || !strcmp(strPath.c_str(), strDevicePath))
      {
        CStdString strComm(strPath);
        if (FindComPort(strComm))
        {
          snprintf(deviceList[iFound  ].path, sizeof(deviceList[iFound].path), "%s", strPath.c_str());
          snprintf(deviceList[iFound++].comm, sizeof(deviceList[iFound].path), "%s", strComm.c_str());
        }
      }
    }
    udev_device_unref(dev);
  }

  udev_enumerate_unref(enumerate);
  udev_unref(udev);
#elif defined(__WINDOWS__)
  HDEVINFO hDevHandle;
  DWORD    required = 0, iMemberIndex = 0;
  int      nBufferSize = 0;

  SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
  deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

  SP_DEVINFO_DATA devInfoData;
  devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

  if ((hDevHandle = SetupDiGetClassDevs(&USB_RAW_GUID, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE)) == INVALID_HANDLE_VALUE)
    return iFound;

  BOOL bResult = true;
  TCHAR *buffer = NULL;
  PSP_DEVICE_INTERFACE_DETAIL_DATA devicedetailData;
  while(bResult && iFound < iBufSize)
  {
    bResult = SetupDiEnumDeviceInfo(hDevHandle, iMemberIndex, &devInfoData);

    if (bResult)
      bResult = SetupDiEnumDeviceInterfaces(hDevHandle, 0, &USB_RAW_GUID, iMemberIndex, &deviceInterfaceData);

    if(!bResult)
    {
      SetupDiDestroyDeviceInfoList(hDevHandle);
      delete []buffer;
      buffer = NULL;
      return iFound;
    }

    iMemberIndex++;
    BOOL bDetailResult = false;
    {
      // As per MSDN, Get the required buffer size. Call SetupDiGetDeviceInterfaceDetail with a 
      // NULL DeviceInterfaceDetailData pointer, a DeviceInterfaceDetailDataSize of zero, 
      // and a valid RequiredSize variable. In response to such a call, this function returns 
      // the required buffer size at RequiredSize and fails with GetLastError returning 
      // ERROR_INSUFFICIENT_BUFFER. 
      // Allocate an appropriately sized buffer and call the function again to get the interface details. 

      SetupDiGetDeviceInterfaceDetail(hDevHandle, &deviceInterfaceData, NULL, 0, &required, NULL);

      buffer = new TCHAR[required];
      devicedetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) buffer;
      devicedetailData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
      nBufferSize = required;
    }

    bDetailResult = SetupDiGetDeviceInterfaceDetail(hDevHandle, &deviceInterfaceData, devicedetailData, nBufferSize , &required, NULL);
    if(!bDetailResult)
      continue;

    if (strDevicePath && strcmp(strDevicePath, devicedetailData->DevicePath) != 0)
      continue;

    CStdString strVendorId;
    CStdString strProductId;
    CStdString strTmp(devicedetailData->DevicePath);
    strVendorId.assign(strTmp.substr(strTmp.Find("vid_") + 4, 4));
    strProductId.assign(strTmp.substr(strTmp.Find("pid_") + 4, 4));
    if (strTmp.Find("&mi_") >= 0 && strTmp.Find("&mi_00") < 0)
      continue;

    int iVendor, iProduct;
    sscanf(strVendorId, "%x", &iVendor);
    sscanf(strProductId, "%x", &iProduct);
    if (iVendor != CEC_VID || iProduct != CEC_PID)
      continue;

    HKEY hDeviceKey = SetupDiOpenDevRegKey(hDevHandle, &devInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
    if (!hDeviceKey)
      continue;

    TCHAR strPortName[256];
    strPortName[0] = _T('\0');
    DWORD dwSize = sizeof(strPortName);
    DWORD dwType = 0;

    /* search the registry */
    if ((RegQueryValueEx(hDeviceKey, _T("PortName"), NULL, &dwType, reinterpret_cast<LPBYTE>(strPortName), &dwSize) == ERROR_SUCCESS) && (dwType == REG_SZ))
    {
      if (_tcslen(strPortName) > 3 && _tcsnicmp(strPortName, _T("COM"), 3) == 0 &&
        _ttoi(&(strPortName[3])) > 0)
      {
        snprintf(deviceList[iFound  ].path, sizeof(deviceList[iFound].path), "%s", devicedetailData->DevicePath);
        snprintf(deviceList[iFound++].comm, sizeof(deviceList[iFound].path), "%s", strPortName);
      }
    }

    RegCloseKey(hDeviceKey);
  }
#elif defined(__FreeBSD__)
  char devicePath[PATH_MAX + 1];
  int i;

  for (i = 0; i < 8; ++i)
  {
    (void)snprintf(devicePath, sizeof(devicePath), "/dev/ttyU%d", i);
    if (!access(devicePath, 0))
    {
      snprintf(deviceList[iFound  ].path, sizeof(deviceList[iFound].path), "%s", devicePath);
      snprintf(deviceList[iFound++].comm, sizeof(deviceList[iFound].path), "%s", devicePath);
    }
  }
#else
  //silence "unused" warnings
  void *tmp = (void*)deviceList;
  tmp = (void *)strDevicePath;
#endif

  iBufSize = 0; /* silence "unused" warning on linux/osx */

  return iFound;
}
