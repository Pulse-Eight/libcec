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
  class CPHCommandHandler;

  class CImageViewOnCheck : public CThread
  {
  public:
    CImageViewOnCheck(CPHCommandHandler* handler):
      m_handler(handler) {}
    virtual ~CImageViewOnCheck(void);

    void* Process(void);

  private:
    CPHCommandHandler* m_handler;
    CEvent             m_event;
  };

  class CPHCommandHandler : public CCECCommandHandler
  {
    friend class CImageViewOnCheck;
  public:
    CPHCommandHandler(CCECBusDevice *busDevice,
                      int32_t iTransmitTimeout = CEC_DEFAULT_TRANSMIT_TIMEOUT,
                      int32_t iTransmitWait = CEC_DEFAULT_TRANSMIT_WAIT,
                      int8_t iTransmitRetries = CEC_DEFAULT_TRANSMIT_RETRIES,
                      int64_t iActiveSourcePending = 0);
    virtual ~CPHCommandHandler(void);

    bool InitHandler(void);

  protected:
    /**
     * How far the TV is into its power-up sequence. It routes 0.0.0.0 to 0.0.0.0 first, and
     * settles on the HDMI port it was last on second.
     */
    enum PHPowerUpState
    {
      PH_POWER_UNKNOWN,
      PH_POWERING_UP,
      PH_POWERED_UP
    };

    virtual bool ActivateSource(bool bTransmitDelayedCommandsOnly = false);
    virtual int HandleUserControlPressed(const cec_command& command);
    virtual int HandleUserControlRelease(const cec_command& command);
    virtual int HandleDeviceVendorId(const cec_command& command);
    virtual int HandleRoutingChange(const cec_command& command);
    virtual int HandleSetStreamPath(const cec_command& command);
    virtual int HandleStandby(const cec_command& command);

    bool ClaimRouteWhilePoweringUp(const cec_command& command, uint16_t iOldAddress, uint16_t iNewAddress);

    uint8_t            m_iLastKeyCode;
    CImageViewOnCheck* m_imageViewOnCheck;
    PHPowerUpState     m_powerUpState;
  };
};
