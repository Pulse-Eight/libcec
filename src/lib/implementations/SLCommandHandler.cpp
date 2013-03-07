/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2013 Pulse-Eight Limited.  All rights reserved.
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

#include "env.h"
#include "SLCommandHandler.h"

#include "lib/platform/util/timeutils.h"
#include "lib/devices/CECBusDevice.h"
#include "lib/devices/CECPlaybackDevice.h"
#include "lib/CECProcessor.h"
#include "lib/LibCEC.h"

using namespace CEC;
using namespace PLATFORM;

#define SL_COMMAND_UNKNOWN_01           0x01
#define SL_COMMAND_UNKNOWN_02           0x02

#define SL_COMMAND_TYPE_HDDRECORDER_DISC  0x01
#define SL_COMMAND_TYPE_VCR               0x02
#define SL_COMMAND_TYPE_DVDPLAYER         0x03
#define SL_COMMAND_TYPE_HDDRECORDER_DISC2 0x04
#define SL_COMMAND_TYPE_HDDRECORDER       0x05

#define SL_COMMAND_REQUEST_POWER_STATUS 0xa0
#define SL_COMMAND_POWER_ON             0x03
#define SL_COMMAND_CONNECT_REQUEST      0x04
#define SL_COMMAND_SET_DEVICE_MODE      0x05

#define LIB_CEC     m_busDevice->GetProcessor()->GetLib()
#define ToString(p) LIB_CEC->ToString(p)

CSLCommandHandler::CSLCommandHandler(CCECBusDevice *busDevice,
                                     int32_t iTransmitTimeout /* = CEC_DEFAULT_TRANSMIT_TIMEOUT */,
                                     int32_t iTransmitWait /* = CEC_DEFAULT_TRANSMIT_WAIT */,
                                     int8_t iTransmitRetries /* = CEC_DEFAULT_TRANSMIT_RETRIES */,
                                     int64_t iActiveSourcePending /* = 0 */) :
    CCECCommandHandler(busDevice, iTransmitTimeout, iTransmitWait, iTransmitRetries, iActiveSourcePending),
    m_bSLEnabled(false),
    m_bActiveSourceSent(false)
{
  m_vendorId = CEC_VENDOR_LG;

  /* LG devices don't always reply to CEC version requests, so just set it to 1.3a */
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

  if (m_busDevice->GetLogicalAddress() != CECDEVICE_TV)
    return true;

  CCECBusDevice *primary = m_processor->GetPrimaryDevice();
  if (primary && primary->GetLogicalAddress() != CECDEVICE_UNREGISTERED)
  {
    /* imitate LG devices */
    if (m_busDevice->GetLogicalAddress() != primary->GetLogicalAddress())
    {
      primary->SetVendorId(CEC_VENDOR_LG);
      primary->ReplaceHandler(false);
    }

    if (m_busDevice->GetLogicalAddress() == CECDEVICE_TV)
    {
      /* start as 'in transition standby->on' */
      primary->SetPowerStatus(CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON);
      primary->TransmitPowerState(CECDEVICE_TV, false);

      /* send the vendor id */
      primary->TransmitVendorID(CECDEVICE_BROADCAST, false, false);
    }
  }

  return true;
}

int CSLCommandHandler::HandleActiveSource(const cec_command &command)
{
  if (command.parameters.size == 2)
  {
    uint16_t iAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
    CCECBusDevice *device = m_processor->GetDeviceByPhysicalAddress(iAddress);
    if (device)
      device->MarkAsActiveSource();

    {
      CLockObject lock(m_SLMutex);
      m_bActiveSourceSent = false;
    }

    return COMMAND_HANDLED;
  }

  return CEC_ABORT_REASON_INVALID_OPERAND;

}

int CSLCommandHandler::HandleDeviceVendorId(const cec_command &command)
{
  SetVendorId(command);

  if (!SLInitialised() && command.initiator == CECDEVICE_TV)
  {
    CCECBusDevice *destination = m_processor->GetDevice(command.destination);
    if (destination && (destination->GetLogicalAddress() == CECDEVICE_BROADCAST || destination->IsHandledByLibCEC()))
    {
      cec_logical_address initiator = destination->GetLogicalAddress();
      if (initiator == CECDEVICE_BROADCAST)
        initiator = m_processor->GetPrimaryDevice()->GetLogicalAddress();

      cec_command response;
      cec_command::Format(response, initiator, command.initiator, CEC_OPCODE_FEATURE_ABORT);
      Transmit(response, false, true);
      return COMMAND_HANDLED;
    }
  }

  return CCECCommandHandler::HandleDeviceVendorId(command);
}

int CSLCommandHandler::HandleVendorCommand(const cec_command &command)
{
  if (!m_processor->IsHandledByLibCEC(command.destination))
    return true;

  if (command.parameters.size == 1 &&
      command.parameters[0] == SL_COMMAND_UNKNOWN_01)
  {
    HandleVendorCommand01(command);
    return COMMAND_HANDLED;
  }
  else if (command.parameters.size == 2 &&
      command.parameters[0] == SL_COMMAND_POWER_ON)
  {
    HandleVendorCommandPowerOn(command);
    return COMMAND_HANDLED;
  }
  else if (command.parameters.size == 2 &&
      command.parameters[0] == SL_COMMAND_CONNECT_REQUEST)
  {
    HandleVendorCommandSLConnect(command);
    return COMMAND_HANDLED;
  }
  else if (command.parameters.size == 1 &&
      command.parameters[0] == SL_COMMAND_REQUEST_POWER_STATUS)
  {
    HandleVendorCommandPowerOnStatus(command);
    return COMMAND_HANDLED;
  }

  return CCECCommandHandler::HandleVendorCommand(command);
}

void CSLCommandHandler::HandleVendorCommand01(const cec_command &command)
{
  m_processor->GetPrimaryDevice()->SetPowerStatus(CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON);
  TransmitVendorCommand0205(command.destination, command.initiator);

  CCECBusDevice* dev = m_processor->GetDevice(command.destination);
  if (dev && dev->IsHandledByLibCEC() && dev->IsActiveSource())
    dev->TransmitActiveSource(false);
}

void CSLCommandHandler::TransmitVendorCommand0205(const cec_logical_address iSource, const cec_logical_address iDestination)
{
  cec_command response;
  cec_command::Format(response, iSource, iDestination, CEC_OPCODE_VENDOR_COMMAND);
  response.PushBack(SL_COMMAND_UNKNOWN_02);
  response.PushBack(SL_COMMAND_TYPE_HDDRECORDER);

  Transmit(response, false, true);
  SetSLInitialised();
}

void CSLCommandHandler::HandleVendorCommandPowerOn(const cec_command &command)
{
  if (command.initiator != CECDEVICE_TV)
    return;

  CCECBusDevice *device = m_processor->GetPrimaryDevice();
  if (device)
  {
    SetSLInitialised();
    device->MarkAsActiveSource();
    device->SetPowerStatus(CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON);
    device->TransmitPowerState(command.initiator, true);

    CEvent::Sleep(2000);
    device->SetPowerStatus(CEC_POWER_STATUS_ON);
    device->TransmitPowerState(command.initiator, false);
    device->TransmitPhysicalAddress(false);
    {
      CLockObject lock(m_SLMutex);
      m_bActiveSourceSent = false;
    }
  }
}
void CSLCommandHandler::HandleVendorCommandPowerOnStatus(const cec_command &command)
{
  if (command.destination != CECDEVICE_BROADCAST)
  {
    CCECBusDevice *device = m_processor->GetPrimaryDevice();
    if (device)
    {
      device->SetPowerStatus(CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON);
      device->TransmitPowerState(command.initiator, true);
      device->SetPowerStatus(CEC_POWER_STATUS_ON);
    }
  }
}

void CSLCommandHandler::HandleVendorCommandSLConnect(const cec_command &command)
{
  SetSLInitialised();
  TransmitVendorCommandSetDeviceMode(command.destination, command.initiator, CEC_DEVICE_TYPE_RECORDING_DEVICE);

  ActivateSource();
}

void CSLCommandHandler::TransmitVendorCommandSetDeviceMode(const cec_logical_address iSource, const cec_logical_address iDestination, const cec_device_type type)
{
  cec_command response;
  cec_command::Format(response, iSource, iDestination, CEC_OPCODE_VENDOR_COMMAND);
  response.PushBack(SL_COMMAND_SET_DEVICE_MODE);
  response.PushBack((uint8_t)type);
  Transmit(response, false, true);
}

int CSLCommandHandler::HandleGiveDeckStatus(const cec_command &command)
{
  if (!m_processor->CECInitialised() ||
      !m_processor->IsHandledByLibCEC(command.destination))
    return CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND;

  CCECPlaybackDevice *device = CCECBusDevice::AsPlaybackDevice(GetDevice(command.destination));
  if (!device || command.parameters.size == 0)
    return CEC_ABORT_REASON_INVALID_OPERAND;

  device->SetDeckStatus(CEC_DECK_INFO_OTHER_STATUS_LG);
  if (command.parameters[0] == CEC_STATUS_REQUEST_ON)
  {
    device->TransmitDeckStatus(command.initiator, true);
    if (!ActiveSourceSent())
      ActivateSource();
    return COMMAND_HANDLED;
  }
  else if (command.parameters[0] == CEC_STATUS_REQUEST_ONCE)
  {
    device->TransmitDeckStatus(command.initiator, true);
    return COMMAND_HANDLED;
  }

  return CCECCommandHandler::HandleGiveDeckStatus(command);
}

int CSLCommandHandler::HandleGiveDevicePowerStatus(const cec_command &command)
{
  if (m_processor->CECInitialised() && m_processor->IsHandledByLibCEC(command.destination) && command.initiator == CECDEVICE_TV)
  {
    CCECBusDevice *device = GetDevice(command.destination);
    if (device && device->GetCurrentPowerStatus() != CEC_POWER_STATUS_ON)
    {
      device->TransmitPowerState(command.initiator, true);
      device->SetPowerStatus(CEC_POWER_STATUS_ON);
    }
    else
    {
      if (!ActiveSourceSent())
      {
        device->SetPowerStatus(CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON);
        device->TransmitPowerState(command.initiator, true);
        ActivateSource();
      }
      else if (m_resetPowerState.IsSet() && m_resetPowerState.TimeLeft() > 0)
      {
        /* TODO assume that we've bugged out. the return button no longer works after this */
        LIB_CEC->AddLog(CEC_LOG_WARNING, "FIXME: LG seems to have bugged out. resetting to 'in transition standby to on'. the return button will not work");
        {
          CLockObject lock(m_SLMutex);
          m_bActiveSourceSent = false;
        }
        device->SetPowerStatus(CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON);
        device->TransmitPowerState(command.initiator, true);
        device->SetPowerStatus(CEC_POWER_STATUS_ON);
        m_resetPowerState.Init(5000);
      }
      else
      {
        device->TransmitPowerState(command.initiator, true);
        m_resetPowerState.Init(5000);
      }
    }

    return COMMAND_HANDLED;
  }

  return CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND;
}

int CSLCommandHandler::HandleRequestActiveSource(const cec_command &command)
{
  if (m_processor->CECInitialised())
  {
    if (ActiveSourceSent())
      LIB_CEC->AddLog(CEC_LOG_DEBUG, ">> %i requests active source, ignored", (uint8_t) command.initiator);
    else
      ActivateSource();
    return COMMAND_HANDLED;
  }
  return CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND;
}

int CSLCommandHandler::HandleFeatureAbort(const cec_command &command)
{
  if (command.parameters.size == 0 && m_processor->GetPrimaryDevice()->GetCurrentPowerStatus() == CEC_POWER_STATUS_ON && !SLInitialised() &&
      command.initiator == CECDEVICE_TV)
  {
    m_processor->GetPrimaryDevice()->TransmitPowerState(command.initiator, false);
    m_processor->GetPrimaryDevice()->TransmitVendorID(CECDEVICE_BROADCAST, false, false);
  }

  return CCECCommandHandler::HandleFeatureAbort(command);
}

int CSLCommandHandler::HandleStandby(const cec_command &command)
{
  if (command.initiator == CECDEVICE_TV)
  {
    CLockObject lock(m_SLMutex);
    m_bActiveSourceSent = false;
  }

  return CCECCommandHandler::HandleStandby(command);
}

void CSLCommandHandler::ResetSLState(void)
{
  LIB_CEC->AddLog(CEC_LOG_NOTICE, "resetting SL initialised state");
  CLockObject lock(m_SLMutex);
  m_bSLEnabled = false;
  m_bActiveSourceSent = false;
  m_processor->GetPrimaryDevice()->SetPowerStatus(CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON);
}

void CSLCommandHandler::SetSLInitialised(void)
{
  LIB_CEC->AddLog(CEC_LOG_NOTICE, "SL initialised");
  CLockObject lock(m_SLMutex);
  m_bSLEnabled = true;
}

bool CSLCommandHandler::SLInitialised(void)
{
  CLockObject lock(m_SLMutex);
  return m_bSLEnabled;
}

bool CSLCommandHandler::ActiveSourceSent(void)
{
  CLockObject lock(m_SLMutex);
  return m_bActiveSourceSent;
}

bool CSLCommandHandler::PowerOn(const cec_logical_address iInitiator, const cec_logical_address iDestination)
{
  if (iDestination != CECDEVICE_TV)
  {
    /* LG devices only allow themselves to be woken up by the TV with a vendor command */
    cec_command command;

    if (!m_bSLEnabled)
      TransmitVendorID(CECDEVICE_TV, iDestination, CEC_VENDOR_LG, false);

    cec_command::Format(command, CECDEVICE_TV, iDestination, CEC_OPCODE_VENDOR_COMMAND);
    command.PushBack(SL_COMMAND_POWER_ON);
    command.PushBack(0);
    return Transmit(command, false, false);
  }

  return CCECCommandHandler::PowerOn(iInitiator, iDestination);
}

void CSLCommandHandler::VendorPreActivateSourceHook(void)
{
  CCECPlaybackDevice *device = m_busDevice->AsPlaybackDevice();
  if (device)
    device->SetDeckStatus(!device->IsActiveSource() ? CEC_DECK_INFO_OTHER_STATUS : CEC_DECK_INFO_OTHER_STATUS_LG);
}
