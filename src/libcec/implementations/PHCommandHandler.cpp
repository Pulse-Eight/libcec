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
#include "PHCommandHandler.h"

#include "devices/CECBusDevice.h"
#include "CECProcessor.h"
#include "LibCEC.h"
#include "CECClient.h"

using namespace CEC;
using namespace P8PLATFORM;

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
    m_iLastKeyCode(CEC_USER_CONTROL_CODE_UNKNOWN)
{
  m_imageViewOnCheck = new CImageViewOnCheck(this);
  m_vendorId = CEC_VENDOR_PHILIPS;
  m_bOPTSendDeckStatusUpdateOnActiveSource = false;
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
    //XXX hack to use this handler for the primary device
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

bool CPHCommandHandler::TransmitVendorID(const cec_logical_address iInitiator, const cec_logical_address iDestination, uint64_t UNUSED(iVendorId), bool bIsReply)
{
  // XXX hack around the hack in CPHCommandHandler::InitHandler
  return CCECCommandHandler::TransmitVendorID(iInitiator, iDestination, CEC_VENDOR_PULSE_EIGHT, bIsReply);
}
