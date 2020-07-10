#pragma once
/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2013 Pulse-Eight Limited.  All rights reserved.
 * libCEC(R) is an original work, containing original code.
 *
 * libCEC(R) is a trademark of Pulse-Eight Limited.
 *
 * IMX adpater port is Copyright (C) 2013 by Stephan Rafin
 *
 * You can redistribute this file and/or modify
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
 */

#if defined(HAVE_IMX_API)

#include "p8-platform/threads/mutex.h"
#include "p8-platform/threads/threads.h"
#include "p8-platform/sockets/socket.h"
#include "adapter/AdapterCommunication.h"
#include <map>

#define IMX_ADAPTER_VID 0x0471 /*FIXME TBD*/
#define IMX_ADAPTER_PID 0x1001



namespace P8PLATFORM
{
  class CCDevSocket;
};


namespace CEC
{
  class CIMXCECAdapterCommunication : public IAdapterCommunication, public P8PLATFORM::CThread
  {
  public:
    /*!
     * @brief Create a new USB-CEC communication handler.
     * @param callback The callback to use for incoming CEC commands.
     */
    CIMXCECAdapterCommunication(IAdapterCommunicationCallback *callback);
    virtual ~CIMXCECAdapterCommunication(void);

    /** @name IAdapterCommunication implementation */
    ///{
    bool Open(uint32_t iTimeoutMs = CEC_DEFAULT_CONNECT_TIMEOUT, bool bSkipChecks = false, bool bStartListening = true);
    void Close(void);
    bool IsOpen(void);
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
    std::string GetPortName(void) { return std::string("IMX"); }
    uint16_t GetPhysicalAddress(void);
    bool SetControlledMode(bool UNUSED(controlled)) { return true; }
    cec_vendor_id GetVendorId(void);
    bool SupportsSourceLogicalAddress(const cec_logical_address address) { return address > CECDEVICE_TV && address <= CECDEVICE_BROADCAST; }
    cec_adapter_type GetAdapterType(void) { return ADAPTERTYPE_IMX; }
    uint16_t GetAdapterVendorId(void) const { return IMX_ADAPTER_VID; }
    uint16_t GetAdapterProductId(void) const { return IMX_ADAPTER_PID; }
    void HandleLogicalAddressLost(cec_logical_address UNUSED(oldAddress));
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
    bool IsInitialised(void) const { return m_bInitialised; };
    bool UnregisterLogicalAddress(void);
    bool RegisterLogicalAddress(const cec_logical_address address);

    std::string                 m_strError; /**< current error message */

    cec_logical_address         m_logicalAddress;

    mutable P8PLATFORM::CMutex  m_mutex;
    P8PLATFORM::CCDevSocket     *m_dev;	/**< the device connection */
    bool                        m_bLogicalAddressRegistered;
    bool                        m_bInitialised;

    P8PLATFORM::CMutex          m_messageMutex;
    uint32_t                    m_iNextMessage;
  };

};

#endif
