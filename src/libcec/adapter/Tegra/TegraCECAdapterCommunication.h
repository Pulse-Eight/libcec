#pragma once
/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2013 Pulse-Eight Limited.  All rights reserved.
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

#if defined(HAVE_TEGRA_API)

#include "p8-platform/threads/mutex.h"
#include "p8-platform/threads/threads.h"
#include "p8-platform/sockets/socket.h"
#include "adapter/AdapterCommunication.h"
#include <map>

#define TEGRA_ADAPTER_VID 0x0001
#define TEGRA_ADAPTER_PID 0x0001

namespace P8PLATFORM
{
  class CCDevSocket;
};


namespace CEC
{
  class CAdapterMessageQueueEntry;

  class TegraCECAdapterCommunication : public IAdapterCommunication, public P8PLATFORM::CThread
  {
  public:
    /*!
     * @brief Create a new USB-CEC communication handler.
     * @param callback The callback to use for incoming CEC commands.
     */
    TegraCECAdapterCommunication(IAdapterCommunicationCallback *callback);
    virtual ~TegraCECAdapterCommunication(void);

    /** @name IAdapterCommunication implementation */
    ///{
    bool Open(uint32_t iTimeoutMs = CEC_DEFAULT_CONNECT_TIMEOUT, bool bSkipChecks = false, bool bStartListening = true);
    void Close(void);
    void writeTegraCEC(unsigned char func, int isStart, int isFinish, int isBroadcast);
    bool IsOpen(void);
    std::string GetError(void) const;
    cec_adapter_message_state Write(const cec_command &data, bool &bRetry, uint8_t iLineTimeout, bool bIsReply);

    bool SetLineTimeout(uint8_t UNUSED(iTimeout)) { return true; }
    bool StartBootloader(void) { return false; }
    bool SetLogicalAddresses(const cec_logical_addresses &addresses);
    cec_logical_addresses GetLogicalAddresses(void) const override;
    bool PingAdapter(void) { return IsInitialised(); }
    uint16_t GetFirmwareVersion(void);
    uint32_t GetFirmwareBuildDate(void) { return 0; }
    bool IsRunningLatestFirmware(void) { return true; }
    bool PersistConfiguration(const libcec_configuration & UNUSED(configuration)) { return false; }
    bool GetConfiguration(libcec_configuration & UNUSED(configuration)) { return false; }
    std::string GetPortName(void) { return std::string("TEGRACEC"); }
    uint16_t GetPhysicalAddress(void);
    bool SetControlledMode(bool UNUSED(controlled)) { return true; }
	bool SaveConfiguration(const libcec_configuration & UNUSED(configuration)) override { return false; }
    bool SetAutoMode(bool UNUSED(automode)) override { return false; }
    cec_vendor_id GetVendorId(void);
    bool SupportsSourceLogicalAddress(const cec_logical_address address) { return address > CECDEVICE_TV && address <= CECDEVICE_BROADCAST; }
    cec_adapter_type GetAdapterType(void) { return ADAPTERTYPE_TEGRA; }
    uint16_t GetAdapterVendorId(void) const { return TEGRA_ADAPTER_VID; }
    uint16_t GetAdapterProductId(void) const { return TEGRA_ADAPTER_PID; }
    void HandleLogicalAddressLost(cec_logical_address oldAddress);
    void SetActiveSource(bool UNUSED(bSetTo), bool UNUSED(bClientUnregistered)) {}
#if CEC_LIB_VERSION_MAJOR >= 5
    bool GetStats(struct cec_adapter_stats* UNUSED(stats)) override { return false; }
#endif

    ///}

    /** @name P8PLATFORM::CThread implementation */
    ///{
    void *Process(void);
    ///}

  private:
    bool IsInitialised(void) const { return m_dev != 0; };
    int fd;
    int fdAddr;
    bool devOpen;
    std::string                 m_strError; /**< current error message */

    bool                        m_bLogicalAddressChanged;
    cec_logical_addresses       m_logicalAddresses;

    P8PLATFORM::CMutex            m_mutex;
    P8PLATFORM::CCDevSocket       *m_dev;	/**< the device connection */
    
    P8PLATFORM::CMutex            m_messageMutex;
    uint32_t                    m_iNextMessage;
    std::map<uint32_t, CAdapterMessageQueueEntry *> m_messages;
  };
  
};

#endif
