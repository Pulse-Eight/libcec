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
#include "platform/util/timeutils.h"
#include "PHCommandHandler.h"

#include "devices/CECBusDevice.h"
#include "CECProcessor.h"
#include "LibCEC.h"
#include "CECClient.h"

using namespace CEC;

#define LIB_CEC     m_busDevice->GetProcessor()->GetLib()
#define ToString(p) LIB_CEC->ToString(p)

#define TV_ON_CHECK_TIME_MS 5000

CImageViewOnCheck::~CImageViewOnCheck(void)
{
  StopThread(-1);
  m_event.Broadcast();
  StopThread();
}

void* CImageViewOnCheck::Process(void)
{
  CCECBusDevice* tv = m_handler->m_processor->GetDevice(CECDEVICE_TV);
  cec_power_status status(CEC_POWER_STATUS_UNKNOWN);
  while (status != CEC_POWER_STATUS_ON)
  {
    m_event.Wait(TV_ON_CHECK_TIME_MS);
    if (!IsRunning())
      return NULL;

    status = tv->GetPowerStatus(m_handler->m_busDevice->GetLogicalAddress());

    if (status != CEC_POWER_STATUS_ON &&
        status != CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON)
    {
      CLockObject lock(m_handler->m_mutex);
      tv->OnImageViewOnSent(false);
      m_handler->m_iActiveSourcePending = GetTimeMs();
    }
  }
  return NULL;
}

CPHCommandHandler::CPHCommandHandler(CCECBusDevice *busDevice,
                                     int32_t iTransmitTimeout /* = CEC_DEFAULT_TRANSMIT_TIMEOUT */,
                                     int32_t iTransmitWait /* = CEC_DEFAULT_TRANSMIT_WAIT */,
                                     int8_t iTransmitRetries /* = CEC_DEFAULT_TRANSMIT_RETRIES */,
                                     int64_t iActiveSourcePending /* = 0 */) :
    CCECCommandHandler(busDevice, iTransmitTimeout, iTransmitWait, iTransmitRetries, iActiveSourcePending),
    m_iLastKeyCode(CEC_USER_CONTROL_CODE_UNKNOWN),
    m_powerUpState(PH_POWER_UNKNOWN)
{
  m_imageViewOnCheck = new CImageViewOnCheck(this);
  m_vendorId = CEC_VENDOR_PHILIPS;
}

CPHCommandHandler::~CPHCommandHandler(void)
{
  delete m_imageViewOnCheck;
}

bool CPHCommandHandler::InitHandler(void)
{
  CCECBusDevice *primary = m_processor->GetPrimaryDevice();
  if (primary && primary->GetLogicalAddress() != CECDEVICE_UNREGISTERED)
  {
    /* imitate philips: the vendor id picks the handler for our own device, and is the one
       we announce on the bus */
    if (m_busDevice->GetLogicalAddress() == CECDEVICE_TV &&
        primary && m_busDevice->GetLogicalAddress() != primary->GetLogicalAddress())
    {
      primary->SetVendorId(CEC_VENDOR_PHILIPS);
      primary->ReplaceHandler(false);
    }
  }

  return CCECCommandHandler::InitHandler();
}

bool CPHCommandHandler::ActivateSource(bool bTransmitDelayedCommandsOnly /* = false */)
{

  CCECBusDevice* tv = m_processor->GetDevice(CECDEVICE_TV);
  if (m_busDevice->IsActiveSource() &&
      m_busDevice->IsHandledByLibCEC() &&
      tv && tv->GetCurrentPowerStatus() != CEC_POWER_STATUS_ON &&
      !bTransmitDelayedCommandsOnly)
  {
    // tv sometimes ignores image view on. check the power status of the tv in 5 seconds, and retry when it failed to power up
    if (m_imageViewOnCheck && !m_imageViewOnCheck->IsRunning())
      return m_imageViewOnCheck->CreateThread(false);
  }

  return CCECCommandHandler::ActivateSource(bTransmitDelayedCommandsOnly);
}

int CPHCommandHandler::HandleUserControlPressed(const cec_command& command)
{
  // TV sends key presses without releases when holding a button
  if (m_iLastKeyCode == command.parameters[0])
  {
    // TV keeps sending key presses after pressing the display information key once (arguably another firmware bug)
    // So we only allow holding buttons forwarded from the 'device menu control feature' (see cec specs 1.3a table 27)
    if (m_iLastKeyCode <= CEC_USER_CONTROL_CODE_LEFT_DOWN ||  
        m_iLastKeyCode == CEC_USER_CONTROL_CODE_EXIT || 
       (m_iLastKeyCode >= CEC_USER_CONTROL_CODE_NUMBER0 && m_iLastKeyCode <= CEC_USER_CONTROL_CODE_NUMBER9) || 
       (m_iLastKeyCode >= CEC_USER_CONTROL_CODE_F1_BLUE && m_iLastKeyCode <= CEC_USER_CONTROL_CODE_F5))
    {
      cec_command release;
      release.parameters.size = 0;
      release.opcode = CEC_OPCODE_USER_CONTROL_RELEASE;
      release.initiator = command.initiator;
      release.destination = command.destination;
      CCECCommandHandler::HandleUserControlRelease(release);
    }
    else
    {
      return COMMAND_HANDLED;
    }
  }

  m_iLastKeyCode = command.parameters[0];

  return CCECCommandHandler::HandleUserControlPressed(command);
}

int CPHCommandHandler::HandleUserControlRelease(const cec_command& command)
{
  m_iLastKeyCode = CEC_USER_CONTROL_CODE_UNKNOWN;

  return CCECCommandHandler::HandleUserControlRelease(command);
}

int CPHCommandHandler::HandleDeviceVendorId(const cec_command& command)
{
  m_busDevice->SetPowerStatus(CEC_POWER_STATUS_ON);
  return CCECCommandHandler::HandleDeviceVendorId(command);
}

int CPHCommandHandler::HandleRoutingChange(const cec_command& command)
{
  if (command.parameters.size == 4 &&
      ClaimRouteWhilePoweringUp(command,
                                ((uint16_t)command.parameters[0] << 8) | (uint16_t)command.parameters[1],
                                ((uint16_t)command.parameters[2] << 8) | (uint16_t)command.parameters[3]))
    return COMMAND_HANDLED;

  return CCECCommandHandler::HandleRoutingChange(command);
}

int CPHCommandHandler::HandleSetStreamPath(const cec_command& command)
{
  /* the TV puts the address it routes away from in front of the one it routes to, which a
     '<set stream path>' has no room for */
  if (command.parameters.size >= 4 &&
      ClaimRouteWhilePoweringUp(command,
                                ((uint16_t)command.parameters[0] << 8) | (uint16_t)command.parameters[1],
                                ((uint16_t)command.parameters[2] << 8) | (uint16_t)command.parameters[3]))
    return COMMAND_HANDLED;

  return CCECCommandHandler::HandleSetStreamPath(command);
}

int CPHCommandHandler::HandleStandby(const cec_command& command)
{
  if (command.initiator == CECDEVICE_TV)
    m_powerUpState = PH_POWER_UNKNOWN;

  return CCECCommandHandler::HandleStandby(command);
}

bool CPHCommandHandler::ClaimRouteWhilePoweringUp(const cec_command& command, uint16_t iOldAddress, uint16_t iNewAddress)
{
  if (command.initiator != CECDEVICE_TV || m_powerUpState == PH_POWERED_UP)
    return false;

  if (iOldAddress == CEC_PHYSICAL_ADDRESS_TV && iNewAddress == CEC_PHYSICAL_ADDRESS_TV)
  {
    /* the TV opens its power-up sequence by routing its own address to itself */
    m_powerUpState = PH_POWERING_UP;
    return false;
  }

  if (iOldAddress != CEC_PHYSICAL_ADDRESS_TV || m_powerUpState != PH_POWERING_UP)
  {
    m_powerUpState = PH_POWERED_UP;
    return false;
  }

  /* the TV closes it on the HDMI port it was last on, but picks an address under that port
     that isn't the one it was showing. leave the active source where it is and tell the TV */
  CCECBusDevice* activeSource = m_processor->GetDevices()->GetActiveSource();
  if (!activeSource || !activeSource->IsHandledByLibCEC())
    return false;

  uint16_t iActiveAddress = activeSource->GetCurrentPhysicalAddress();
  if (iActiveAddress == CEC_INVALID_PHYSICAL_ADDRESS ||
      (iActiveAddress & 0xF000) != (iNewAddress & 0xF000))
  {
    m_powerUpState = PH_POWERED_UP;
    return false;
  }

  LIB_CEC->AddLog(CEC_LOG_DEBUG, "the TV routed to %04x while powering up, keeping %s (%X) at %04x as the active source", iNewAddress, activeSource->GetLogicalAddressName(), activeSource->GetLogicalAddress(), iActiveAddress);

  CCECBusDevice* tv = GetDevice(command.initiator);
  if (tv)
    tv->SetPowerStatus(CEC_POWER_STATUS_ON);
  activeSource->TransmitActiveSource(true);

  return true;
}
