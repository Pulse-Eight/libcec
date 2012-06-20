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
#include "../CECClient.h"
#include "../CECProcessor.h"
#include "../LibCEC.h"
#include "../CECTypeUtils.h"
#include "../platform/util/util.h"

using namespace CEC;
using namespace std;
using namespace PLATFORM;

#define LIB_CEC     m_busDevice->GetProcessor()->GetLib()
#define ToString(p) CCECTypeUtils::ToString(p)

CCECCommandHandler::CCECCommandHandler(CCECBusDevice *busDevice) :
    m_busDevice(busDevice),
    m_processor(m_busDevice->GetProcessor()),
    m_iTransmitTimeout(CEC_DEFAULT_TRANSMIT_TIMEOUT),
    m_iTransmitWait(CEC_DEFAULT_TRANSMIT_WAIT),
    m_iTransmitRetries(CEC_DEFAULT_TRANSMIT_RETRIES),
    m_bHandlerInited(false),
    m_bOPTSendDeckStatusUpdateOnActiveSource(false),
    m_vendorId(CEC_VENDOR_UNKNOWN),
    m_waitForResponse(new CWaitForResponse),
    m_bActiveSourcePending(false)
{
}

CCECCommandHandler::~CCECCommandHandler(void)
{
  DELETE_AND_NULL(m_waitForResponse);
}

bool CCECCommandHandler::HandleCommand(const cec_command &command)
{
  if (command.opcode_set == 0)
    return HandlePoll(command);

  int iHandled(CEC_ABORT_REASON_UNRECOGNIZED_OPCODE);

  CCECClient *client = m_busDevice->GetClient();
  if (client)
    client->AddCommand(command);

  switch(command.opcode)
  {
  case CEC_OPCODE_REPORT_POWER_STATUS:
    iHandled = HandleReportPowerStatus(command);
    break;
  case CEC_OPCODE_CEC_VERSION:
    iHandled = HandleDeviceCecVersion(command);
    break;
  case CEC_OPCODE_SET_MENU_LANGUAGE:
    iHandled = HandleSetMenuLanguage(command);
    break;
  case CEC_OPCODE_GIVE_PHYSICAL_ADDRESS:
    iHandled = HandleGivePhysicalAddress(command);
    break;
  case CEC_OPCODE_GET_MENU_LANGUAGE:
    iHandled = HandleGiveMenuLanguage(command);
    break;
  case CEC_OPCODE_GIVE_OSD_NAME:
    iHandled = HandleGiveOSDName(command);
    break;
  case CEC_OPCODE_GIVE_DEVICE_VENDOR_ID:
    iHandled = HandleGiveDeviceVendorId(command);
    break;
  case CEC_OPCODE_DEVICE_VENDOR_ID:
    iHandled = HandleDeviceVendorId(command);
    break;
  case CEC_OPCODE_VENDOR_COMMAND_WITH_ID:
    iHandled = HandleDeviceVendorCommandWithId(command);
    break;
  case CEC_OPCODE_GIVE_DECK_STATUS:
    iHandled = HandleGiveDeckStatus(command);
    break;
  case CEC_OPCODE_DECK_CONTROL:
    iHandled = HandleDeckControl(command);
    break;
  case CEC_OPCODE_MENU_REQUEST:
    iHandled = HandleMenuRequest(command);
    break;
  case CEC_OPCODE_GIVE_DEVICE_POWER_STATUS:
    iHandled = HandleGiveDevicePowerStatus(command);
    break;
  case CEC_OPCODE_GET_CEC_VERSION:
    iHandled = HandleGetCecVersion(command);
    break;
  case CEC_OPCODE_USER_CONTROL_PRESSED:
    iHandled = HandleUserControlPressed(command);
    break;
  case CEC_OPCODE_USER_CONTROL_RELEASE:
    iHandled = HandleUserControlRelease(command);
    break;
  case CEC_OPCODE_GIVE_AUDIO_STATUS:
    iHandled = HandleGiveAudioStatus(command);
    break;
  case CEC_OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS:
    iHandled = HandleGiveSystemAudioModeStatus(command);
    break;
  case CEC_OPCODE_SYSTEM_AUDIO_MODE_REQUEST:
    iHandled = HandleSystemAudioModeRequest(command);
    break;
  case CEC_OPCODE_REPORT_AUDIO_STATUS:
    iHandled = HandleReportAudioStatus(command);
    break;
  case CEC_OPCODE_SYSTEM_AUDIO_MODE_STATUS:
    iHandled = HandleSystemAudioModeStatus(command);
    break;
  case CEC_OPCODE_SET_SYSTEM_AUDIO_MODE:
    iHandled = HandleSetSystemAudioMode(command);
    break;
  case CEC_OPCODE_REQUEST_ACTIVE_SOURCE:
    iHandled = HandleRequestActiveSource(command);
    break;
  case CEC_OPCODE_SET_STREAM_PATH:
    iHandled = HandleSetStreamPath(command);
    break;
  case CEC_OPCODE_ROUTING_CHANGE:
    iHandled = HandleRoutingChange(command);
    break;
  case CEC_OPCODE_ROUTING_INFORMATION:
    iHandled = HandleRoutingInformation(command);
    break;
  case CEC_OPCODE_STANDBY:
    iHandled = HandleStandby(command);
    break;
  case CEC_OPCODE_ACTIVE_SOURCE:
    iHandled = HandleActiveSource(command);
    break;
  case CEC_OPCODE_REPORT_PHYSICAL_ADDRESS:
    iHandled = HandleReportPhysicalAddress(command);
    break;
  case CEC_OPCODE_SET_OSD_NAME:
    iHandled = HandleSetOSDName(command);
    break;
  case CEC_OPCODE_IMAGE_VIEW_ON:
    iHandled = HandleImageViewOn(command);
    break;
  case CEC_OPCODE_TEXT_VIEW_ON:
    iHandled = HandleTextViewOn(command);
    break;
  case CEC_OPCODE_FEATURE_ABORT:
    iHandled = HandleFeatureAbort(command);
    break;
  case CEC_OPCODE_VENDOR_COMMAND:
    iHandled = HandleVendorCommand(command);
    break;
  case CEC_OPCODE_VENDOR_REMOTE_BUTTON_DOWN:
    iHandled = HandleVendorRemoteButtonDown(command);
    break;
  case CEC_OPCODE_VENDOR_REMOTE_BUTTON_UP:
    iHandled = HandleVendorRemoteButtonUp(command);
    break;
  case CEC_OPCODE_PLAY:
    // libCEC (currently) doesn't need to do anything with this, since player applications handle it
    // but it should not respond with a feature abort
    iHandled = COMMAND_HANDLED;
    break;
  default:
    break;
  }

  if (iHandled == COMMAND_HANDLED)
    m_waitForResponse->Received((command.opcode == CEC_OPCODE_FEATURE_ABORT && command.parameters.size > 0) ? (cec_opcode)command.parameters[0] : command.opcode);
  else
    UnhandledCommand(command, (cec_abort_reason)iHandled);

  return iHandled == COMMAND_HANDLED;
}

int CCECCommandHandler::HandleActiveSource(const cec_command &command)
{
  if (command.parameters.size == 2)
  {
    uint16_t iAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
    CCECBusDevice *device = m_processor->GetDeviceByPhysicalAddress(iAddress);
    if (device)
    {
      device->MarkAsActiveSource();
      return COMMAND_HANDLED;
    }
  }

  return CEC_ABORT_REASON_INVALID_OPERAND;
}

int CCECCommandHandler::HandleDeckControl(const cec_command &command)
{
  CCECPlaybackDevice *device = CCECBusDevice::AsPlaybackDevice(GetDevice(command.destination));
  if (device && command.parameters.size > 0)
  {
    device->SetDeckControlMode((cec_deck_control_mode) command.parameters[0]);
    return COMMAND_HANDLED;
  }

  return CEC_ABORT_REASON_INVALID_OPERAND;
}

int CCECCommandHandler::HandleDeviceCecVersion(const cec_command &command)
{
  if (command.parameters.size == 1)
  {
    CCECBusDevice *device = GetDevice(command.initiator);
    if (device)
      device->SetCecVersion((cec_version) command.parameters[0]);

    return COMMAND_HANDLED;
  }

  return CEC_ABORT_REASON_INVALID_OPERAND;
}

int CCECCommandHandler::HandleDeviceVendorCommandWithId(const cec_command & UNUSED(command))
{
  return CEC_ABORT_REASON_REFUSED;
}

int CCECCommandHandler::HandleDeviceVendorId(const cec_command &command)
{
  SetVendorId(command);
  return COMMAND_HANDLED;
}

int CCECCommandHandler::HandleFeatureAbort(const cec_command &command)
{
  if (command.parameters.size == 2 &&
        (command.parameters[1] == CEC_ABORT_REASON_UNRECOGNIZED_OPCODE ||
         command.parameters[1] == CEC_ABORT_REASON_REFUSED))
    m_processor->GetDevice(command.initiator)->SetUnsupportedFeature((cec_opcode)command.parameters[0]);
  return COMMAND_HANDLED;
}

int CCECCommandHandler::HandleGetCecVersion(const cec_command &command)
{
  if (m_processor->CECInitialised() && m_processor->IsHandledByLibCEC(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device && device->TransmitCECVersion(command.initiator))
      return COMMAND_HANDLED;
    return CEC_ABORT_REASON_INVALID_OPERAND;
  }

  return CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND;
}

int CCECCommandHandler::HandleGiveAudioStatus(const cec_command &command)
{
  if (m_processor->CECInitialised() && m_processor->IsHandledByLibCEC(command.destination))
  {
    CCECAudioSystem *device = CCECBusDevice::AsAudioSystem(GetDevice(command.destination));
    if (device && device->TransmitAudioStatus(command.initiator))
      return COMMAND_HANDLED;
    return CEC_ABORT_REASON_INVALID_OPERAND;
  }

  return CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND;
}

int CCECCommandHandler::HandleGiveDeckStatus(const cec_command &command)
{
  if (m_processor->CECInitialised() && m_processor->IsHandledByLibCEC(command.destination))
  {
    CCECPlaybackDevice *device = CCECBusDevice::AsPlaybackDevice(GetDevice(command.destination));
    if (device && device->TransmitDeckStatus(command.initiator))
      return COMMAND_HANDLED;
    return CEC_ABORT_REASON_INVALID_OPERAND;
  }

  return CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND;
}

int CCECCommandHandler::HandleGiveDevicePowerStatus(const cec_command &command)
{
  if (m_processor->CECInitialised() && m_processor->IsHandledByLibCEC(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device && device->TransmitPowerState(command.initiator))
      return COMMAND_HANDLED;
    return CEC_ABORT_REASON_INVALID_OPERAND;
  }

  return CEC_ABORT_REASON_INVALID_OPERAND;
}

int CCECCommandHandler::HandleGiveDeviceVendorId(const cec_command &command)
{
  if (m_processor->CECInitialised() && m_processor->IsHandledByLibCEC(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device && device->TransmitVendorID(command.initiator))
      return COMMAND_HANDLED;
  }

  return CEC_ABORT_REASON_INVALID_OPERAND;
}

int CCECCommandHandler::HandleGiveOSDName(const cec_command &command)
{
  if (m_processor->CECInitialised() && m_processor->IsHandledByLibCEC(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device && device->TransmitOSDName(command.initiator))
      return COMMAND_HANDLED;
  }

  return CEC_ABORT_REASON_INVALID_OPERAND;
}

int CCECCommandHandler::HandleGivePhysicalAddress(const cec_command &command)
{
  if (m_processor->CECInitialised() && m_processor->IsHandledByLibCEC(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device && device->TransmitPhysicalAddress())
      return COMMAND_HANDLED;
    return CEC_ABORT_REASON_INVALID_OPERAND;
  }

  return CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND;
}

int CCECCommandHandler::HandleGiveMenuLanguage(const cec_command &command)
{
  if (m_processor->CECInitialised() && m_processor->IsHandledByLibCEC(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device && device->TransmitSetMenuLanguage(command.initiator))
      return COMMAND_HANDLED;
    return CEC_ABORT_REASON_INVALID_OPERAND;
  }

  return CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND;
}

int CCECCommandHandler::HandleGiveSystemAudioModeStatus(const cec_command &command)
{
  if (m_processor->CECInitialised() && m_processor->IsHandledByLibCEC(command.destination))
  {
    CCECAudioSystem *device = CCECBusDevice::AsAudioSystem(GetDevice(command.destination));
    if (device && device->TransmitSystemAudioModeStatus(command.initiator))
      return COMMAND_HANDLED;
    return CEC_ABORT_REASON_INVALID_OPERAND;
  }

  return CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND;
}

int CCECCommandHandler::HandleImageViewOn(const cec_command &command)
{
  m_processor->GetDevice(command.initiator)->MarkAsActiveSource();
  return COMMAND_HANDLED;
}

int CCECCommandHandler::HandleMenuRequest(const cec_command &command)
{
  if (m_processor->CECInitialised() && m_processor->IsHandledByLibCEC(command.destination))
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device)
    {
      CCECClient *client = device->GetClient();
      if (client)
      {
        if (command.parameters[0] == CEC_MENU_REQUEST_TYPE_ACTIVATE)
        {
          if (client->MenuStateChanged(CEC_MENU_STATE_ACTIVATED) == 1)
            device->SetMenuState(CEC_MENU_STATE_ACTIVATED);
        }
        else if (command.parameters[0] == CEC_MENU_REQUEST_TYPE_DEACTIVATE)
        {
          if (client->MenuStateChanged(CEC_MENU_STATE_DEACTIVATED) == 1)
            device->SetMenuState(CEC_MENU_STATE_DEACTIVATED);
        }
      }
      if (device->TransmitMenuState(command.initiator))
        return COMMAND_HANDLED;
    }
    return CEC_ABORT_REASON_INVALID_OPERAND;
  }

  return CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND;
}

bool CCECCommandHandler::HandlePoll(const cec_command &command)
{
  m_busDevice->HandlePoll(command.destination);
  return true;
}

int CCECCommandHandler::HandleReportAudioStatus(const cec_command &command)
{
  if (command.parameters.size == 1)
  {
    CCECAudioSystem *device = CCECBusDevice::AsAudioSystem(GetDevice(command.initiator));
    if (device)
    {
      device->SetAudioStatus(command.parameters[0]);
      return COMMAND_HANDLED;
    }
  }
  return CEC_ABORT_REASON_INVALID_OPERAND;
}

int CCECCommandHandler::HandleReportPhysicalAddress(const cec_command &command)
{
  if (command.parameters.size == 3)
  {
    uint16_t iNewAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
    SetPhysicalAddress(command.initiator, iNewAddress);
    return COMMAND_HANDLED;
  }
  return CEC_ABORT_REASON_INVALID_OPERAND;
}

int CCECCommandHandler::HandleReportPowerStatus(const cec_command &command)
{
  if (command.parameters.size == 1)
  {
    CCECBusDevice *device = GetDevice(command.initiator);
    if (device)
    {
      device->SetPowerStatus((cec_power_status) command.parameters[0]);
      return COMMAND_HANDLED;
    }
  }
  return CEC_ABORT_REASON_INVALID_OPERAND;
}

int CCECCommandHandler::HandleRequestActiveSource(const cec_command &command)
{
  if (m_processor->CECInitialised())
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, ">> %i requests active source", (uint8_t) command.initiator);
    m_processor->GetDevice(command.initiator)->SetPowerStatus(CEC_POWER_STATUS_ON);

    vector<CCECBusDevice *> devices;
    for (size_t iDevicePtr = 0; iDevicePtr < GetMyDevices(devices); iDevicePtr++)
      devices[iDevicePtr]->TransmitActiveSource();
  }

  return COMMAND_HANDLED;
}

int CCECCommandHandler::HandleRoutingChange(const cec_command &command)
{
  if (command.parameters.size == 4)
  {
    uint16_t iOldAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
    uint16_t iNewAddress = ((uint16_t)command.parameters[2] << 8) | ((uint16_t)command.parameters[3]);

    CCECBusDevice *device = GetDevice(command.initiator);
    if (device)
    {
      device->SetStreamPath(iNewAddress, iOldAddress);
      return COMMAND_HANDLED;
    }
  }

  return CEC_ABORT_REASON_INVALID_OPERAND;
}

int CCECCommandHandler::HandleRoutingInformation(const cec_command &command)
{
  if (command.parameters.size == 2)
  {
    uint16_t iNewAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
    CCECBusDevice *device = m_processor->GetDeviceByPhysicalAddress(iNewAddress);
    if (device)
    {
      device->MarkAsActiveSource();
      return COMMAND_HANDLED;
    }
  }

  return CEC_ABORT_REASON_INVALID_OPERAND;
}

int CCECCommandHandler::HandleSetMenuLanguage(const cec_command &command)
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
      return COMMAND_HANDLED;
    }
  }

  return CEC_ABORT_REASON_INVALID_OPERAND;
}

int CCECCommandHandler::HandleSetOSDName(const cec_command &command)
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

      return COMMAND_HANDLED;
    }
  }

  return CEC_ABORT_REASON_INVALID_OPERAND;
}

int CCECCommandHandler::HandleSetStreamPath(const cec_command &command)
{
  if (!m_processor->CECInitialised())
    return CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND;

  if (command.parameters.size >= 2)
  {
    uint16_t iStreamAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
    LIB_CEC->AddLog(CEC_LOG_DEBUG, ">> %i sets stream path to physical address %04x", command.initiator, iStreamAddress);

    /* one of the device handled by libCEC has been made active */
    CCECBusDevice *device = GetDeviceByPhysicalAddress(iStreamAddress);
    if (device && device->IsHandledByLibCEC())
    {
      device->ActivateSource();
      return COMMAND_HANDLED;
    }
  }

  return CEC_ABORT_REASON_INVALID_OPERAND;
}

int CCECCommandHandler::HandleSystemAudioModeRequest(const cec_command &command)
{
  if (m_processor->CECInitialised() && m_processor->IsHandledByLibCEC(command.destination))
  {
    CCECAudioSystem *device = CCECBusDevice::AsAudioSystem(GetDevice(command.destination));
    if (device)
    {
      if (command.parameters.size >= 2)
      {
        device->SetPowerStatus(CEC_POWER_STATUS_ON);
        device->SetSystemAudioModeStatus(CEC_SYSTEM_AUDIO_STATUS_ON);
        uint16_t iNewAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
        CCECBusDevice *newActiveDevice = GetDeviceByPhysicalAddress(iNewAddress);
        if (newActiveDevice)
          newActiveDevice->MarkAsActiveSource();
        if (device->TransmitSetSystemAudioMode(command.initiator))
          return COMMAND_HANDLED;
      }
      else
      {
        device->SetSystemAudioModeStatus(CEC_SYSTEM_AUDIO_STATUS_OFF);
        if (device->TransmitSetSystemAudioMode(command.initiator))
          return COMMAND_HANDLED;
      }
    }
  }

  return CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND;
}

int CCECCommandHandler::HandleStandby(const cec_command &command)
{
  CCECBusDevice *device = GetDevice(command.initiator);
  if (device)
    device->SetPowerStatus(CEC_POWER_STATUS_STANDBY);

  return COMMAND_HANDLED;
}

int CCECCommandHandler::HandleSystemAudioModeStatus(const cec_command &command)
{
  if (command.parameters.size == 1)
  {
    CCECAudioSystem *device = CCECBusDevice::AsAudioSystem(GetDevice(command.initiator));
    if (device)
    {
      device->SetSystemAudioModeStatus((cec_system_audio_status)command.parameters[0]);
      return COMMAND_HANDLED;
    }
  }

  return CEC_ABORT_REASON_INVALID_OPERAND;
}

int CCECCommandHandler::HandleSetSystemAudioMode(const cec_command &command)
{
  if (command.parameters.size == 1)
  {
    CCECAudioSystem *device = CCECBusDevice::AsAudioSystem(GetDevice(command.initiator));
    if (device)
    {
      device->SetSystemAudioModeStatus((cec_system_audio_status)command.parameters[0]);
      return COMMAND_HANDLED;
    }
  }

  return CEC_ABORT_REASON_INVALID_OPERAND;
}

int CCECCommandHandler::HandleTextViewOn(const cec_command &command)
{
  m_processor->GetDevice(command.initiator)->MarkAsActiveSource();
  return COMMAND_HANDLED;
}

int CCECCommandHandler::HandleUserControlPressed(const cec_command &command)
{
  if (!m_processor->CECInitialised() ||
      !m_processor->IsHandledByLibCEC(command.destination))
    return CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND;

  if (command.parameters.size == 0)
    return CEC_ABORT_REASON_INVALID_OPERAND;

  CCECBusDevice *device = GetDevice(command.destination);
  if (!device)
    return CEC_ABORT_REASON_INVALID_OPERAND;

  CCECClient *client = device->GetClient();
  if (client)
    client->AddKey();

  if (command.parameters[0] <= CEC_USER_CONTROL_CODE_MAX)
    client->SetCurrentButton((cec_user_control_code) command.parameters[0]);

  if (command.parameters[0] == CEC_USER_CONTROL_CODE_POWER ||
      command.parameters[0] == CEC_USER_CONTROL_CODE_POWER_ON_FUNCTION)
  {
    bool bPowerOn(true);

    // CEC_USER_CONTROL_CODE_POWER operates as a toggle
    // assume CEC_USER_CONTROL_CODE_POWER_ON_FUNCTION does not
    if (command.parameters[0] == CEC_USER_CONTROL_CODE_POWER)
    {
      cec_power_status status = device->GetCurrentPowerStatus();
      bPowerOn = !(status == CEC_POWER_STATUS_ON || status == CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON);
    }

    if (bPowerOn)
    {
      device->ActivateSource();
    }
    else
    {
      device->MarkAsInactiveSource();
      device->TransmitInactiveSource();
      device->SetMenuState(CEC_MENU_STATE_DEACTIVATED);
    }
  }

  return COMMAND_HANDLED;
}

int CCECCommandHandler::HandleUserControlRelease(const cec_command &command)
{
  if (!m_processor->CECInitialised() ||
      !m_processor->IsHandledByLibCEC(command.destination))
    return CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND;

  CCECClient *client = m_processor->GetClient(command.destination);
  if (client)
    client->AddKey();

  return COMMAND_HANDLED;
}

int CCECCommandHandler::HandleVendorCommand(const cec_command & UNUSED(command))
{
  return CEC_ABORT_REASON_REFUSED;
}

void CCECCommandHandler::UnhandledCommand(const cec_command &command, const cec_abort_reason reason)
{
  if (m_processor->IsHandledByLibCEC(command.destination))
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "sending abort with opcode %02x and reason '%s' to %s", command.opcode, ToString(reason), ToString(command.initiator));
    m_processor->TransmitAbort(command.destination, command.initiator, command.opcode, reason);
  }
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
  return m_processor->GetDevice(iLogicalAddress);
}

CCECBusDevice *CCECCommandHandler::GetDeviceByPhysicalAddress(uint16_t iPhysicalAddress) const
{
  return m_processor->GetDeviceByPhysicalAddress(iPhysicalAddress);
}

bool CCECCommandHandler::SetVendorId(const cec_command &command)
{
  bool bChanged(false);
  if (command.parameters.size < 3)
  {
    LIB_CEC->AddLog(CEC_LOG_WARNING, "invalid vendor ID received");
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
  if (!m_processor->IsHandledByLibCEC(iAddress))
  {
    CCECBusDevice *otherDevice = m_processor->GetDeviceByPhysicalAddress(iNewAddress);
    CCECClient *client = otherDevice ? otherDevice->GetClient() : NULL;

    CCECBusDevice *device = m_processor->GetDevice(iAddress);
    if (device)
      device->SetPhysicalAddress(iNewAddress);
    else
    {
      LIB_CEC->AddLog(CEC_LOG_DEBUG, "device with logical address %X not found", iAddress);
    }

    /* another device reported the same physical address as ours */
    if (client)
      client->ResetPhysicalAddress();
  }
  else
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "ignore physical address report for device %s (%X) because it's marked as handled by libCEC", ToString(iAddress), iAddress);
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
    LIB_CEC->AddLog(CEC_LOG_ERROR, "not transmitting a command without a valid initiator");
    return bReturn;
  }

  {
    uint8_t iTries(0), iMaxTries(!command.opcode_set ? 1 : m_iTransmitRetries + 1);
    while (!bReturn && ++iTries <= iMaxTries && !m_busDevice->IsUnsupportedFeature(command.opcode))
    {
      if ((bReturn = m_processor->Transmit(command)) == true)
      {
        LIB_CEC->AddLog(CEC_LOG_DEBUG, "command transmitted");
        if (bExpectResponse)
        {
          bReturn = m_waitForResponse->Wait(expectedResponse);
          LIB_CEC->AddLog(CEC_LOG_DEBUG, bReturn ? "expected response received (%X: %s)" : "expected response not received (%X: %s)", (int)expectedResponse, ToString(expectedResponse));
        }
      }
    }
  }

  return bReturn;
}

bool CCECCommandHandler::ActivateSource(void)
{
  if (m_busDevice->IsActiveSource() &&
    m_busDevice->IsHandledByLibCEC())
  {
    {
      CLockObject lock(m_mutex);
      m_bActiveSourcePending = false;
    }

    m_busDevice->SetPowerStatus(CEC_POWER_STATUS_ON);
    m_busDevice->SetMenuState(CEC_MENU_STATE_ACTIVATED);

    bool bActiveSourceFailed = !m_busDevice->TransmitImageViewOn() ||
                               !m_busDevice->TransmitActiveSource() ||
                               !m_busDevice->TransmitMenuState(CECDEVICE_TV);

    if (!bActiveSourceFailed)
    {
      CCECPlaybackDevice *playbackDevice = m_busDevice->AsPlaybackDevice();
      if (playbackDevice && SendDeckStatusUpdateOnActiveSource())
        bActiveSourceFailed = !playbackDevice->TransmitDeckStatus(CECDEVICE_TV);
    }

    if (bActiveSourceFailed)
    {
      LIB_CEC->AddLog(CEC_LOG_DEBUG, "failed to make '%s' the active source. will retry later", m_busDevice->GetLogicalAddressName());
      CLockObject lock(m_mutex);
      m_bActiveSourcePending = true;
      return false;
    }

    m_bHandlerInited = true;
  }
  return true;
}

void CCECCommandHandler::SignalOpcode(cec_opcode opcode)
{
  m_waitForResponse->Received(opcode);
}

bool CCECCommandHandler::ActiveSourcePending(void)
{
  CLockObject lock(m_mutex);
  return m_bActiveSourcePending;
}
