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
#include "../CECProcessor.h"
#include "../LibCEC.h"

#define VL_POWER_CHANGE 0x20
#define VL_POWERED_UP   0x00
#define VL_POWERED_DOWN 0x01

using namespace CEC;
using namespace PLATFORM;

CVLCommandHandler::CVLCommandHandler(CCECBusDevice *busDevice) :
    CCECCommandHandler(busDevice),
    m_bActiveSourcePending(false),
    m_bPowerUpEventReceived(false)
{
  m_vendorId = CEC_VENDOR_PANASONIC;

  /* use the VL commandhandler for the primary device that is handled by libCEC */
  if (busDevice->GetLogicalAddress() == CECDEVICE_TV)
  {
    CCECBusDevice *primary = m_processor->GetPrimaryDevice();
    if (primary && m_busDevice->GetLogicalAddress() != primary->GetLogicalAddress())
    {
      primary->SetVendorId(CEC_VENDOR_PANASONIC);
      primary->ReplaceHandler(false);
    }
  }
}

bool CVLCommandHandler::InitHandler(void)
{
  CCECBusDevice *primary = m_processor->GetPrimaryDevice();
  if (primary->GetType() == CEC_DEVICE_TYPE_RECORDING_DEVICE)
    return m_processor->ChangeDeviceType(CEC_DEVICE_TYPE_RECORDING_DEVICE, CEC_DEVICE_TYPE_PLAYBACK_DEVICE);

  return CCECCommandHandler::InitHandler();
}

bool CVLCommandHandler::HandleDeviceVendorCommandWithId(const cec_command &command)
{
  if (command.initiator == CECDEVICE_TV &&
      command.destination == CECDEVICE_BROADCAST &&
      command.parameters.At(3) == VL_POWER_CHANGE)
  {
    if (command.parameters.At(4) == VL_POWERED_UP)
    {
      CLibCEC::AddLog(CEC_LOG_DEBUG, "TV powered up");
      {
        CLockObject lock(m_mutex);
        m_bPowerUpEventReceived = true;
      }
      m_processor->TransmitPendingActiveSourceCommands();
    }
    else if (command.parameters.At(4) == VL_POWERED_DOWN)
      CLibCEC::AddLog(CEC_LOG_DEBUG, "TV powered down");
    else if (command.parameters.At(4) == VL_POWERED_DOWN)
      CLibCEC::AddLog(CEC_LOG_DEBUG, "unknown vendor command");

    return true;
  }

  return CCECCommandHandler::HandleDeviceVendorCommandWithId(command);
}

bool CVLCommandHandler::TransmitActiveSource(const cec_logical_address iInitiator, uint16_t iPhysicalAddress)
{
  bool bPowerUpEventReceived(false);

  CCECBusDevice *tv = m_processor->m_busDevices[CECDEVICE_TV];
  if (tv && tv->GetVendorId(false) == CEC_VENDOR_PANASONIC)
  {
    CVLCommandHandler *handler = static_cast<CVLCommandHandler *>(tv->GetHandler());
    bPowerUpEventReceived = handler ? handler->PowerUpEventReceived() : false;
  }

  if (!bPowerUpEventReceived)
  {
    CLockObject lock(m_mutex);
    // wait until we received the event
    m_bActiveSourcePending = true;
    return true;
  }
  else
  {
    // transmit standard active source message
    return CCECCommandHandler::TransmitActiveSource(iInitiator, iPhysicalAddress);
  }
}

bool CVLCommandHandler::TransmitPendingActiveSourceCommands(void)
{
  bool bTransmitCommand(false);
  {
    CLockObject lock(m_mutex);
    bTransmitCommand = m_bActiveSourcePending;
    m_bActiveSourcePending = false;
  }

  if (bTransmitCommand)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "transmitting delayed activate source command");
    return CCECCommandHandler::TransmitActiveSource(m_busDevice->GetLogicalAddress(), m_busDevice->GetPhysicalAddress());
  }
  return true;
}

bool CVLCommandHandler::PowerUpEventReceived(void)
{
  {
    CLockObject lock(m_mutex);
    if (m_bPowerUpEventReceived)
      return true;
  }

  cec_power_status powerStatus = m_busDevice->GetPowerStatus();

  CLockObject lock(m_mutex);
  m_bPowerUpEventReceived = (powerStatus == CEC_POWER_STATUS_ON);
  return m_bPowerUpEventReceived;
}
