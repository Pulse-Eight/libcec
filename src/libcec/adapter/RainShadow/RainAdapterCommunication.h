#pragma once
/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC RainShadow Code Copyright (C) 2017 Gerald Dachs
 * based heavily on:
 * libCEC Exynos Code Copyright (C) 2014 Valentin Manea
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


#if defined(HAVE_RAINSHADOW_API)

#include <stdlib.h>

#include <p8-platform/threads/mutex.h>
#include <p8-platform/threads/threads.h>
#include <p8-platform/sockets/socket.h>
#include "../AdapterCommunication.h"
//#include <map>

#define DATA_SIZE 256
#define CEC_RAINSHADOW_SERIAL_DEFAULT_BAUDRATE 115200L

typedef enum {
  OSD_NAME_REQUEST_NEEDED,
  OSD_NAME_REQUEST_SENT,
  OSD_NAME_REQUEST_DONE,
} tOsdNameRequestState;

namespace CEC
{

  class CRainAdapterCommunication : public IAdapterCommunication, public P8PLATFORM::CThread
  {
  public:
    /*!
       * @brief Create a new USB-CEC communication handler.
       * @param callback The callback to use for incoming CEC commands.
       * @param strPort The name of the com port to use.
       * @param iBaudRate The baudrate to use on the com port connection.
       */
    CRainAdapterCommunication(IAdapterCommunicationCallback *callback, const char *strPort, uint32_t iBaudRate = CEC_RAINSHADOW_SERIAL_DEFAULT_BAUDRATE);
    virtual ~CRainAdapterCommunication(void);

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
    cec_logical_addresses GetLogicalAddresses(void);
    bool PingAdapter(void) { return IsInitialised(); }
    uint16_t GetFirmwareVersion(void);
    uint32_t GetFirmwareBuildDate(void) { return 0; }
    bool IsRunningLatestFirmware(void) { return true; }
    bool PersistConfiguration(const libcec_configuration & UNUSED(configuration)) { return false; }
    bool GetConfiguration(libcec_configuration & UNUSED(configuration)) { return false; }
    std::string GetPortName(void);
    uint16_t GetPhysicalAddress(void);
    bool SetControlledMode(bool UNUSED(controlled)) { return true; }
    cec_vendor_id GetVendorId(void);
    bool SupportsSourceLogicalAddress(const cec_logical_address address) { return address > CECDEVICE_TV && address <= CECDEVICE_BROADCAST; }
    cec_adapter_type GetAdapterType(void) { return ADAPTERTYPE_RAINSHADOW; }
    uint16_t GetAdapterVendorId(void) const { return 1; }
    uint16_t GetAdapterProductId(void) const { return 1; }
    void HandleLogicalAddressLost(cec_logical_address oldAddress);
    void SetActiveSource(bool UNUSED(bSetTo), bool UNUSED(bClientUnregistered)) {}
    ///}

    void ProcessMessage(char *buffer);

    /** @name P8PLATFORM::CThread implementation */
    ///{
    void *Process(void);
    ///}

  private:
    bool IsInitialised(void) const { return 1; };

    bool InternalSetLogicalAddresses(const unsigned int log_addr);

    /**
     * hex_to_bin - convert a hex digit to its real value
     * @ch: ascii character represents hex digit
     *
     * hex_to_bin() converts one hex digit to its actual value or -1 in case of bad
     * input.
     */
    int hex_to_bin(char ch);

    /**
     * hex2bin - convert an ascii hexadecimal string to its binary representation
     * @dst: binary result
     * @src: ascii hexadecimal string
     * @count: result length
     *
     * Return 0 on success, -1 in case of bad input.
     */
    int hex2bin(uint8_t *dst, const char *src, size_t count);

    /**
     * SetAdapterPhysicalAddress - get the physical address from the system and report it to the adapter
     *
     * Return true on success, else false.
     */
    bool SetAdapterPhysicalAddress(void);

    /**
     * SetAdapterConfigrationBits - set the adapter configuration bits
     *
     * Return true on success, else false.
     */
    bool SetAdapterConfigurationBits(void);

    /**
     * SetAdapterOsdName - set the adapter OSD name
     * @packet: packet that contains the OSD name
     *
     * Return true on success, else false.
     */
    bool SetAdapterOsdName(const cec_datapacket &packet);

    /**
     * WriteAdapterCommand - writes the adapter command and waits for the response
     * @command: the command to write
     * @response: the response to wait for
     *
     * Return true on success, else false.
     */
    bool WriteAdapterCommand(char *command, const char *response);

    P8PLATFORM::ISocket *         m_port;                 /**< the com port connection */
    std::string                   m_strError; /**< current error message */
    char                          m_response[DATA_SIZE]; /**< current response from adapter */
    P8PLATFORM::CMutex            m_mutex;
    P8PLATFORM::CCondition<bool>  m_condition;
    bool                          m_gotResponse;
    bool                          m_bLogicalAddressChanged;
    cec_logical_addresses         m_logicalAddresses;
    tOsdNameRequestState          m_osdNameRequestState;
  };
};
#endif
