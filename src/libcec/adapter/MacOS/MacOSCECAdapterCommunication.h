#pragma once
/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC MacOS Code is Copyright (C) 2020 Adam Engström
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

#if defined(HAVE_MACOS_API)

#include "p8-platform/threads/mutex.h"
#include "p8-platform/threads/threads.h"
#include "../AdapterCommunication.h"
#include "DisplayPortAux.h"
#include <map>

namespace CEC
{
  /*!
   * @brief Communication handler for MacOS DisplayPort -> HDMI adapters
   *
   * Uses DisplayPort CEC-Tunneling-over-AUX feature to communicate with CEC
   * devces.
   *
   * This will only work with _some_ specific DP adapters, please see
   * https://hverkuil.home.xs4all.nl/cec-status.txt for more information on
   * supported adapters.
   */
  class CMacOSCECAdapterCommunication : public IAdapterCommunication, public P8PLATFORM::CThread
  {
  public:
    /*!
     * @brief Create a new MacOS HDMI CEC communication handler.
     * @param callback The callback to use for incoming CEC commands.
     */
    CMacOSCECAdapterCommunication(IAdapterCommunicationCallback *callback);
    virtual ~CMacOSCECAdapterCommunication(void);

    /** @name IAdapterCommunication implementation */
    ///{
    bool Open(uint32_t iTimeoutMs = CEC_DEFAULT_CONNECT_TIMEOUT, bool bSkipChecks = false, bool bStartListening = true);
    void Close(void);
    bool IsOpen(void) { return m_dpAux.IsOpen(); };
    std::string GetError(void) const;
    cec_adapter_message_state Write(const cec_command &data, bool &bRetry, uint8_t iLineTimeout, bool bIsReply);

    bool SetLineTimeout(uint8_t UNUSED(iTimeout)) { return true; }
    bool StartBootloader(void) { return false; }
    bool SetLogicalAddresses(const cec_logical_addresses &addresses);
    cec_logical_addresses GetLogicalAddresses(void) const;
    bool PingAdapter(void) { return IsInitialised(); }
    uint16_t GetFirmwareVersion(void);
    uint32_t GetFirmwareBuildDate(void) { return 0; }
    bool IsRunningLatestFirmware(void) { return true; }
    bool SaveConfiguration(const libcec_configuration & UNUSED(configuration)) { return false; }
    bool GetConfiguration(libcec_configuration & UNUSED(configuration)) { return false; }
    bool SetAutoMode(bool UNUSED(automode)) { return false; }
    std::string GetPortName(void) { return std::string("MACOS"); }
    uint16_t GetPhysicalAddress(void);
    bool SetControlledMode(bool UNUSED(controlled)) { return true; }
    cec_vendor_id GetVendorId(void);
    bool SupportsSourceLogicalAddress(const cec_logical_address address) { return address > CECDEVICE_TV && address <= CECDEVICE_BROADCAST; }
    cec_adapter_type GetAdapterType(void) { return ADAPTERTYPE_MACOS; }
    uint16_t GetAdapterVendorId(void) const { return 1; }
    uint16_t GetAdapterProductId(void) const { return 1; }
    void SetActiveSource(bool UNUSED(bSetTo), bool UNUSED(bClientUnregistered)) {}
    #if CEC_LIB_VERSION_MAJOR >= 5
    bool GetStats(struct cec_adapter_stats* UNUSED(stats)) { return false; }
    #endif
    ///}

    /** @name P8PLATFORM::CThread implementation */
    ///{
    void *Process(void);
    ///}

  private:
    bool IsInitialised(void) const { return 1; };

    std::string                 m_strError; /**< current error message */

    cec_logical_addresses       m_logicalAddresses;

    mutable P8PLATFORM::CMutex  m_mutex;
    DisplayPortAux              m_dpAux;
  };
};
#endif
