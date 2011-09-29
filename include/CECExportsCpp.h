#pragma once
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

namespace CEC
{
  class ICECDevice
  {
  public:
    /*!
     * @see cec_open
     */
    virtual bool Open(const char *strPort, int iTimeoutMs = 10000) = 0;

    /*!
     * @see cec_find_devices
     */
    virtual int  FindDevices(std::vector<cec_device> &deviceList, const char *strDevicePath = NULL) = 0;

    /*!
     * @see cec_ping
     */
    virtual bool Ping(void) = 0;

    /*!
     * @see cec_start_bootloader
     */
    virtual bool StartBootloader(void) = 0;

    /*!
     * @see cec_power_off_devices
     */
    virtual bool PowerOffDevices(cec_logical_address address = CECDEVICE_BROADCAST) = 0;

    /*!
     * @see cec_power_on_devices
     */
    virtual bool PowerOnDevices(cec_logical_address address = CECDEVICE_BROADCAST) = 0;

    /*!
     * @see cec_standby_devices
     */
    virtual bool StandbyDevices(cec_logical_address address = CECDEVICE_BROADCAST) = 0;

    /*!
     * @see cec_set_active_view
     */
    virtual bool SetActiveView(void) = 0;

    /*!
     * @see cec_set_inactive_view
     */
    virtual bool SetInactiveView(void) = 0;

    /*!
     * @see cec_get_next_log_message
     */
    virtual bool GetNextLogMessage(cec_log_message *message) = 0;

    /*!
     * @see cec_get_next_keypress
     */
    virtual bool GetNextKeypress(cec_keypress *key) = 0;

    /*!
     * @see cec_transmit
     */
    virtual bool Transmit(const cec_frame &data, bool bWaitForAck = true, int64_t iTimeout = (int64_t) 5000) = 0;

    /*!
     * @see cec_set_logical_address
     */
    virtual bool SetLogicalAddress(cec_logical_address iLogicalAddress) = 0;

    /*!
     * @deprecated use SetLogicalAddress() instead
     */
    virtual bool SetAckMask(uint16_t iMask) = 0;

    /*!
     * @see cec_get_min_version
     */
    virtual int GetMinVersion(void) = 0;

    /*!
     * @see cec_get_lib_version
     */
    virtual int GetLibVersion(void) = 0;
  };
};

extern DECLSPEC void * CECCreate(const char *strDeviceName, CEC::cec_logical_address iLogicalAddress = CEC::CECDEVICE_PLAYBACKDEVICE1, int iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS);

#if !defined(DLL_EXPORT)
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <conio.h>

static HINSTANCE g_libCEC = NULL;
static int g_iLibCECInstanceCount = 0;

/*!
 * @see cec_init
 */
inline CEC::ICECDevice *LoadLibCec(const char *strName, CEC::cec_logical_address iLogicalAddress = CEC::CECDEVICE_PLAYBACKDEVICE1, int iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS)
{
  typedef void* (__cdecl*_CreateLibCec)(const char *);
  _CreateLibCec CreateLibCec;

  if (!g_libCEC)
    g_libCEC = LoadLibrary("libcec.dll");
  if (!g_libCEC)
    return NULL;

  ++g_iLibCECInstanceCount;
  CreateLibCec = (_CreateLibCec) (GetProcAddress(g_libCEC, "CECCreate"));
  if (!CreateLibCec)
    return NULL;
  return static_cast< CEC::ICECDevice* > (CreateLibCec(strName, iLogicalAddress, iPhysicalAddress));
}

/*!
 * @brief Unload the given libcec instance.
 * @param device The instance to unload.
 */
inline void UnloadLibCec(CEC::ICECDevice *device)
{
  delete device;

  if (--g_iLibCECInstanceCount == 0)
  {
    FreeLibrary(g_libCEC);
    g_libCEC = NULL;
  }
};

#else

/*!
 * @see cec_init
 */
inline CEC::ICECDevice *LoadLibCec(const char *strName, CEC::cec_logical_address iLogicalAddress = CEC::CECDEVICE_PLAYBACKDEVICE1, int iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS)
{
  return (CEC::ICECDevice*) CECCreate(strName, iLogicalAddress, iPhysicalAddress);
};

/*!
 * @brief Unload the given libcec instance.
 * @param device The instance to unload.
 */
inline void UnloadLibCec(CEC::ICECDevice *device)
{
  delete device;
};
#endif

#endif
