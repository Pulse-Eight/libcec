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

#include <cectypes.h>

namespace CEC
{
  class CCECBusDevice;

  class CCECCommandHandler
  {
  public:
    CCECCommandHandler(CCECBusDevice *busDevice);
    virtual ~CCECCommandHandler(void) {};

    virtual bool HandleCommand(const cec_command &command);
    virtual cec_vendor_id GetVendorId(void) { return CEC_VENDOR_UNKNOWN; };

  protected:
    virtual bool HandleDeviceCecVersion(const cec_command &command);
    virtual bool HandleDeviceVendorCommandWithId(const cec_command &command);
    virtual bool HandleDeviceVendorId(const cec_command &command);
    virtual bool HandleGetCecVersion(const cec_command &command);
    virtual bool HandleGiveAudioStatus(const cec_command &command);
    virtual bool HandleGiveDeckStatus(const cec_command &command);
    virtual bool HandleGiveDevicePowerStatus(const cec_command &command);
    virtual bool HandleGiveDeviceVendorId(const cec_command &command);
    virtual bool HandleGiveOSDName(const cec_command &command);
    virtual bool HandleGivePhysicalAddress(const cec_command &command);
    virtual bool HandleMenuRequest(const cec_command &command);
    virtual bool HandleReportPowerStatus(const cec_command &command);
    virtual bool HandleRequestActiveSource(const cec_command &command);
    virtual bool HandleRoutingChange(const cec_command &command);
    virtual bool HandleSetMenuLanguage(const cec_command &command);
    virtual bool HandleSetStreamPath(const cec_command &command);
    virtual bool HandleUserControlPressed(const cec_command &command);
    virtual bool HandleUserControlRelease(const cec_command &command);
    virtual void UnhandledCommand(const cec_command &command);

    virtual CCECBusDevice *GetDevice(cec_logical_address iLogicalAddress) const;
    CCECBusDevice *m_busDevice;
  };
};
