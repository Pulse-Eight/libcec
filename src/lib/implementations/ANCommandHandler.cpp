/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2013 Pulse-Eight Limited.  All rights reserved.
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
#include "ANCommandHandler.h"

#include "lib/devices/CECBusDevice.h"
#include "lib/CECProcessor.h"
#include "lib/LibCEC.h"
#include "lib/CECClient.h"

using namespace CEC;

#define LIB_CEC     m_busDevice->GetProcessor()->GetLib()
#define ToString(p) LIB_CEC->ToString(p)

CANCommandHandler::CANCommandHandler(CCECBusDevice *busDevice,
                                     int32_t iTransmitTimeout /* = CEC_DEFAULT_TRANSMIT_TIMEOUT */,
                                     int32_t iTransmitWait /* = CEC_DEFAULT_TRANSMIT_WAIT */,
                                     int8_t iTransmitRetries /* = CEC_DEFAULT_TRANSMIT_RETRIES */,
                                     int64_t iActiveSourcePending /* = 0 */) :
    CCECCommandHandler(busDevice, iTransmitTimeout, iTransmitWait, iTransmitRetries, iActiveSourcePending)
{
  m_vendorId = CEC_VENDOR_SAMSUNG;
  m_bOPTSendDeckStatusUpdateOnActiveSource = false;
}

int CANCommandHandler::HandleVendorRemoteButtonDown(const cec_command &command)
{
  if (command.parameters.size == 0)
    return CEC_ABORT_REASON_INVALID_OPERAND;

  if (!m_processor->CECInitialised())
    return CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND;

  CCECClient *client = m_processor->GetClient(command.destination);
  if (!client)
    return CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND;

  cec_keypress key;
  key.duration = CEC_BUTTON_TIMEOUT;
  key.keycode = (cec_user_control_code)command.parameters[0];

  if (client)
    client->AddKey(key);

  return COMMAND_HANDLED;
}

int CANCommandHandler::HandleVendorRemoteButtonUp(const cec_command &command)
{
  return HandleUserControlRelease(command);
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
