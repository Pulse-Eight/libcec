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
  if (m_processor->IsRunning() && command.parameters.size > 0)
  {
    cec_keypress key;
    key.duration = CEC_BUTTON_TIMEOUT;
    key.keycode = CEC_USER_CONTROL_CODE_UNKNOWN;

    switch (command.parameters[0])
    {
    case CEC_USER_CONTROL_CODE_AN_RETURN:
      key.keycode = m_processor->GetClientVersion() >= CEC_CLIENT_VERSION_1_5_0 ?
        CEC_USER_CONTROL_CODE_AN_RETURN :
        CEC_USER_CONTROL_CODE_EXIT;
      break;
    case CEC_USER_CONTROL_CODE_AN_CHANNELS_LIST:
      key.keycode = CEC_USER_CONTROL_CODE_AN_CHANNELS_LIST;
      break;
    default:
      break;
    }

    if (key.keycode != CEC_USER_CONTROL_CODE_UNKNOWN)
      CLibCEC::AddKey(key);
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

bool CANCommandHandler::PowerOn(const cec_logical_address iInitiator, const cec_logical_address iDestination)
{
  if (iDestination == CECDEVICE_AUDIOSYSTEM)
  {
    /* Samsung AVR devices need to be woken up with key CEC_USER_CONTROL_CODE_POWER_ON_FUNCTION */
    return TransmitKeypress(iInitiator, iDestination, CEC_USER_CONTROL_CODE_POWER_ON_FUNCTION) &&
        TransmitKeyRelease(iInitiator, iDestination);
  }

  return CCECCommandHandler::PowerOn(iInitiator, iDestination);
}
