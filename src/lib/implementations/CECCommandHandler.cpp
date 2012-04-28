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

#include "CECCommandHandler.h"
#include "../devices/CECBusDevice.h"
#include "../devices/CECAudioSystem.h"
#include "../devices/CECPlaybackDevice.h"
#include "../CECProcessor.h"
#include "../LibCEC.h"

using namespace CEC;
using namespace std;
using namespace PLATFORM;

CCECCommandHandler::CCECCommandHandler(CCECBusDevice *busDevice) :
    m_busDevice(busDevice),
    m_processor(m_busDevice->GetProcessor()),
    m_iTransmitTimeout(CEC_DEFAULT_TRANSMIT_TIMEOUT),
    m_iTransmitWait(CEC_DEFAULT_TRANSMIT_WAIT),
    m_iTransmitRetries(CEC_DEFAULT_TRANSMIT_RETRIES),
    m_bHandlerInited(false),
    m_bOPTSendDeckStatusUpdateOnActiveSource(false),
    m_vendorId(CEC_VENDOR_UNKNOWN),
    m_waitForResponse(new CWaitForResponse)
{
}

CCECCommandHandler::~CCECCommandHandler(void)
{
  delete m_waitForResponse;
}

bool CCECCommandHandler::HandleCommand(const cec_command &command)
{
  if (command.opcode_set == 0)
    return HandlePoll(command);

  bool bHandled(true);

  CLibCEC::AddCommand(command);

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
    if (m_processor->IsInitialised())
      HandleGivePhysicalAddress(command);
    break;
  case CEC_OPCODE_GET_MENU_LANGUAGE:
    if (m_processor->IsInitialised())
      HandleGiveMenuLanguage(command);
    break;
  case CEC_OPCODE_GIVE_OSD_NAME:
    if (m_processor->IsInitialised())
      HandleGiveOSDName(command);
    break;
  case CEC_OPCODE_GIVE_DEVICE_VENDOR_ID:
    if (m_processor->IsInitialised())
      HandleGiveDeviceVendorId(command);
    break;
  case CEC_OPCODE_DEVICE_VENDOR_ID:
    HandleDeviceVendorId(command);
    break;
  case CEC_OPCODE_VENDOR_COMMAND_WITH_ID:
    HandleDeviceVendorCommandWithId(command);
    break;
  case CEC_OPCODE_GIVE_DECK_STATUS:
    if (m_processor->IsInitialised())
      HandleGiveDeckStatus(command);
    break;
  case CEC_OPCODE_DECK_CONTROL:
    HandleDeckControl(command);
    break;
  case CEC_OPCODE_MENU_REQUEST:
    if (m_processor->IsInitialised())
      HandleMenuRequest(command);
    break;
  case CEC_OPCODE_GIVE_DEVICE_POWER_STATUS:
    if (m_processor->IsInitialised())
      HandleGiveDevicePowerStatus(command);
    break;
  case CEC_OPCODE_GET_CEC_VERSION:
    if (m_processor->IsInitialised())
      HandleGetCecVersion(command);
    break;
  case CEC_OPCODE_USER_CONTROL_PRESSED:
    if (m_processor->IsInitialised())
      HandleUserControlPressed(command);
    break;
  case CEC_OPCODE_USER_CONTROL_RELEASE:
    if (m_processor->IsInitialised())
      HandleUserControlRelease(command);
    break;
  case CEC_OPCODE_GIVE_AUDIO_STATUS:
    if (m_processor->IsInitialised())
      HandleGiveAudioStatus(command);
    break;
  case CEC_OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS:
    if (m_processor->IsInitialised())
      HandleGiveSystemAudioModeStatus(command);
    break;
  case CEC_OPCODE_SYSTEM_AUDIO_MODE_REQUEST:
    if (m_processor->IsInitialised())
      HandleSystemAudioModeRequest(command);
    break;
  case CEC_OPCODE_REPORT_AUDIO_STATUS:
    HandleReportAudioStatus(command);
    break;
  case CEC_OPCODE_SYSTEM_AUDIO_MODE_STATUS:
    HandleSystemAudioModeStatus(command);
    break;
  case CEC_OPCODE_SET_SYSTEM_AUDIO_MODE:
    HandleSetSystemAudioMode(command);
    break;
  case CEC_OPCODE_REQUEST_ACTIVE_SOURCE:
    if (m_processor->IsInitialised())
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
    if (m_processor->IsInitialised())
      HandleStandby(command);
    break;
  case CEC_OPCODE_ACTIVE_SOURCE:
    HandleActiveSource(command);
    break;
  case CEC_OPCODE_REPORT_PHYSICAL_ADDRESS:
    HandleReportPhysicalAddress(command);
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
  case CEC_OPCODE_FEATURE_ABORT:
    HandleFeatureAbort(command);
    break;
  case CEC_OPCODE_VENDOR_COMMAND:
    HandleVendorCommand(command);
    break;
  default:
    UnhandledCommand(command);
    bHandled = false;
    break;
  }

  if (bHandled)
    m_waitForResponse->Received((command.opcode == CEC_OPCODE_FEATURE_ABORT && command.parameters.size > 0) ? (cec_opcode)command.parameters[0] : command.opcode);

  return bHandled;
}

bool CCECCommandHandler::HandleActiveSource(const cec_command &command)
{
  if (command.parameters.size == 2)
  {
    uint16_t iAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
    return m_processor->SetActiveSource(iAddress);
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
  if (m_processor->IsRunning() && m_busDevice->MyLogicalAddressContains(command.destination))
    m_processor->TransmitAbort(command.initiator, command.opcode, CEC_ABORT_REASON_REFUSED);

  return true;
}

bool CCECCommandHandler::HandleDeviceVendorId(const cec_command &command)
{
  return SetVendorId(command);
}

bool CCECCommandHandler::HandleFeatureAbort(const cec_command &command)
{
  if (command.parameters.size == 2 &&
        (command.parameters[1] == CEC_ABORT_REASON_UNRECOGNIZED_OPCODE ||
         command.parameters[1] == CEC_ABORT_REASON_REFUSED))
    m_processor->m_busDevices[command.initiator]->SetUnsupportedFeature((cec_opcode)command.parameters[0]);
  return true;
}

bool CCECCommandHandler::HandleGetCecVersion(const cec_command &command)
{
  if (m_processor->IsRunning() && m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device)
      return device->TransmitCECVersion(command.initiator);
  }

  return false;
}

bool CCECCommandHandler::HandleGiveAudioStatus(const cec_command &command)
{
  if (m_processor->IsRunning() && m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device && device->GetType() == CEC_DEVICE_TYPE_AUDIO_SYSTEM)
      return ((CCECAudioSystem *) device)->TransmitAudioStatus(command.initiator);
  }

  return false;
}

bool CCECCommandHandler::HandleGiveDeckStatus(const cec_command &command)
{
  if (m_processor->IsRunning() && m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device && (device->GetType() == CEC_DEVICE_TYPE_PLAYBACK_DEVICE || device->GetType() == CEC_DEVICE_TYPE_RECORDING_DEVICE))
      return ((CCECPlaybackDevice *) device)->TransmitDeckStatus(command.initiator);
  }

  return false;
}

bool CCECCommandHandler::HandleGiveDevicePowerStatus(const cec_command &command)
{
  if (m_processor->IsRunning() && m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device)
      return device->TransmitPowerState(command.initiator);
  }

  return false;
}

bool CCECCommandHandler::HandleGiveDeviceVendorId(const cec_command &command)
{
  if (m_processor->IsRunning() && m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device)
      return device->TransmitVendorID(command.initiator);
  }

  return false;
}

bool CCECCommandHandler::HandleGiveOSDName(const cec_command &command)
{
  if (m_processor->IsRunning() && m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device)
      return device->TransmitOSDName(command.initiator);
  }

  return false;
}

bool CCECCommandHandler::HandleGivePhysicalAddress(const cec_command &command)
{
  if (m_processor->IsRunning() && m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device)
      return device->TransmitPhysicalAddress();
  }

  return false;
}

bool CCECCommandHandler::HandleGiveMenuLanguage(const cec_command &command)
{
  if (m_processor->IsRunning() && m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device)
      return device->TransmitSetMenuLanguage(command.initiator);
  }

  return false;
}

bool CCECCommandHandler::HandleGiveSystemAudioModeStatus(const cec_command &command)
{
  if (m_processor->IsRunning() && m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device && device->GetType() == CEC_DEVICE_TYPE_AUDIO_SYSTEM)
      return ((CCECAudioSystem *) device)->TransmitSystemAudioModeStatus(command.initiator);
  }

  return false;
}

bool CCECCommandHandler::HandleImageViewOn(const cec_command &command)
{
  m_processor->m_busDevices[command.initiator]->SetActiveSource();
  return true;
}

bool CCECCommandHandler::HandleMenuRequest(const cec_command &command)
{
  if (m_processor->IsRunning() && m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device)
    {
      if (command.parameters[0] == CEC_MENU_REQUEST_TYPE_ACTIVATE)
      {
        if (CLibCEC::MenuStateChanged(CEC_MENU_STATE_ACTIVATED) == 1)
          device->SetMenuState(CEC_MENU_STATE_ACTIVATED);
      }
      else if (command.parameters[0] == CEC_MENU_REQUEST_TYPE_DEACTIVATE)
      {
        if (CLibCEC::MenuStateChanged(CEC_MENU_STATE_DEACTIVATED) == 1)
          device->SetMenuState(CEC_MENU_STATE_DEACTIVATED);
      }
      return device->TransmitMenuState(command.initiator);
    }
  }

  return false;
}

bool CCECCommandHandler::HandlePoll(const cec_command &command)
{
  m_busDevice->HandlePoll(command.destination);
  return true;
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
  if (m_processor->IsRunning())
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, ">> %i requests active source", (uint8_t) command.initiator);
    m_processor->m_busDevices[command.initiator]->SetPowerStatus(CEC_POWER_STATUS_ON);

    vector<CCECBusDevice *> devices;
    for (size_t iDevicePtr = 0; iDevicePtr < GetMyDevices(devices); iDevicePtr++)
      devices[iDevicePtr]->TransmitActiveSource();

    return true;
  }
  return false;
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
    m_processor->SetActiveSource(iNewAddress);
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
  if (m_processor->IsRunning() && command.parameters.size >= 2)
  {
    uint16_t iStreamAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
    CLibCEC::AddLog(CEC_LOG_DEBUG, ">> %i sets stream path to physical address %04x", command.initiator, iStreamAddress);

    /* one of the device handled by libCEC has been made active */
    CCECBusDevice *device = GetDeviceByPhysicalAddress(iStreamAddress);
    if (device && m_busDevice->MyLogicalAddressContains(device->GetLogicalAddress()))
    {
      device->SetActiveSource();
      device->TransmitImageViewOn();
      device->TransmitActiveSource();

      device->SetMenuState(CEC_MENU_STATE_ACTIVATED);
      device->TransmitMenuState(command.initiator);
    }
  }
  return false;
}

bool CCECCommandHandler::HandleSystemAudioModeRequest(const cec_command &command)
{
  if (m_processor->IsRunning() && m_busDevice->MyLogicalAddressContains(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device && device->GetType() == CEC_DEVICE_TYPE_AUDIO_SYSTEM)
    {
      if (command.parameters.size >= 2)
      {
        device->SetPowerStatus(CEC_POWER_STATUS_ON);
        ((CCECAudioSystem *) device)->SetSystemAudioModeStatus(CEC_SYSTEM_AUDIO_STATUS_ON);
        uint16_t iNewAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
        CCECBusDevice *newActiveDevice = GetDeviceByPhysicalAddress(iNewAddress);
        if (newActiveDevice)
          newActiveDevice->SetActiveSource();
        return ((CCECAudioSystem *) device)->TransmitSetSystemAudioMode(command.initiator);
      }
      else
      {
        ((CCECAudioSystem *) device)->SetSystemAudioModeStatus(CEC_SYSTEM_AUDIO_STATUS_OFF);
        return ((CCECAudioSystem *) device)->TransmitSetSystemAudioMode(command.initiator);
      }
    }
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

bool CCECCommandHandler::HandleSystemAudioModeStatus(const cec_command &command)
{
  if (command.parameters.size == 1)
  {
    CCECBusDevice *device = GetDevice(command.initiator);
    if (device && device->GetType() == CEC_DEVICE_TYPE_AUDIO_SYSTEM)
    {
      ((CCECAudioSystem *)device)->SetSystemAudioModeStatus((cec_system_audio_status)command.parameters[0]);
      return true;
    }
  }

  return false;
}

bool CCECCommandHandler::HandleSetSystemAudioMode(const cec_command &command)
{
  if (command.parameters.size == 1)
  {
    CCECBusDevice *device = GetDevice(command.initiator);
    if (device && device->GetType() == CEC_DEVICE_TYPE_AUDIO_SYSTEM)
    {
      ((CCECAudioSystem *)device)->SetSystemAudioModeStatus((cec_system_audio_status)command.parameters[0]);
      return true;
    }
  }

  return false;
}

bool CCECCommandHandler::HandleTextViewOn(const cec_command &command)
{
  m_processor->m_busDevices[command.initiator]->SetActiveSource();
  return true;
}

bool CCECCommandHandler::HandleUserControlPressed(const cec_command &command)
{
  if (m_processor->IsRunning() &&
      m_busDevice->MyLogicalAddressContains(command.destination) &&
      command.parameters.size > 0)
  {
    CLibCEC::AddKey();
    if (command.parameters[0] <= CEC_USER_CONTROL_CODE_MAX)
      CLibCEC::SetCurrentButton((cec_user_control_code) command.parameters[0]);

    if (command.parameters[0] == CEC_USER_CONTROL_CODE_POWER ||
        command.parameters[0] == CEC_USER_CONTROL_CODE_POWER_ON_FUNCTION)
    {
      bool bPowerOn(true);
      CCECBusDevice *device = GetDevice(command.destination);
      if (!device)
        return true;

      // CEC_USER_CONTROL_CODE_POWER operates as a toggle
      // assume CEC_USER_CONTROL_CODE_POWER_ON_FUNCTION does not
      if (command.parameters[0] == CEC_USER_CONTROL_CODE_POWER)
      {
        cec_power_status status = device->GetPowerStatus();
        bPowerOn = !(status == CEC_POWER_STATUS_ON || status == CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON);
      }

      if (bPowerOn)
      {
        device->SetActiveSource();
        device->TransmitImageViewOn();
        device->TransmitActiveSource();

        if (device->GetType() == CEC_DEVICE_TYPE_PLAYBACK_DEVICE ||
            device->GetType() == CEC_DEVICE_TYPE_RECORDING_DEVICE)
          ((CCECPlaybackDevice *)device)->TransmitDeckStatus(command.initiator);
      }
      else
      {
        device->SetInactiveSource();
        device->TransmitInactiveSource();
        device->SetMenuState(CEC_MENU_STATE_DEACTIVATED);
      }
    }

    return true;
  }
  return false;
}

bool CCECCommandHandler::HandleUserControlRelease(const cec_command &command)
{
  if (m_processor->IsRunning() && m_busDevice->MyLogicalAddressContains(command.destination))
    CLibCEC::AddKey();

  return true;
}

bool CCECCommandHandler::HandleVendorCommand(const cec_command & UNUSED(command))
{
  return true;
}

void CCECCommandHandler::UnhandledCommand(const cec_command &command)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "unhandled command with opcode %02x from address %d", command.opcode, command.initiator);
}

size_t CCECCommandHandler::GetMyDevices(vector<CCECBusDevice *> &devices) const
{
  size_t iReturn(0);

  cec_logical_addresses addresses = m_processor->GetLogicalAddresses();
  for (uint8_t iPtr = CECDEVICE_TV; iPtr < CECDEVICE_BROADCAST; iPtr++)
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
    device = m_processor->m_busDevices[iLogicalAddress];

  return device;
}

CCECBusDevice *CCECCommandHandler::GetDeviceByPhysicalAddress(uint16_t iPhysicalAddress) const
{
  return m_processor->GetDeviceByPhysicalAddress(iPhysicalAddress);
}

CCECBusDevice *CCECCommandHandler::GetDeviceByType(cec_device_type type) const
{
  return m_processor->GetDeviceByType(type);
}

bool CCECCommandHandler::SetVendorId(const cec_command &command)
{
  bool bChanged(false);
  if (command.parameters.size < 3)
  {
    CLibCEC::AddLog(CEC_LOG_WARNING, "invalid vendor ID received");
    return bChanged;
  }

  uint64_t iVendorId = ((uint64_t)command.parameters[0] << 16) +
                       ((uint64_t)command.parameters[1] << 8) +
                        (uint64_t)command.parameters[2];

  CCECBusDevice *device = GetDevice((cec_logical_address) command.initiator);
  if (device)
    bChanged = device->SetVendorId(iVendorId);
  return bChanged;
}

void CCECCommandHandler::SetPhysicalAddress(cec_logical_address iAddress, uint16_t iNewAddress)
{
  if (!m_busDevice->MyLogicalAddressContains(iAddress))
  {
    bool bOurAddress(m_processor->GetPhysicalAddress() == iNewAddress);
    GetDevice(iAddress)->SetPhysicalAddress(iNewAddress);
    if (bOurAddress)
    {
      /* another device reported the same physical address as ours
       * since we don't have physical address detection yet, we'll just use the
       * given address, increased by 0x100 for now */
      m_processor->SetPhysicalAddress(iNewAddress + 0x100);
    }
  }
}

bool CCECCommandHandler::PowerOn(const cec_logical_address iInitiator, const cec_logical_address iDestination)
{
  if (iDestination == CECDEVICE_TV)
    return TransmitImageViewOn(iInitiator, iDestination);

  return TransmitKeypress(iInitiator, iDestination, CEC_USER_CONTROL_CODE_POWER) &&
    TransmitKeyRelease(iInitiator, iDestination);
}

bool CCECCommandHandler::TransmitImageViewOn(const cec_logical_address iInitiator, const cec_logical_address iDestination)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_IMAGE_VIEW_ON);

  return Transmit(command);
}

bool CCECCommandHandler::TransmitStandby(const cec_logical_address iInitiator, const cec_logical_address iDestination)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_STANDBY);

  return Transmit(command);
}

bool CCECCommandHandler::TransmitRequestActiveSource(const cec_logical_address iInitiator, bool bWaitForResponse /* = true */)
{
  cec_command command;
  cec_command::Format(command, iInitiator, CECDEVICE_BROADCAST, CEC_OPCODE_REQUEST_ACTIVE_SOURCE);

  return Transmit(command, !bWaitForResponse);
}

bool CCECCommandHandler::TransmitRequestCecVersion(const cec_logical_address iInitiator, const cec_logical_address iDestination, bool bWaitForResponse /* = true */)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_GET_CEC_VERSION);

  return Transmit(command, !bWaitForResponse);
}

bool CCECCommandHandler::TransmitRequestMenuLanguage(const cec_logical_address iInitiator, const cec_logical_address iDestination, bool bWaitForResponse /* = true */)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_GET_MENU_LANGUAGE);

  return Transmit(command, !bWaitForResponse);
}

bool CCECCommandHandler::TransmitRequestOSDName(const cec_logical_address iInitiator, const cec_logical_address iDestination, bool bWaitForResponse /* = true */)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_GIVE_OSD_NAME);

  return Transmit(command, !bWaitForResponse);
}

bool CCECCommandHandler::TransmitRequestPhysicalAddress(const cec_logical_address iInitiator, const cec_logical_address iDestination, bool bWaitForResponse /* = true */)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_GIVE_PHYSICAL_ADDRESS);

  return Transmit(command, !bWaitForResponse);
}

bool CCECCommandHandler::TransmitRequestPowerStatus(const cec_logical_address iInitiator, const cec_logical_address iDestination, bool bWaitForResponse /* = true */)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_GIVE_DEVICE_POWER_STATUS);

  return Transmit(command, !bWaitForResponse);
}

bool CCECCommandHandler::TransmitRequestVendorId(const cec_logical_address iInitiator, const cec_logical_address iDestination, bool bWaitForResponse /* = true */)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_GIVE_DEVICE_VENDOR_ID);

  return Transmit(command, !bWaitForResponse);
}

bool CCECCommandHandler::TransmitActiveSource(const cec_logical_address iInitiator, uint16_t iPhysicalAddress)
{
  cec_command command;
  cec_command::Format(command, iInitiator, CECDEVICE_BROADCAST, CEC_OPCODE_ACTIVE_SOURCE);
  command.parameters.PushBack((uint8_t) ((iPhysicalAddress >> 8) & 0xFF));
  command.parameters.PushBack((uint8_t) (iPhysicalAddress & 0xFF));

  return Transmit(command);
}

bool CCECCommandHandler::TransmitCECVersion(const cec_logical_address iInitiator, const cec_logical_address iDestination, cec_version cecVersion)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_CEC_VERSION);
  command.parameters.PushBack((uint8_t)cecVersion);

  return Transmit(command);
}

bool CCECCommandHandler::TransmitInactiveSource(const cec_logical_address iInitiator, uint16_t iPhysicalAddress)
{
  cec_command command;
  cec_command::Format(command, iInitiator, CECDEVICE_TV, CEC_OPCODE_INACTIVE_SOURCE);
  command.parameters.PushBack((iPhysicalAddress >> 8) & 0xFF);
  command.parameters.PushBack(iPhysicalAddress & 0xFF);

  return Transmit(command);
}

bool CCECCommandHandler::TransmitMenuState(const cec_logical_address iInitiator, const cec_logical_address iDestination, cec_menu_state menuState)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_MENU_STATUS);
  command.parameters.PushBack((uint8_t)menuState);

  return Transmit(command);
}

bool CCECCommandHandler::TransmitOSDName(const cec_logical_address iInitiator, const cec_logical_address iDestination, CStdString strDeviceName)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_SET_OSD_NAME);
  for (size_t iPtr = 0; iPtr < strDeviceName.length(); iPtr++)
    command.parameters.PushBack(strDeviceName.at(iPtr));

  return Transmit(command);
}

bool CCECCommandHandler::TransmitOSDString(const cec_logical_address iInitiator, const cec_logical_address iDestination, cec_display_control duration, const char *strMessage)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_SET_OSD_STRING);
  command.parameters.PushBack((uint8_t)duration);

  size_t iLen = strlen(strMessage);
  if (iLen > 13) iLen = 13;

  for (size_t iPtr = 0; iPtr < iLen; iPtr++)
    command.parameters.PushBack(strMessage[iPtr]);

  return Transmit(command);
}

bool CCECCommandHandler::TransmitPhysicalAddress(const cec_logical_address iInitiator, uint16_t iPhysicalAddress, cec_device_type type)
{
  cec_command command;
  cec_command::Format(command, iInitiator, CECDEVICE_BROADCAST, CEC_OPCODE_REPORT_PHYSICAL_ADDRESS);
  command.parameters.PushBack((uint8_t) ((iPhysicalAddress >> 8) & 0xFF));
  command.parameters.PushBack((uint8_t) (iPhysicalAddress & 0xFF));
  command.parameters.PushBack((uint8_t) (type));

  return Transmit(command);
}

bool CCECCommandHandler::TransmitSetMenuLanguage(const cec_logical_address iInitiator, const char lang[3])
{
  cec_command command;
  command.Format(command, iInitiator, CECDEVICE_BROADCAST, CEC_OPCODE_SET_MENU_LANGUAGE);
  command.parameters.PushBack((uint8_t) lang[0]);
  command.parameters.PushBack((uint8_t) lang[1]);
  command.parameters.PushBack((uint8_t) lang[2]);

  return Transmit(command);
}

bool CCECCommandHandler::TransmitPoll(const cec_logical_address iInitiator, const cec_logical_address iDestination)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_NONE);

  return Transmit(command);
}

bool CCECCommandHandler::TransmitPowerState(const cec_logical_address iInitiator, const cec_logical_address iDestination, cec_power_status state)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_REPORT_POWER_STATUS);
  command.parameters.PushBack((uint8_t) state);

  return Transmit(command);
}

bool CCECCommandHandler::TransmitVendorID(const cec_logical_address iInitiator, uint64_t iVendorId)
{
  cec_command command;
  cec_command::Format(command, iInitiator, CECDEVICE_BROADCAST, CEC_OPCODE_DEVICE_VENDOR_ID);

  command.parameters.PushBack((uint8_t) (((uint64_t)iVendorId >> 16) & 0xFF));
  command.parameters.PushBack((uint8_t) (((uint64_t)iVendorId >> 8) & 0xFF));
  command.parameters.PushBack((uint8_t) ((uint64_t)iVendorId & 0xFF));

  return Transmit(command);
}

bool CCECCommandHandler::TransmitAudioStatus(const cec_logical_address iInitiator, const cec_logical_address iDestination, uint8_t state)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_REPORT_AUDIO_STATUS);
  command.parameters.PushBack(state);

  return Transmit(command);
}

bool CCECCommandHandler::TransmitSetSystemAudioMode(const cec_logical_address iInitiator, const cec_logical_address iDestination, cec_system_audio_status state)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_SET_SYSTEM_AUDIO_MODE);
  command.parameters.PushBack((uint8_t)state);

  return Transmit(command);
}

bool CCECCommandHandler::TransmitSetStreamPath(uint16_t iStreamPath)
{
  cec_command command;
  cec_command::Format(command, m_busDevice->GetLogicalAddress(), CECDEVICE_BROADCAST, CEC_OPCODE_SET_STREAM_PATH);
  command.parameters.PushBack((uint8_t) ((iStreamPath >> 8) & 0xFF));
  command.parameters.PushBack((uint8_t) (iStreamPath        & 0xFF));

  return Transmit(command);
}

bool CCECCommandHandler::TransmitSystemAudioModeStatus(const cec_logical_address iInitiator, const cec_logical_address iDestination, cec_system_audio_status state)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_SYSTEM_AUDIO_MODE_STATUS);
  command.parameters.PushBack((uint8_t)state);

  return Transmit(command);
}

bool CCECCommandHandler::TransmitDeckStatus(const cec_logical_address iInitiator, const cec_logical_address iDestination, cec_deck_info state)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_DECK_STATUS);
  command.PushBack((uint8_t)state);

  return Transmit(command);
}

bool CCECCommandHandler::TransmitKeypress(const cec_logical_address iInitiator, const cec_logical_address iDestination, cec_user_control_code key, bool bWait /* = true */)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_USER_CONTROL_PRESSED);
  command.parameters.PushBack((uint8_t)key);

  return Transmit(command, !bWait);
}

bool CCECCommandHandler::TransmitKeyRelease(const cec_logical_address iInitiator, const cec_logical_address iDestination, bool bWait /* = true */)
{
  cec_command command;
  cec_command::Format(command, iInitiator, iDestination, CEC_OPCODE_USER_CONTROL_RELEASE);

  return Transmit(command, !bWait);
}

bool CCECCommandHandler::Transmit(cec_command &command, bool bSuppressWait /* = false */)
{
  bool bReturn(false);
  cec_opcode expectedResponse(cec_command::GetResponseOpcode(command.opcode));
  bool bExpectResponse(expectedResponse != CEC_OPCODE_NONE && !bSuppressWait);
  command.transmit_timeout = m_iTransmitTimeout;

  if (command.initiator == CECDEVICE_UNKNOWN)
  {
    CLibCEC::AddLog(CEC_LOG_ERROR, "not transmitting a command without a valid initiator");
    return bReturn;
  }

  {
    uint8_t iTries(0), iMaxTries(!command.opcode_set ? 1 : m_iTransmitRetries + 1);
    while (!bReturn && ++iTries <= iMaxTries && !m_busDevice->IsUnsupportedFeature(command.opcode))
    {
      if ((bReturn = m_processor->Transmit(command)) == true)
      {
        CLibCEC::AddLog(CEC_LOG_DEBUG, "command transmitted");
        if (bExpectResponse)
        {
          bReturn = m_waitForResponse->Wait(expectedResponse);
          CLibCEC::AddLog(CEC_LOG_DEBUG, bReturn ? "expected response received (%X: %s)" : "expected response not received (%X: %s)", (int)expectedResponse, m_processor->ToString(expectedResponse));
        }
      }
    }
  }

  return bReturn;
}

bool CCECCommandHandler::ActivateSource(void)
{
  if (m_busDevice->IsActiveSource() &&
    m_busDevice->GetStatus(false) == CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC)
  {
    m_busDevice->SetPowerStatus(CEC_POWER_STATUS_ON);
    m_busDevice->SetMenuState(CEC_MENU_STATE_ACTIVATED);

    m_busDevice->TransmitImageViewOn();
    m_busDevice->TransmitActiveSource();
    m_busDevice->TransmitMenuState(CECDEVICE_TV);
    if ((m_busDevice->GetType() == CEC_DEVICE_TYPE_PLAYBACK_DEVICE ||
      m_busDevice->GetType() == CEC_DEVICE_TYPE_RECORDING_DEVICE) &&
      SendDeckStatusUpdateOnActiveSource())
      ((CCECPlaybackDevice *)m_busDevice)->TransmitDeckStatus(CECDEVICE_TV);
    m_bHandlerInited = true;
  }
  return true;
}

void CCECCommandHandler::SignalOpcode(cec_opcode opcode)
{
  m_waitForResponse->Received(opcode);
}
