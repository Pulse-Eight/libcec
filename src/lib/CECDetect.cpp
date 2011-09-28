/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011 Pulse-Eight Limited.  All rights reserved.
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

#include "CECDetect.h"
#include "libPlatform/os-dependent.h"
#include "util/StdString.h"
#if !defined(__WINDOWS__)
#include <dirent.h>
#include <libudev.h>
#include <poll.h>
#endif
#include <string.h>

#define CEC_VID 0x2548
#define CEC_PID 0x1001

using namespace CEC;
using namespace std;

#if !defined(__WINDOWS__)
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

int CCECDetect::FindDevices(vector<cec_device> &deviceList, const char *strDevicePath /* = NULL */)
{
  int iFound(0);

#if !defined(__WINDOWS__)
  struct udev *udev;
  if (!(udev = udev_new()))
    return -1;

  struct udev_enumerate *enumerate;
  struct udev_list_entry *devices, *dev_list_entry;
  struct udev_device *dev;
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

    dev = udev_device_get_parent(udev_device_get_parent(dev));
    if (!dev)
      continue;
    if (!udev_device_get_sysattr_value(dev,"idVendor") || !udev_device_get_sysattr_value(dev, "idProduct"))
    {
      udev_device_unref(dev);
      continue;
    }

    int iVendor, iProduct;
    sscanf(udev_device_get_sysattr_value(dev, "idVendor"), "%x", &iVendor);
    sscanf(udev_device_get_sysattr_value(dev, "idProduct"), "%x", &iProduct);
    if (iVendor == CEC_VID && iProduct == CEC_PID)
    {
      CStdString strPath(udev_device_get_syspath(dev));
      if (strDevicePath && strcmp(strPath.c_str(), strDevicePath))
        continue;

      CStdString strComm(strPath);
      if (FindComPort(strComm))
      {
        cec_device foundDev;
        foundDev.path = strPath;
        foundDev.comm = strComm;
        deviceList.push_back(foundDev);
        ++iFound;
      }
    }
    udev_device_unref(dev);
  }

  udev_enumerate_unref(enumerate);
  udev_unref(udev);
#endif

  return iFound;
}
