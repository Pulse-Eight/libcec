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

#include "CECBusDevice.h"
#include "../CECProcessor.h"
#include "../implementations/ANCommandHandler.h"
#include "../implementations/CECCommandHandler.h"
#include "../implementations/SLCommandHandler.h"
#include "../implementations/VLCommandHandler.h"
#include "../platform/timeutils.h"

using namespace CEC;

CCECBusDevice::CCECBusDevice(CCECProcessor *processor, cec_logical_address iLogicalAddress, uint16_t iPhysicalAddress) :
  m_iPhysicalAddress(iPhysicalAddress),
  m_iStreamPath(0),
  m_iLogicalAddress(iLogicalAddress),
  m_powerStatus(CEC_POWER_STATUS_UNKNOWN),
  m_processor(processor),
  m_bMenuActive(true),
  m_bActiveSource(false),
  m_iVendorClass(CEC_VENDOR_UNKNOWN),
  m_iLastCommandSent(0),
  m_iLastActive(0),
  m_cecVersion(CEC_VERSION_UNKNOWN)
{
  m_handler = new CCECCommandHandler(this);

  for (unsigned int iPtr = 0; iPtr < 4; iPtr++)
    m_menuLanguage.language[iPtr] = '?';
  m_menuLanguage.language[3] = 0;
  m_menuLanguage.device = iLogicalAddress;

  m_vendor.vendor = CEC_VENDOR_UNKNOWN;
  m_type          = CEC_DEVICE_TYPE_RESERVED;
  m_strDeviceName = CCECCommandHandler::ToString(m_iLogicalAddress);
}

CCECBusDevice::~CCECBusDevice(void)
{
  m_condition.Broadcast();
  delete m_handler;
}

void CCECBusDevice::AddLog(cec_log_level level, const CStdString &strMessage)
{
  m_processor->AddLog(level, strMessage);
}

bool CCECBusDevice::HandleCommand(const cec_command &command)
{
  CLockObject lock(&m_mutex);
  m_iLastActive = GetTimeMs();
  m_handler->HandleCommand(command);
  m_condition.Signal();
  return true;
}

void CCECBusDevice::PollVendorId(void)
{
  CLockObject lock(&m_mutex);
  if (m_iLastActive > 0 && m_iLogicalAddress != CECDEVICE_BROADCAST &&
      m_vendor.vendor == CEC_VENDOR_UNKNOWN &&
      GetTimeMs() - m_iLastCommandSent > 5000 &&
      !m_processor->IsMonitoring())
  {
    CStdString strLog;
    strLog.Format("<< requesting vendor ID of '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
    AddLog(CEC_LOG_NOTICE, strLog);
    m_iLastCommandSent = GetTimeMs();

    cec_command command;
    cec_command::format(command, GetMyLogicalAddress(), m_iLogicalAddress, CEC_OPCODE_GIVE_DEVICE_VENDOR_ID);
    if (m_processor->Transmit(command))
      m_condition.Wait(&m_mutex, 1000);
  }
}

bool CCECBusDevice::PowerOn(void)
{
  cec_power_status current = GetPowerStatus();
  if (current != CEC_POWER_STATUS_ON &&
      current != CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON)
  {
    CStdString strLog;
    strLog.Format("<< powering on '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
    AddLog(CEC_LOG_DEBUG, strLog.c_str());

    SetPowerStatus(CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON);

    cec_command command;
    cec_command::format(command, GetMyLogicalAddress(), m_iLogicalAddress, CEC_OPCODE_IMAGE_VIEW_ON);

    return m_processor->Transmit(command);
  }

  return true;
}

bool CCECBusDevice::Standby(void)
{
  CStdString strLog;
  strLog.Format("<< putting '%s' (%X) in standby mode", GetLogicalAddressName(), m_iLogicalAddress);
  AddLog(CEC_LOG_DEBUG, strLog.c_str());

  cec_command command;
  cec_command::format(command, GetMyLogicalAddress(), m_iLogicalAddress, CEC_OPCODE_STANDBY);

  return m_processor->Transmit(command);
}

/** @name Getters */
//@{
cec_version CCECBusDevice::GetCecVersion(void)
{
  if (m_cecVersion == CEC_VERSION_UNKNOWN)
  {
    if (!MyLogicalAddressContains(m_iLogicalAddress))
    {
      CStdString strLog;
      strLog.Format("<< requesting CEC version of '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
      AddLog(CEC_LOG_NOTICE, strLog);
      cec_command command;
      cec_command::format(command, GetMyLogicalAddress(), m_iLogicalAddress, CEC_OPCODE_GET_CEC_VERSION);
      CLockObject lock(&m_mutex);
      if (m_processor->Transmit(command))
        m_condition.Wait(&m_mutex, 1000);
    }
  }

  return m_cecVersion;
}

const char* CCECBusDevice::GetLogicalAddressName(void) const
{
  return CCECCommandHandler::ToString(m_iLogicalAddress);
}

cec_menu_language &CCECBusDevice::GetMenuLanguage(void)
{
  if (!strcmp(m_menuLanguage.language, "???"))
  {
    if (!MyLogicalAddressContains(m_iLogicalAddress))
    {
      CStdString strLog;
      strLog.Format("<< requesting menu language of '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
      AddLog(CEC_LOG_NOTICE, strLog);
      cec_command command;
      cec_command::format(command, GetMyLogicalAddress(), m_iLogicalAddress, CEC_OPCODE_GET_MENU_LANGUAGE);
      CLockObject lock(&m_mutex);
      if (m_processor->Transmit(command))
        m_condition.Wait(&m_mutex, 1000);
    }
  }

  return m_menuLanguage;
}

cec_logical_address CCECBusDevice::GetMyLogicalAddress(void) const
{
  return m_processor->GetLogicalAddress();
}

uint16_t CCECBusDevice::GetMyPhysicalAddress(void) const
{
  return m_processor->GetPhysicalAddress();
}

cec_power_status CCECBusDevice::GetPowerStatus(void)
{
  if (m_powerStatus == CEC_POWER_STATUS_UNKNOWN)
  {
    if (!MyLogicalAddressContains(m_iLogicalAddress))
    {
      CStdString strLog;
      strLog.Format("<< requesting power status of '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
      AddLog(CEC_LOG_NOTICE, strLog);
      cec_command command;
      cec_command::format(command, GetMyLogicalAddress(), m_iLogicalAddress, CEC_OPCODE_GIVE_DEVICE_POWER_STATUS);
      CLockObject lock(&m_mutex);
      if (m_processor->Transmit(command))
        m_condition.Wait(&m_mutex, 1000);
    }
  }

  return m_powerStatus;
}

const cec_vendor &CCECBusDevice::GetVendor(void)
{
  if (m_vendor.vendor == CEC_VENDOR_UNKNOWN)
  {
    if (!MyLogicalAddressContains(m_iLogicalAddress))
    {
      CStdString strLog;
      strLog.Format("<< requesting vendor ID of '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
      AddLog(CEC_LOG_NOTICE, strLog);
      cec_command command;
      cec_command::format(command, GetMyLogicalAddress(), m_iLogicalAddress, CEC_OPCODE_GIVE_DEVICE_VENDOR_ID);
      CLockObject lock(&m_mutex);

      if (m_processor->Transmit(command))
        m_condition.Wait(&m_mutex, 1000);
    }
  }

  return m_vendor;
}

bool CCECBusDevice::MyLogicalAddressContains(cec_logical_address address) const
{
  return m_processor->HasLogicalAddress(address);
}

//@}

/** @name Setters */
//@{
void CCECBusDevice::SetCecVersion(const cec_version newVersion)
{
  CStdString strLog;
  m_cecVersion = newVersion;

  switch (newVersion)
  {
  case CEC_VERSION_1_2:
    strLog.Format("%s (%X): CEC version 1.2", GetLogicalAddressName(), m_iLogicalAddress);
    break;
  case CEC_VERSION_1_2A:
    strLog.Format("%s (%X): CEC version 1.2a", GetLogicalAddressName(), m_iLogicalAddress);
    break;
  case CEC_VERSION_1_3:
    strLog.Format("%s (%X): CEC version 1.3", GetLogicalAddressName(), m_iLogicalAddress);
    break;
  case CEC_VERSION_1_3A:
    strLog.Format("%s (%X): CEC version 1.3a", GetLogicalAddressName(), m_iLogicalAddress);
    break;
  default:
    strLog.Format("%s (%X): unknown CEC version", GetLogicalAddressName(), m_iLogicalAddress);
    m_cecVersion = CEC_VERSION_UNKNOWN;
    break;
  }
  AddLog(CEC_LOG_DEBUG, strLog);
}

void CCECBusDevice::SetMenuLanguage(const cec_menu_language &language)
{
  if (language.device == m_iLogicalAddress)
  {
    CStdString strLog;
    strLog.Format(">> %s (%X): menu language set to '%s'", GetLogicalAddressName(), m_iLogicalAddress, language.language);
    m_processor->AddLog(CEC_LOG_DEBUG, strLog);
    m_menuLanguage = language;
  }
}

void CCECBusDevice::SetPhysicalAddress(uint16_t iNewAddress)
{
  if (iNewAddress > 0)
  {
    CStdString strLog;
    strLog.Format(">> %s (%X): physical address changed from %04x to %04x", GetLogicalAddressName(), m_iLogicalAddress, m_iPhysicalAddress, iNewAddress);
    AddLog(CEC_LOG_DEBUG, strLog.c_str());

    m_iPhysicalAddress = iNewAddress;
  }
}

void CCECBusDevice::SetStreamPath(uint16_t iNewAddress, uint16_t iOldAddress /* = 0 */)
{
  if (iNewAddress > 0)
  {
    CStdString strLog;
    strLog.Format(">> %s (%X): stream path changed from %04x to %04x", GetLogicalAddressName(), m_iLogicalAddress, iOldAddress, iNewAddress);
    AddLog(CEC_LOG_DEBUG, strLog.c_str());

    m_iStreamPath = iNewAddress;

    if (iNewAddress > 0)
      SetPowerStatus(CEC_POWER_STATUS_ON);
  }
}

void CCECBusDevice::SetPowerStatus(const cec_power_status powerStatus)
{
  if (m_powerStatus != powerStatus)
  {
    CStdString strLog;
    strLog.Format(">> %s (%X): power status changed from '%s' to '%s'", GetLogicalAddressName(), m_iLogicalAddress, CCECCommandHandler::ToString(m_powerStatus), CCECCommandHandler::ToString(powerStatus));
    m_processor->AddLog(CEC_LOG_DEBUG, strLog);
    m_powerStatus = powerStatus;
  }
}

void CCECBusDevice::SetVendorId(uint64_t iVendorId, uint8_t iVendorClass /* = 0 */)
{
  m_vendor.vendor = (cec_vendor_id)iVendorId;
  m_iVendorClass = iVendorClass;

  switch (iVendorId)
  {
  case CEC_VENDOR_SAMSUNG:
    if (m_handler->GetVendorId() != CEC_VENDOR_SAMSUNG)
    {
      delete m_handler;
      m_handler = new CANCommandHandler(this);
    }
    break;
  case CEC_VENDOR_LG:
    if (m_handler->GetVendorId() != CEC_VENDOR_LG)
    {
      delete m_handler;
      m_handler = new CSLCommandHandler(this);
    }
    break;
  case CEC_VENDOR_PANASONIC:
    if (m_handler->GetVendorId() != CEC_VENDOR_PANASONIC)
    {
      delete m_handler;
      m_handler = new CVLCommandHandler(this);
    }
    break;
  default:
    if (m_handler->GetVendorId() != CEC_VENDOR_UNKNOWN)
    {
      delete m_handler;
      m_handler = new CCECCommandHandler(this);
    }
    break;
  }

  CStdString strLog;
  strLog.Format("%s (%X): vendor = %s (%06x) class = %2x", GetLogicalAddressName(), m_iLogicalAddress, GetVendorName(), GetVendorId(), GetVendorClass());
  m_processor->AddLog(CEC_LOG_DEBUG, strLog.c_str());
}
//@}

/** @name Transmit methods */
//@{
bool CCECBusDevice::TransmitActiveSource(void)
{
  if (m_powerStatus != CEC_POWER_STATUS_ON)
  {
    CStdString strLog;
    strLog.Format("<< %s (%X) is not powered on", GetLogicalAddressName(), m_iLogicalAddress);
    AddLog(CEC_LOG_DEBUG, strLog);
  }
  else if (m_bActiveSource)
  {
    CStdString strLog;
    strLog.Format("<< %s (%X) -> broadcast (F): active source (%4x)", GetLogicalAddressName(), m_iLogicalAddress, m_iPhysicalAddress);
    AddLog(CEC_LOG_NOTICE, strLog);

    cec_command command;
    cec_command::format(command, m_iLogicalAddress, CECDEVICE_BROADCAST, CEC_OPCODE_ACTIVE_SOURCE);
    command.parameters.push_back((uint8_t) ((m_iPhysicalAddress >> 8) & 0xFF));
    command.parameters.push_back((uint8_t) (m_iPhysicalAddress & 0xFF));

    return m_processor->Transmit(command);
  }
  else
  {
    CStdString strLog;
    strLog.Format("<< %s (%X) is not the active source", GetLogicalAddressName(), m_iLogicalAddress);
    AddLog(CEC_LOG_DEBUG, strLog);
  }

  return false;
}

bool CCECBusDevice::TransmitCECVersion(cec_logical_address dest)
{
  CStdString strLog;
  strLog.Format("<< %s (%X) -> %s (%X): cec version ", GetLogicalAddressName(), m_iLogicalAddress, CCECCommandHandler::ToString(dest), dest);
  switch (m_cecVersion)
  {
  case CEC_VERSION_1_2:
    strLog.append("1.2");
    break;
  case CEC_VERSION_1_2A:
    strLog.append("1.2a");
    break;
  case CEC_VERSION_1_3:
    strLog.append("1.3");
    break;
  case CEC_VERSION_1_3A:
    strLog.append("1.3a");
    break;
  default:
    strLog.append("unknown");
    break;
  }
  AddLog(CEC_LOG_NOTICE, strLog);

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, dest, CEC_OPCODE_CEC_VERSION);
  command.parameters.push_back(m_cecVersion);

  return m_processor->Transmit(command);
}

bool CCECBusDevice::TransmitInactiveView(void)
{
  CStdString strLog;
  strLog.Format("<< %s (%X) -> broadcast (F): inactive view", GetLogicalAddressName(), m_iLogicalAddress);
  AddLog(CEC_LOG_NOTICE, strLog);

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, CECDEVICE_BROADCAST, CEC_OPCODE_INACTIVE_SOURCE);
  command.parameters.push_back((m_iPhysicalAddress >> 8) & 0xFF);
  command.parameters.push_back(m_iPhysicalAddress & 0xFF);

  return m_processor->Transmit(command);
}

bool CCECBusDevice::TransmitMenuState(cec_logical_address dest)
{
  CStdString strLog;
  strLog.Format("<< %s (%X) -> %s (%X): ", GetLogicalAddressName(), m_iLogicalAddress, CCECCommandHandler::ToString(dest), dest);
  if (m_bMenuActive)
    strLog.append("menu active");
  else
    strLog.append("menu inactive");
  AddLog(CEC_LOG_NOTICE, strLog);

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, dest, CEC_OPCODE_MENU_STATUS);
  command.parameters.push_back(m_bMenuActive ? (uint8_t) CEC_MENU_STATE_ACTIVATED : (uint8_t) CEC_MENU_STATE_DEACTIVATED);

  return m_processor->Transmit(command);
}

bool CCECBusDevice::TransmitOSDName(cec_logical_address dest)
{
  CStdString strLog;
  strLog.Format("<< %s (%X) -> %s (%X): OSD name '%s'", GetLogicalAddressName(), m_iLogicalAddress, CCECCommandHandler::ToString(dest), dest, m_strDeviceName.c_str());
  AddLog(CEC_LOG_NOTICE, strLog.c_str());

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, dest, CEC_OPCODE_SET_OSD_NAME);
  for (unsigned int iPtr = 0; iPtr < m_strDeviceName.length(); iPtr++)
    command.parameters.push_back(m_strDeviceName.at(iPtr));

  return m_processor->Transmit(command);
}

bool CCECBusDevice::TransmitOSDString(cec_logical_address dest, cec_display_control duration, const char *strMessage)
{
  CStdString strLog;
  strLog.Format("<< %s (%X) -> %s (%X): display OSD message '%s'", GetLogicalAddressName(), m_iLogicalAddress, CCECCommandHandler::ToString(dest), dest, strMessage);
  AddLog(CEC_LOG_NOTICE, strLog.c_str());

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, dest, CEC_OPCODE_SET_OSD_STRING);
  command.parameters.push_back((uint8_t)duration);

  unsigned int iLen = strlen(strMessage);
  if (iLen > 13) iLen = 13;

  for (unsigned int iPtr = 0; iPtr < iLen; iPtr++)
    command.parameters.push_back(strMessage[iPtr]);

  return m_processor->Transmit(command);
}

bool CCECBusDevice::TransmitPhysicalAddress(void)
{
  CStdString strLog;
  strLog.Format("<< %s (%X) -> broadcast (F): physical adddress %4x", GetLogicalAddressName(), m_iLogicalAddress, m_iPhysicalAddress);
  AddLog(CEC_LOG_NOTICE, strLog.c_str());

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, CECDEVICE_BROADCAST, CEC_OPCODE_REPORT_PHYSICAL_ADDRESS);
  command.parameters.push_back((uint8_t) ((m_iPhysicalAddress >> 8) & 0xFF));
  command.parameters.push_back((uint8_t) (m_iPhysicalAddress & 0xFF));
  command.parameters.push_back((uint8_t) (m_type));

  return m_processor->Transmit(command);
}

bool CCECBusDevice::TransmitPoll(cec_logical_address dest)
{
  bool bReturn(false);

  if (dest == CECDEVICE_UNKNOWN)
    dest = m_iLogicalAddress;

  CStdString strLog;
  strLog.Format("<< %s (%X) -> %s (%X): POLL", GetLogicalAddressName(), m_iLogicalAddress, CCECCommandHandler::ToString(dest), dest);
  AddLog(CEC_LOG_NOTICE, strLog.c_str());

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, dest, CEC_OPCODE_NONE);
  CLockObject lock(&m_mutex);

  bReturn = m_processor->Transmit(command);
  AddLog(CEC_LOG_DEBUG, bReturn ? ">> POLL sent" : ">> POLL not sent");
  return bReturn;
}

bool CCECBusDevice::TransmitPowerState(cec_logical_address dest)
{
  CStdString strLog;
  strLog.Format("<< %s (%X) -> %s (%X): %s", GetLogicalAddressName(), m_iLogicalAddress, CCECCommandHandler::ToString(dest), dest, CCECCommandHandler::ToString(m_powerStatus));
  AddLog(CEC_LOG_NOTICE, strLog.c_str());

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, dest, CEC_OPCODE_REPORT_POWER_STATUS);
  command.parameters.push_back((uint8_t) m_powerStatus);

  return m_processor->Transmit(command);
}

bool CCECBusDevice::TransmitVendorID(cec_logical_address dest)
{
  CStdString strLog;
  strLog.Format("<< %s (%X) -> %s (%X): vendor id feature abort", GetLogicalAddressName(), m_iLogicalAddress, CCECCommandHandler::ToString(dest), dest);
  AddLog(CEC_LOG_NOTICE, strLog);

  m_processor->TransmitAbort(dest, CEC_OPCODE_GIVE_DEVICE_VENDOR_ID);
  return false;
}
//@}
