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

#include "ANCommandHandler.h"
#include "../devices/CECBusDevice.h"
#include "../CECProcessor.h"
#include "../LibCEC.h"

using namespace CEC;

CANCommandHandler::CANCommandHandler(CCECBusDevice *busDevice) :
    CCECCommandHandler(busDevice)
{
  m_vendorId = CEC_VENDOR_SAMSUNG;
  m_bOPTSendDeckStatusUpdateOnActiveSource = false;
}

bool CANCommandHandler::HandleVendorRemoteButtonDown(const cec_command &command)
{
  if (m_processor->IsStarted() && command.parameters.size > 0)
  {
    cec_keypress key;
    key.duration = CEC_BUTTON_TIMEOUT;
    key.keycode = CEC_USER_CONTROL_CODE_UNKNOWN;

    switch (command.parameters[0])
    {
    case CEC_AN_USER_CONTROL_CODE_RETURN:
      key.keycode = CEC_USER_CONTROL_CODE_EXIT;
      break;
    default:
      break;
    }

    if (key.keycode != CEC_USER_CONTROL_CODE_UNKNOWN)
    {
      CLibCEC::AddLog(CEC_LOG_DEBUG, "key pressed: %1x", key.keycode);
      m_busDevice->GetProcessor()->AddKey(key);
    }
  }

  return true;
}

bool CANCommandHandler::HandleCommand(const cec_command &command)
{
  bool bHandled(false);
  if (m_busDevice->MyLogicalAddressContains(command.destination))
  {
    switch(command.opcode)
    {
    case CEC_OPCODE_VENDOR_REMOTE_BUTTON_DOWN:
      bHandled = true;
      HandleVendorRemoteButtonDown(command);
      break;
    case CEC_OPCODE_VENDOR_REMOTE_BUTTON_UP:
      bHandled = true;
      HandleUserControlRelease(command);
      break;
    default:
      break;
    }
  }

  if (!bHandled)
    bHandled = CCECCommandHandler::HandleCommand(command);

  return bHandled;
}
