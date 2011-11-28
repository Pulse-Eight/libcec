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

#define ToString(p) CCECCommandHandler::ToString(p)

CCECBusDevice::CCECBusDevice(CCECProcessor *processor, cec_logical_address iLogicalAddress, uint16_t iPhysicalAddress) :
  m_type(CEC_DEVICE_TYPE_RESERVED),
  m_iPhysicalAddress(iPhysicalAddress),
  m_iStreamPath(0),
  m_iLogicalAddress(iLogicalAddress),
  m_powerStatus(CEC_POWER_STATUS_UNKNOWN),
  m_processor(processor),
  m_vendor(CEC_VENDOR_UNKNOWN),
  m_menuState(CEC_MENU_STATE_ACTIVATED),
  m_bActiveSource(false),
  m_iLastCommandSent(0),
  m_iLastActive(0),
  m_cecVersion(CEC_VERSION_UNKNOWN),
  m_deviceStatus(CEC_DEVICE_STATUS_UNKNOWN)
{
  m_handler = new CCECCommandHandler(this);

  for (unsigned int iPtr = 0; iPtr < 4; iPtr++)
    m_menuLanguage.language[iPtr] = '?';
  m_menuLanguage.language[3] = 0;
  m_menuLanguage.device = iLogicalAddress;

  m_strDeviceName = ToString(m_iLogicalAddress);
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
  CLockObject lock(&m_transmitMutex);
  m_iLastActive = GetTimeMs();
  m_handler->HandleCommand(command);
  if (m_deviceStatus != CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC)
    m_deviceStatus = CEC_DEVICE_STATUS_PRESENT;
  m_condition.Signal();
  return true;
}

void CCECBusDevice::PollVendorId(void)
{
  CLockObject lock(&m_transmitMutex);
  if (m_iLastActive > 0 && m_iLogicalAddress != CECDEVICE_BROADCAST &&
      m_vendor == CEC_VENDOR_UNKNOWN &&
      GetTimeMs() - m_iLastCommandSent > 5000 &&
      !m_processor->IsMonitoring())
  {
    CStdString strLog;
    strLog.Format("<< requesting vendor ID of '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
    AddLog(CEC_LOG_NOTICE, strLog);
    m_iLastCommandSent = GetTimeMs();

    cec_command command;
    cec_command::Format(command, GetMyLogicalAddress(), m_iLogicalAddress, CEC_OPCODE_GIVE_DEVICE_VENDOR_ID);
    if (m_processor->Transmit(command))
      m_condition.Wait(&m_transmitMutex, 1000);
  }
}

bool CCECBusDevice::PowerOn(void)
{
   CStdString strLog;
   strLog.Format("<< powering on '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
   AddLog(CEC_LOG_DEBUG, strLog.c_str());

   cec_command command;
   cec_command::Format(command, GetMyLogicalAddress(), m_iLogicalAddress, CEC_OPCODE_IMAGE_VIEW_ON);
   if (m_processor->Transmit(command))
   {
     GetPowerStatus();
     return true;
  }

  return false;
}

bool CCECBusDevice::Standby(void)
{
  CStdString strLog;
  strLog.Format("<< putting '%s' (%X) in standby mode", GetLogicalAddressName(), m_iLogicalAddress);
  AddLog(CEC_LOG_DEBUG, strLog.c_str());

  cec_command command;
  cec_command::Format(command, GetMyLogicalAddress(), m_iLogicalAddress, CEC_OPCODE_STANDBY);

  return m_processor->Transmit(command);
}

/** @name Getters */
//@{
cec_version CCECBusDevice::GetCecVersion(void)
{
  CLockObject lock(&m_mutex);
  if (m_cecVersion == CEC_VERSION_UNKNOWN)
  {
    lock.Leave();
    RequestCecVersion();
    lock.Lock();
  }

  return m_cecVersion;
}

bool CCECBusDevice::RequestCecVersion(void)
{
  bool bReturn(false);
  if (!MyLogicalAddressContains(m_iLogicalAddress))
  {
    CStdString strLog;
    strLog.Format("<< requesting CEC version of '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
    AddLog(CEC_LOG_NOTICE, strLog);
    cec_command command;
    cec_command::Format(command, GetMyLogicalAddress(), m_iLogicalAddress, CEC_OPCODE_GET_CEC_VERSION);
    CLockObject lock(&m_transmitMutex);
    if (m_processor->Transmit(command))
      bReturn = m_condition.Wait(&m_transmitMutex, 1000);
  }
  return bReturn;
}

const char* CCECBusDevice::GetLogicalAddressName(void) const
{
  return ToString(m_iLogicalAddress);
}

cec_menu_language &CCECBusDevice::GetMenuLanguage(void)
{
  CLockObject lock(&m_mutex);
  if (!strcmp(m_menuLanguage.language, "???"))
  {
    lock.Leave();
    RequestMenuLanguage();
    lock.Lock();
  }
  return m_menuLanguage;
}

bool CCECBusDevice::RequestMenuLanguage(void)
{
  bool bReturn(false);
  if (!MyLogicalAddressContains(m_iLogicalAddress))
  {
    CStdString strLog;
    strLog.Format("<< requesting menu language of '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
    AddLog(CEC_LOG_NOTICE, strLog);
    cec_command command;
    cec_command::Format(command, GetMyLogicalAddress(), m_iLogicalAddress, CEC_OPCODE_GET_MENU_LANGUAGE);
    CLockObject lock(&m_transmitMutex);
    if (m_processor->Transmit(command))
      bReturn = m_condition.Wait(&m_transmitMutex, 1000);
  }
  return bReturn;
}

cec_logical_address CCECBusDevice::GetMyLogicalAddress(void) const
{
  return m_processor->GetLogicalAddress();
}

uint16_t CCECBusDevice::GetMyPhysicalAddress(void) const
{
  return m_processor->GetPhysicalAddress();
}

uint16_t CCECBusDevice::GetPhysicalAddress(bool bRefresh /* = true */)
{
  if (GetStatus() == CEC_DEVICE_STATUS_PRESENT)
  {
    CLockObject lock(&m_mutex);
    if (m_iPhysicalAddress == 0xFFFF || bRefresh)
    {
      lock.Leave();
      RequestPhysicalAddress();
      lock.Lock();
    }
  }

  CLockObject lock(&m_mutex);
  return m_iPhysicalAddress;
}

bool CCECBusDevice::RequestPhysicalAddress(void)
{
  bool bReturn(false);
  if (!MyLogicalAddressContains(m_iLogicalAddress))
  {
    CStdString strLog;
    strLog.Format("<< requesting physical address of '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
    AddLog(CEC_LOG_NOTICE, strLog);
    cec_command command;
    cec_command::Format(command, GetMyLogicalAddress(), m_iLogicalAddress, CEC_OPCODE_GIVE_PHYSICAL_ADDRESS);
    CLockObject lock(&m_transmitMutex);
    if (m_processor->Transmit(command))
      bReturn = m_condition.Wait(&m_transmitMutex, 1000);
  }
  return bReturn;
}

cec_power_status CCECBusDevice::GetPowerStatus(void)
{
  CLockObject lock(&m_mutex);
  if (m_powerStatus == CEC_POWER_STATUS_UNKNOWN)
  {
    lock.Leave();
    RequestPowerStatus();
    lock.Lock();
  }
  return m_powerStatus;
}

bool CCECBusDevice::RequestPowerStatus(void)
{
  bool bReturn(false);
  if (!MyLogicalAddressContains(m_iLogicalAddress))
  {
    CStdString strLog;
    strLog.Format("<< requesting power status of '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
    AddLog(CEC_LOG_NOTICE, strLog);
    cec_command command;
    cec_command::Format(command, GetMyLogicalAddress(), m_iLogicalAddress, CEC_OPCODE_GIVE_DEVICE_POWER_STATUS);
    CLockObject lock(&m_transmitMutex);
    if (m_processor->Transmit(command))
      bReturn = m_condition.Wait(&m_transmitMutex, 1000);
  }
  return bReturn;
}

cec_vendor_id CCECBusDevice::GetVendorId(void)
{
  CLockObject lock(&m_mutex);
  if (m_vendor == CEC_VENDOR_UNKNOWN)
  {
    lock.Leave();
    RequestVendorId();
    lock.Lock();
  }
  return m_vendor;
}

bool CCECBusDevice::RequestVendorId(void)
{
  bool bReturn(false);
  if (!MyLogicalAddressContains(m_iLogicalAddress))
  {
    CStdString strLog;
    strLog.Format("<< requesting vendor ID of '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
    AddLog(CEC_LOG_NOTICE, strLog);
    cec_command command;
    cec_command::Format(command, GetMyLogicalAddress(), m_iLogicalAddress, CEC_OPCODE_GIVE_DEVICE_VENDOR_ID);

    CLockObject lock(&m_transmitMutex);
    if (m_processor->Transmit(command))
      bReturn = m_condition.Wait(&m_transmitMutex, 1000);
  }
  return bReturn;
}

const char *CCECBusDevice::GetVendorName(void)
{
  return ToString(GetVendorId());
}

bool CCECBusDevice::MyLogicalAddressContains(cec_logical_address address) const
{
  return m_processor->HasLogicalAddress(address);
}

cec_bus_device_status CCECBusDevice::GetStatus(void)
{
  CLockObject lock(&m_mutex);
  if (m_deviceStatus == CEC_DEVICE_STATUS_UNKNOWN)
  {
    lock.Leave();
    bool bPollAcked = m_processor->PollDevice(m_iLogicalAddress);

    lock.Lock();
    m_deviceStatus = bPollAcked ? CEC_DEVICE_STATUS_PRESENT : CEC_DEVICE_STATUS_NOT_PRESENT;
  }

  return m_deviceStatus;
}

//@}

/** @name Setters */
//@{
void CCECBusDevice::SetCecVersion(const cec_version newVersion)
{
  m_cecVersion = newVersion;

  CStdString strLog;
  strLog.Format("%s (%X): CEC version %s", GetLogicalAddressName(), m_iLogicalAddress, ToString(newVersion));
  AddLog(CEC_LOG_DEBUG, strLog);
}

void CCECBusDevice::SetMenuLanguage(const cec_menu_language &language)
{
  CLockObject lock(&m_mutex);
  if (language.device == m_iLogicalAddress)
  {
    CStdString strLog;
    strLog.Format(">> %s (%X): menu language set to '%s'", GetLogicalAddressName(), m_iLogicalAddress, language.language);
    m_processor->AddLog(CEC_LOG_DEBUG, strLog);
    m_menuLanguage = language;
  }
}

void CCECBusDevice::SetOSDName(CStdString strName)
{
  CLockObject lock(&m_mutex);
  if (m_strDeviceName != strName)
  {
    CStdString strLog;
    strLog.Format(">> %s (%X): osd name set to '%s'", GetLogicalAddressName(), m_iLogicalAddress, strName);
    m_processor->AddLog(CEC_LOG_DEBUG, strLog);
    m_strDeviceName = strName;
  }
}

void CCECBusDevice::SetMenuState(const cec_menu_state state)
{
  CLockObject lock(&m_mutex);
  if (m_menuState != state)
  {
    CStdString strLog;
    strLog.Format(">> %s (%X): menu state set to '%s'", GetLogicalAddressName(), m_iLogicalAddress, ToString(m_menuState));
    m_processor->AddLog(CEC_LOG_DEBUG, strLog);
    m_menuState = state;
  }
}

void CCECBusDevice::SetInactiveDevice(void)
{
  CLockObject lock(&m_mutex);
  m_bActiveSource = false;
}

void CCECBusDevice::SetActiveDevice(void)
{
  CLockObject lock(&m_mutex);

  for (int iPtr = 0; iPtr < 16; iPtr++)
    if (iPtr != m_iLogicalAddress)
      m_processor->m_busDevices[iPtr]->SetInactiveDevice();

  m_bActiveSource = true;
  m_powerStatus   = CEC_POWER_STATUS_ON;
}

bool CCECBusDevice::TryLogicalAddress(void)
{
  CStdString strLog;
  strLog.Format("trying logical address '%s'", GetLogicalAddressName());
  AddLog(CEC_LOG_DEBUG, strLog);

  m_processor->SetAckMask(0x1 << m_iLogicalAddress);
  if (!TransmitPoll(m_iLogicalAddress))
  {
    strLog.Format("using logical address '%s'", GetLogicalAddressName());
    AddLog(CEC_LOG_NOTICE, strLog);
    SetDeviceStatus(CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC);

    return true;
  }

  strLog.Format("logical address '%s' already taken", GetLogicalAddressName());
  AddLog(CEC_LOG_DEBUG, strLog);
  SetDeviceStatus(CEC_DEVICE_STATUS_PRESENT);
  return false;
}

void CCECBusDevice::SetDeviceStatus(const cec_bus_device_status newStatus)
{
  CLockObject lock(&m_mutex);
  switch (newStatus)
  {
  case CEC_DEVICE_STATUS_UNKNOWN:
    m_iStreamPath      = 0;
    m_powerStatus      = CEC_POWER_STATUS_UNKNOWN;
    m_vendor           = CEC_VENDOR_UNKNOWN;
    m_menuState        = CEC_MENU_STATE_ACTIVATED;
    m_bActiveSource    = false;
    m_iLastCommandSent = 0;
    m_iLastActive      = 0;
    m_cecVersion       = CEC_VERSION_UNKNOWN;
    m_deviceStatus     = newStatus;
    break;
  case CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC:
    m_iStreamPath      = 0;
    m_powerStatus      = CEC_POWER_STATUS_ON;
    m_vendor           = CEC_VENDOR_UNKNOWN;
    m_menuState        = CEC_MENU_STATE_ACTIVATED;
    m_bActiveSource    = false;
    m_iLastCommandSent = 0;
    m_iLastActive      = 0;
    m_cecVersion       = CEC_VERSION_1_3A;
    m_deviceStatus     = newStatus;
    break;
  case CEC_DEVICE_STATUS_PRESENT:
  case CEC_DEVICE_STATUS_NOT_PRESENT:
    m_deviceStatus = newStatus;
    break;
  }
}

void CCECBusDevice::SetPhysicalAddress(uint16_t iNewAddress)
{
  CLockObject lock(&m_mutex);
  if (iNewAddress > 0 && m_iPhysicalAddress != iNewAddress)
  {
    CStdString strLog;
    strLog.Format(">> %s (%X): physical address changed from %04x to %04x", GetLogicalAddressName(), m_iLogicalAddress, m_iPhysicalAddress, iNewAddress);
    AddLog(CEC_LOG_DEBUG, strLog.c_str());

    m_iPhysicalAddress = iNewAddress;
  }
}

void CCECBusDevice::SetStreamPath(uint16_t iNewAddress, uint16_t iOldAddress /* = 0 */)
{
  CLockObject lock(&m_mutex);
  if (iNewAddress > 0)
  {
    CStdString strLog;
    strLog.Format(">> %s (%X): stream path changed from %04x to %04x", GetLogicalAddressName(), m_iLogicalAddress, iOldAddress == 0 ? m_iStreamPath : iOldAddress, iNewAddress);
    AddLog(CEC_LOG_DEBUG, strLog.c_str());

    m_iStreamPath = iNewAddress;

    if (iNewAddress > 0)
    {
      lock.Leave();
      SetPowerStatus(CEC_POWER_STATUS_ON);
    }
  }
}

void CCECBusDevice::SetPowerStatus(const cec_power_status powerStatus)
{
  CLockObject lock(&m_mutex);
  if (m_powerStatus != powerStatus)
  {
    CStdString strLog;
    strLog.Format(">> %s (%X): power status changed from '%s' to '%s'", GetLogicalAddressName(), m_iLogicalAddress, ToString(m_powerStatus), ToString(powerStatus));
    m_processor->AddLog(CEC_LOG_DEBUG, strLog);
    m_powerStatus = powerStatus;
  }
}

void CCECBusDevice::SetVendorId(uint64_t iVendorId)
{
  {
    CLockObject lock(&m_mutex);
    m_vendor = (cec_vendor_id)iVendorId;

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
  }

  CStdString strLog;
  strLog.Format("%s (%X): vendor = %s (%06x)", GetLogicalAddressName(), m_iLogicalAddress, GetVendorName(), m_vendor);
  m_processor->AddLog(CEC_LOG_DEBUG, strLog.c_str());
}
//@}

/** @name Transmit methods */
//@{
bool CCECBusDevice::TransmitActiveSource(void)
{
  CLockObject lock(&m_mutex);
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
    cec_command::Format(command, m_iLogicalAddress, CECDEVICE_BROADCAST, CEC_OPCODE_ACTIVE_SOURCE);
    command.parameters.PushBack((uint8_t) ((m_iPhysicalAddress >> 8) & 0xFF));
    command.parameters.PushBack((uint8_t) (m_iPhysicalAddress & 0xFF));

    lock.Leave();
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
  CLockObject lock(&m_mutex);
  CStdString strLog;
  strLog.Format("<< %s (%X) -> %s (%X): cec version %s", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest, ToString(m_cecVersion));
  AddLog(CEC_LOG_NOTICE, strLog);

  cec_command command;
  cec_command::Format(command, m_iLogicalAddress, dest, CEC_OPCODE_CEC_VERSION);
  command.parameters.PushBack((uint8_t)m_cecVersion);

  lock.Leave();
  return m_processor->Transmit(command);
}

bool CCECBusDevice::TransmitInactiveView(void)
{
  CStdString strLog;
  strLog.Format("<< %s (%X) -> broadcast (F): inactive view", GetLogicalAddressName(), m_iLogicalAddress);
  AddLog(CEC_LOG_NOTICE, strLog);

  cec_command command;
  cec_command::Format(command, m_iLogicalAddress, CECDEVICE_BROADCAST, CEC_OPCODE_INACTIVE_SOURCE);
  command.parameters.PushBack((m_iPhysicalAddress >> 8) & 0xFF);
  command.parameters.PushBack(m_iPhysicalAddress & 0xFF);

  return m_processor->Transmit(command);
}

bool CCECBusDevice::TransmitMenuState(cec_logical_address dest)
{
  CStdString strLog;
  strLog.Format("<< %s (%X) -> %s (%X): menu state '%s'", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest, ToString(m_menuState));
  AddLog(CEC_LOG_NOTICE, strLog);

  cec_command command;
  cec_command::Format(command, m_iLogicalAddress, dest, CEC_OPCODE_MENU_STATUS);
  command.parameters.PushBack((uint8_t)m_menuState);

  return m_processor->Transmit(command);
}

bool CCECBusDevice::TransmitOSDName(cec_logical_address dest)
{
  CLockObject lock(&m_mutex);
  CStdString strLog;
  strLog.Format("<< %s (%X) -> %s (%X): OSD name '%s'", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest, m_strDeviceName.c_str());
  AddLog(CEC_LOG_NOTICE, strLog.c_str());

  cec_command command;
  cec_command::Format(command, m_iLogicalAddress, dest, CEC_OPCODE_SET_OSD_NAME);
  for (unsigned int iPtr = 0; iPtr < m_strDeviceName.length(); iPtr++)
    command.parameters.PushBack(m_strDeviceName.at(iPtr));

  lock.Leave();
  return m_processor->Transmit(command);
}

bool CCECBusDevice::TransmitOSDString(cec_logical_address dest, cec_display_control duration, const char *strMessage)
{
  CLockObject lock(&m_mutex);
  CStdString strLog;
  strLog.Format("<< %s (%X) -> %s (%X): display OSD message '%s'", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest, strMessage);
  AddLog(CEC_LOG_NOTICE, strLog.c_str());

  cec_command command;
  cec_command::Format(command, m_iLogicalAddress, dest, CEC_OPCODE_SET_OSD_STRING);
  command.parameters.PushBack((uint8_t)duration);

  unsigned int iLen = strlen(strMessage);
  if (iLen > 13) iLen = 13;

  for (unsigned int iPtr = 0; iPtr < iLen; iPtr++)
    command.parameters.PushBack(strMessage[iPtr]);

  lock.Leave();
  return m_processor->Transmit(command);
}

bool CCECBusDevice::TransmitPhysicalAddress(void)
{
  CLockObject lock(&m_mutex);
  CStdString strLog;
  strLog.Format("<< %s (%X) -> broadcast (F): physical adddress %4x", GetLogicalAddressName(), m_iLogicalAddress, m_iPhysicalAddress);
  AddLog(CEC_LOG_NOTICE, strLog.c_str());

  cec_command command;
  cec_command::Format(command, m_iLogicalAddress, CECDEVICE_BROADCAST, CEC_OPCODE_REPORT_PHYSICAL_ADDRESS);
  command.parameters.PushBack((uint8_t) ((m_iPhysicalAddress >> 8) & 0xFF));
  command.parameters.PushBack((uint8_t) (m_iPhysicalAddress & 0xFF));
  command.parameters.PushBack((uint8_t) (m_type));

  lock.Leave();
  return m_processor->Transmit(command);
}

bool CCECBusDevice::TransmitPoll(cec_logical_address dest)
{
  bool bReturn(false);
  if (dest == CECDEVICE_UNKNOWN)
    dest = m_iLogicalAddress;

  CStdString strLog;
  strLog.Format("<< %s (%X) -> %s (%X): POLL", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest);
  AddLog(CEC_LOG_NOTICE, strLog.c_str());

  cec_command command;
  cec_command::Format(command, m_iLogicalAddress, dest, CEC_OPCODE_NONE);

  {
    CLockObject lock(&m_transmitMutex);
    bReturn = m_processor->Transmit(command);
  }

  AddLog(CEC_LOG_DEBUG, bReturn ? ">> POLL sent" : ">> POLL not sent");

  if (bReturn)
  {
    CLockObject lock(&m_mutex);
    m_iLastActive = GetTimeMs();
  }

  return bReturn;
}

bool CCECBusDevice::TransmitPowerState(cec_logical_address dest)
{
  CLockObject lock(&m_mutex);
  CStdString strLog;
  strLog.Format("<< %s (%X) -> %s (%X): %s", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest, ToString(m_powerStatus));
  AddLog(CEC_LOG_NOTICE, strLog.c_str());

  cec_command command;
  cec_command::Format(command, m_iLogicalAddress, dest, CEC_OPCODE_REPORT_POWER_STATUS);
  command.parameters.PushBack((uint8_t) m_powerStatus);

  lock.Leave();
  return m_processor->Transmit(command);
}

bool CCECBusDevice::TransmitVendorID(cec_logical_address dest)
{
  CLockObject lock(&m_mutex);
  if (m_vendor == CEC_VENDOR_UNKNOWN)
  {
    CStdString strLog;
    strLog.Format("<< %s (%X) -> %s (%X): vendor id feature abort", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest);
    AddLog(CEC_LOG_NOTICE, strLog);

    lock.Leave();
    m_processor->TransmitAbort(dest, CEC_OPCODE_GIVE_DEVICE_VENDOR_ID);
    return false;
  }
  else
  {
    CStdString strLog;
    strLog.Format("<< %s (%X) -> %s (%X): vendor id %s (%x)", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest, ToString(m_vendor), (uint64_t)m_vendor);
    AddLog(CEC_LOG_NOTICE, strLog);

    cec_command command;
    cec_command::Format(command, m_iLogicalAddress, CECDEVICE_BROADCAST, CEC_OPCODE_DEVICE_VENDOR_ID);

    command.parameters.PushBack((uint8_t) (((uint64_t)m_vendor >> 16) & 0xFF));
    command.parameters.PushBack((uint8_t) (((uint64_t)m_vendor >> 8) & 0xFF));
    command.parameters.PushBack((uint8_t) ((uint64_t)m_vendor & 0xFF));

    lock.Leave();
    return m_processor->Transmit(command);
  }
}

bool CCECBusDevice::SendKeypress(cec_user_control_code key, bool bWait /* = false */)
{
  {
    CLockObject lock(&m_transmitMutex);
    cec_command command;
    cec_command::Format(command, m_processor->GetLogicalAddress(), m_iLogicalAddress, CEC_OPCODE_USER_CONTROL_PRESSED);
    command.parameters.PushBack(key);

    if (bWait)
    {
      if (m_processor->Transmit(command))
        return m_condition.Wait(&m_transmitMutex, 1000);
      return false;
    }

    return m_processor->Transmit(command);
  }
}

bool CCECBusDevice::SendKeyRelease(bool bWait /* = false */)
{
  {
    CLockObject lock(&m_transmitMutex);
    cec_command command;
    cec_command::Format(command, m_processor->GetLogicalAddress(), m_iLogicalAddress, CEC_OPCODE_USER_CONTROL_RELEASE);

    if (bWait)
    {
      if (m_processor->Transmit(command))
        return m_condition.Wait(&m_transmitMutex, 1000);
      return false;
    }
    else
    {
      return m_processor->Transmit(command);
    }
  }
}
//@}
