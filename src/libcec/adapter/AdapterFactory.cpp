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
#include "AdapterFactory.h"

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "LibCEC.h"
#include "CECProcessor.h"

#if defined(HAVE_P8_USB)
#include "Pulse-Eight/USBCECAdapterDetection.h"
#include "Pulse-Eight/USBCECAdapterCommunication.h"
#endif

#if defined(HAVE_RPI_API)
#include "RPi/RPiCECAdapterDetection.h"
#include "RPi/RPiCECAdapterCommunication.h"
#endif

#if defined(HAVE_TDA995X_API)
#include "TDA995x/TDA995xCECAdapterDetection.h"
#include "TDA995x/TDA995xCECAdapterCommunication.h"
#endif

#if defined(HAVE_EXYNOS_API)
#include "Exynos/ExynosCECAdapterDetection.h"
#include "Exynos/ExynosCECAdapterCommunication.h"
#endif

#if defined(HAVE_LINUX_API)
#include "Linux/LinuxCECAdapterDetection.h"
#include "Linux/LinuxCECAdapterCommunication.h"
#endif

#if defined(HAVE_AOCEC_API)
#include "AOCEC/AOCECAdapterDetection.h"
#include "AOCEC/AOCECAdapterCommunication.h"
#endif

#if defined(HAVE_IMX_API)
#include "IMX/IMXCECAdapterDetection.h"
#include "IMX/IMXCECAdapterCommunication.h"
#endif

#if defined(HAVE_TEGRA_API)
#include "Tegra/TegraCECAdapterDetection.h"
#include "Tegra/TegraCECAdapterCommunication.h"
#include "Tegra/TegraCECDev.h"
#endif

using namespace CEC;

namespace
{
  // Base display name for an adapter type. USB adapters are external dongles;
  // every other supported back-end is an HDMI CEC output on the host itself.
  const char *AdapterBaseName(cec_adapter_type type)
  {
    switch (type)
    {
      case ADAPTERTYPE_P8_EXTERNAL:
      case ADAPTERTYPE_P8_DAUGHTERBOARD:
        return "USB-CEC Adapter";
      default:
        return "HDMI";
    }
  }

  // Give every detected adapter a human-readable display name. Adapters are
  // grouped by type: a single adapter of a type keeps the bare base name
  // ("HDMI", "USB-CEC Adapter"), while multiple adapters of the same type are
  // numbered ("HDMI 1", "HDMI 2", ...). Numbering follows strComPath order - the
  // USB-tree location / device-node path - which is stable regardless of the
  // order in which /dev/ttyACM* or /dev/cec* nodes happen to be enumerated.
  void AssignAdapterNames(cec_adapter_descriptor *deviceList, uint8_t count)
  {
    std::map<cec_adapter_type, std::vector<uint8_t> > byType;
    for (uint8_t iPtr = 0; iPtr < count; iPtr++)
      byType[deviceList[iPtr].adapterType].push_back(iPtr);

    for (std::map<cec_adapter_type, std::vector<uint8_t> >::iterator it = byType.begin(); it != byType.end(); ++it)
    {
      std::vector<uint8_t> &indices = it->second;
      const char *strBase = AdapterBaseName(it->first);

      if (indices.size() == 1)
      {
        snprintf(deviceList[indices[0]].strDeviceName, sizeof(deviceList[indices[0]].strDeviceName), "%s", strBase);
        continue;
      }

      std::sort(indices.begin(), indices.end(), [deviceList](uint8_t a, uint8_t b) {
        return strcmp(deviceList[a].strComPath, deviceList[b].strComPath) < 0;
      });

      for (size_t iPos = 0; iPos < indices.size(); iPos++)
        snprintf(deviceList[indices[iPos]].strDeviceName, sizeof(deviceList[indices[iPos]].strDeviceName),
                 "%s %u", strBase, (unsigned int)(iPos + 1));
    }
  }
}

int8_t CAdapterFactory::FindAdapters(cec_adapter *deviceList, uint8_t iBufSize, const char *strDevicePath /* = NULL */)
{
  cec_adapter_descriptor devices[50];
  int8_t iReturn = DetectAdapters(devices, iBufSize, strDevicePath);
  for (int8_t iPtr = 0; iPtr < iReturn && iPtr < iBufSize; iPtr++)
  {
    // strncpy() here makes gcc warn that it may truncate without terminating,
    // since it can't tell the source is bounded. copy the measured length and
    // zero the remainder by hand, which keeps strncpy's zero-fill semantics
    size_t iCommLen = strnlen(devices[iPtr].strComName, sizeof(deviceList[iPtr].comm) - 1);
    memcpy(deviceList[iPtr].comm, devices[iPtr].strComName, iCommLen);
    memset(deviceList[iPtr].comm + iCommLen, 0, sizeof(deviceList[iPtr].comm) - iCommLen);

    size_t iPathLen = strnlen(devices[iPtr].strComPath, sizeof(deviceList[iPtr].path) - 1);
    memcpy(deviceList[iPtr].path, devices[iPtr].strComPath, iPathLen);
    memset(deviceList[iPtr].path + iPathLen, 0, sizeof(deviceList[iPtr].path) - iPathLen);
  }
  return iReturn;
}

int8_t CAdapterFactory::DetectAdapters(cec_adapter_descriptor *deviceList, uint8_t iBufSize, const char *strDevicePath /* = NULL */)
{
  int8_t iAdaptersFound(0);

#if defined(HAVE_P8_USB)
  if (!CUSBCECAdapterDetection::CanAutodetect())
  {
    if (m_lib)
      m_lib->AddLog(CEC_LOG_WARNING, "libCEC has not been compiled with detection code for the Pulse-Eight USB-CEC Adapter, so the path to the COM port has to be provided to libCEC if this adapter is being used");
  }
  else
    iAdaptersFound += CUSBCECAdapterDetection::FindAdapters(deviceList, iBufSize, strDevicePath);
#else
  m_lib->AddLog(CEC_LOG_WARNING, "libCEC has not been compiled with support for the Pulse-Eight USB-CEC Adapter");
#endif

#if defined(HAVE_RPI_API)
  if (iAdaptersFound < iBufSize && CRPiCECAdapterDetection::FindAdapter() &&
      (!strDevicePath || !strcmp(strDevicePath, CEC_RPI_VIRTUAL_COM)))
  {
    memset(&deviceList[iAdaptersFound], 0, sizeof(cec_adapter_descriptor));
    snprintf(deviceList[iAdaptersFound].strComPath, sizeof(deviceList[iAdaptersFound].strComPath), CEC_RPI_VIRTUAL_PATH);
    snprintf(deviceList[iAdaptersFound].strComName, sizeof(deviceList[iAdaptersFound].strComName), CEC_RPI_VIRTUAL_COM);
    deviceList[iAdaptersFound].iVendorId = RPI_ADAPTER_VID;
    deviceList[iAdaptersFound].iProductId = RPI_ADAPTER_PID;
    deviceList[iAdaptersFound].adapterType = ADAPTERTYPE_RPI;
    iAdaptersFound++;
  }
#endif

#if defined(HAVE_TDA995X_API)
  if (iAdaptersFound < iBufSize && CTDA995xCECAdapterDetection::FindAdapter() &&
      (!strDevicePath || !strcmp(strDevicePath, CEC_TDA995x_VIRTUAL_COM)))
  {
    memset(&deviceList[iAdaptersFound], 0, sizeof(cec_adapter_descriptor));
    snprintf(deviceList[iAdaptersFound].strComPath, sizeof(deviceList[iAdaptersFound].strComPath), CEC_TDA995x_PATH);
    snprintf(deviceList[iAdaptersFound].strComName, sizeof(deviceList[iAdaptersFound].strComName), CEC_TDA995x_VIRTUAL_COM);
    deviceList[iAdaptersFound].iVendorId = TDA995X_ADAPTER_VID;
    deviceList[iAdaptersFound].iProductId = TDA995X_ADAPTER_PID;
    deviceList[iAdaptersFound].adapterType = ADAPTERTYPE_TDA995x;
    iAdaptersFound++;
  }
#endif

#if defined(HAVE_EXYNOS_API)
  if (iAdaptersFound < iBufSize && CExynosCECAdapterDetection::FindAdapter())
  {
    memset(&deviceList[iAdaptersFound], 0, sizeof(cec_adapter_descriptor));
    snprintf(deviceList[iAdaptersFound].strComPath, sizeof(deviceList[iAdaptersFound].strComPath), CEC_EXYNOS_PATH);
    snprintf(deviceList[iAdaptersFound].strComName, sizeof(deviceList[iAdaptersFound].strComName), CEC_EXYNOS_VIRTUAL_COM);
    deviceList[iAdaptersFound].iVendorId = 0;
    deviceList[iAdaptersFound].iProductId = 0;
    deviceList[iAdaptersFound].adapterType = ADAPTERTYPE_EXYNOS;
    iAdaptersFound++;
  }
#endif

#if defined(HAVE_LINUX_API)
  {
    // Enumerate every capable /dev/cec* node as a separate adapter so a board
    // with more than one HDMI port (e.g. a Raspberry Pi with /dev/cec0 and
    // /dev/cec1) can be addressed per-port. The legacy "Linux" name still
    // matches all of them so existing callers keep working.
    std::vector<std::string> paths;
    CLinuxCECAdapterDetection::FindAdapters(paths);
    for (size_t iPath = 0; iPath < paths.size() && iAdaptersFound < iBufSize; iPath++)
    {
      const std::string &strPath = paths[iPath];
      if (strDevicePath && strcmp(strDevicePath, strPath.c_str()) && strcmp(strDevicePath, CEC_LINUX_VIRTUAL_COM))
        continue;

      memset(&deviceList[iAdaptersFound], 0, sizeof(cec_adapter_descriptor));
      snprintf(deviceList[iAdaptersFound].strComPath, sizeof(deviceList[iAdaptersFound].strComPath), "%s", strPath.c_str());
      snprintf(deviceList[iAdaptersFound].strComName, sizeof(deviceList[iAdaptersFound].strComName), "%s", strPath.c_str());
      deviceList[iAdaptersFound].iVendorId = 0;
      deviceList[iAdaptersFound].iProductId = 0;
      deviceList[iAdaptersFound].adapterType = ADAPTERTYPE_LINUX;
      iAdaptersFound++;
    }
  }
#endif

#if defined(HAVE_AOCEC_API)
  if (iAdaptersFound < iBufSize && CAOCECAdapterDetection::FindAdapter())
  {
    memset(&deviceList[iAdaptersFound], 0, sizeof(cec_adapter_descriptor));
    snprintf(deviceList[iAdaptersFound].strComPath, sizeof(deviceList[iAdaptersFound].strComPath), CEC_AOCEC_PATH);
    snprintf(deviceList[iAdaptersFound].strComName, sizeof(deviceList[iAdaptersFound].strComName), CEC_AOCEC_VIRTUAL_COM);
    deviceList[iAdaptersFound].iVendorId = 0;
    deviceList[iAdaptersFound].iProductId = 0;
    deviceList[iAdaptersFound].adapterType = ADAPTERTYPE_AOCEC;
    iAdaptersFound++;
  }
#endif

#if defined(HAVE_IMX_API)
  if (iAdaptersFound < iBufSize && CIMXCECAdapterDetection::FindAdapter() &&
      (!strDevicePath || !strcmp(strDevicePath, CEC_IMX_VIRTUAL_COM)))
  {
    memset(&deviceList[iAdaptersFound], 0, sizeof(cec_adapter_descriptor));
    snprintf(deviceList[iAdaptersFound].strComPath, sizeof(deviceList[iAdaptersFound].strComPath), CEC_IMX_PATH);
    snprintf(deviceList[iAdaptersFound].strComName, sizeof(deviceList[iAdaptersFound].strComName), CEC_IMX_VIRTUAL_COM);
    deviceList[iAdaptersFound].iVendorId = IMX_ADAPTER_VID;
    deviceList[iAdaptersFound].iProductId = IMX_ADAPTER_PID;
    deviceList[iAdaptersFound].adapterType = ADAPTERTYPE_IMX;
    iAdaptersFound++;
  }
#endif

#if defined(HAVE_TEGRA_API)
  if (iAdaptersFound < iBufSize && TegraCECAdapterDetection::FindAdapter() &&
      (!strDevicePath || !strcmp(strDevicePath, CEC_TDA995x_VIRTUAL_COM)))
  {
    memset(&deviceList[iAdaptersFound], 0, sizeof(cec_adapter_descriptor));
    snprintf(deviceList[iAdaptersFound].strComPath, sizeof(deviceList[iAdaptersFound].strComPath), TEGRA_CEC_DEV_PATH);
    snprintf(deviceList[iAdaptersFound].strComName, sizeof(deviceList[iAdaptersFound].strComName), TEGRA_CEC_DEV_PATH);
    deviceList[iAdaptersFound].iVendorId = TEGRA_ADAPTER_VID;
    deviceList[iAdaptersFound].iProductId = TEGRA_ADAPTER_PID;
    deviceList[iAdaptersFound].adapterType = ADAPTERTYPE_TEGRA;
    iAdaptersFound++;
  }
#endif

// #if defined(HAVE_TEGRA_API)
//   iAdaptersFound++;
// #endif

#if !defined(HAVE_RPI_API) && !defined(HAVE_P8_USB) && !defined(HAVE_TDA995X_API) && !defined(HAVE_EXYNOS_API) && !defined(HAVE_LINUX_API) && !defined(HAVE_AOCEC_API) && !defined(HAVE_IMX_API) && !defined(HAVE_TEGRA_API)
#error "libCEC doesn't have support for any type of adapter. please check your build system or configuration"
#endif

  if (iAdaptersFound > 0)
    AssignAdapterNames(deviceList, (uint8_t)iAdaptersFound);

  return iAdaptersFound;
}

IAdapterCommunication *CAdapterFactory::GetInstance(const char *strPort, uint16_t iBaudRate)
{
#if defined(HAVE_TDA995X_API)
  if (!strcmp(strPort, CEC_TDA995x_VIRTUAL_COM))
    return new CTDA995xCECAdapterCommunication(m_lib->m_cec);
#endif

#if defined(HAVE_TEGRA_API)
  if (!strcmp(strPort, TEGRA_CEC_DEV_PATH))
    return new TegraCECAdapterCommunication(m_lib->m_cec);
#endif

#if defined(HAVE_EXYNOS_API)
  if (!strcmp(strPort, CEC_EXYNOS_VIRTUAL_COM))
    return new CExynosCECAdapterCommunication(m_lib->m_cec);
#endif

#if defined(HAVE_LINUX_API)
  if (!strcmp(strPort, CEC_LINUX_VIRTUAL_COM))
    return new CLinuxCECAdapterCommunication(m_lib->m_cec);
  if (!strncmp(strPort, CEC_LINUX_PATH_PREFIX, strlen(CEC_LINUX_PATH_PREFIX)))
    return new CLinuxCECAdapterCommunication(m_lib->m_cec, strPort);
#endif

#if defined(HAVE_AOCEC_API)
  if (!strcmp(strPort, CEC_AOCEC_VIRTUAL_COM))
    return new CAOCECAdapterCommunication(m_lib->m_cec);
#endif

#if defined(HAVE_RPI_API)
  if (!strcmp(strPort, CEC_RPI_VIRTUAL_COM))
    return new CRPiCECAdapterCommunication(m_lib->m_cec);
#endif

#if defined(HAVE_IMX_API)
  if (!strcmp(strPort, CEC_IMX_VIRTUAL_COM))
    return new CIMXCECAdapterCommunication(m_lib->m_cec);
#endif

#if defined(HAVE_P8_USB)
  return new CUSBCECAdapterCommunication(m_lib->m_cec, strPort, iBaudRate);
#endif

#if !defined(HAVE_RPI_API) && !defined(HAVE_P8_USB) && !defined(HAVE_TDA995X_API) && !defined(HAVE_EXYNOS_API) && !defined(HAVE_LINUX_API) && !defined(HAVE_AOCEC_API) && !defined(HAVE_IMX_API)
  return NULL;
#endif
}

void CAdapterFactory::InitVideoStandalone(void)
{
#if defined(HAVE_RPI_API)
  CRPiCECAdapterCommunication::InitHost();
#endif
}
