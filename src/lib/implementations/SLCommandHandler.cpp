/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011 Pulse-Eight Limited.  All rights reserved.
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
#include "../platform/timeutils.h"

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
    m_bAwaitingReceiveFailed(false),
    m_bSLEnabled(false),
    m_bVendorIdSent(false)
{
  CCECBusDevice *primary = m_processor->GetPrimaryDevice();

  /* imitate LG devices */
  if (m_busDevice->GetLogicalAddress() != primary->GetLogicalAddress())
    primary->SetVendorId(CEC_VENDOR_LG, false);

  /* LG TVs don't always reply to CEC version requests, so just set it to 1.3a */
  if (m_busDevice->GetLogicalAddress() == CECDEVICE_TV)
    m_busDevice->SetCecVersion(CEC_VERSION_1_3A);

  /* LG devices always return "korean" as language */
  cec_menu_language lang;
  lang.device = m_busDevice->GetLogicalAddress();
  snprintf(lang.language, 4, "eng");
  m_busDevice->SetMenuLanguage(lang);
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

void CSLCommandHandler::TransmitVendorCommand05(const cec_logical_address iSource, const cec_logical_address iDestination)
{
  m_bSLEnabled = true;
  cec_command response;
  cec_command::Format(response, iSource, iDestination, CEC_OPCODE_VENDOR_COMMAND);
  response.PushBack(SL_COMMAND_CONNECT_ACCEPT);
  response.PushBack((uint8_t)iSource);
  Transmit(response, false);
}

void CSLCommandHandler::HandleVendorCommandPowerOn(const cec_command &command)
{
  CCECBusDevice *device = m_processor->GetPrimaryDevice();
  if (device)
  {
    m_bSLEnabled = true;
    SetLGDeckStatus();
    device->SetPowerStatus(CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON);
    device->TransmitPowerState(command.initiator);
    device->TransmitVendorID(command.initiator);
    TransmitPowerOn(device->GetLogicalAddress(), command.initiator);
  }
}

void CSLCommandHandler::HandleVendorCommandSLConnect(const cec_command &command)
{
  m_bSLEnabled = true;
  m_processor->m_busDevices[command.initiator]->SetActiveSource();
  m_processor->m_busDevices[command.destination]->TransmitActiveSource();
  TransmitVendorCommand05(command.destination, command.initiator);
  SetLGDeckStatus();
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

bool CSLCommandHandler::TransmitLGVendorId(const cec_logical_address iInitiator, const cec_logical_address iDestination)
{
  cec_command response;
  cec_command::Format(response, iInitiator, iDestination, CEC_OPCODE_DEVICE_VENDOR_ID);
  response.parameters.PushBack((uint8_t) (((uint64_t)CEC_VENDOR_LG >> 16) & 0xFF));
  response.parameters.PushBack((uint8_t) (((uint64_t)CEC_VENDOR_LG >> 8) & 0xFF));
  response.parameters.PushBack((uint8_t) ((uint64_t)CEC_VENDOR_LG & 0xFF));

  Transmit(response, false);

  cec_command::Format(response, iInitiator, iDestination, CEC_OPCODE_GIVE_DEVICE_VENDOR_ID);
  Transmit(response);

  return true;
}

bool CSLCommandHandler::HandleCommand(const cec_command &command)
{
  bool bHandled(false);

  if (m_processor->IsStarted() && (m_busDevice->MyLogicalAddressContains(command.destination) ||
      command.destination == CECDEVICE_BROADCAST))
  {
    switch(command.opcode)
    {
    case CEC_OPCODE_VENDOR_COMMAND:
      bHandled = HandleVendorCommand(command);
      break;
    case CEC_OPCODE_FEATURE_ABORT:
      {
        if (!m_bVendorIdSent)
        {
          m_bVendorIdSent = true;
          TransmitLGVendorId(m_processor->GetLogicalAddresses().primary, CECDEVICE_BROADCAST);
          m_processor->GetPrimaryDevice()->SetPowerStatus(CEC_POWER_STATUS_STANDBY);
        }
      }
      bHandled = true;
      break;
    default:
      break;
    }
  }

  if (!bHandled)
    bHandled = CCECCommandHandler::HandleCommand(command);

  return bHandled;
}

void CSLCommandHandler::HandlePoll(const cec_logical_address iInitiator, const cec_logical_address iDestination)
{
  CCECCommandHandler::HandlePoll(iInitiator, iDestination);
  m_bAwaitingReceiveFailed = true;
}

bool CSLCommandHandler::HandleReceiveFailed(void)
{
  if (m_bAwaitingReceiveFailed)
  {
    m_bAwaitingReceiveFailed = false;
    return false;
  }

  return true;
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

  return true;
}

bool CSLCommandHandler::TransmitPowerOn(const cec_logical_address iInitiator, const cec_logical_address iDestination)
{
  if (iDestination != CECDEVICE_BROADCAST &&
      iDestination != CECDEVICE_TV &&
      m_processor->m_busDevices[iDestination]->GetVendorId(false) == CEC_VENDOR_LG)
  {
    cec_command command;
    cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_VENDOR_COMMAND);
    command.parameters.PushBack((uint8_t)SL_COMMAND_POWER_ON);
    command.parameters.PushBack(0x00);
    return Transmit(command, false);
  }

  return CCECCommandHandler::TransmitImageViewOn(iInitiator, iDestination);
}

bool CSLCommandHandler::HandleActiveSource(const cec_command &command)
{
  if (command.parameters.size == 2)
  {
    uint16_t iAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
    if (iAddress == 0)
      return m_processor->GetPrimaryDevice()->TransmitPhysicalAddress();
    else
      return m_processor->SetActiveSource(iAddress);
  }

  return true;
}
