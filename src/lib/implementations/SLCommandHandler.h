#pragma once
/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2012 Pulse-Eight Limited.  All rights reserved.
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

#include "CECCommandHandler.h"
#include "../platform/util/timeutils.h"

namespace CEC
{
  class CSLCommandHandler : public CCECCommandHandler
  {
  public:
    CSLCommandHandler(CCECBusDevice *busDevice);
    virtual ~CSLCommandHandler(void) {};

    bool InitHandler(void);
    bool ActivateSource(void);

  protected:
    bool HandleActiveSource(const cec_command &command);
    bool HandleDeviceVendorId(const cec_command &command);
    bool HandleVendorCommand(const cec_command &command);

    void HandleVendorCommand01(const cec_command &command);
    void TransmitVendorCommand0205(const cec_logical_address iSource, const cec_logical_address iDestination);

    void HandleVendorCommandPowerOn(const cec_command &command);
    void HandleVendorCommandPowerOnStatus(const cec_command &command);

    void HandleVendorCommandSLConnect(const cec_command &command);
    void TransmitVendorCommandSetDeviceMode(const cec_logical_address iSource, const cec_logical_address iDestination, const cec_device_type type);

    bool HandleGiveDevicePowerStatus(const cec_command &command);
    bool HandleGiveDeckStatus(const cec_command &command);
    bool HandleRequestActiveSource(const cec_command &command);
    bool HandleFeatureAbort(const cec_command &command);
    bool HandleStandby(const cec_command &command);
    bool TransmitMenuState(const cec_logical_address UNUSED(iInitiator), const cec_logical_address UNUSED(iDestination), cec_menu_state UNUSED(menuState)) { return true; }
    bool PowerOn(const cec_logical_address iInitiator, const cec_logical_address iDestination);

    void ResetSLState(void);
    bool SLInitialised(void);
    void SetSLInitialised(void);
    bool ActiveSourceSent(void);

    bool               m_bSLEnabled;
    bool               m_bActiveSourceSent;
    PLATFORM::CTimeout m_resetPowerState;
    PLATFORM::CMutex   m_SLMutex;
  };
};
