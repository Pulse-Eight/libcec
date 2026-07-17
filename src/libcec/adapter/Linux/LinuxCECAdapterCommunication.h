#pragma once
/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC Linux CEC Adapter is Copyright (C) 2017-2018 Jonas Karlman
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
#include <string>
#include <vector>
#include "platform/threads/threads.h"
#include "../AdapterCommunication.h"

namespace CEC
{
  class CLinuxCECAdapterCommunication : public IAdapterCommunication, public CThread
  {
  public:
    /*!
     * @brief Test whether a device node presents the required CEC capabilities.
     * @param strPath The device node to probe (e.g. /dev/cec0).
     * @return True when the node exists and is capable.
     */
    static bool IsCapableDevice(const std::string &strPath);

    /*!
     * @brief Scan /dev/cec* for every node that presents the required CEC
     *        capabilities, in ascending order.
     * @param paths Filled with the capable device paths.
     */
    static void FindDevicePaths(std::vector<std::string> &paths);

    /*!
     * @brief Scan /dev/cec* for the first node that presents the required CEC
     *        capabilities.
     * @param strPath Set to the discovered device path on success.
     * @return True when a capable device node was found.
     */
    static bool FindDevicePath(std::string &strPath);

    /*!
     * @brief Create a new Linux CEC communication handler.
     * @param callback The callback to use for incoming CEC commands.
     * @param strPath The device node to open, or empty to autodetect the first
     *        capable node.
     */
    CLinuxCECAdapterCommunication(IAdapterCommunicationCallback *callback, const std::string &strPath = "");
    virtual ~CLinuxCECAdapterCommunication(void);

    /** @name IAdapterCommunication implementation */
    ///{
    bool Open(uint32_t iTimeoutMs = CEC_DEFAULT_CONNECT_TIMEOUT, bool bSkipChecks = false, bool bStartListening = true) override;
    void Close(void) override;
    bool IsOpen(void) override;
    cec_adapter_message_state Write(const cec_command &data, bool &bRetry, uint8_t iLineTimeout, bool bIsReply) override;

    bool SetLineTimeout(uint8_t UNUSED(iTimeout)) override { return true; }
    bool StartBootloader(void) override { return false; }
    bool SetLogicalAddresses(const cec_logical_addresses &addresses) override;
    cec_logical_addresses GetLogicalAddresses(void) const override;
    bool PingAdapter(void) override { return true; }
    uint16_t GetFirmwareVersion(void) override { return 0; }
    uint32_t GetFirmwareBuildDate(void) override { return 0; }
    bool IsRunningLatestFirmware(void) override { return true; }
    bool SetControlledMode(bool UNUSED(controlled)) override { return true; }
    bool SaveConfiguration(const libcec_configuration & UNUSED(configuration)) override { return false; }
    bool SetAutoMode(bool UNUSED(automode)) override { return false; }
    bool GetConfiguration(libcec_configuration & UNUSED(configuration)) override { return false; }
    std::string GetPortName(void) override { return std::string("LINUX"); }
    uint16_t GetPhysicalAddress(void) override;
    cec_vendor_id GetVendorId(void) override;
    bool SupportsSourceLogicalAddress(const cec_logical_address address) override { return address > CECDEVICE_TV && address <= CECDEVICE_BROADCAST; }
    cec_adapter_type GetAdapterType(void) override { return ADAPTERTYPE_LINUX; }
    uint16_t GetAdapterVendorId(void) const override { return 1; }
    uint16_t GetAdapterProductId(void) const override { return 1; }
    void SetActiveSource(bool UNUSED(bSetTo), bool UNUSED(bClientUnregistered)) override {}
#if CEC_LIB_VERSION_MAJOR >= 5
    bool GetStats(struct cec_adapter_stats* UNUSED(stats)) override { return false; }
#endif
    ///}

    /** @name CThread implementation */
    ///{
    void *Process(void) override;
    ///}

  private:
    /*!
     * @brief Release the fd and mark the adapter closed when an ioctl reports
     *        that the device node has been removed (adapter unregistered).
     * @param err The errno captured from the failing ioctl/select call.
     * @return True when the device was gone and the fd has been released.
     */
    bool DeviceGone(int err);

    int         m_fd;
    std::string m_path;
  };
};

#endif
