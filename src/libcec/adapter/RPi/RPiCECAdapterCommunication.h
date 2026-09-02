#pragma once
/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2026 Pulse-Eight Limited.  All rights reserved.
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
#if defined(HAVE_RPI_API)

#include "adapter/AdapterCommunication.h"
#include "platform/threads/threads.h"
#include "platform/util/buffer.h"

#include <atomic>

#define RPI_ADAPTER_VID 0x2708
#define RPI_ADAPTER_PID 0x1001

extern "C" {
#include <interface/vmcs_host/vc_cecservice.h>
#include <interface/vchiq_arm/vchiq_if.h>
}

namespace CEC
{
  class CRPiCECAdapterMessageQueue;
  class CRPiCECAdapterCommunication;

  /*!
   * @brief One callback from the VideoCore userland, waiting to be handled.
   */
  struct rpi_callback
  {
    bool     bTVService; /**< true when it came from the TV service, false from the CEC service */
    uint32_t header;     /**< the CEC callback header, or the TV service reason */
    uint32_t p0;
    uint32_t p1;
    uint32_t p2;
    uint32_t p3;
  };

  /*!
   * @brief A bus change to report to libCEC, decoded from a VideoCore callback.
   */
  struct rpi_bus_change
  {
    bool     bAddressLost; /**< true to report a lost logical address, false a new physical address */
    uint16_t address;      /**< the logical address that was lost, or the new physical address */
  };

  /*!
   * @brief Reports bus changes to libCEC, off the callback dispatch thread.
   *
   * Both changes make libCEC transmit, and a transmit blocks until VideoCore
   * confirms it with a VC_CEC_TX callback. Reporting them from the thread that
   * delivers those callbacks would leave each one waiting for a confirmation
   * that only it could deliver, so they get a thread of their own.
   */
  class CRPiCECAdapterBusChangeThread : public CThread
  {
  public:
    CRPiCECAdapterBusChangeThread(CRPiCECAdapterCommunication *com) :
        m_com(com) {}
    virtual ~CRPiCECAdapterBusChangeThread(void) { StopThread(); }

    void *Process(void);

    /*!
     * @brief Queue a change to report. Called from the dispatch thread.
     * @return false when the queue is full, so the change was dropped
     */
    bool Queue(const rpi_bus_change &change) { return m_changes.Push(change); }
    void Clear(void) { m_changes.Clear(); }

  private:
    CRPiCECAdapterCommunication *m_com;
    SyncedBuffer<rpi_bus_change> m_changes;
  };

  class CRPiCECAdapterCommunication : public IAdapterCommunication, public CThread
  {
  public:
    /*!
     * @brief Create a new USB-CEC communication handler.
     * @param callback The callback to use for incoming CEC commands.
     */
    CRPiCECAdapterCommunication(IAdapterCommunicationCallback *callback);
    virtual ~CRPiCECAdapterCommunication(void);

    /** @name IAdapterCommunication implementation */
    ///{
    bool Open(uint32_t iTimeoutMs = CEC_DEFAULT_CONNECT_TIMEOUT, bool bSkipChecks = false, bool bStartListening = true);
    void Close(void);
    bool IsOpen(void) { return m_bInitialised; };
    std::string GetError(void) const;
    cec_adapter_message_state Write(const cec_command &data, bool &bRetry, uint8_t iLineTimeout, bool bIsReply);

    bool SetLineTimeout(uint8_t UNUSED(iTimeout)) { return true; };
    bool StartBootloader(void) { return false; };
    bool SetLogicalAddresses(const cec_logical_addresses &addresses);
    cec_logical_addresses GetLogicalAddresses(void) const;
    bool PingAdapter(void) { return m_bInitialised; };
    uint16_t GetFirmwareVersion(void);
    uint32_t GetFirmwareBuildDate(void) { return 0; };
    bool IsRunningLatestFirmware(void) { return true; };
    bool SaveConfiguration(const libcec_configuration & UNUSED(configuration)) { return false; };
    bool GetConfiguration(libcec_configuration & UNUSED(configuration)) { return false; };
    bool SetAutoMode(bool UNUSED(automode)) { return false; }
    std::string GetPortName(void) { std::string strReturn("RPI"); return strReturn; };
    uint16_t GetPhysicalAddress(void);
    bool SetControlledMode(bool UNUSED(controlled)) { return true; };
    cec_vendor_id GetVendorId(void) { return CEC_VENDOR_BROADCOM; }
    bool SupportsSourceLogicalAddress(const cec_logical_address address) { return address > CECDEVICE_TV && address < CECDEVICE_BROADCAST; }
    cec_adapter_type GetAdapterType(void) { return ADAPTERTYPE_RPI; };
    uint16_t GetAdapterVendorId(void) const { return RPI_ADAPTER_VID; }
    uint16_t GetAdapterProductId(void) const { return RPI_ADAPTER_PID; }
    void SetActiveSource(bool UNUSED(bSetTo), bool UNUSED(bClientUnregistered)) {}
    #if CEC_LIB_VERSION_MAJOR >= 5
    bool GetStats(struct cec_adapter_stats* UNUSED(stats)) { return false; }
    #endif
    ///}

    bool IsInitialised(void);
    void QueueCallback(bool bTVService, uint32_t header, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3);

    static void InitHost(void);

  private:
    void *Process(void);
    void OnDataReceived(uint32_t header, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3);
    void OnTVServiceCallback(uint32_t reason, uint32_t p0, uint32_t p1);
    cec_logical_address GetLogicalAddress(void) const;
    bool UnregisterLogicalAddress(void);
    bool RegisterLogicalAddress(const cec_logical_address address, uint32_t iTimeoutMs = CEC_DEFAULT_CONNECT_TIMEOUT);
    void SetDisableCallback(const bool disable);

    bool m_bInitialised;   /**< true when the connection is initialised, false otherwise */
    std::string m_strError; /**< current error message */
    CRPiCECAdapterMessageQueue *m_queue;
    cec_logical_address         m_logicalAddress;

    bool                          m_bLogicalAddressChanged;
    CCondition<bool>              m_logicalAddressCondition;
    mutable CMutex                m_mutex;
    cec_logical_address           m_previousLogicalAddress;
    bool                          m_bLogicalAddressRegistered;

    bool                          m_bDisableCallbacks;

    SyncedBuffer<rpi_callback>    m_callbacks;
    std::atomic<uint32_t>         m_iDroppedCallbacks;
    CRPiCECAdapterBusChangeThread m_busChanges;
  };
};

#endif
