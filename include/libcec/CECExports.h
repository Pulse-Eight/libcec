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

#ifndef CECEXPORTS_H_
#define CECEXPORTS_H_

#include <libcec/CECTypes.h>

namespace CEC
{
  class ICECAdapter
  {
  public:
    virtual ~ICECAdapter() {};
    /*! @name Adapter methods */
    //@{
    /*!
     * @see cec_open
     */
    virtual bool Open(const char *strPort, uint32_t iTimeoutMs = 10000) = 0;

    /*!
     * @see cec_close
     */
    virtual void Close(void) = 0;

    /*!
     * @see cec_find_adapters
     */
    virtual int8_t FindAdapters(cec_adapter *deviceList, uint8_t iBufSize, const char *strDevicePath = NULL) = 0;

    /*!
     * @see cec_ping_adapters
     */
    virtual bool PingAdapter(void) = 0;

    /*!
     * @see cec_start_bootloader
     */
    virtual bool StartBootloader(void) = 0;
    //@}

    /*!
     * @see cec_get_min_version
     */
    virtual int8_t GetMinVersion(void) = 0;

    /*!
     * @see cec_get_lib_version
     */
    virtual int8_t GetLibVersion(void) = 0;

    /*!
     * @see cec_get_next_log_message
     */
    virtual bool GetNextLogMessage(cec_log_message *message) = 0;

    /*!
     * @see cec_get_next_keypress
     */
    virtual bool GetNextKeypress(cec_keypress *key) = 0;

    /*!
     * @see cec_get_next_command
     */
    virtual bool GetNextCommand(cec_command *command) = 0;

    /*!
     * @see cec_transmit
     */
    virtual bool Transmit(const cec_command &data, bool bWaitForAck = true) = 0;

    /*!
     * @see cec_set_logical_address
     */
    virtual bool SetLogicalAddress(cec_logical_address iLogicalAddress) = 0;

    /*!
     * @see cec_power_on_devices
     */
    virtual bool PowerOnDevices(cec_logical_address address = CECDEVICE_TV) = 0;

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
  };
};

extern "C" DECLSPEC void * CECCreate(const char *strDeviceName, CEC::cec_logical_address iLogicalAddress = CEC::CECDEVICE_PLAYBACKDEVICE1, uint16_t iPhysicalAddress = CEC_DEFAULT_PHYSICAL_ADDRESS);
extern "C" DECLSPEC void CECDestroy(CEC::ICECAdapter *instance);

#endif /* CECEXPORTS_H_ */
