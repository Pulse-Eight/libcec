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
#include "CECCommandHandler.h"
#include "platform/threads/threads.h"

namespace CEC
{
  class CSYCommandHandler;

  /**
   * Asks the TV for its power status a moment after it sent a vendor command, which a sony
   * does when it powers up without announcing it.
   */
  class CPowerStatusCheck : public CThread
  {
  public:
    CPowerStatusCheck(CSYCommandHandler* handler) :
      m_handler(handler) {}
    virtual ~CPowerStatusCheck(void);

    void* Process(void);

  private:
    CSYCommandHandler* m_handler;
    CEvent             m_event;
  };

  class CSYCommandHandler : public CCECCommandHandler
  {
    friend class CPowerStatusCheck;
  public:
    CSYCommandHandler(CCECBusDevice *busDevice,
                      int32_t iTransmitTimeout = CEC_DEFAULT_TRANSMIT_TIMEOUT,
                      int32_t iTransmitWait = CEC_DEFAULT_TRANSMIT_WAIT,
                      int8_t iTransmitRetries = CEC_DEFAULT_TRANSMIT_RETRIES,
                      int64_t iActiveSourcePending = 0);
    virtual ~CSYCommandHandler(void);

  protected:
    int HandleDeviceVendorCommandWithId(const cec_command &command);
    int HandleGiveDevicePowerStatus(const cec_command &command);
    void OnPowerStatusChanged(const cec_power_status oldStatus, const cec_power_status newStatus);
    bool TransmitVendorID(const cec_logical_address iInitiator, const cec_logical_address iDestination, uint64_t iVendorId, bool bIsReply);

    /** @return True when the set treats this device as powered off whatever we report. */
    bool TreatedAsPoweredOff(CCECBusDevice* device);

    /** the year the TV was made, 0 when it can't be read from an EDID */
    uint16_t           m_iModelYear;
    CPowerStatusCheck* m_powerStatusCheck;
  };
};
