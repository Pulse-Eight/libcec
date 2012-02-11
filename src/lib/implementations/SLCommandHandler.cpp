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

#include "SLCommandHandler.h"
#include "../devices/CECBusDevice.h"
#include "../devices/CECPlaybackDevice.h"
#include "../CECProcessor.h"

using namespace CEC;

#define SL_COMMAND_UNKNOWN_01           0x01
#define SL_COMMAND_UNKNOWN_02           0x02
#define SL_COMMAND_UNKNOWN_03           0x05

#define SL_COMMAND_REQUEST_POWER_STATUS 0xa0
#define SL_COMMAND_POWER_ON             0x03
#define SL_COMMAND_CONNECT_REQUEST      0x04
#define SL_COMMAND_CONNECT_ACCEPT       0x05

CSLCommandHandler::CSLCommandHandler(CCECBusDevice *busDevice) :
    CCECCommandHandler(busDevice),
    m_bSLEnabled(false),
    m_bPowerStateReset(false)
{
  m_vendorId = CEC_VENDOR_LG;
  CCECBusDevice *primary = m_processor->GetPrimaryDevice();

  /* imitate LG devices */
  if (primary && m_busDevice->GetLogicalAddress() != primary->GetLogicalAddress())
    primary->SetVendorId(CEC_VENDOR_LG);
  SetLGDeckStatus();

  /* LG TVs don't always reply to CEC version requests, so just set it to 1.3a */
  if (m_busDevice->GetLogicalAddress() == CECDEVICE_TV)
    m_busDevice->SetCecVersion(CEC_VERSION_1_3A);

  /* LG devices always return "korean" as language */
  cec_menu_language lang;
  lang.device = m_busDevice->GetLogicalAddress();
  snprintf(lang.language, 4, "eng");
  m_busDevice->SetMenuLanguage(lang);
}

bool CSLCommandHandler::InitHandler(void)
{
  if (m_bHandlerInited)
    return true;
  m_bHandlerInited = true;

  /* reply with LGs vendor id */
  CCECBusDevice *primary = m_processor->GetPrimaryDevice();
  if (m_busDevice->GetLogicalAddress() != primary->GetLogicalAddress())
    primary->TransmitVendorID(CECDEVICE_TV, false);

  primary->SetPowerStatus(CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON);
  return true;
}

bool CSLCommandHandler::ActivateSource(void)
{
  CCECBusDevice *primary = m_processor->GetPrimaryDevice();
  primary->SetActiveSource();
  primary->TransmitActiveSource();
  return true;
}

bool CSLCommandHandler::HandleActiveSource(const cec_command &command)
{
  if (command.parameters.size == 2)
  {
    uint16_t iAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
    if (iAddress != m_busDevice->GetPhysicalAddress(false))
      m_bSLEnabled = false;
    return m_processor->SetActiveSource(iAddress);
  }

  return true;
}

bool CSLCommandHandler::HandleDeviceVendorId(const cec_command &command)
{
  SetVendorId(command);

  cec_command response;
  cec_command::Format(response, m_processor->GetLogicalAddress(), command.initiator, CEC_OPCODE_FEATURE_ABORT);
  return Transmit(response);
}

bool CSLCommandHandler::HandleFeatureAbort(const cec_command &command)
{
  CCECBusDevice *primary = m_processor->GetPrimaryDevice();
  if (primary->GetPowerStatus(false) == CEC_POWER_STATUS_ON && !m_bPowerStateReset && !m_bSLEnabled)
  {
    m_bPowerStateReset = true;
    primary->SetPowerStatus(CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON);
  }

  return CCECCommandHandler::HandleFeatureAbort(command);
}

bool CSLCommandHandler::HandleGivePhysicalAddress(const cec_command &command)
{
  if (m_processor->IsRunning() && m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device)
      return device->TransmitPhysicalAddress();
  }

  return false;
}

bool CSLCommandHandler::HandleVendorCommand(const cec_command &command)
{
  if (command.parameters.size == 1 &&
      command.parameters[0] == SL_COMMAND_UNKNOWN_01)
  {
    HandleVendorCommand01(command);
    return true;
  }
  else if (command.parameters.size == 2 &&
      command.parameters[0] == SL_COMMAND_POWER_ON)
  {
    HandleVendorCommandPowerOn(command);
    return true;
  }
  else if (command.parameters.size == 2 &&
      command.parameters[0] == SL_COMMAND_CONNECT_REQUEST)
  {
    HandleVendorCommandSLConnect(command);
    return true;
  }
  else if (command.parameters.size == 1 &&
      command.parameters[0] == SL_COMMAND_REQUEST_POWER_STATUS)
  {
    HandleVendorCommandPowerOnStatus(command);
    return true;
  }

  return false;
}

void CSLCommandHandler::HandleVendorCommand01(const cec_command &command)
{
  TransmitVendorCommand0205(command.destination, command.initiator);
}

void CSLCommandHandler::TransmitVendorCommand0205(const cec_logical_address iSource, const cec_logical_address iDestination)
{
  cec_command response;
  cec_command::Format(response, iSource, iDestination, CEC_OPCODE_VENDOR_COMMAND);
  response.PushBack(SL_COMMAND_UNKNOWN_02);
  response.PushBack(SL_COMMAND_UNKNOWN_03);

  Transmit(response, false);
}

void CSLCommandHandler::HandleVendorCommandPowerOn(const cec_command &command)
{
  CCECBusDevice *device = m_processor->GetPrimaryDevice();
  if (device)
  {
    m_bSLEnabled = true;

    device->SetPowerStatus(CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON); //XXX
    device->TransmitPowerState(command.initiator);
    device->SetPowerStatus(CEC_POWER_STATUS_ON);

    SetLGDeckStatus();
    device->SetActiveSource();
    TransmitImageViewOn(device->GetLogicalAddress(), command.initiator);
  }
}
void CSLCommandHandler::HandleVendorCommandPowerOnStatus(const cec_command &command)
{
  if (command.destination != CECDEVICE_BROADCAST)
  {
    CCECBusDevice *device = m_processor->m_busDevices[m_processor->GetLogicalAddresses().primary];
    device->SetPowerStatus(CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON);
    device->TransmitPowerState(command.initiator);
    device->SetPowerStatus(CEC_POWER_STATUS_ON);
  }
}

void CSLCommandHandler::HandleVendorCommandSLConnect(const cec_command &command)
{
  m_bSLEnabled = true;
  SetLGDeckStatus();

  CCECBusDevice *primary = m_processor->GetPrimaryDevice();

  primary->SetActiveSource();
  TransmitImageViewOn(primary->GetLogicalAddress(), command.initiator);
  TransmitVendorCommand05(primary->GetLogicalAddress(), command.initiator);
}

void CSLCommandHandler::TransmitVendorCommand05(const cec_logical_address iSource, const cec_logical_address iDestination)
{
  cec_command response;
  cec_command::Format(response, iSource, iDestination, CEC_OPCODE_VENDOR_COMMAND);
  response.PushBack(SL_COMMAND_CONNECT_ACCEPT);
  response.PushBack((uint8_t)m_processor->m_busDevices[iSource]->GetType());
  Transmit(response, false);
}

void CSLCommandHandler::SetLGDeckStatus(void)
{
  /* LG TVs only route keypresses when the deck status is set to 0x20 */
  CCECBusDevice *device = m_processor->GetDeviceByType(CEC_DEVICE_TYPE_PLAYBACK_DEVICE);
  if (device)
    ((CCECPlaybackDevice *)device)->SetDeckStatus(CEC_DECK_INFO_OTHER_STATUS_LG);

  device = m_processor->GetDeviceByType(CEC_DEVICE_TYPE_RECORDING_DEVICE);
  if (device)
    ((CCECPlaybackDevice *)device)->SetDeckStatus(CEC_DECK_INFO_OTHER_STATUS_LG);
}
