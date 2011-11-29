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

#include "CECCommandHandler.h"
#include "../devices/CECBusDevice.h"
#include "../devices/CECAudioSystem.h"
#include "../devices/CECPlaybackDevice.h"
#include "../CECProcessor.h"

using namespace CEC;
using namespace std;

CCECCommandHandler::CCECCommandHandler(CCECBusDevice *busDevice)
{
  m_busDevice = busDevice;
}

bool CCECCommandHandler::HandleCommand(const cec_command &command)
{
  bool bHandled(true);

  CStdString strLog;
  strLog.Format(">> %s (%X) -> %s (%X): %s (%2X)", m_busDevice->GetProcessor()->ToString(command.initiator), command.initiator, m_busDevice->GetProcessor()->ToString(command.destination), command.destination, m_busDevice->GetProcessor()->ToString(command.opcode), command.opcode);
  m_busDevice->AddLog(CEC_LOG_NOTICE, strLog);

  switch(command.opcode)
  {
  case CEC_OPCODE_REPORT_POWER_STATUS:
    HandleReportPowerStatus(command);
    break;
  case CEC_OPCODE_CEC_VERSION:
    HandleDeviceCecVersion(command);
    break;
  case CEC_OPCODE_SET_MENU_LANGUAGE:
    HandleSetMenuLanguage(command);
    break;
  case CEC_OPCODE_GIVE_PHYSICAL_ADDRESS:
    HandleGivePhysicalAddress(command);
    break;
  case CEC_OPCODE_GIVE_OSD_NAME:
     HandleGiveOSDName(command);
    break;
  case CEC_OPCODE_GIVE_DEVICE_VENDOR_ID:
    HandleGiveDeviceVendorId(command);
    break;
  case CEC_OPCODE_DEVICE_VENDOR_ID:
    HandleDeviceVendorId(command);
    break;
  case CEC_OPCODE_VENDOR_COMMAND_WITH_ID:
    HandleDeviceVendorCommandWithId(command);
    break;
  case CEC_OPCODE_GIVE_DECK_STATUS:
    HandleGiveDeckStatus(command);
    break;
  case CEC_OPCODE_DECK_CONTROL:
    HandleDeckControl(command);
    break;
  case CEC_OPCODE_MENU_REQUEST:
    HandleMenuRequest(command);
    break;
  case CEC_OPCODE_GIVE_DEVICE_POWER_STATUS:
    HandleGiveDevicePowerStatus(command);
    break;
  case CEC_OPCODE_GET_CEC_VERSION:
    HandleGetCecVersion(command);
    break;
  case CEC_OPCODE_USER_CONTROL_PRESSED:
    HandleUserControlPressed(command);
    break;
  case CEC_OPCODE_USER_CONTROL_RELEASE:
    HandleUserControlRelease(command);
    break;
  case CEC_OPCODE_GIVE_AUDIO_STATUS:
    HandleGiveAudioStatus(command);
    break;
  case CEC_OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS:
    HandleGiveSystemAudioModeStatus(command);
    break;
  case CEC_OPCODE_SYSTEM_AUDIO_MODE_REQUEST:
    HandleSetSystemAudioModeRequest(command);
    break;
  case CEC_OPCODE_REQUEST_ACTIVE_SOURCE:
    HandleRequestActiveSource(command);
    break;
  case CEC_OPCODE_SET_STREAM_PATH:
    HandleSetStreamPath(command);
    break;
  case CEC_OPCODE_ROUTING_CHANGE:
    HandleRoutingChange(command);
    break;
  case CEC_OPCODE_ROUTING_INFORMATION:
    HandleRoutingInformation(command);
    break;
  case CEC_OPCODE_STANDBY:
    HandleStandby(command);
    break;
  case CEC_OPCODE_ACTIVE_SOURCE:
    HandleActiveSource(command);
    break;
  case CEC_OPCODE_REPORT_PHYSICAL_ADDRESS:
    HandleReportPhysicalAddress(command);
    break;
  case CEC_OPCODE_REPORT_AUDIO_STATUS:
    HandleReportAudioStatus(command);
    break;
  case CEC_OPCODE_SYSTEM_AUDIO_MODE_STATUS:
    HandleSystemAudioStatus(command);
    break;
  case CEC_OPCODE_SET_OSD_NAME:
    HandleSetOSDName(command);
    break;
  case CEC_OPCODE_IMAGE_VIEW_ON:
    HandleImageViewOn(command);
    break;
  case CEC_OPCODE_TEXT_VIEW_ON:
    HandleTextViewOn(command);
    break;
  default:
    UnhandledCommand(command);
    bHandled = false;
    break;
  }

  m_busDevice->GetProcessor()->AddCommand(command);
  return bHandled;
}

bool CCECCommandHandler::HandleActiveSource(const cec_command &command)
{
  if (command.parameters.size == 2)
  {
    uint16_t iAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
    return m_busDevice->GetProcessor()->SetStreamPath(iAddress);
  }

  return true;
}

bool CCECCommandHandler::HandleDeckControl(const cec_command &command)
{
  CCECBusDevice *device = GetDevice(command.destination);
  if (device && (device->GetType() == CEC_DEVICE_TYPE_PLAYBACK_DEVICE || device->GetType() == CEC_DEVICE_TYPE_RECORDING_DEVICE) && command.parameters.size > 0)
  {
    ((CCECPlaybackDevice *) device)->SetDeckControlMode((cec_deck_control_mode) command.parameters[0]);
    return true;
  }

  return false;
}

bool CCECCommandHandler::HandleDeviceCecVersion(const cec_command &command)
{
  if (command.parameters.size == 1)
  {
    CCECBusDevice *device = GetDevice(command.initiator);
    if (device)
      device->SetCecVersion((cec_version) command.parameters[0]);
  }

  return true;
}

bool CCECCommandHandler::HandleDeviceVendorCommandWithId(const cec_command &command)
{
  if (m_busDevice->MyLogicalAddressContains(command.destination))
    m_busDevice->GetProcessor()->TransmitAbort(command.initiator, command.opcode, CEC_ABORT_REASON_REFUSED);

  return true;
}

bool CCECCommandHandler::HandleDeviceVendorId(const cec_command &command)
{
  SetVendorId(command);
  return true;
}

bool CCECCommandHandler::HandleGetCecVersion(const cec_command &command)
{
  if (m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device)
      return device->TransmitCECVersion(command.initiator);
  }

  return false;
}

bool CCECCommandHandler::HandleGiveAudioStatus(const cec_command &command)
{
  if (m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device && device->GetType() == CEC_DEVICE_TYPE_AUDIO_SYSTEM)
      return ((CCECAudioSystem *) device)->TransmitAudioStatus(command.initiator);
  }

  return false;
}

bool CCECCommandHandler::HandleGiveDeckStatus(const cec_command &command)
{
  if (m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device && (device->GetType() == CEC_DEVICE_TYPE_PLAYBACK_DEVICE || device->GetType() == CEC_DEVICE_TYPE_RECORDING_DEVICE))
      return ((CCECPlaybackDevice *) device)->TransmitDeckStatus(command.initiator);
  }

  return false;
}

bool CCECCommandHandler::HandleGiveDevicePowerStatus(const cec_command &command)
{
  if (m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device)
      return device->TransmitPowerState(command.initiator);
  }

  return false;
}

bool CCECCommandHandler::HandleGiveDeviceVendorId(const cec_command &command)
{
  if (m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device)
      return device->TransmitVendorID(command.initiator);
  }

  return false;
}

bool CCECCommandHandler::HandleGiveOSDName(const cec_command &command)
{
  if (m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device)
      return device->TransmitOSDName(command.initiator);
  }

  return false;
}

bool CCECCommandHandler::HandleGivePhysicalAddress(const cec_command &command)
{
  if (m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device)
      return device->TransmitPhysicalAddress();
  }

  return false;
}

bool CCECCommandHandler::HandleGiveSystemAudioModeStatus(const cec_command &command)
{
  if (m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device && device->GetType() == CEC_DEVICE_TYPE_AUDIO_SYSTEM)
      return ((CCECAudioSystem *) device)->TransmitSystemAudioModeStatus(command.initiator);
  }

  return false;
}

bool CCECCommandHandler::HandleImageViewOn(const cec_command &command)
{
  m_busDevice->GetProcessor()->SetActiveSource(command.initiator);
  return true;
}

bool CCECCommandHandler::HandleMenuRequest(const cec_command &command)
{
  if (m_busDevice->MyLogicalAddressContains(command.destination))
  {
    if (command.parameters[0] == CEC_MENU_REQUEST_TYPE_QUERY)
    {
      CCECBusDevice *device = GetDevice(command.destination);
      if (device)
        return device->TransmitMenuState(command.initiator);
    }
  }

  return false;
}

bool CCECCommandHandler::HandleReportAudioStatus(const cec_command &command)
{
  if (command.parameters.size == 1)
  {
    CCECBusDevice *device = GetDevice(command.initiator);
    if (device && device->GetType() == CEC_DEVICE_TYPE_AUDIO_SYSTEM)
    {
      ((CCECAudioSystem *)device)->SetAudioStatus(command.parameters[0]);
      return true;
    }
  }
  return false;
}

bool CCECCommandHandler::HandleReportPhysicalAddress(const cec_command &command)
{
  if (command.parameters.size == 3)
  {
    uint16_t iNewAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
    SetPhysicalAddress(command.initiator, iNewAddress);
  }
  return true;
}

bool CCECCommandHandler::HandleReportPowerStatus(const cec_command &command)
{
  if (command.parameters.size == 1)
  {
    CCECBusDevice *device = GetDevice(command.initiator);
    if (device)
      device->SetPowerStatus((cec_power_status) command.parameters[0]);
  }
  return true;
}

bool CCECCommandHandler::HandleRequestActiveSource(const cec_command &command)
{
  CStdString strLog;
  strLog.Format(">> %i requests active source", (uint8_t) command.initiator);
  m_busDevice->AddLog(CEC_LOG_DEBUG, strLog.c_str());

  vector<CCECBusDevice *> devices;
  for (int iDevicePtr = (int)GetMyDevices(devices)-1; iDevicePtr >=0; iDevicePtr--)
    devices[iDevicePtr]->TransmitActiveSource();

  return true;
}

bool CCECCommandHandler::HandleRoutingChange(const cec_command &command)
{
  if (command.parameters.size == 4)
  {
    uint16_t iOldAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
    uint16_t iNewAddress = ((uint16_t)command.parameters[2] << 8) | ((uint16_t)command.parameters[3]);

    CCECBusDevice *device = GetDevice(command.initiator);
    if (device)
      device->SetStreamPath(iNewAddress, iOldAddress);
  }
  return true;
}

bool CCECCommandHandler::HandleRoutingInformation(const cec_command &command)
{
  if (command.parameters.size == 2)
  {
    uint16_t iNewAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
    m_busDevice->GetProcessor()->SetStreamPath(iNewAddress);
  }

  return false;
}

bool CCECCommandHandler::HandleSetMenuLanguage(const cec_command &command)
{
  if (command.parameters.size == 3)
  {
    CCECBusDevice *device = GetDevice(command.initiator);
    if (device)
    {
      cec_menu_language language;
      language.device = command.initiator;
      for (uint8_t iPtr = 0; iPtr < 4; iPtr++)
        language.language[iPtr] = command.parameters[iPtr];
      language.language[3] = 0;
      device->SetMenuLanguage(language);
      return true;
    }
  }
  return false;
}

bool CCECCommandHandler::HandleSetOSDName(const cec_command &command)
{
  if (command.parameters.size > 0)
  {
    CCECBusDevice *device = GetDevice(command.initiator);
    if (device)
    {
      char buf[1024];
      for (uint8_t iPtr = 0; iPtr < command.parameters.size; iPtr++)
        buf[iPtr] = (char)command.parameters[iPtr];
      buf[command.parameters.size] = 0;

      CStdString strName(buf);
      device->SetOSDName(strName);

      return true;
    }
  }
  return false;
}

bool CCECCommandHandler::HandleSetStreamPath(const cec_command &command)
{
  if (command.parameters.size >= 2)
  {
    uint16_t iStreamAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
    CStdString strLog;
    strLog.Format(">> %i sets stream path to physical address %04x", command.initiator, iStreamAddress);
    m_busDevice->AddLog(CEC_LOG_DEBUG, strLog.c_str());

    if (m_busDevice->GetProcessor()->SetStreamPath(iStreamAddress))
    {
      CCECBusDevice *device = GetDeviceByPhysicalAddress(iStreamAddress);
      if (device)
      {
        return device->TransmitActiveSource() &&
            device->TransmitMenuState(command.initiator);
      }
    }
  }
  return false;
}

bool CCECCommandHandler::HandleSetSystemAudioModeRequest(const cec_command &command)
{
  if (m_busDevice->MyLogicalAddressContains(command.destination) && command.parameters.size >= 1)
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device&& device->GetType() == CEC_DEVICE_TYPE_AUDIO_SYSTEM)
      return ((CCECAudioSystem *) device)->SetSystemAudioMode(command);
  }
  return false;
}

bool CCECCommandHandler::HandleStandby(const cec_command &command)
{
  CCECBusDevice *device = GetDevice(command.initiator);
  if (device)
    device->SetPowerStatus(CEC_POWER_STATUS_STANDBY);

  return true;
}

bool CCECCommandHandler::HandleSystemAudioStatus(const cec_command &command)
{
  CCECBusDevice *device = GetDevice(command.initiator);
  if (device && device->GetType() == CEC_DEVICE_TYPE_AUDIO_SYSTEM)
  {
    ((CCECAudioSystem *)device)->SetSystemAudioMode(command);
    return true;
  }

  return false;
}

bool CCECCommandHandler::HandleTextViewOn(const cec_command &command)
{
  m_busDevice->GetProcessor()->SetActiveSource(command.initiator);
  return true;
}

bool CCECCommandHandler::HandleUserControlPressed(const cec_command &command)
{
  if (m_busDevice->MyLogicalAddressContains(command.destination) && command.parameters.size > 0)
  {
    m_busDevice->GetProcessor()->AddKey();

    if (command.parameters[0] <= CEC_USER_CONTROL_CODE_MAX)
    {
      CStdString strLog;
      strLog.Format("key pressed: %x", command.parameters[0]);
      m_busDevice->AddLog(CEC_LOG_DEBUG, strLog.c_str());

      if (command.parameters[0] == CEC_USER_CONTROL_CODE_POWER ||
          command.parameters[0] == CEC_USER_CONTROL_CODE_POWER_ON_FUNCTION)
      {
        CCECBusDevice *device = GetDevice(command.destination);
        if (device)
          device->SetPowerStatus(CEC_POWER_STATUS_ON);
      }

      m_busDevice->GetProcessor()->SetCurrentButton((cec_user_control_code) command.parameters[0]);
      return true;
    }
  }
  return false;
}

bool CCECCommandHandler::HandleUserControlRelease(const cec_command &command)
{
  if (m_busDevice->MyLogicalAddressContains(command.destination))
    m_busDevice->GetProcessor()->AddKey();

  return true;
}

void CCECCommandHandler::UnhandledCommand(const cec_command &command)
{
  CStdString strLog;
  strLog.Format("unhandled command with opcode %02x from address %d", command.opcode, command.initiator);
  m_busDevice->AddLog(CEC_LOG_DEBUG, strLog);
}

unsigned int CCECCommandHandler::GetMyDevices(vector<CCECBusDevice *> &devices) const
{
  unsigned int iReturn(0);

  cec_logical_addresses addresses = m_busDevice->GetProcessor()->GetLogicalAddresses();
  for (uint8_t iPtr = 0; iPtr < 16; iPtr++)
  {
    if (addresses[iPtr])
    {
      devices.push_back(GetDevice((cec_logical_address) iPtr));
      ++iReturn;
    }
  }

  return iReturn;
}

CCECBusDevice *CCECCommandHandler::GetDevice(cec_logical_address iLogicalAddress) const
{
  CCECBusDevice *device = NULL;

  if (iLogicalAddress >= CECDEVICE_TV && iLogicalAddress <= CECDEVICE_BROADCAST)
    device = m_busDevice->GetProcessor()->m_busDevices[iLogicalAddress];

  return device;
}

CCECBusDevice *CCECCommandHandler::GetDeviceByPhysicalAddress(uint16_t iPhysicalAddress) const
{
  return m_busDevice->GetProcessor()->GetDeviceByPhysicalAddress(iPhysicalAddress);
}

CCECBusDevice *CCECCommandHandler::GetDeviceByType(cec_device_type type) const
{
  return m_busDevice->GetProcessor()->GetDeviceByType(type);
}

void CCECCommandHandler::SetVendorId(const cec_command &command)
{
  if (command.parameters.size < 3)
  {
    m_busDevice->AddLog(CEC_LOG_WARNING, "invalid vendor ID received");
    return;
  }

  uint64_t iVendorId = ((uint64_t)command.parameters[0] << 16) +
                       ((uint64_t)command.parameters[1] << 8) +
                        (uint64_t)command.parameters[2];

  CCECBusDevice *device = GetDevice((cec_logical_address) command.initiator);
  if (device)
    device->SetVendorId(iVendorId);
}

void CCECCommandHandler::SetPhysicalAddress(cec_logical_address iAddress, uint16_t iNewAddress)
{
  if (!m_busDevice->MyLogicalAddressContains(iAddress))
  {
    bool bOurAddress(m_busDevice->GetProcessor()->GetPhysicalAddress() == iNewAddress);
    GetDevice(iAddress)->SetPhysicalAddress(iNewAddress);
    if (bOurAddress)
    {
      /* another device reported the same physical address as ours
       * since we don't have physical address detection yet, we'll just use the
       * given address, increased by 0x100 for now */
      m_busDevice->GetProcessor()->SetPhysicalAddress(iNewAddress + 0x100);
    }
  }
}
