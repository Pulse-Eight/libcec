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

#define SL_COMMAND_UNKNOWN_01        0x01
#define SL_COMMAND_UNKNOWN_02        0x02
#define SL_COMMAND_UNKNOWN_03        0x05

#define SL_COMMAND_REQUEST_VENDOR_ID 0xa0
#define SL_COMMAND_CONNECT_REQUEST   0x04
#define SL_COMMAND_CONNECT_ACCEPT    0x05

CSLCommandHandler::CSLCommandHandler(CCECBusDevice *busDevice) :
    CCECCommandHandler(busDevice),
    m_bAwaitingReceiveFailed(false),
    m_bSLEnabled(false),
    m_bSkipNextVendorId(false)
{
}

bool CSLCommandHandler::HandleVendorCommand(const cec_command &command)
{
  if (command.parameters.size == 1 &&
      command.parameters[0] == SL_COMMAND_UNKNOWN_01)
  {
    /* enable SL */
    cec_command response;
    cec_command::Format(response, command.destination, command.initiator, CEC_OPCODE_VENDOR_COMMAND, m_iTransmitTimeout);
    response.PushBack(SL_COMMAND_UNKNOWN_02);
    response.PushBack(SL_COMMAND_UNKNOWN_03);

    Transmit(response);
    return true;
  }
  else if (command.parameters.size == 2 &&
      command.parameters[0] == SL_COMMAND_CONNECT_REQUEST)
  {
    /* enable SL */
    cec_command response;
    cec_command::Format(response, command.destination, command.initiator, CEC_OPCODE_VENDOR_COMMAND, m_iTransmitTimeout);
    response.PushBack(SL_COMMAND_CONNECT_ACCEPT);
    response.PushBack(m_busDevice->GetProcessor()->GetLogicalAddresses().primary);
    Transmit(response);

    /* set deck status for the playback device */
    TransmitDeckStatus(command.initiator);
    return true;
  }
  else if (command.parameters.size == 1 &&
      command.parameters[0] == SL_COMMAND_REQUEST_VENDOR_ID)
  {
//    if (command.destination != CECDEVICE_BROADCAST)
//      m_busDevice->GetProcessor()->m_busDevices[m_busDevice->GetProcessor()->GetLogicalAddresses().primary]->TransmitPowerState(command.initiator);
    return true;
  }

  return false;
}

void CSLCommandHandler::TransmitDeckStatus(const cec_logical_address iDestination)
{
  /* set deck status for the playback device */
  CCECBusDevice *primary = m_busDevice->GetProcessor()->m_busDevices[m_busDevice->GetProcessor()->GetLogicalAddresses().primary];
  if (primary->GetType() == CEC_DEVICE_TYPE_PLAYBACK_DEVICE || primary->GetType() == CEC_DEVICE_TYPE_RECORDING_DEVICE)
  {
    ((CCECPlaybackDevice *)primary)->SetDeckStatus(CEC_DECK_INFO_OTHER_STATUS_LG);
    ((CCECPlaybackDevice *)primary)->TransmitDeckStatus(iDestination);
  }
}

bool CSLCommandHandler::TransmitLGVendorId(const cec_logical_address iInitiator, const cec_logical_address iDestination)
{
  cec_command response;
  cec_command::Format(response, iInitiator, iDestination, CEC_OPCODE_DEVICE_VENDOR_ID, m_iTransmitTimeout);
  response.parameters.PushBack((uint8_t) (((uint64_t)CEC_VENDOR_LG >> 16) & 0xFF));
  response.parameters.PushBack((uint8_t) (((uint64_t)CEC_VENDOR_LG >> 8) & 0xFF));
  response.parameters.PushBack((uint8_t) ((uint64_t)CEC_VENDOR_LG & 0xFF));

  Transmit(response);
  return true;
}

bool CSLCommandHandler::HandleGiveDeviceVendorId(const cec_command &command)
{
  /* imitate LG devices */
  CCECBusDevice *device = GetDevice(command.destination);
  if (device)
    device->SetVendorId(CEC_VENDOR_LG);

  return CCECCommandHandler::HandleGiveDeviceVendorId(command);
}

bool CSLCommandHandler::HandleCommand(const cec_command &command)
{
  bool bHandled(false);
  if (m_busDevice->MyLogicalAddressContains(command.destination) ||
      command.destination == CECDEVICE_BROADCAST)
  {
    switch(command.opcode)
    {
    case CEC_OPCODE_VENDOR_COMMAND:
      bHandled = HandleVendorCommand(command);
      break;
    case CEC_OPCODE_GIVE_DECK_STATUS:
      {
        if (command.parameters.size == 1)
        {
          if (command.parameters[0] == CEC_STATUS_REQUEST_ONCE ||
              command.parameters[0] == CEC_STATUS_REQUEST_ON)
            TransmitDeckStatus(command.initiator);
          else
            CCECCommandHandler::HandleCommand(command);
        }
        bHandled = true;
      }
      break;
    case CEC_OPCODE_FEATURE_ABORT:
      {
        if (!m_bSkipNextVendorId)
        {
          m_bSkipNextVendorId = true;
          TransmitLGVendorId(m_busDevice->GetProcessor()->GetLogicalAddresses().primary, CECDEVICE_BROADCAST);
        }
        else
        {
          m_bSkipNextVendorId = false;
        }
        m_bSLEnabled = false;
      }
      bHandled = true;
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
  if (m_bSLEnabled)
    return true;
  m_bSLEnabled = true;

  m_busDevice->GetProcessor()->SetStandardLineTimeout(3);
  m_busDevice->GetProcessor()->SetRetryLineTimeout(3);

  CCECBusDevice *primary = m_busDevice->GetProcessor()->m_busDevices[m_busDevice->GetProcessor()->GetLogicalAddresses().primary];
  if (m_busDevice->GetLogicalAddress() != primary->GetLogicalAddress())
    primary->SetVendorId(CEC_VENDOR_LG, false);

  if (m_busDevice->GetLogicalAddress() == CECDEVICE_TV)
  {
    primary->TransmitVendorID(CECDEVICE_TV);
    primary->TransmitPhysicalAddress();

    cec_command command;
    cec_command::Format(command, m_busDevice->GetProcessor()->GetLogicalAddresses().primary, CECDEVICE_TV, CEC_OPCODE_GIVE_DEVICE_VENDOR_ID, m_iTransmitTimeout);
    Transmit(command);

    /* LG TVs don't always reply to CEC version requests, so just set it to 1.3a */
    m_busDevice->SetCecVersion(CEC_VERSION_1_3A);
  }

  /* LG devices always return "korean" as language */
  cec_menu_language lang;
  lang.device = m_busDevice->GetLogicalAddress();
  snprintf(lang.language, 4, "eng");
  m_busDevice->SetMenuLanguage(lang);

  /* increase the transmit timeout because the tv is keeping the bus busy at times */
  m_iTransmitTimeout = 5000;

  if (m_busDevice->GetLogicalAddress() == CECDEVICE_TV)
  {
    /* LG TVs only route keypresses when the deck status is set to 0x20 */
    cec_logical_addresses addr = m_busDevice->GetProcessor()->GetLogicalAddresses();
    for (uint8_t iPtr = 0; iPtr < 15; iPtr++)
    {
      CCECBusDevice *device = m_busDevice->GetProcessor()->m_busDevices[iPtr];

      if (addr[iPtr])
      {
        if (device && (device->GetType() == CEC_DEVICE_TYPE_PLAYBACK_DEVICE ||
                       device->GetType() == CEC_DEVICE_TYPE_RECORDING_DEVICE))
        {
          ((CCECPlaybackDevice *)device)->SetDeckStatus(CEC_DECK_INFO_OTHER_STATUS_LG);
          TransmitDeckStatus(CECDEVICE_TV);
        }
      }
    }
  }

  return true;
}
