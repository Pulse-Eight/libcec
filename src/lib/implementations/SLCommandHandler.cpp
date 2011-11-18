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

#include "SLCommandHandler.h"
#include "../devices/CECBusDevice.h"
#include "../CECProcessor.h"

using namespace CEC;

CSLCommandHandler::CSLCommandHandler(CCECBusDevice *busDevice) :
    CCECCommandHandler(busDevice)
{
}

bool CSLCommandHandler::HandleVendorCommand(const cec_command &command)
{
  if (command.parameters.size == 1 &&
      command.parameters[0] == 0xA0)
  {
    /* enable SL */
    cec_command response;
    cec_command::Format(response, m_busDevice->GetLogicalAddress(), command.initiator, CEC_OPCODE_VENDOR_COMMAND);
    response.PushBack(0x02);
    response.PushBack(0x05);

    return m_busDevice->GetProcessor()->Transmit(response);
  }

  return false;
}

bool CSLCommandHandler::HandleGiveDeviceVendorId(const cec_command &command)
{
  /* imitate LG devices */
  CCECBusDevice *device = GetDevice(command.destination);
  if (device)
    device->SetVendorId(CEC_VENDOR_LG);

  return CCECCommandHandler::HandleGiveDeviceVendorId(command);
}

bool CSLCommandHandler::HandleCommand(const cec_command &command)
{
  bool bHandled(false);
  if (m_busDevice->MyLogicalAddressContains(command.destination))
  {
    switch(command.opcode)
    {
    case CEC_OPCODE_VENDOR_COMMAND:
      bHandled = HandleVendorCommand(command);
      break;
    default:
      break;
    }
  }

  if (!bHandled)
    bHandled = CCECCommandHandler::HandleCommand(command);

  return bHandled;
}
