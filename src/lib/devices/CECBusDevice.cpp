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

#define ToString(p) m_processor->ToString(p)

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
  delete m_handler;
}

void CCECBusDevice::AddLog(cec_log_level level, const CStdString &strMessage)
{
  m_processor->AddLog(level, strMessage);
}

bool CCECBusDevice::HandleCommand(const cec_command &command)
{
  bool bHandled(false);

  /* update "last active" */
  {
    CLockObject lock(&m_writeMutex);
    m_iLastActive = GetTimeMs();
  }

  /* handle the command */
  bHandled = m_handler->HandleCommand(command);

  /* change status to present */
  if (bHandled)
  {
    CLockObject lock(&m_writeMutex);
    if (m_deviceStatus != CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC)
    {
      if (m_deviceStatus != CEC_DEVICE_STATUS_PRESENT)
      {
        CStdString strLog;
        strLog.Format("device %s (%x) status changed to present after command %s", GetLogicalAddressName(), (uint8_t)GetLogicalAddress(), ToString(command.opcode));
        AddLog(CEC_LOG_DEBUG, strLog);
      }
      m_deviceStatus = CEC_DEVICE_STATUS_PRESENT;
    }
  }

  return bHandled;
}

bool CCECBusDevice::PowerOn(void)
{
  CStdString strLog;
  strLog.Format("<< powering on '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
  AddLog(CEC_LOG_DEBUG, strLog.c_str());

  if (m_handler->TransmitPowerOn(GetMyLogicalAddress(), m_iLogicalAddress))
  {
    {
      CLockObject lock(&m_mutex);
      m_powerStatus = CEC_POWER_STATUS_UNKNOWN;
    }
    cec_power_status status = GetPowerStatus();
    if (status == CEC_POWER_STATUS_STANDBY || status == CEC_POWER_STATUS_UNKNOWN)
    {
      /* sending the normal power on command appears to have failed */
      CStdString strLog;
      strLog.Format("<< sending power on keypress to '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
      AddLog(CEC_LOG_DEBUG, strLog.c_str());

      TransmitKeypress(CEC_USER_CONTROL_CODE_POWER);
      return TransmitKeyRelease();
    }
    return true;
  }

  return false;
}

bool CCECBusDevice::Standby(void)
{
  CStdString strLog;
  strLog.Format("<< putting '%s' (%X) in standby mode", GetLogicalAddressName(), m_iLogicalAddress);
  AddLog(CEC_LOG_DEBUG, strLog.c_str());

  return m_handler->TransmitStandby(GetMyLogicalAddress(), m_iLogicalAddress);
}

/** @name Getters */
//@{
cec_version CCECBusDevice::GetCecVersion(bool bUpdate /* = false */)
{
  CLockObject lock(&m_mutex);
  if (GetStatus() == CEC_DEVICE_STATUS_PRESENT &&
      (bUpdate || m_cecVersion == CEC_VERSION_UNKNOWN))
    RequestCecVersion();

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

    bReturn = m_handler->TransmitRequestCecVersion(GetMyLogicalAddress(), m_iLogicalAddress);
  }
  return bReturn;
}

const char* CCECBusDevice::GetLogicalAddressName(void) const
{
  return ToString(m_iLogicalAddress);
}

cec_menu_language &CCECBusDevice::GetMenuLanguage(bool bUpdate /* = false */)
{
  CLockObject lock(&m_mutex);
  if (GetStatus() == CEC_DEVICE_STATUS_PRESENT &&
      (bUpdate || !strcmp(m_menuLanguage.language, "???")))
    RequestMenuLanguage();

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
    bReturn = m_handler->TransmitRequestMenuLanguage(GetMyLogicalAddress(), m_iLogicalAddress);
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

CStdString CCECBusDevice::GetOSDName(bool bUpdate /* = false */)
{
  CLockObject lock(&m_mutex);
  if (GetStatus() == CEC_DEVICE_STATUS_PRESENT &&
      (bUpdate || m_strDeviceName.Equals(ToString(m_iLogicalAddress))) &&
      m_type != CEC_DEVICE_TYPE_TV)
    RequestOSDName();

  return m_strDeviceName;
}

bool CCECBusDevice::RequestOSDName(void)
{
  bool bReturn(false);
  if (!MyLogicalAddressContains(m_iLogicalAddress))
  {
    CStdString strLog;
    strLog.Format("<< requesting OSD name of '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
    AddLog(CEC_LOG_NOTICE, strLog);
    bReturn = m_handler->TransmitRequestOSDName(GetMyLogicalAddress(), m_iLogicalAddress);
  }
  return bReturn;
}

uint16_t CCECBusDevice::GetPhysicalAddress(bool bUpdate /* = false */)
{
  CLockObject lock(&m_mutex);
  if (GetStatus() == CEC_DEVICE_STATUS_PRESENT &&
      (m_iPhysicalAddress == 0xFFFF || bUpdate))
  {
    if (!RequestPhysicalAddress())
      AddLog(CEC_LOG_ERROR, "failed to request the physical address");
  }

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
    bReturn = m_handler->TransmitRequestPhysicalAddress(GetMyLogicalAddress(), m_iLogicalAddress);
  }
  return bReturn;
}

cec_power_status CCECBusDevice::GetPowerStatus(bool bUpdate /* = false */)
{
  CLockObject lock(&m_mutex);
  if (GetStatus() == CEC_DEVICE_STATUS_PRESENT &&
      (bUpdate || m_powerStatus == CEC_POWER_STATUS_UNKNOWN))
    RequestPowerStatus();

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
    bReturn = m_handler->TransmitRequestPowerStatus(GetMyLogicalAddress(), m_iLogicalAddress);
  }
  return bReturn;
}

cec_vendor_id CCECBusDevice::GetVendorId(bool bUpdate /* = false */)
{
  CLockObject lock(&m_mutex);
  if (GetStatus() == CEC_DEVICE_STATUS_PRESENT &&
      (bUpdate || m_vendor == CEC_VENDOR_UNKNOWN))
    RequestVendorId();

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
    bReturn = m_handler->TransmitRequestVendorId(GetMyLogicalAddress(), m_iLogicalAddress);
  }
  return bReturn;
}

const char *CCECBusDevice::GetVendorName(bool bUpdate /* = false */)
{
  return ToString(GetVendorId(bUpdate));
}

bool CCECBusDevice::MyLogicalAddressContains(cec_logical_address address) const
{
  return m_processor->HasLogicalAddress(address);
}

bool CCECBusDevice::NeedsPoll(void)
{
  bool bSendPoll(false);
  switch (m_iLogicalAddress)
  {
  case CECDEVICE_PLAYBACKDEVICE3:
    if (m_processor->m_busDevices[CECDEVICE_PLAYBACKDEVICE2]->GetStatus() == CEC_DEVICE_STATUS_PRESENT)
      bSendPoll = true;
    break;
  case CECDEVICE_PLAYBACKDEVICE2:
    if (m_processor->m_busDevices[CECDEVICE_PLAYBACKDEVICE1]->GetStatus() == CEC_DEVICE_STATUS_PRESENT)
      bSendPoll = true;
    break;
  case CECDEVICE_RECORDINGDEVICE3:
    if (m_processor->m_busDevices[CECDEVICE_RECORDINGDEVICE2]->GetStatus() == CEC_DEVICE_STATUS_PRESENT)
      bSendPoll = true;
    break;
  case CECDEVICE_RECORDINGDEVICE2:
    if (m_processor->m_busDevices[CECDEVICE_RECORDINGDEVICE1]->GetStatus() == CEC_DEVICE_STATUS_PRESENT)
      bSendPoll = true;
    break;
  case CECDEVICE_TUNER4:
    if (m_processor->m_busDevices[CECDEVICE_TUNER3]->GetStatus() == CEC_DEVICE_STATUS_PRESENT)
      bSendPoll = true;
    break;
  case CECDEVICE_TUNER3:
    if (m_processor->m_busDevices[CECDEVICE_TUNER2]->GetStatus() == CEC_DEVICE_STATUS_PRESENT)
      bSendPoll = true;
    break;
  case CECDEVICE_TUNER2:
    if (m_processor->m_busDevices[CECDEVICE_TUNER1]->GetStatus() == CEC_DEVICE_STATUS_PRESENT)
      bSendPoll = true;
    break;
  case CECDEVICE_AUDIOSYSTEM:
  case CECDEVICE_PLAYBACKDEVICE1:
  case CECDEVICE_RECORDINGDEVICE1:
  case CECDEVICE_TUNER1:
  case CECDEVICE_TV:
    bSendPoll = true;
    break;
  default:
    break;
  }

  return bSendPoll;
}

cec_bus_device_status CCECBusDevice::GetStatus(bool bForcePoll /* = false */)
{
  CLockObject lock(&m_writeMutex);
  if (m_deviceStatus == CEC_DEVICE_STATUS_UNKNOWN || bForcePoll)
  {
    lock.Leave();
    bool bPollAcked(false);
    if (bForcePoll || NeedsPoll())
      bPollAcked = m_processor->PollDevice(m_iLogicalAddress);

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
  CLockObject lock(&m_writeMutex);
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
  CLockObject lock(&m_writeMutex);
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
  CLockObject lock(&m_writeMutex);
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
  CLockObject lock(&m_writeMutex);
  m_bActiveSource = false;
}

void CCECBusDevice::SetActiveDevice(void)
{
  CLockObject lock(&m_writeMutex);

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
  CLockObject lock(&m_writeMutex);
  switch (newStatus)
  {
  case CEC_DEVICE_STATUS_UNKNOWN:
    m_iStreamPath      = 0;
    m_powerStatus      = CEC_POWER_STATUS_UNKNOWN;
    m_vendor           = CEC_VENDOR_UNKNOWN;
    m_menuState        = CEC_MENU_STATE_ACTIVATED;
    m_bActiveSource    = false;
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
  CLockObject lock(&m_writeMutex);
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
  CLockObject lock(&m_writeMutex);
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
  CLockObject lock(&m_writeMutex);
  if (m_powerStatus != powerStatus)
  {
    CStdString strLog;
    strLog.Format(">> %s (%X): power status changed from '%s' to '%s'", GetLogicalAddressName(), m_iLogicalAddress, ToString(m_powerStatus), ToString(powerStatus));
    m_processor->AddLog(CEC_LOG_DEBUG, strLog);
    m_powerStatus = powerStatus;
  }
}

bool CCECBusDevice::SetVendorId(uint64_t iVendorId, bool bInitHandler /* = true */)
{
  bool bVendorChanged(false);

  {
    CLockObject lock(&m_writeMutex);
    bVendorChanged = (m_vendor != (cec_vendor_id)iVendorId);
    m_vendor = (cec_vendor_id)iVendorId;

    if (bVendorChanged)
      delete m_handler;

    switch (iVendorId)
    {
    case CEC_VENDOR_SAMSUNG:
      if (bVendorChanged)
        m_handler = new CANCommandHandler(this);
      break;
    case CEC_VENDOR_LG:
      if (bVendorChanged)
        m_handler = new CSLCommandHandler(this);
      break;
    case CEC_VENDOR_PANASONIC:
      if (bVendorChanged)
        m_handler = new CVLCommandHandler(this);
      break;
    default:
      if (bVendorChanged)
        m_handler = new CCECCommandHandler(this);
      break;
    }
  }

  if (bVendorChanged && bInitHandler)
    m_handler->InitHandler();

  CStdString strLog;
  strLog.Format("%s (%X): vendor = %s (%06x)", GetLogicalAddressName(), m_iLogicalAddress, ToString(m_vendor), m_vendor);
  m_processor->AddLog(CEC_LOG_DEBUG, strLog.c_str());

  return bVendorChanged;
}
//@}

/** @name Transmit methods */
//@{
bool CCECBusDevice::TransmitActiveSource(void)
{
  bool bSendActiveSource(false);

  {
    CLockObject lock(&m_writeMutex);
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
      bSendActiveSource = true;
    }
    else
    {
      CStdString strLog;
      strLog.Format("<< %s (%X) is not the active source", GetLogicalAddressName(), m_iLogicalAddress);
      AddLog(CEC_LOG_DEBUG, strLog);
    }
  }

  return bSendActiveSource ? m_handler->TransmitActiveSource(m_iLogicalAddress, m_iPhysicalAddress) : false;
}

bool CCECBusDevice::TransmitCECVersion(cec_logical_address dest)
{
  cec_version version;
  {
    CLockObject lock(&m_writeMutex);
    CStdString strLog;
    strLog.Format("<< %s (%X) -> %s (%X): cec version %s", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest, ToString(m_cecVersion));
    AddLog(CEC_LOG_NOTICE, strLog);
    version = m_cecVersion;
  }

  return m_handler->TransmitCECVersion(m_iLogicalAddress, dest, version);
}

bool CCECBusDevice::TransmitInactiveSource(void)
{
  uint16_t iPhysicalAddress;
  {
    CLockObject lock(&m_writeMutex);
    CStdString strLog;
    strLog.Format("<< %s (%X) -> broadcast (F): inactive source", GetLogicalAddressName(), m_iLogicalAddress);
    AddLog(CEC_LOG_NOTICE, strLog);
    iPhysicalAddress = m_iPhysicalAddress;
  }

  return m_handler->TransmitInactiveSource(m_iLogicalAddress, iPhysicalAddress);
}

bool CCECBusDevice::TransmitMenuState(cec_logical_address dest)
{
  cec_menu_state menuState;
  {
    CLockObject lock(&m_writeMutex);
    CStdString strLog;
    strLog.Format("<< %s (%X) -> %s (%X): menu state '%s'", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest, ToString(m_menuState));
    AddLog(CEC_LOG_NOTICE, strLog);
    menuState = m_menuState;
  }

  return m_handler->TransmitMenuState(m_iLogicalAddress, dest, menuState);
}

bool CCECBusDevice::TransmitOSDName(cec_logical_address dest)
{
  CStdString strDeviceName;
  {
    CLockObject lock(&m_writeMutex);
    CStdString strLog;
    strLog.Format("<< %s (%X) -> %s (%X): OSD name '%s'", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest, m_strDeviceName.c_str());
    AddLog(CEC_LOG_NOTICE, strLog.c_str());
    strDeviceName = m_strDeviceName;
  }

  return m_handler->TransmitOSDName(m_iLogicalAddress, dest, strDeviceName);
}

bool CCECBusDevice::TransmitOSDString(cec_logical_address dest, cec_display_control duration, const char *strMessage)
{
  CStdString strLog;
  strLog.Format("<< %s (%X) -> %s (%X): display OSD message '%s'", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest, strMessage);
  AddLog(CEC_LOG_NOTICE, strLog.c_str());

  return m_handler->TransmitOSDString(m_iLogicalAddress, dest, duration, strMessage);
}

bool CCECBusDevice::TransmitPhysicalAddress(void)
{
  uint16_t iPhysicalAddress;
  cec_device_type type;
  {
    CLockObject lock(&m_writeMutex);
    if (m_iPhysicalAddress == 0xffff)
      return false;

    CStdString strLog;
    strLog.Format("<< %s (%X) -> broadcast (F): physical adddress %4x", GetLogicalAddressName(), m_iLogicalAddress, m_iPhysicalAddress);
    AddLog(CEC_LOG_NOTICE, strLog.c_str());

    iPhysicalAddress = m_iPhysicalAddress;
    type = m_type;
  }

  return m_handler->TransmitPhysicalAddress(m_iLogicalAddress, iPhysicalAddress, type);
}

bool CCECBusDevice::TransmitPoll(cec_logical_address dest)
{
  bool bReturn(false);
  if (dest == CECDEVICE_UNKNOWN)
    dest = m_iLogicalAddress;

  CStdString strLog;
  strLog.Format("<< %s (%X) -> %s (%X): POLL", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest);
  AddLog(CEC_LOG_NOTICE, strLog.c_str());
  bReturn = m_handler->TransmitPoll(m_iLogicalAddress, dest);
  AddLog(CEC_LOG_DEBUG, bReturn ? ">> POLL sent" : ">> POLL not sent");

  if (bReturn)
  {
    CLockObject lock(&m_writeMutex);
    m_iLastActive = GetTimeMs();
  }

  return bReturn;
}

bool CCECBusDevice::TransmitPowerState(cec_logical_address dest)
{
  cec_power_status state;
  {
    CLockObject lock(&m_writeMutex);
    CStdString strLog;
    strLog.Format("<< %s (%X) -> %s (%X): %s", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest, ToString(m_powerStatus));
    AddLog(CEC_LOG_NOTICE, strLog.c_str());
    state = m_powerStatus;
  }

  return m_handler->TransmitPowerState(m_iLogicalAddress, dest, state);
}

bool CCECBusDevice::TransmitVendorID(cec_logical_address dest)
{
  uint64_t iVendorId;
  {
    CLockObject lock(&m_writeMutex);
    iVendorId = (uint64_t)m_vendor;
  }

  if (iVendorId == CEC_VENDOR_UNKNOWN)
  {
    CStdString strLog;
    strLog.Format("<< %s (%X) -> %s (%X): vendor id feature abort", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest);
    AddLog(CEC_LOG_NOTICE, strLog);

    m_processor->TransmitAbort(dest, CEC_OPCODE_GIVE_DEVICE_VENDOR_ID);
    return false;
  }
  else
  {
    CStdString strLog;
    strLog.Format("<< %s (%X) -> %s (%X): vendor id %s (%x)", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest, ToString((cec_vendor_id)iVendorId), iVendorId);
    AddLog(CEC_LOG_NOTICE, strLog);

    return m_handler->TransmitVendorID(m_iLogicalAddress, iVendorId);
  }
}

bool CCECBusDevice::TransmitKeypress(cec_user_control_code key)
{
  return m_handler->TransmitKeypress(m_processor->GetLogicalAddress(), m_iLogicalAddress, key);
}

bool CCECBusDevice::TransmitKeyRelease(void)
{
  return m_handler->TransmitKeyRelease(m_processor->GetLogicalAddress(), m_iLogicalAddress);
}
//@}
