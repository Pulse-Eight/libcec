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
#include "devices/CECDeviceMap.h"
#include "platform/util/edid.h"
#include "CECProcessor.h"
#include "LibCEC.h"
#include "CECClient.h"

using namespace CEC;

#define LIB_CEC     m_busDevice->GetProcessor()->GetLib()
#define ToString(p) LIB_CEC->ToString(p)

/* the first model year that mirrors its own power state onto the sources it lists */
#define AN_POWER_STATUS_QUIRKS_MODEL_YEAR 2016

/* the vendor command a samsung sends once it powered up, and the reply it expects */
#define AN_COMMAND_POWERED_UP             0x23
#define AN_COMMAND_POWERED_UP_REPLY       0x24

CANCommandHandler::CANCommandHandler(CCECBusDevice *busDevice,
                                     int32_t iTransmitTimeout /* = CEC_DEFAULT_TRANSMIT_TIMEOUT */,
                                     int32_t iTransmitWait /* = CEC_DEFAULT_TRANSMIT_WAIT */,
                                     int8_t iTransmitRetries /* = CEC_DEFAULT_TRANSMIT_RETRIES */,
                                     int64_t iActiveSourcePending /* = 0 */) :
    CCECCommandHandler(busDevice, iTransmitTimeout, iTransmitWait, iTransmitRetries, iActiveSourcePending),
    m_iModelYear(CEDIDParser::GetModelYear())
{
  m_vendorId = CEC_VENDOR_SAMSUNG;
  if (busDevice->GetLogicalAddress() == CECDEVICE_TV)
  {
    // disable auto mode, as this may wake up the TV randomly (samsung 2017+ bug)
    m_busDevice->GetProcessor()->SetAutoMode(false);
  }
}

bool CANCommandHandler::HasPowerStatusQuirks(void) const
{
  /* every set from 2016 on has them. an EDID is not there to be read on every combination of
     hardware, and a set old enough to be without them is rarer than a missing EDID, so treat
     an unknown year as recent */
  return m_iModelYear == 0 || m_iModelYear >= AN_POWER_STATUS_QUIRKS_MODEL_YEAR;
}

void CANCommandHandler::OnPowerStatusChanged(const cec_power_status UNUSED(oldStatus), const cec_power_status newStatus)
{
  if (m_busDevice->GetLogicalAddress() != CECDEVICE_TV || !HasPowerStatusQuirks())
    return;

  CCECBusDevice* activeSource = m_processor->GetDevices()->GetActiveSource();
  if (!activeSource || !activeSource->IsHandledByLibCEC())
    return;

  /* the TV lists the source as powered on for as long as we say it is, and that list is what
     it offers the user, so keep the source in step with the set */
  if (newStatus == CEC_POWER_STATUS_STANDBY ||
      newStatus == CEC_POWER_STATUS_IN_TRANSITION_ON_TO_STANDBY)
  {
    activeSource->SetPowerStatus(CEC_POWER_STATUS_STANDBY);
    activeSource->TransmitPowerState(CECDEVICE_BROADCAST, false);
  }
  else if (newStatus == CEC_POWER_STATUS_ON ||
           newStatus == CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON)
  {
    activeSource->ActivateSource();
  }
}

int CANCommandHandler::HandleGiveDevicePowerStatus(const cec_command &command)
{
  if (m_processor->CECInitialised() &&
      m_processor->IsHandledByLibCEC(command.destination) &&
      HasPowerStatusQuirks())
  {
    CCECBusDevice* tv = m_processor->GetDevice(CECDEVICE_TV);
    CCECBusDevice* device = GetDevice(command.destination);
    if (tv && device && tv->GetCurrentPowerStatus() == CEC_POWER_STATUS_STANDBY)
    {
      /* answer for the set rather than for ourselves while it sleeps, or its source list
         disagrees with what it is showing */
      device->SetPowerStatus(CEC_POWER_STATUS_STANDBY);
      device->TransmitPowerState(command.initiator, true);
      return COMMAND_HANDLED;
    }
  }

  return CCECCommandHandler::HandleGiveDevicePowerStatus(command);
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
  // samsung's vendor id, followed by the command it sends once it powered up
  if (command.parameters.size < 4 ||
      command.parameters[0] != 0x00 ||
      command.parameters[1] != 0x00 ||
      command.parameters[2] != 0xf0 ||
      command.parameters[3] != AN_COMMAND_POWERED_UP)
    return CCECCommandHandler::HandleDeviceVendorCommandWithId(command);

  // the device that sent it is powered up
  CCECBusDevice* device = GetDevice(command.initiator);
  if (device)
    device->SetPowerStatus(CEC_POWER_STATUS_ON);

  /* it is also sent as a broadcast, which leaves no address to answer from - take the power
     status from it and say nothing back */
  if (!m_processor->IsHandledByLibCEC(command.destination))
    return COMMAND_HANDLED;

  cec_command response;
  cec_command::Format(response, command.destination, command.initiator, CEC_OPCODE_VENDOR_COMMAND_WITH_ID);

  // samsung vendor id
  response.parameters.PushBack(0x00); response.parameters.PushBack(0x00); response.parameters.PushBack(0xf0);

  // this is the reply that samsung audio systems send. other device types may want another one
  response.parameters.PushBack(AN_COMMAND_POWERED_UP_REPLY);
  response.parameters.PushBack(0x00);
  response.parameters.PushBack(0x80);

  Transmit(response, false, true);
  return COMMAND_HANDLED;
}

int CANCommandHandler::HandleSetMenuLanguage(const cec_command &command)
{
  if (m_processor->CECInitialised() && command.initiator == CECDEVICE_TV && command.destination == CECDEVICE_BROADCAST)
  {
    m_processor->GetDevice(command.initiator)->SetPowerStatus(CEC_POWER_STATUS_ON);
  }

  return CCECCommandHandler::HandleSetMenuLanguage(command);
}
