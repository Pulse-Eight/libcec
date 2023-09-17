/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC Linux CEC Adapter is Copyright (C) 2017 Jonas Karlman
 * based heavily on:
 * libCEC AOCEC Code is Copyright (C) 2016 Gerald Dachs
 * libCEC Exynos Code is Copyright (C) 2014 Valentin Manea
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

#if defined(HAVE_LINUX_API)
#include "LinuxCECAdapterDetection.h"

#include <dirent.h>
#include "p8-platform/util/StringUtils.h"

#if defined(HAVE_LIBUDEV)
extern "C" {
#include <libudev.h>
}
#endif

using namespace CEC;

bool CLinuxCECAdapterDetection::IsAdapter(const char *strPort)
{
  return !strncmp(strPort, "/dev/cec", 8);
}

uint8_t CLinuxCECAdapterDetection::FindAdaptersUdev(cec_adapter_descriptor *deviceList, uint8_t iBufSize, const char *strDevicePath /* = NULL */)
{
  uint8_t iFound(0);

#if defined(HAVE_LIBUDEV)
  struct udev *udev;
  if (!(udev = udev_new()))
    return 0;

  struct udev_enumerate *enumerate;
  struct udev_list_entry *devices, *dev_list_entry;
  struct udev_device *dev;
  enumerate = udev_enumerate_new(udev);

  udev_enumerate_add_match_subsystem(enumerate, "cec");
  udev_enumerate_scan_devices(enumerate);
  devices = udev_enumerate_get_list_entry(enumerate);
  udev_list_entry_foreach(dev_list_entry, devices)
  {
    const char *strPath;
    strPath = udev_list_entry_get_name(dev_list_entry);

    dev = udev_device_new_from_syspath(udev, strPath);
    if (!dev)
      continue;

    const char *strPort;
    strPort = udev_device_get_devnode(dev);

    if (!strDevicePath || !strcmp(strPath, strDevicePath))
    {
      snprintf(deviceList[iFound].strComPath, sizeof(deviceList[iFound].strComPath), "%s", strPath);
      snprintf(deviceList[iFound].strComName, sizeof(deviceList[iFound].strComName), "%s", strPort);
      deviceList[iFound].iVendorId = 0;
      deviceList[iFound].iProductId = 0;
      deviceList[iFound].adapterType = ADAPTERTYPE_LINUX;
      iFound++;
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

uint8_t CLinuxCECAdapterDetection::FindAdaptersLinux(cec_adapter_descriptor *deviceList, uint8_t iBufSize, const char *strDevicePath /* = NULL */)
{
  uint8_t iFound(0);

  std::string strSysfsPath("/sys/bus/cec/devices");
  DIR *dir;

  if ((dir = opendir(strSysfsPath.c_str())) != NULL)
  {
    struct dirent *dent;

    while ((dent = readdir(dir)) != NULL)
    {
      std::string strDevice = StringUtils::Format("%s/%s", strSysfsPath.c_str(), dent->d_name);

      if (strncmp(dent->d_name, "cec", 3))
        continue;

      if (strDevicePath && strcmp(strDevice.c_str(), strDevicePath))
        continue;

      snprintf(deviceList[iFound].strComPath, sizeof(deviceList[iFound].strComPath), "%s", strDevice.c_str());
      snprintf(deviceList[iFound].strComName, sizeof(deviceList[iFound].strComName), "/dev/%s", dent->d_name);
      deviceList[iFound].iVendorId = 0;
      deviceList[iFound].iProductId = 0;
      deviceList[iFound].adapterType = ADAPTERTYPE_LINUX;
      iFound++;

      if (iFound >= iBufSize)
        break;
    }

    closedir(dir);
  }

  return iFound;
}

uint8_t CLinuxCECAdapterDetection::FindAdapters(cec_adapter_descriptor *deviceList, uint8_t iBufSize, const char *strDevicePath /* = NULL */)
{
  uint8_t iFound(0);
  iFound = FindAdaptersUdev(deviceList, iBufSize, strDevicePath);
  if (iFound == 0)
    iFound = FindAdaptersLinux(deviceList, iBufSize, strDevicePath);
  return iFound;
}

#endif
