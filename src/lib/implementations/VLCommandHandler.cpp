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

#include "VLCommandHandler.h"
#include "../devices/CECBusDevice.h"
#include "../devices/CECPlaybackDevice.h"
#include "../devices/CECTV.h"
#include "../CECProcessor.h"
#include "../LibCEC.h"
#include "../CECClient.h"

#define VL_POWER_CHANGE 0x20
#define VL_POWERED_UP   0x00
#define VL_POWERED_DOWN 0x01
#define VL_UNKNOWN1     0x06

using namespace CEC;
using namespace PLATFORM;

#define LIB_CEC     m_busDevice->GetProcessor()->GetLib()
#define ToString(p) LIB_CEC->ToString(p)

// wait this amount of ms before trying to switch sources after receiving the message from the TV that it's powered on
#define SOURCE_SWITCH_DELAY_MS 1000

CVLCommandHandler::CVLCommandHandler(CCECBusDevice *busDevice) :
    CCECCommandHandler(busDevice),
    m_iPowerUpEventReceived(0)
{
  m_vendorId = CEC_VENDOR_PANASONIC;
}

bool CVLCommandHandler::InitHandler(void)
{
  CCECBusDevice *primary = m_processor->GetPrimaryDevice();
  if (primary && primary->GetLogicalAddress() != CECDEVICE_UNREGISTERED)
  {
    /* use the VL commandhandler for the primary device that is handled by libCEC */
    if (m_busDevice->GetLogicalAddress() == CECDEVICE_TV)
    {
      if (primary && m_busDevice->GetLogicalAddress() != primary->GetLogicalAddress())
      {
        primary->SetVendorId(CEC_VENDOR_PANASONIC);
        primary->ReplaceHandler(false);
      }
    }

    if (primary->GetType() == CEC_DEVICE_TYPE_RECORDING_DEVICE)
      return m_processor->GetPrimaryClient()->ChangeDeviceType(CEC_DEVICE_TYPE_RECORDING_DEVICE, CEC_DEVICE_TYPE_PLAYBACK_DEVICE);

    m_processor->GetTV()->RequestPowerStatus(primary->GetLogicalAddress(), false);
  }

  return CCECCommandHandler::InitHandler();
}

int CVLCommandHandler::HandleDeviceVendorCommandWithId(const cec_command &command)
{
  if (command.parameters[0] != 0x00 ||
      command.parameters[1] != 0x80 ||
      command.parameters[2] != 0x45)
    return CEC_ABORT_REASON_INVALID_OPERAND;

  if (command.initiator == CECDEVICE_TV &&
      command.parameters.At(3) == VL_UNKNOWN1)
  {
    // set the power up event time
    {
      CLockObject lock(m_mutex);
      if (m_iPowerUpEventReceived == 0)
        m_iPowerUpEventReceived = GetTimeMs();
    }
    // mark the TV as powered on
    m_processor->GetTV()->SetPowerStatus(CEC_POWER_STATUS_ON);
  }
  else if (command.initiator == CECDEVICE_TV &&
      command.destination == CECDEVICE_BROADCAST &&
      command.parameters.At(3) == VL_POWER_CHANGE)
  {
    if (command.parameters.At(4) == VL_POWERED_UP)
    {
      // set the power up event time
      {
        CLockObject lock(m_mutex);
        if (m_iPowerUpEventReceived == 0)
          m_iPowerUpEventReceived = GetTimeMs();
      }
      // mark the TV as powered on
      m_processor->GetTV()->SetPowerStatus(CEC_POWER_STATUS_ON);
    }
    else if (command.parameters.At(4) == VL_POWERED_DOWN)
    {
      // reset the power up event time
      {
        CLockObject lock(m_mutex);
        m_iPowerUpEventReceived = 0;
      }
      // mark the TV as powered off
      m_processor->GetTV()->SetPowerStatus(CEC_POWER_STATUS_STANDBY);
    }
    else
      LIB_CEC->AddLog(CEC_LOG_DEBUG, "skipping unknown vendor command");

    return COMMAND_HANDLED;
  }

  return CCECCommandHandler::HandleDeviceVendorCommandWithId(command);
}

bool CVLCommandHandler::PowerUpEventReceived(void)
{
  bool bPowerUpEventReceived(true);

  if (m_busDevice->GetLogicalAddress() != CECDEVICE_TV)
  {
    // get the status from the TV
    CCECBusDevice *tv = m_processor->GetTV();
    if (tv && tv->GetCurrentVendorId() == CEC_VENDOR_PANASONIC)
    {
      CVLCommandHandler *handler = static_cast<CVLCommandHandler *>(tv->GetHandler());
      bPowerUpEventReceived = handler ? handler->PowerUpEventReceived() : false;
      tv->MarkHandlerReady();
    }
  }
  else
  {
    // get the current status
    {
      CLockObject lock(m_mutex);
      bPowerUpEventReceived = m_iPowerUpEventReceived > 0 &&
                              GetTimeMs() - m_iPowerUpEventReceived > SOURCE_SWITCH_DELAY_MS;
    }

    // if we didn't receive the event, check if the TV is already marked as powered on
    if (!bPowerUpEventReceived && m_busDevice->GetCurrentPowerStatus() == CEC_POWER_STATUS_ON)
    {
      CLockObject lock(m_mutex);
      m_iPowerUpEventReceived = GetTimeMs();
      bPowerUpEventReceived = true;
    }
  }

  return bPowerUpEventReceived;
}

int CVLCommandHandler::HandleStandby(const cec_command &command)
{
  // reset the power up event time
  {
    CLockObject lock(m_mutex);
    m_iPowerUpEventReceived = 0;
  }

  return CCECCommandHandler::HandleStandby(command);
}

int CVLCommandHandler::HandleVendorCommand(const cec_command &command)
{
  // some vendor command voodoo that will enable more buttons on the remote
  if (command.parameters.size == 3 &&
      command.parameters[0] == 0x10 &&
      command.parameters[1] == 0x01 &&
      command.parameters[2] == 0x05)
  {
    cec_command response;
    cec_command::Format(response, command.destination, command.initiator, CEC_OPCODE_VENDOR_COMMAND);
    uint8_t iResponseData[] = {0x10, 0x02, 0xFF, 0xFF, 0x00, 0x05, 0x05, 0x45, 0x55, 0x5c, 0x58, 0x32};
    response.PushArray(12, iResponseData);

    Transmit(response, true);

    return COMMAND_HANDLED;
  }

  return CEC_ABORT_REASON_INVALID_OPERAND;
}

bool CVLCommandHandler::SourceSwitchAllowed(void)
{
  return PowerUpEventReceived();
}

int CVLCommandHandler::HandleSystemAudioModeRequest(const cec_command &command)
{
  if (command.initiator == CECDEVICE_TV)
  {
    // set the power up event time
    {
      CLockObject lock(m_mutex);
      if (m_iPowerUpEventReceived == 0)
        m_iPowerUpEventReceived = GetTimeMs();
    }
    // mark the TV as powered on
    m_processor->GetTV()->SetPowerStatus(CEC_POWER_STATUS_ON);
  }

  return CCECCommandHandler::HandleSystemAudioModeRequest(command);
}
