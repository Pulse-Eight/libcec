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
#include "SYCommandHandler.h"

#include "devices/CECBusDevice.h"
#include "platform/util/edid.h"
#include "CECProcessor.h"
#include "LibCEC.h"

using namespace CEC;

#define LIB_CEC     m_busDevice->GetProcessor()->GetLib()
#define ToString(p) LIB_CEC->ToString(p)

/* how long to let the TV settle before asking whether it powered up */
#define SY_POWER_STATUS_CHECK_TIME_MS 500

/* the first model year that cuts the power to its sources when the set is switched off */
#define SY_POWERS_OFF_SOURCES_MODEL_YEAR 2016

CPowerStatusCheck::~CPowerStatusCheck(void)
{
  StopThread(-1);
  m_event.Broadcast();
  StopThread();
}

void* CPowerStatusCheck::Process(void)
{
  m_event.Wait(SY_POWER_STATUS_CHECK_TIME_MS);
  if (!IsRunning())
    return NULL;

  CCECBusDevice* tv = m_handler->m_processor->GetDevice(CECDEVICE_TV);
  if (tv)
    tv->RequestPowerStatus(m_handler->m_processor->GetLogicalAddress(), true, false);

  return NULL;
}

CSYCommandHandler::CSYCommandHandler(CCECBusDevice *busDevice,
                                     int32_t iTransmitTimeout /* = CEC_DEFAULT_TRANSMIT_TIMEOUT */,
                                     int32_t iTransmitWait /* = CEC_DEFAULT_TRANSMIT_WAIT */,
                                     int8_t iTransmitRetries /* = CEC_DEFAULT_TRANSMIT_RETRIES */,
                                     int64_t iActiveSourcePending /* = 0 */) :
    CCECCommandHandler(busDevice, iTransmitTimeout, iTransmitWait, iTransmitRetries, iActiveSourcePending),
    m_iModelYear(CEDIDParser::GetModelYear())
{
  m_vendorId = CEC_VENDOR_SONY;
  m_powerStatusCheck = new CPowerStatusCheck(this);
}

CSYCommandHandler::~CSYCommandHandler(void)
{
  delete m_powerStatusCheck;
}

bool CSYCommandHandler::TransmitVendorID(const cec_logical_address iInitiator, const cec_logical_address iDestination, uint64_t UNUSED(iVendorId), bool bIsReply)
{
  /* announce ourselves rather than imitating the TV, which a sony does not ask us to do */
  return CCECCommandHandler::TransmitVendorID(iInitiator, iDestination, CEC_VENDOR_PULSE_EIGHT, bIsReply);
}

int CSYCommandHandler::HandleDeviceVendorCommandWithId(const cec_command &command)
{
  if (command.initiator == CECDEVICE_TV &&
      command.parameters.size >= 3 &&
      ((uint32_t)command.parameters[0] << 16 | (uint32_t)command.parameters[1] << 8 | (uint32_t)command.parameters[2]) == (uint32_t)CEC_VENDOR_SONY)
  {
    /* a sony sends this when it wakes, without reporting that its power status changed */
    if (m_powerStatusCheck && !m_powerStatusCheck->IsRunning())
      m_powerStatusCheck->CreateThread(false);
  }

  return CCECCommandHandler::HandleDeviceVendorCommandWithId(command);
}

bool CSYCommandHandler::TreatedAsPoweredOff(CCECBusDevice* device)
{
  /* a sony reads a source it isn't showing as switched off, so telling it otherwise only
     makes its source list disagree with itself */
  if (!device->IsActiveSource())
    return true;

  /* a set from 2016 on cuts the power to its sources when it is switched off. an EDID is not
     there to be read on every combination of hardware, and a set old enough to keep them
     powered is rarer than a missing EDID, so treat an unknown year as recent */
  if (m_iModelYear == 0 || m_iModelYear >= SY_POWERS_OFF_SOURCES_MODEL_YEAR)
  {
    CCECBusDevice* tv = m_processor->GetDevice(CECDEVICE_TV);
    if (tv && tv->GetCurrentPowerStatus() != CEC_POWER_STATUS_ON &&
        tv->GetCurrentPowerStatus() != CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON)
      return true;
  }

  return false;
}

int CSYCommandHandler::HandleGiveDevicePowerStatus(const cec_command &command)
{
  if (m_processor->CECInitialised() &&
      m_processor->IsHandledByLibCEC(command.destination))
  {
    CCECBusDevice* device = GetDevice(command.destination);
    if (device && TreatedAsPoweredOff(device))
    {
      /* answer for the set without touching the cached status, which stays what the client
         set it to */
      TransmitPowerState(command.destination, command.initiator, CEC_POWER_STATUS_STANDBY, true);
      return COMMAND_HANDLED;
    }
  }

  return CCECCommandHandler::HandleGiveDevicePowerStatus(command);
}

void CSYCommandHandler::OnPowerStatusChanged(const cec_power_status UNUSED(oldStatus), const cec_power_status newStatus)
{
  if (m_busDevice->GetLogicalAddress() != CECDEVICE_TV ||
      (newStatus != CEC_POWER_STATUS_ON && newStatus != CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON))
    return;

  /* a sony forgets the sources it saw while it was off, so introduce them again */
  CECDEVICEVEC devices;
  m_processor->GetDevices()->GetLibCECControlled(devices);
  for (CECDEVICEVEC::iterator it = devices.begin(); it != devices.end(); it++)
  {
    (*it)->TransmitPhysicalAddress(false);
    (*it)->TransmitOSDName(CECDEVICE_TV, false);
  }
}
