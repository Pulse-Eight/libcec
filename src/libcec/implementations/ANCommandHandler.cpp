/*
 * This file is part of the libCEC(R) library.
 *
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
#include "ANCommandHandler.h"

#include "devices/CECBusDevice.h"
#include "CECProcessor.h"
#include "LibCEC.h"
#include "CECClient.h"

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
  if (busDevice->GetLogicalAddress() == CECDEVICE_TV)
  {
    // disable auto mode, as this may wake up the TV randomly (samsung 2017+ bug)
    m_busDevice->GetProcessor()->SetAutoMode(false);
  }
}

int CANCommandHandler::HandleVendorRemoteButtonDown(const cec_command &command)
{
  if (command.parameters.size == 0)
    return CEC_ABORT_REASON_INVALID_OPERAND;

  if (!m_processor->CECInitialised())
    return CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND;

  CECClientPtr client = m_processor->GetClient(command.destination);
  if (!client)
    return CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND;

  cec_keypress key;
  key.duration = CEC_BUTTON_TIMEOUT;
  key.keycode = (cec_user_control_code)command.parameters[0];

  if (client)
    client->AddKey(key);

  return COMMAND_HANDLED;
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

int CANCommandHandler::HandleDeviceVendorCommandWithId(const cec_command &command)
{
  if (!m_processor->IsHandledByLibCEC(command.destination) && command.destination != CECDEVICE_BROADCAST)
    return CEC_ABORT_REASON_INVALID_OPERAND;

  // samsung's vendor id
  if (command.parameters[0] == 0x00 && command.parameters[1] == 0x00 && command.parameters[2] == 0xf0)
  {
    // unknown vendor command sent to devices
    if (command.parameters[3] == 0x23)
    {
      cec_command response;
      cec_command::Format(response, command.destination, command.initiator, CEC_OPCODE_VENDOR_COMMAND_WITH_ID);

      // samsung vendor id
      response.parameters.PushBack(0x00); response.parameters.PushBack(0x00); response.parameters.PushBack(0xf0);

      // XXX see bugzid 2164. reply sent back by audio systems, we might have to send something different
      response.parameters.PushBack(0x24);
      response.parameters.PushBack(0x00);
      response.parameters.PushBack(0x80);

      Transmit(response, false, true);
      return COMMAND_HANDLED;
    }
  }
  return CEC_ABORT_REASON_INVALID_OPERAND;
}

int CANCommandHandler::HandleSetMenuLanguage(const cec_command &command)
{
  if (m_processor->CECInitialised() && command.initiator == CECDEVICE_TV && command.destination == CECDEVICE_BROADCAST)
  {
    m_processor->GetDevice(command.initiator)->SetPowerStatus(CEC_POWER_STATUS_ON);
  }

  return CCECCommandHandler::HandleSetMenuLanguage(command);
}
