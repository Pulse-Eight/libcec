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

#include "CECBusDevice.h"
#include "../CECProcessor.h"
#include "../implementations/ANCommandHandler.h"
#include "../implementations/CECCommandHandler.h"
#include "../implementations/SLCommandHandler.h"
#include "../implementations/VLCommandHandler.h"
#include "../LibCEC.h"
#include "../platform/util/timeutils.h"

using namespace CEC;
using namespace PLATFORM;

#define ToString(p) m_processor->ToString(p)

CCECBusDevice::CCECBusDevice(CCECProcessor *processor, cec_logical_address iLogicalAddress, uint16_t iPhysicalAddress /* = CEC_INVALID_PHYSICAL_ADDRESS */) :
  m_type                  (CEC_DEVICE_TYPE_RESERVED),
  m_iPhysicalAddress      (iPhysicalAddress),
  m_iStreamPath           (CEC_INVALID_PHYSICAL_ADDRESS),
  m_iLogicalAddress       (iLogicalAddress),
  m_powerStatus           (CEC_POWER_STATUS_UNKNOWN),
  m_processor             (processor),
  m_vendor                (CEC_VENDOR_UNKNOWN),
  m_bReplaceHandler       (false),
  m_menuState             (CEC_MENU_STATE_ACTIVATED),
  m_bActiveSource         (false),
  m_iLastActive           (0),
  m_iLastPowerStateUpdate (0),
  m_cecVersion            (CEC_VERSION_UNKNOWN),
  m_deviceStatus          (CEC_DEVICE_STATUS_UNKNOWN),
  m_iHandlerUseCount      (0),
  m_bAwaitingReceiveFailed(false),
  m_bVendorIdRequested    (false)
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

bool CCECBusDevice::HandleCommand(const cec_command &command)
{
  bool bHandled(false);

  /* update "last active" */
  {
    CLockObject lock(m_mutex);
    m_iLastActive = GetTimeMs();

    /* don't call GetStatus() here, just read the value with the mutex locked */
    if (m_deviceStatus != CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC && command.opcode_set == 1)
      m_deviceStatus = CEC_DEVICE_STATUS_PRESENT;

    MarkBusy();
  }

  /* handle the command */
  bHandled = m_handler->HandleCommand(command);

  /* change status to present */
  if (bHandled)
  {
    CLockObject lock(m_mutex);
    if (m_deviceStatus != CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC)
    {
      if (m_deviceStatus != CEC_DEVICE_STATUS_PRESENT)
        CLibCEC::AddLog(CEC_LOG_DEBUG, "device %s (%x) status changed to present after command %s", GetLogicalAddressName(), (uint8_t)GetLogicalAddress(), ToString(command.opcode));
      m_deviceStatus = CEC_DEVICE_STATUS_PRESENT;
    }
  }

  MarkReady();
  return bHandled;
}

bool CCECBusDevice::PowerOn(void)
{
  bool bReturn(false);
  GetVendorId(); // ensure that we got the vendor id, because the implementations vary per vendor

  MarkBusy();
  cec_power_status currentStatus = GetPowerStatus(false);
  if (currentStatus != CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON &&
    currentStatus != CEC_POWER_STATUS_ON)
  {
    CLibCEC::AddLog(CEC_LOG_NOTICE, "<< powering on '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
    if (m_handler->PowerOn(GetMyLogicalAddress(), m_iLogicalAddress))
    {
      SetPowerStatus(CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON);
      bReturn = true;
    }
  }
  else
  {
    CLibCEC::AddLog(CEC_LOG_NOTICE, "'%s' (%X) is already '%s'", GetLogicalAddressName(), m_iLogicalAddress, ToString(currentStatus));
  }

  MarkReady();
  return bReturn;
}

bool CCECBusDevice::Standby(void)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "<< putting '%s' (%X) in standby mode", GetLogicalAddressName(), m_iLogicalAddress);
  MarkBusy();
  bool bReturn = m_handler->TransmitStandby(GetMyLogicalAddress(), m_iLogicalAddress);
  MarkReady();
  return bReturn;
}

/** @name Getters */
//@{
cec_version CCECBusDevice::GetCecVersion(bool bUpdate /* = false */)
{
  bool bIsPresent(GetStatus() == CEC_DEVICE_STATUS_PRESENT);
  bool bRequestUpdate(false);
  {
    CLockObject lock(m_mutex);
    bRequestUpdate = bIsPresent &&
        (bUpdate || m_cecVersion == CEC_VERSION_UNKNOWN);
  }

  if (bRequestUpdate)
  {
    CheckVendorIdRequested();
    RequestCecVersion();
  }

  CLockObject lock(m_mutex);
  return m_cecVersion;
}

bool CCECBusDevice::RequestActiveSource(bool bWaitForResponse /* = true */)
{
  bool bReturn(false);

  if (MyLogicalAddressContains(m_iLogicalAddress))
  {
    MarkBusy();
    CLibCEC::AddLog(CEC_LOG_NOTICE, "<< requesting active source");

    bReturn = m_handler->TransmitRequestActiveSource(GetMyLogicalAddress(), bWaitForResponse);
    MarkReady();
  }
  return bReturn;
}

bool CCECBusDevice::RequestCecVersion(bool bWaitForResponse /* = true */)
{
  bool bReturn(false);

  if (!MyLogicalAddressContains(m_iLogicalAddress) &&
      !IsUnsupportedFeature(CEC_OPCODE_GET_CEC_VERSION))
  {
    MarkBusy();
    CLibCEC::AddLog(CEC_LOG_NOTICE, "<< requesting CEC version of '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);

    bReturn = m_handler->TransmitRequestCecVersion(GetMyLogicalAddress(), m_iLogicalAddress, bWaitForResponse);
    MarkReady();
  }
  return bReturn;
}

const char* CCECBusDevice::GetLogicalAddressName(void) const
{
  return ToString(m_iLogicalAddress);
}

cec_menu_language &CCECBusDevice::GetMenuLanguage(bool bUpdate /* = false */)
{
  bool bIsPresent(GetStatus() == CEC_DEVICE_STATUS_PRESENT);
  bool bRequestUpdate(false);
  {
    CLockObject lock(m_mutex);
    bRequestUpdate = (bIsPresent &&
        (bUpdate || !strcmp(m_menuLanguage.language, "???")));
  }

  if (bRequestUpdate)
  {
    CheckVendorIdRequested();
    RequestMenuLanguage();
  }

  CLockObject lock(m_mutex);
  return m_menuLanguage;
}

bool CCECBusDevice::RequestMenuLanguage(bool bWaitForResponse /* = true */)
{
  bool bReturn(false);

  if (!MyLogicalAddressContains(m_iLogicalAddress) &&
      !IsUnsupportedFeature(CEC_OPCODE_GET_MENU_LANGUAGE))
  {
    MarkBusy();
    CLibCEC::AddLog(CEC_LOG_NOTICE, "<< requesting menu language of '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
    bReturn = m_handler->TransmitRequestMenuLanguage(GetMyLogicalAddress(), m_iLogicalAddress, bWaitForResponse);
    MarkReady();
  }
  return bReturn;
}

cec_menu_state CCECBusDevice::GetMenuState(void)
{
  CLockObject lock(m_mutex);
  return m_menuState;
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
  bool bIsPresent(GetStatus() == CEC_DEVICE_STATUS_PRESENT);
  bool bRequestUpdate(false);
  {
    CLockObject lock(m_mutex);
    bRequestUpdate = bIsPresent &&
        (bUpdate || m_strDeviceName.Equals(ToString(m_iLogicalAddress))) &&
        m_type != CEC_DEVICE_TYPE_TV;
  }

  if (bRequestUpdate)
  {
    CheckVendorIdRequested();
    RequestOSDName();
  }

  CLockObject lock(m_mutex);
  return m_strDeviceName;
}

bool CCECBusDevice::RequestOSDName(bool bWaitForResponse /* = true */)
{
  bool bReturn(false);

  if (!MyLogicalAddressContains(m_iLogicalAddress) &&
      !IsUnsupportedFeature(CEC_OPCODE_GIVE_OSD_NAME))
  {
    MarkBusy();
    CLibCEC::AddLog(CEC_LOG_NOTICE, "<< requesting OSD name of '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
    bReturn = m_handler->TransmitRequestOSDName(GetMyLogicalAddress(), m_iLogicalAddress, bWaitForResponse);
    MarkReady();
  }
  return bReturn;
}

uint16_t CCECBusDevice::GetPhysicalAddress(bool bSuppressUpdate /* = true */)
{
  if (!bSuppressUpdate)
  {
    bool bIsPresent(GetStatus() == CEC_DEVICE_STATUS_PRESENT);
    bool bRequestUpdate(false);
    {
      CLockObject lock(m_mutex);
      bRequestUpdate = bIsPresent && m_iPhysicalAddress == CEC_INVALID_PHYSICAL_ADDRESS;
    }

    if (bRequestUpdate)
    {
      CheckVendorIdRequested();
      if (!RequestPhysicalAddress())
        CLibCEC::AddLog(CEC_LOG_ERROR, "failed to request the physical address");
    }
  }

  CLockObject lock(m_mutex);
  return m_iPhysicalAddress;
}

bool CCECBusDevice::RequestPhysicalAddress(bool bWaitForResponse /* = true */)
{
  bool bReturn(false);

  if (!MyLogicalAddressContains(m_iLogicalAddress))
  {
    MarkBusy();
    CLibCEC::AddLog(CEC_LOG_NOTICE, "<< requesting physical address of '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
    bReturn = m_handler->TransmitRequestPhysicalAddress(GetMyLogicalAddress(), m_iLogicalAddress, bWaitForResponse);
    MarkReady();
  }
  return bReturn;
}

cec_power_status CCECBusDevice::GetPowerStatus(bool bUpdate /* = false */)
{
  bool bIsPresent(GetStatus() == CEC_DEVICE_STATUS_PRESENT);
  bool bRequestUpdate(false);
  {
    CLockObject lock(m_mutex);
    bRequestUpdate = (bIsPresent &&
        (bUpdate || m_powerStatus == CEC_POWER_STATUS_UNKNOWN ||
            m_powerStatus == CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON ||
            m_powerStatus == CEC_POWER_STATUS_IN_TRANSITION_ON_TO_STANDBY ||
            GetTimeMs() - m_iLastPowerStateUpdate >= CEC_POWER_STATE_REFRESH_TIME));
  }

  if (bRequestUpdate)
  {
    CheckVendorIdRequested();
    RequestPowerStatus();
  }

  CLockObject lock(m_mutex);
  return m_powerStatus;
}

bool CCECBusDevice::RequestPowerStatus(bool bWaitForResponse /* = true */)
{
  bool bReturn(false);

  if (!MyLogicalAddressContains(m_iLogicalAddress) &&
      !IsUnsupportedFeature(CEC_OPCODE_GIVE_DEVICE_POWER_STATUS))
  {
    MarkBusy();
    CLibCEC::AddLog(CEC_LOG_NOTICE, "<< requesting power status of '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
    bReturn = m_handler->TransmitRequestPowerStatus(GetMyLogicalAddress(), m_iLogicalAddress, bWaitForResponse);
    MarkReady();
  }
  return bReturn;
}

cec_vendor_id CCECBusDevice::GetVendorId(bool bUpdate /* = false */)
{
  bool bIsPresent(GetStatus() == CEC_DEVICE_STATUS_PRESENT);
  bool bRequestUpdate(false);
  {
    CLockObject lock(m_mutex);
    bRequestUpdate = (bIsPresent &&
        (bUpdate || m_vendor == CEC_VENDOR_UNKNOWN));
  }

  if (bRequestUpdate)
    RequestVendorId();

  CLockObject lock(m_mutex);
  return m_vendor;
}

bool CCECBusDevice::RequestVendorId(bool bWaitForResponse /* = true */)
{
  bool bReturn(false);

  if (!MyLogicalAddressContains(m_iLogicalAddress) && GetMyLogicalAddress() != CECDEVICE_UNKNOWN)
  {
    MarkBusy();
    CLibCEC::AddLog(CEC_LOG_NOTICE, "<< requesting vendor ID of '%s' (%X)", GetLogicalAddressName(), m_iLogicalAddress);
    bReturn = m_handler->TransmitRequestVendorId(GetMyLogicalAddress(), m_iLogicalAddress, bWaitForResponse);
    MarkReady();

    if (bWaitForResponse)
      ReplaceHandler(true);
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
    {
      cec_bus_device_status status = m_processor->m_busDevices[CECDEVICE_PLAYBACKDEVICE2]->GetStatus();
      bSendPoll = (status == CEC_DEVICE_STATUS_PRESENT || status == CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC);
    }
    break;
  case CECDEVICE_PLAYBACKDEVICE2:
    {
      cec_bus_device_status status = m_processor->m_busDevices[CECDEVICE_PLAYBACKDEVICE1]->GetStatus();
      bSendPoll = (status == CEC_DEVICE_STATUS_PRESENT || status == CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC);
    }
    break;
  case CECDEVICE_RECORDINGDEVICE3:
    {
      cec_bus_device_status status = m_processor->m_busDevices[CECDEVICE_RECORDINGDEVICE2]->GetStatus();
      bSendPoll = (status == CEC_DEVICE_STATUS_PRESENT || status == CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC);
    }
    break;
  case CECDEVICE_RECORDINGDEVICE2:
    {
      cec_bus_device_status status = m_processor->m_busDevices[CECDEVICE_RECORDINGDEVICE1]->GetStatus();
      bSendPoll = (status == CEC_DEVICE_STATUS_PRESENT || status == CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC);
    }
    break;
  case CECDEVICE_TUNER4:
    {
      cec_bus_device_status status = m_processor->m_busDevices[CECDEVICE_TUNER3]->GetStatus();
      bSendPoll = (status == CEC_DEVICE_STATUS_PRESENT || status == CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC);
    }
    break;
  case CECDEVICE_TUNER3:
    {
      cec_bus_device_status status = m_processor->m_busDevices[CECDEVICE_TUNER2]->GetStatus();
      bSendPoll = (status == CEC_DEVICE_STATUS_PRESENT || status == CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC);
    }
    break;
  case CECDEVICE_TUNER2:
    {
      cec_bus_device_status status = m_processor->m_busDevices[CECDEVICE_TUNER1]->GetStatus();
      bSendPoll = (status == CEC_DEVICE_STATUS_PRESENT || status == CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC);
    }
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

cec_bus_device_status CCECBusDevice::GetStatus(bool bForcePoll /* = false */, bool bSuppressPoll /* = false */)
{
  cec_bus_device_status status(CEC_DEVICE_STATUS_UNKNOWN);
  bool bNeedsPoll(false);

  {
    CLockObject lock(m_mutex);
    status = m_deviceStatus;
    bNeedsPoll = !bSuppressPoll &&
        (bForcePoll || m_deviceStatus == CEC_DEVICE_STATUS_UNKNOWN);
  }

  if (bNeedsPoll)
  {
    bool bPollAcked(false);
    if (bNeedsPoll && NeedsPoll())
      bPollAcked = m_processor->PollDevice(m_iLogicalAddress);

    status = bPollAcked ? CEC_DEVICE_STATUS_PRESENT : CEC_DEVICE_STATUS_NOT_PRESENT;
    SetDeviceStatus(status);
  }

  return status;
}

//@}

/** @name Setters */
//@{
void CCECBusDevice::SetCecVersion(const cec_version newVersion)
{
  if (m_cecVersion != newVersion)
    CLibCEC::AddLog(CEC_LOG_DEBUG, "%s (%X): CEC version %s", GetLogicalAddressName(), m_iLogicalAddress, ToString(newVersion));
  m_cecVersion = newVersion;
}

void CCECBusDevice::SetMenuLanguage(const cec_menu_language &language)
{
  CLockObject lock(m_mutex);
  if (language.device == m_iLogicalAddress &&
      strcmp(language.language, m_menuLanguage.language))
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, ">> %s (%X): menu language set to '%s'", GetLogicalAddressName(), m_iLogicalAddress, language.language);
    m_menuLanguage = language;
  }
}

void CCECBusDevice::SetOSDName(CStdString strName)
{
  CLockObject lock(m_mutex);
  if (m_strDeviceName != strName)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, ">> %s (%X): osd name set to '%s'", GetLogicalAddressName(), m_iLogicalAddress, strName.c_str());
    m_strDeviceName = strName;
  }
}

void CCECBusDevice::SetMenuState(const cec_menu_state state)
{
  CLockObject lock(m_mutex);
  if (m_menuState != state)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, ">> %s (%X): menu state set to '%s'", GetLogicalAddressName(), m_iLogicalAddress, ToString(m_menuState));
    m_menuState = state;
  }
}

void CCECBusDevice::SetInactiveSource(void)
{
  {
    CLockObject lock(m_mutex);
    if (m_bActiveSource)
      CLibCEC::AddLog(CEC_LOG_DEBUG, "marking %s (%X) as inactive source", GetLogicalAddressName(), m_iLogicalAddress);
    m_bActiveSource = false;
  }
}

void CCECBusDevice::SetActiveSource(void)
{
  CLockObject lock(m_mutex);
  if (!m_bActiveSource)
    CLibCEC::AddLog(CEC_LOG_DEBUG, "making %s (%x) the active source", GetLogicalAddressName(), m_iLogicalAddress);
  else
    CLibCEC::AddLog(CEC_LOG_DEBUG, "%s (%x) was already marked as active source", GetLogicalAddressName(), m_iLogicalAddress);

  for (int iPtr = 0; iPtr < 16; iPtr++)
    if (iPtr != m_iLogicalAddress)
      m_processor->m_busDevices[iPtr]->SetInactiveSource();

  m_bActiveSource = true;
  SetPowerStatus(CEC_POWER_STATUS_ON);
}

bool CCECBusDevice::TryLogicalAddress(void)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "trying logical address '%s'", GetLogicalAddressName());

  m_processor->SetAckMask(0);
  if (!TransmitPoll(m_iLogicalAddress))
  {
    CLibCEC::AddLog(CEC_LOG_NOTICE, "using logical address '%s'", GetLogicalAddressName());
    SetDeviceStatus(CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC);

    return true;
  }

  CLibCEC::AddLog(CEC_LOG_DEBUG, "logical address '%s' already taken", GetLogicalAddressName());
  SetDeviceStatus(CEC_DEVICE_STATUS_PRESENT);
  return false;
}

void CCECBusDevice::ResetDeviceStatus(void)
{
  CLockObject lock(m_mutex);
  SetPowerStatus   (CEC_POWER_STATUS_UNKNOWN);
  SetVendorId      (CEC_VENDOR_UNKNOWN);
  SetMenuState     (CEC_MENU_STATE_ACTIVATED);
  SetCecVersion    (CEC_VERSION_UNKNOWN);
  SetStreamPath    (CEC_INVALID_PHYSICAL_ADDRESS);
  SetOSDName       (ToString(m_iLogicalAddress));
  SetInactiveSource();
  m_iLastActive = 0;
  m_unsupportedFeatures.clear();
}

void CCECBusDevice::SetDeviceStatus(const cec_bus_device_status newStatus)
{
  {
    CLockObject lock(m_mutex);
    switch (newStatus)
    {
    case CEC_DEVICE_STATUS_UNKNOWN:
      if (m_deviceStatus != newStatus)
        CLibCEC::AddLog(CEC_LOG_DEBUG, "device status of %s changed into 'unknown'", ToString(m_iLogicalAddress));
      ResetDeviceStatus();
      m_deviceStatus = newStatus;
      break;
    case CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC:
      if (m_deviceStatus != newStatus)
        CLibCEC::AddLog(CEC_LOG_DEBUG, "device status of %s changed into 'handled by libCEC'", ToString(m_iLogicalAddress));
      SetPowerStatus   (CEC_POWER_STATUS_ON);
      SetVendorId      (CEC_VENDOR_UNKNOWN);
      SetMenuState     (CEC_MENU_STATE_ACTIVATED);
      SetCecVersion    (CEC_VERSION_1_3A);
      SetStreamPath    (CEC_INVALID_PHYSICAL_ADDRESS);
      SetInactiveSource();
      m_iLastActive   = 0;
      m_deviceStatus  = newStatus;
      break;
    case CEC_DEVICE_STATUS_PRESENT:
      if (m_deviceStatus != newStatus)
        CLibCEC::AddLog(CEC_LOG_DEBUG, "device status of %s changed into 'present'", ToString(m_iLogicalAddress));
      m_deviceStatus = newStatus;
      break;
    case CEC_DEVICE_STATUS_NOT_PRESENT:
      if (m_deviceStatus != newStatus)
      {
        CLibCEC::AddLog(CEC_LOG_DEBUG, "device status of %s changed into 'not present'", ToString(m_iLogicalAddress));
        ResetDeviceStatus();
        m_deviceStatus = newStatus;
      }
      break;
    }
  }

  if (newStatus == CEC_DEVICE_STATUS_PRESENT)
    RequestVendorId(false);
}

void CCECBusDevice::SetPhysicalAddress(uint16_t iNewAddress)
{
  CLockObject lock(m_mutex);
  if (iNewAddress > 0 && m_iPhysicalAddress != iNewAddress)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, ">> %s (%X): physical address changed from %04x to %04x", GetLogicalAddressName(), m_iLogicalAddress, m_iPhysicalAddress, iNewAddress);
    m_iPhysicalAddress = iNewAddress;
  }
}

void CCECBusDevice::SetStreamPath(uint16_t iNewAddress, uint16_t iOldAddress /* = CEC_INVALID_PHYSICAL_ADDRESS */)
{
  CLockObject lock(m_mutex);
  if (iNewAddress != m_iStreamPath)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, ">> %s (%X): stream path changed from %04x to %04x", GetLogicalAddressName(), m_iLogicalAddress, iOldAddress == 0 ? m_iStreamPath : iOldAddress, iNewAddress);
    m_iStreamPath = iNewAddress;
  }

  CCECBusDevice *device = m_processor->GetDeviceByPhysicalAddress(iNewAddress);
  if (device)
  {
    // if a device is found with the new physical address, mark it as active, which will automatically mark all other devices as inactive
    device->SetActiveSource();
  }
  else
  {
    // try to find the device with the old address, and mark it as inactive when found
    device = m_processor->GetDeviceByPhysicalAddress(iOldAddress);
    if (device)
      device->SetInactiveSource();
  }
}

void CCECBusDevice::SetPowerStatus(const cec_power_status powerStatus)
{
  CLockObject lock(m_mutex);
  if (m_powerStatus != powerStatus)
  {
    m_iLastPowerStateUpdate = GetTimeMs();
    CLibCEC::AddLog(CEC_LOG_DEBUG, ">> %s (%X): power status changed from '%s' to '%s'", GetLogicalAddressName(), m_iLogicalAddress, ToString(m_powerStatus), ToString(powerStatus));
    m_powerStatus = powerStatus;
  }
}

void CCECBusDevice::MarkBusy(void)
{
  CLockObject handlerLock(m_handlerMutex);
  ++m_iHandlerUseCount;
}

void CCECBusDevice::MarkReady(void)
{
  CLockObject handlerLock(m_handlerMutex);
  if (m_iHandlerUseCount > 0)
    --m_iHandlerUseCount;
}

bool CCECBusDevice::ReplaceHandler(bool bActivateSource /* = true */)
{
  bool bInitHandler(false);
  {
    CTryLockObject lock(m_mutex);
    if (!lock.IsLocked())
      return false;

    CLockObject handlerLock(m_handlerMutex);
    if (m_iHandlerUseCount > 0)
      return false;

    MarkBusy();

    if (m_vendor != m_handler->GetVendorId())
    {
      if (CCECCommandHandler::HasSpecificHandler(m_vendor))
      {
        CLibCEC::AddLog(CEC_LOG_DEBUG, "replacing the command handler for device '%s' (%x)", GetLogicalAddressName(), GetLogicalAddress());
        delete m_handler;

        switch (m_vendor)
        {
        case CEC_VENDOR_SAMSUNG:
          m_handler = new CANCommandHandler(this);
          break;
        case CEC_VENDOR_LG:
          m_handler = new CSLCommandHandler(this);
          break;
        case CEC_VENDOR_PANASONIC:
          m_handler = new CVLCommandHandler(this);
          break;
        default:
          m_handler = new CCECCommandHandler(this);
          break;
        }

        m_handler->SetVendorId(m_vendor);
        bInitHandler = true;
      }
    }
  }

  if (bInitHandler)
  {
    m_handler->InitHandler();

    if (bActivateSource && m_processor->GetLogicalAddresses().IsSet(m_iLogicalAddress) && m_processor->IsInitialised() && IsActiveSource())
      m_handler->ActivateSource();
  }

  MarkReady();

  return true;
}

bool CCECBusDevice::SetVendorId(uint64_t iVendorId)
{
  bool bVendorChanged(false);

  {
    CLockObject lock(m_mutex);
    bVendorChanged = (m_vendor != (cec_vendor_id)iVendorId);
    m_vendor = (cec_vendor_id)iVendorId;
  }

  if (bVendorChanged)
    CLibCEC::AddLog(CEC_LOG_DEBUG, "%s (%X): vendor = %s (%06x)", GetLogicalAddressName(), m_iLogicalAddress, ToString(m_vendor), m_vendor);

  return bVendorChanged;
}
//@}

/** @name Transmit methods */
//@{
bool CCECBusDevice::TransmitActiveSource(void)
{
  bool bSendActiveSource(false);

  {
    CLockObject lock(m_mutex);
    if (m_powerStatus != CEC_POWER_STATUS_ON && m_powerStatus != CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON)
      CLibCEC::AddLog(CEC_LOG_DEBUG, "<< %s (%X) is not powered on", GetLogicalAddressName(), m_iLogicalAddress);
    else if (m_bActiveSource)
    {
      CLibCEC::AddLog(CEC_LOG_NOTICE, "<< %s (%X) -> broadcast (F): active source (%4x)", GetLogicalAddressName(), m_iLogicalAddress, m_iPhysicalAddress);
      bSendActiveSource = true;
    }
    else
      CLibCEC::AddLog(CEC_LOG_DEBUG, "<< %s (%X) is not the active source", GetLogicalAddressName(), m_iLogicalAddress);
  }

  if (bSendActiveSource)
  {
    MarkBusy();
    m_handler->TransmitActiveSource(m_iLogicalAddress, m_iPhysicalAddress);
    MarkReady();
    return true;
  }

  return false;
}

bool CCECBusDevice::TransmitCECVersion(cec_logical_address dest)
{
  cec_version version;
  {
    CLockObject lock(m_mutex);
    CLibCEC::AddLog(CEC_LOG_NOTICE, "<< %s (%X) -> %s (%X): cec version %s", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest, ToString(m_cecVersion));
    version = m_cecVersion;
  }

  MarkBusy();
  bool bReturn = m_handler->TransmitCECVersion(m_iLogicalAddress, dest, version);
  MarkReady();
  return bReturn;
}

bool CCECBusDevice::TransmitImageViewOn(void)
{
  {
    CLockObject lock(m_mutex);
    if (m_powerStatus != CEC_POWER_STATUS_ON && m_powerStatus != CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON)
    {
      CLibCEC::AddLog(CEC_LOG_DEBUG, "<< %s (%X) is not powered on", GetLogicalAddressName(), m_iLogicalAddress);
      return false;
    }
  }

  MarkBusy();
  m_handler->TransmitImageViewOn(m_iLogicalAddress, CECDEVICE_TV);
  MarkReady();
  return true;
}

bool CCECBusDevice::TransmitInactiveSource(void)
{
  uint16_t iPhysicalAddress;
  {
    CLockObject lock(m_mutex);
    CLibCEC::AddLog(CEC_LOG_NOTICE, "<< %s (%X) -> broadcast (F): inactive source", GetLogicalAddressName(), m_iLogicalAddress);
    iPhysicalAddress = m_iPhysicalAddress;
  }

  MarkBusy();
  bool bReturn = m_handler->TransmitInactiveSource(m_iLogicalAddress, iPhysicalAddress);
  MarkReady();
  return bReturn;
}

bool CCECBusDevice::TransmitMenuState(cec_logical_address dest)
{
  cec_menu_state menuState;
  {
    CLockObject lock(m_mutex);
    CLibCEC::AddLog(CEC_LOG_NOTICE, "<< %s (%X) -> %s (%X): menu state '%s'", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest, ToString(m_menuState));
    menuState = m_menuState;
  }

  MarkBusy();
  bool bReturn = m_handler->TransmitMenuState(m_iLogicalAddress, dest, menuState);
  MarkReady();
  return bReturn;
}

bool CCECBusDevice::TransmitOSDName(cec_logical_address dest)
{
  CStdString strDeviceName;
  {
    CLockObject lock(m_mutex);
    CLibCEC::AddLog(CEC_LOG_NOTICE, "<< %s (%X) -> %s (%X): OSD name '%s'", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest, m_strDeviceName.c_str());
    strDeviceName = m_strDeviceName;
  }

  MarkBusy();
  bool bReturn = m_handler->TransmitOSDName(m_iLogicalAddress, dest, strDeviceName);
  MarkReady();
  return bReturn;
}

bool CCECBusDevice::TransmitOSDString(cec_logical_address dest, cec_display_control duration, const char *strMessage)
{
  bool bReturn(false);
  if (!m_processor->m_busDevices[dest]->IsUnsupportedFeature(CEC_OPCODE_SET_OSD_STRING))
  {
    CLibCEC::AddLog(CEC_LOG_NOTICE, "<< %s (%X) -> %s (%X): display OSD message '%s'", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest, strMessage);
    MarkBusy();
    bReturn = m_handler->TransmitOSDString(m_iLogicalAddress, dest, duration, strMessage);
    MarkReady();
  }
  return bReturn;
}

bool CCECBusDevice::TransmitPhysicalAddress(void)
{
  uint16_t iPhysicalAddress;
  cec_device_type type;
  {
    CLockObject lock(m_mutex);
    if (m_iPhysicalAddress == CEC_INVALID_PHYSICAL_ADDRESS)
      return false;

    CLibCEC::AddLog(CEC_LOG_NOTICE, "<< %s (%X) -> broadcast (F): physical adddress %4x", GetLogicalAddressName(), m_iLogicalAddress, m_iPhysicalAddress);
    iPhysicalAddress = m_iPhysicalAddress;
    type = m_type;
  }

  MarkBusy();
  bool bReturn = m_handler->TransmitPhysicalAddress(m_iLogicalAddress, iPhysicalAddress, type);
  MarkReady();
  return bReturn;
}

bool CCECBusDevice::TransmitSetMenuLanguage(cec_logical_address dest)
{
  bool bReturn(false);
  cec_menu_language language = GetMenuLanguage();

  char lang[3];
  {
    CLockObject lock(m_mutex);
    lang[0] = language.language[0];
    lang[1] = language.language[1];
    lang[2] = language.language[2];
  }

  MarkBusy();
  if (lang[0] == '?' && lang[1] == '?' && lang[2] == '?')
  {
      CLibCEC::AddLog(CEC_LOG_NOTICE, "<< %s (%X) -> %s (%X): Menu language feature abort", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest);
      m_processor->TransmitAbort(dest, CEC_OPCODE_GIVE_DEVICE_VENDOR_ID);
      bReturn = true;
  }
  else
  {
      CLibCEC::AddLog(CEC_LOG_NOTICE, "<< %s (%X) -> broadcast (F): Menu language '%s'", GetLogicalAddressName(), m_iLogicalAddress, lang);
      bReturn = m_handler->TransmitSetMenuLanguage(m_iLogicalAddress, lang);
  }
  MarkReady();
  return bReturn;
}

bool CCECBusDevice::TransmitPoll(cec_logical_address dest)
{
  bool bReturn(false);
  if (dest == CECDEVICE_UNKNOWN)
    dest = m_iLogicalAddress;

  CCECBusDevice *destDevice = m_processor->m_busDevices[dest];
  if (destDevice->m_deviceStatus == CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC)
    return bReturn;

  MarkBusy();
  CLibCEC::AddLog(CEC_LOG_NOTICE, "<< %s (%X) -> %s (%X): POLL", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest);
  bReturn = m_handler->TransmitPoll(m_iLogicalAddress, dest);
  CLibCEC::AddLog(CEC_LOG_DEBUG, bReturn ? ">> POLL sent" : ">> POLL not sent");

  CLockObject lock(m_mutex);
  if (bReturn)
  {
    m_iLastActive = GetTimeMs();
    destDevice->m_deviceStatus = CEC_DEVICE_STATUS_PRESENT;
  }
  else
    destDevice->m_deviceStatus = CEC_DEVICE_STATUS_NOT_PRESENT;

  MarkReady();
  return bReturn;
}

bool CCECBusDevice::TransmitPowerState(cec_logical_address dest)
{
  cec_power_status state;
  {
    CLockObject lock(m_mutex);
    CLibCEC::AddLog(CEC_LOG_NOTICE, "<< %s (%X) -> %s (%X): %s", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest, ToString(m_powerStatus));
    state = m_powerStatus;
  }

  MarkBusy();
  bool bReturn = m_handler->TransmitPowerState(m_iLogicalAddress, dest, state);
  MarkReady();
  return bReturn;
}

bool CCECBusDevice::TransmitVendorID(cec_logical_address dest, bool bSendAbort /* = true */)
{
  bool bReturn(false);
  uint64_t iVendorId;
  {
    CLockObject lock(m_mutex);
    iVendorId = (uint64_t)m_vendor;
  }

  MarkBusy();
  if (iVendorId == CEC_VENDOR_UNKNOWN)
  {
    if (bSendAbort)
    {
      CLibCEC::AddLog(CEC_LOG_NOTICE, "<< %s (%X) -> %s (%X): vendor id feature abort", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest);
      m_processor->TransmitAbort(dest, CEC_OPCODE_GIVE_DEVICE_VENDOR_ID);
      bReturn = true;
    }
  }
  else
  {
    CLibCEC::AddLog(CEC_LOG_NOTICE, "<< %s (%X) -> %s (%X): vendor id %s (%x)", GetLogicalAddressName(), m_iLogicalAddress, ToString(dest), dest, ToString((cec_vendor_id)iVendorId), iVendorId);
    bReturn = m_handler->TransmitVendorID(m_iLogicalAddress, iVendorId);
  }
  MarkReady();
  return bReturn;
}

bool CCECBusDevice::TransmitKeypress(cec_user_control_code key, bool bWait /* = true */)
{
  MarkBusy();
  bool bReturn = m_handler->TransmitKeypress(m_processor->GetLogicalAddress(), m_iLogicalAddress, key, bWait);
  MarkReady();
  return bReturn;
}

bool CCECBusDevice::TransmitKeyRelease(bool bWait /* = true */)
{
  MarkBusy();
  bool bReturn = m_handler->TransmitKeyRelease(m_processor->GetLogicalAddress(), m_iLogicalAddress, bWait);
  MarkReady();
  return bReturn;
}

bool CCECBusDevice::IsUnsupportedFeature(cec_opcode opcode)
{
  CLockObject lock(m_mutex);
  bool bUnsupported = (m_unsupportedFeatures.find(opcode) != m_unsupportedFeatures.end());
  if (bUnsupported)
    CLibCEC::AddLog(CEC_LOG_DEBUG, "'%s' is marked as unsupported feature for device '%s'", ToString(opcode), GetLogicalAddressName());
  return bUnsupported;
}

void CCECBusDevice::SetUnsupportedFeature(cec_opcode opcode)
{
  // some commands should never be marked as unsupported
  if (opcode == CEC_OPCODE_VENDOR_COMMAND ||
      opcode == CEC_OPCODE_VENDOR_COMMAND_WITH_ID ||
      opcode == CEC_OPCODE_VENDOR_REMOTE_BUTTON_DOWN ||
      opcode == CEC_OPCODE_VENDOR_REMOTE_BUTTON_UP ||
      opcode == CEC_OPCODE_ABORT ||
      opcode == CEC_OPCODE_FEATURE_ABORT ||
      opcode == CEC_OPCODE_NONE)
    return;

  {
    CLockObject lock(m_mutex);
    if (m_unsupportedFeatures.find(opcode) == m_unsupportedFeatures.end())
    {
      CLibCEC::AddLog(CEC_LOG_DEBUG, "marking opcode '%s' as unsupported feature for device '%s'", ToString(opcode), GetLogicalAddressName());
      m_unsupportedFeatures.insert(opcode);
    }
  }

  // signal threads that are waiting for a reponse
  MarkBusy();
  m_handler->SignalOpcode(cec_command::GetResponseOpcode(opcode));
  MarkReady();
}

bool CCECBusDevice::ActivateSource(void)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "activating source '%s'", ToString(m_iLogicalAddress));
  MarkBusy();
  bool bReturn = m_handler->ActivateSource();
  MarkReady();
  return bReturn;
}

void CCECBusDevice::HandlePoll(cec_logical_address destination)
{
  if (destination >= 0 && destination < CECDEVICE_BROADCAST)
  {
    CCECBusDevice *device = m_processor->m_busDevices[destination];
    if (device)
      device->HandlePollFrom(m_iLogicalAddress);
  }
}

void CCECBusDevice::HandlePollFrom(cec_logical_address initiator)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "<< POLL: %s (%x) -> %s (%x)", ToString(initiator), initiator, ToString(m_iLogicalAddress), m_iLogicalAddress);
  m_bAwaitingReceiveFailed = true;
}

bool CCECBusDevice::HandleReceiveFailed(void)
{
  bool bReturn = m_bAwaitingReceiveFailed;
  m_bAwaitingReceiveFailed = false;
  return bReturn;
}

void CCECBusDevice::CheckVendorIdRequested(void)
{
  bool bRequestVendorId(false);
  {
    CLockObject lock(m_mutex);
    bRequestVendorId = !m_bVendorIdRequested;
    m_bVendorIdRequested = true;
  }

  if (bRequestVendorId)
  {
    ReplaceHandler(false);
    GetVendorId();
  }
}

bool CCECBusDevice::TransmitPendingActiveSourceCommands(void)
{
  MarkBusy();
  bool bReturn = m_handler->TransmitPendingActiveSourceCommands();
  MarkReady();
  return bReturn;
}

//@}
