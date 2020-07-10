/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2015 Pulse-Eight Limited.  All rights reserved.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
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
#include "USBCECAdapterCommands.h"

#include "USBCECAdapterMessage.h"
#include "USBCECAdapterCommunication.h"
#include "LibCEC.h"
#include "CECProcessor.h"
#include "CECTypeUtils.h"
#include "p8-platform/util/util.h"
#include <stdio.h>

using namespace CEC;
using namespace P8PLATFORM;

#define LIB_CEC     m_comm->m_callback->GetLib()
#define ToString(p) CCECTypeUtils::ToString(p)

CUSBCECAdapterCommands::CUSBCECAdapterCommands(CUSBCECAdapterCommunication *comm) :
    m_comm(comm),
    m_bSettingsRetrieved(false),
    m_bSettingAutoEnabled(false),
    m_iSettingLAMask(0),
    m_bNeedsWrite(false),
    m_bControlledMode(false),
    m_adapterType(P8_ADAPTERTYPE_UNKNOWN)
{
  m_savedConfiguration.Clear();
}

cec_datapacket CUSBCECAdapterCommands::RequestSetting(cec_adapter_messagecode msgCode)
{
  cec_datapacket retVal;
  retVal.Clear();

  CCECAdapterMessage params;
  CCECAdapterMessage *message = m_comm->SendCommand(msgCode, params);
  if (!!message &&
      (message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED) &&
      (message->m_rx_len >= 3))
  {
    // shift out start, msgcode and end
    memcpy(retVal.data, &message->m_rx_data[2], message->m_rx_len - 3);
    retVal.size = message->m_rx_len - 3;
  }

  SAFE_DELETE(message);
  return retVal;
}

uint16_t CUSBCECAdapterCommands::RequestFirmwareVersion(void)
{
  m_savedConfiguration.iFirmwareVersion = CEC_FW_VERSION_UNKNOWN;
  unsigned int iFwVersionTry(0);

  while (m_savedConfiguration.iFirmwareVersion == CEC_FW_VERSION_UNKNOWN && iFwVersionTry++ < 3)
  {
#ifdef CEC_DEBUGGING
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: requesting the firmware version");
#endif
    cec_datapacket response = RequestSetting(MSGCODE_FIRMWARE_VERSION);
    if (response.size == 2)
      m_savedConfiguration.iFirmwareVersion = (response[0] << 8 | response[1]);
    else
    {
      LIB_CEC->AddLog(CEC_LOG_WARNING, "the adapter did not respond with a correct firmware version (try %d, size = %d)", iFwVersionTry, response.size);
      CEvent::Sleep(500);
    }
  }

  if (m_savedConfiguration.iFirmwareVersion == CEC_FW_VERSION_UNKNOWN)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: defaulting to firmware version 1");
    m_savedConfiguration.iFirmwareVersion = 1;
  }

  // firmware versions < 2 don't have an autonomous mode
  if (m_savedConfiguration.iFirmwareVersion < 2)
    m_bControlledMode = true;

  return m_savedConfiguration.iFirmwareVersion;
}

bool CUSBCECAdapterCommands::RequestSettingAutoEnabled(void)
{
#ifdef CEC_DEBUGGING
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: requesting autonomous mode setting");
#endif

  cec_datapacket response = RequestSetting(MSGCODE_GET_AUTO_ENABLED);
  if (response.size == 1)
  {
    m_bSettingAutoEnabled = response[0] == 1;
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: autonomous mode = %s", m_bSettingAutoEnabled ? "enabled" : "disabled");
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommands::RequestSettingAutoPowerOn(void)
{
#if CEC_LIB_VERSION_MAJOR >= 5
#ifdef CEC_DEBUGGING
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: requesting auto power on setting");
#endif

  cec_datapacket response = RequestSetting(MSGCODE_GET_AUTO_POWER_ON);
  if (response.size == 1)
  {
    m_savedConfiguration.bAutoPowerOn = response[0];
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: auto power on = %s", m_savedConfiguration.bAutoPowerOn ? "enabled" : "disabled");
    return true;
  }
#endif
  return false;
}

bool CUSBCECAdapterCommands::RequestSettingCECVersion(void)
{
#ifdef CEC_DEBUGGING
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: requesting CEC version setting");
#endif

  cec_datapacket response = RequestSetting(MSGCODE_GET_HDMI_VERSION);
  if (response.size == 1)
  {
    m_savedConfiguration.cecVersion = (cec_version)response[0];
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: CEC version = %s", ToString(m_savedConfiguration.cecVersion));
    return true;
  }
  return false;
}

p8_cec_adapter_type CUSBCECAdapterCommands::RequestAdapterType(void)
{
  if (m_adapterType == P8_ADAPTERTYPE_UNKNOWN)
  {
#ifdef CEC_DEBUGGING
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: requesting adapter type");
#endif

    cec_datapacket response = RequestSetting(MSGCODE_GET_ADAPTER_TYPE);
    if (response.size == 1)
      m_adapterType = (p8_cec_adapter_type)response[0];
  }
  return m_adapterType;
}

uint32_t CUSBCECAdapterCommands::RequestBuildDate(void)
{
  if (m_savedConfiguration.iFirmwareBuildDate == CEC_FW_BUILD_UNKNOWN)
  {
#ifdef CEC_DEBUGGING
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: requesting firmware build date");
#endif

    cec_datapacket response = RequestSetting(MSGCODE_GET_BUILDDATE);
    if (response.size == 4)
      m_savedConfiguration.iFirmwareBuildDate = (uint32_t)response[0] << 24 | (uint32_t)response[1] << 16 | (uint32_t)response[2] << 8 | (uint32_t)response[3];
  }
  return m_savedConfiguration.iFirmwareBuildDate;
}

bool CUSBCECAdapterCommands::RequestSettingDefaultLogicalAddress(void)
{
#ifdef CEC_DEBUGGING
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: requesting default logical address setting");
#endif

  cec_datapacket response = RequestSetting(MSGCODE_GET_DEFAULT_LOGICAL_ADDRESS);
  if (response.size == 1)
  {
    m_savedConfiguration.logicalAddresses.primary = (cec_logical_address)response[0];
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: logical address = %s", ToString(m_savedConfiguration.logicalAddresses.primary));
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommands::RequestSettingDeviceType(void)
{
#ifdef CEC_DEBUGGING
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: requesting device type setting");
#endif
  m_savedConfiguration.deviceTypes.Clear();

  cec_datapacket response = RequestSetting(MSGCODE_GET_DEVICE_TYPE);
  if (response.size == 1)
  {
    m_savedConfiguration.deviceTypes.Add((cec_device_type)response[0]);
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: device type = %s", ToString((cec_device_type)response[0]));
    return true;
  }
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: device type = (not set)");
  return false;
}

bool CUSBCECAdapterCommands::RequestSettingLogicalAddressMask(void)
{
#ifdef CEC_DEBUGGING
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: requesting logical address mask setting");
#endif

  cec_datapacket response = RequestSetting(MSGCODE_GET_LOGICAL_ADDRESS_MASK);
  if (response.size == 2)
  {
    m_iSettingLAMask = ((uint16_t)response[0] << 8) | ((uint16_t)response[1]);
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: logical address mask = %x", m_iSettingLAMask);
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommands::RequestSettingOSDName(void)
{
#ifdef CEC_DEBUGGING
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: requesting OSD name setting");
#endif

  cec_datapacket response = RequestSetting(MSGCODE_GET_OSD_NAME);
  if (response.size == 0)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: device name = (not set)");
    m_savedConfiguration.strDeviceName[0] = (char)0;
    return false;
  }

  memcpy(m_savedConfiguration.strDeviceName, response.data, response.size <= LIBCEC_OSD_NAME_SIZE ? response.size : LIBCEC_OSD_NAME_SIZE);
  if (response.size < LIBCEC_OSD_NAME_SIZE) {
    m_savedConfiguration.strDeviceName[response.size] = (char)0;
  }
  return true;
}

bool CUSBCECAdapterCommands::RequestSettingPhysicalAddress(void)
{
#ifdef CEC_DEBUGGING
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: requesting physical address setting");
#endif

  cec_datapacket response = RequestSetting(MSGCODE_GET_PHYSICAL_ADDRESS);
  if (response.size == 2)
  {
    m_savedConfiguration.iPhysicalAddress = ((uint16_t)response[0] << 8) | ((uint16_t)response[1]);
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: physical address = %04x", m_savedConfiguration.iPhysicalAddress);
    return true;
  }
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: physical address = (not set)");
  return false;
}

bool CUSBCECAdapterCommands::SetSettingAutoEnabled(bool enabled)
{
  bool bReturn(false);

  /* check whether this value was changed */
  {
    CLockObject lock(m_mutex);
    if (m_bSettingAutoEnabled == enabled)
      return bReturn;
    m_bNeedsWrite = true;
  }

  CCECAdapterMessage params;
  params.PushEscaped(enabled ? 1 : 0);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_AUTO_ENABLED, params);
  bReturn = message && message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  SAFE_DELETE(message);

  if (bReturn)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: updating autonomous mode: %s", enabled ? "enabled" : "disabled");
    CLockObject lock(m_mutex);
    m_bSettingAutoEnabled = enabled;
  }
  else
  {
    LIB_CEC->AddLog(CEC_LOG_WARNING, "usbcec: failed to %s autonomous mode", enabled ? "enable" : "disable");
  }

  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingDeviceType(cec_device_type type)
{
  bool bReturn(false);

  /* check whether this value was changed */
  {
    CLockObject lock(m_mutex);
    if (m_savedConfiguration.deviceTypes.types[0] == type)
      return bReturn;
    m_bNeedsWrite = true;
  }

  CCECAdapterMessage params;
  params.PushEscaped((uint8_t)type);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_DEVICE_TYPE, params);
  bReturn = message && message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  SAFE_DELETE(message);

  if (bReturn)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: updated device type: %s -> %s",
        ToString(m_savedConfiguration.deviceTypes.types[0]),
        ToString(type));
    CLockObject lock(m_mutex);
    m_savedConfiguration.deviceTypes.types[0] = type;
  }
  else
  {
    LIB_CEC->AddLog(CEC_LOG_WARNING, "usbcec: failed to update device type to %s", ToString(type));
  }

  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingDefaultLogicalAddress(cec_logical_address address)
{
  bool bReturn(false);

  /* check whether this value was changed */
  {
    CLockObject lock(m_mutex);
    if (m_savedConfiguration.logicalAddresses.primary == address)
      return bReturn;
    m_bNeedsWrite = true;
  }

  CCECAdapterMessage params;
  params.PushEscaped((uint8_t)address);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_DEFAULT_LOGICAL_ADDRESS, params);
  bReturn = message && message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  SAFE_DELETE(message);

  if (bReturn)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: updated default logical address: %s -> %s",
        ToString(m_savedConfiguration.logicalAddresses.primary),
        ToString(address));
    CLockObject lock(m_mutex);
    m_savedConfiguration.logicalAddresses.primary = address;
  }
  else
  {
    LIB_CEC->AddLog(CEC_LOG_WARNING, "usbcec: failed to update default logical address to %s", ToString(address));
  }

  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingLogicalAddressMask(uint16_t iMask)
{
  bool bReturn(false);

  /* check whether this value was changed */
  {
    CLockObject lock(m_mutex);
    if (m_iSettingLAMask == iMask)
      return bReturn;
    m_bNeedsWrite = true;
  }

  CCECAdapterMessage params;
  params.PushEscaped(iMask >> 8);
  params.PushEscaped((uint8_t)iMask);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_LOGICAL_ADDRESS_MASK, params);
  bReturn = message && message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  SAFE_DELETE(message);

  if (bReturn)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: updated logical address mask: %02X -> %02X",
        m_iSettingLAMask,
        iMask);
    CLockObject lock(m_mutex);
    m_iSettingLAMask = iMask;
  }
  else
  {
    LIB_CEC->AddLog(CEC_LOG_WARNING, "usbcec: failed to update logical address mask to %02X", iMask);
  }

  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingPhysicalAddress(uint16_t iPhysicalAddress)
{
  bool bReturn(false);

  /* check whether this value was changed */
  {
    CLockObject lock(m_mutex);
    if (m_savedConfiguration.iPhysicalAddress == iPhysicalAddress)
      return bReturn;
    m_bNeedsWrite = true;
  }

  CCECAdapterMessage params;
  params.PushEscaped(iPhysicalAddress >> 8);
  params.PushEscaped((uint8_t)iPhysicalAddress);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_PHYSICAL_ADDRESS, params);
  bReturn = message && message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  SAFE_DELETE(message);

  if (bReturn)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: updated physical address: %04X -> %04X",
        m_savedConfiguration.iPhysicalAddress,
        iPhysicalAddress);
    CLockObject lock(m_mutex);
    m_savedConfiguration.iPhysicalAddress = iPhysicalAddress;
  }
  else
  {
    LIB_CEC->AddLog(CEC_LOG_WARNING, "usbcec: failed to update physical address to %04X", iPhysicalAddress);
  }

  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingAutoPowerOn(bool autoOn)
{
  bool bReturn(false);
#if CEC_LIB_VERSION_MAJOR >= 5
  if (m_savedConfiguration.iFirmwareVersion < 10)
    // only supported by v10+
    return bReturn;

  // check whether this value changed
  {
    CLockObject lock(m_mutex);
    if ((m_savedConfiguration.bAutoPowerOn == 1) == autoOn)
      return bReturn;
    m_bNeedsWrite = true;
  }

  CCECAdapterMessage params;
  params.PushEscaped(autoOn ? 1 : 0);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_AUTO_POWER_ON, params);
  bReturn = message && message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  SAFE_DELETE(message);

  if (bReturn)
  {
    CLockObject lock(m_mutex);
    m_savedConfiguration.bAutoPowerOn = (autoOn ? 1 : 0);
    LIB_CEC->AddLog(CEC_LOG_NOTICE, "usbcec: auto power on %s", autoOn ? "enabled" : "disabled");
  }
  else
  {
    LIB_CEC->AddLog(CEC_LOG_WARNING, "usbcec: failed to %s auto power on", autoOn ? "enable" : "disable");
  }
#endif

  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingCECVersion(cec_version version)
{
  bool bReturn(false);

  /* check whether this value was changed */
  {
    CLockObject lock(m_mutex);
    if (m_savedConfiguration.cecVersion == version)
      return bReturn;
    m_bNeedsWrite = true;
  }

  CCECAdapterMessage params;
  params.PushEscaped((uint8_t)version);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_HDMI_VERSION, params);
  bReturn = message && message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  SAFE_DELETE(message);

  if (bReturn)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: updated CEC version: %s -> %s",
        ToString(m_savedConfiguration.cecVersion),
        ToString(version));
    CLockObject lock(m_mutex);
    m_savedConfiguration.cecVersion = version;
  }
  else
  {
    LIB_CEC->AddLog(CEC_LOG_WARNING, "usbcec: failed to update CEC version to %s", ToString(version));
  }

  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingOSDName(const char *strOSDName)
{
  bool bReturn(false);

  /* check whether this value was changed */
  if (!strcmp(m_savedConfiguration.strDeviceName, strOSDName))
    return bReturn;

  CCECAdapterMessage params;
  for (size_t iPtr = 0; iPtr < strlen(strOSDName); iPtr++)
    params.PushEscaped(strOSDName[iPtr]);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_OSD_NAME, params);
  bReturn = message && message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  SAFE_DELETE(message);

  if (bReturn)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: updated OSD name: %s -> %s",
        m_savedConfiguration.strDeviceName,
        strOSDName);
    CLockObject lock(m_mutex);
    snprintf(m_savedConfiguration.strDeviceName, LIBCEC_OSD_NAME_SIZE, "%s", strOSDName);
  }
  else
  {
    LIB_CEC->AddLog(CEC_LOG_WARNING, "usbcec: failed to update OSD name to %s", strOSDName);
  }

  return bReturn;
}

bool CUSBCECAdapterCommands::WriteEEPROM(void)
{
  {
    CLockObject lock(m_mutex);
    if (!m_bNeedsWrite)
      return true;
  }

  CCECAdapterMessage params;
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_WRITE_EEPROM, params);
  bool bReturn = message && message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  SAFE_DELETE(message);

  if (bReturn)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: eeprom updated");
    CLockObject lock(m_mutex);
    m_bNeedsWrite = false;
  }
  else
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: failed to update eeprom");
  }

  return bReturn;
}

bool CUSBCECAdapterCommands::SaveConfiguration(const libcec_configuration &configuration)
{
  bool bReturn(false);
  if (m_savedConfiguration.iFirmwareVersion < 2)
    return bReturn;

  if (!RequestSettings())
    return bReturn;

  if (CLibCEC::GetType(configuration.logicalAddresses.primary) != CEC_DEVICE_TYPE_RESERVED)
  {
    bReturn |= SetSettingDeviceType(CLibCEC::GetType(configuration.logicalAddresses.primary));
    bReturn |= SetSettingDefaultLogicalAddress(configuration.logicalAddresses.primary);
    bReturn |= SetSettingLogicalAddressMask(CLibCEC::GetMaskForType(configuration.logicalAddresses.primary));
  }
  else
  {
    bReturn |= SetSettingDeviceType(configuration.deviceTypes[0]);
  }
  bReturn |= SetSettingPhysicalAddress(configuration.iPhysicalAddress);
  bReturn |= SetSettingOSDName(configuration.strDeviceName);
  if (m_savedConfiguration.iFirmwareVersion >= 10)
  {
#if CEC_LIB_VERSION_MAJOR >= 5
    if ((configuration.bAutoPowerOn == 0) || (configuration.bAutoPowerOn == 1))
      bReturn |= SetSettingAutoPowerOn(configuration.bAutoPowerOn == 1);
#else
    bReturn |= SetSettingAutoPowerOn(false);
#endif
  }
  else
  {
    bReturn |= SetSettingCECVersion(configuration.cecVersion);
  }

  return bReturn;
}

bool CUSBCECAdapterCommands::RequestSettings(void)
{
  if (m_savedConfiguration.iFirmwareVersion < 2)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - firmware version %d does not have any eeprom settings", __FUNCTION__, m_savedConfiguration.iFirmwareVersion);
    // settings can only be saved when using firmware v2+
    return false;
  }

  if (m_bSettingsRetrieved)
    return true;

  bool bReturn(true);
  bReturn |= RequestSettingAutoEnabled();
  bReturn |= RequestSettingDefaultLogicalAddress();
  bReturn |= RequestSettingDeviceType();
  bReturn |= RequestSettingLogicalAddressMask();
  bReturn |= RequestSettingOSDName();
  bReturn |= RequestSettingPhysicalAddress();
  if (m_savedConfiguration.iFirmwareVersion >= 10)
    bReturn |= RequestSettingAutoPowerOn();
  else
    bReturn |= RequestSettingCECVersion();

  // don't read the following settings:
  // - auto enabled (always enabled)
  // - default logical address (autodetected)
  // - logical address mask (autodetected)
  // - CEC version (1.3a)

  m_bSettingsRetrieved = true;

  return bReturn;
}

bool CUSBCECAdapterCommands::GetConfiguration(libcec_configuration &configuration)
{
  // get the settings from the eeprom if needed
  if (!RequestSettings())
    return false;

  // copy the settings
  configuration.iFirmwareVersion   = m_savedConfiguration.iFirmwareVersion;
  configuration.iFirmwareBuildDate = m_savedConfiguration.iFirmwareBuildDate;
  configuration.deviceTypes        = m_savedConfiguration.deviceTypes;
  configuration.iPhysicalAddress   = m_savedConfiguration.iPhysicalAddress;
  configuration.cecVersion         = m_savedConfiguration.cecVersion;
#if CEC_LIB_VERSION_MAJOR >= 5
  configuration.bAutoPowerOn       = m_savedConfiguration.bAutoPowerOn;
#endif
  memcpy(configuration.strDeviceName, m_savedConfiguration.strDeviceName, LIBCEC_OSD_NAME_SIZE);

  return true;
}

bool CUSBCECAdapterCommands::PingAdapter(void)
{
#ifdef CEC_DEBUGGING
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: sending ping");
#endif

  CCECAdapterMessage params;
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_PING, params);
  bool bReturn = message && message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  SAFE_DELETE(message);
  return bReturn;
}

bool CUSBCECAdapterCommands::SetAckMask(uint16_t iMask)
{
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: updating ackmask: %04X", iMask);

  CCECAdapterMessage params;
  params.PushEscaped(iMask >> 8);
  params.PushEscaped((uint8_t)iMask);
  CCECAdapterMessage *message  = m_comm->SendCommand(MSGCODE_SET_ACK_MASK, params);
  bool bReturn = message && message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  SAFE_DELETE(message);
  return bReturn;
}

void CUSBCECAdapterCommands::SetActiveSource(bool bSetTo, bool bClientUnregistered)
{
  if (bClientUnregistered) return;
  if (m_savedConfiguration.iFirmwareVersion >= 3)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: updating active source status: %s", bSetTo ? "active" : "inactive");
    CCECAdapterMessage params;
    params.PushEscaped(bSetTo ? 1 : 0);
    CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_ACTIVE_SOURCE, params);
    SAFE_DELETE(message);
  }
}

bool CUSBCECAdapterCommands::StartBootloader(void)
{
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: starting the bootloader");

  CCECAdapterMessage params;
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_START_BOOTLOADER, params);
  bool bReturn = message && message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  SAFE_DELETE(message);
  return bReturn;
}

bool CUSBCECAdapterCommands::SetLineTimeout(uint8_t iTimeout)
{
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: updating line timeout: %u", iTimeout);
  CCECAdapterMessage params;
  params.PushEscaped(iTimeout);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_TRANSMIT_IDLETIME, params);
  bool bReturn = message && message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  SAFE_DELETE(message);
  return bReturn;
}

bool CUSBCECAdapterCommands::SetControlledMode(bool controlled)
{
  {
    CLockObject lock(m_mutex);
    if (m_bControlledMode == controlled)
      return true;
  }

  LIB_CEC->AddLog(CEC_LOG_DEBUG, "usbcec: %s controlled mode", controlled ? "enabling" : "disabling");

  CCECAdapterMessage params;
  params.PushEscaped(controlled ? 1 : 0);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_CONTROLLED, params);
  bool bReturn = message && message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  SAFE_DELETE(message);

  if (bReturn)
  {
    CLockObject lock(m_mutex);
    m_bControlledMode = controlled;
  }

  return bReturn;
}
