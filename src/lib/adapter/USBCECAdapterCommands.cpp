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

#include "USBCECAdapterCommands.h"
#include "../LibCEC.h"
#include "../CECProcessor.h"

using namespace CEC;
using namespace PLATFORM;

CUSBCECAdapterCommands::CUSBCECAdapterCommands(CUSBCECAdapterCommunication *comm) :
    m_comm(comm),
    m_bSettingsRetrieved(false),
    m_bSettingAutoEnabled(false),
    m_settingCecVersion(CEC_VERSION_UNKNOWN),
    m_iSettingLAMask(0),
    m_bNeedsWrite(false),
    m_iBuildDate(CEC_FW_BUILD_UNKNOWN)
{
  m_persistedConfiguration.Clear();
}

cec_datapacket CUSBCECAdapterCommands::RequestSetting(cec_adapter_messagecode msgCode)
{
  cec_datapacket retVal;
  retVal.Clear();

  CCECAdapterMessage params;
  CCECAdapterMessage *message = m_comm->SendCommand(msgCode, params);
  if (message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED)
  {
    retVal = message->response;
    retVal.Shift(2); // shift out start and msgcode
    retVal.size -= 1; // remove end
  }
  delete message;
  return retVal;
}

uint16_t CUSBCECAdapterCommands::RequestFirmwareVersion(void)
{
  m_persistedConfiguration.iFirmwareVersion = CEC_FW_VERSION_UNKNOWN;
  unsigned int iFwVersionTry(0);

  while (m_persistedConfiguration.iFirmwareVersion == CEC_FW_VERSION_UNKNOWN && iFwVersionTry++ < 3)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting the firmware version");
    cec_datapacket response = RequestSetting(MSGCODE_FIRMWARE_VERSION);
    if (response.size == 2)
      m_persistedConfiguration.iFirmwareVersion = (response[0] << 8 | response[1]);
    else
    {
      CLibCEC::AddLog(CEC_LOG_WARNING, "the adapter did not respond with a correct firmware version (try %d)", iFwVersionTry);
      CEvent::Sleep(500);
    }
  }

  if (m_persistedConfiguration.iFirmwareVersion == CEC_FW_VERSION_UNKNOWN)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "defaulting to firmware version 1");
    m_persistedConfiguration.iFirmwareVersion = 1;
  }

  return m_persistedConfiguration.iFirmwareVersion;
}

bool CUSBCECAdapterCommands::RequestSettingAutoEnabled(void)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting autonomous mode setting");

  cec_datapacket response = RequestSetting(MSGCODE_GET_AUTO_ENABLED);
  if (response.size == 1)
  {
    m_bSettingAutoEnabled = response[0] == 1;
    CLibCEC::AddLog(CEC_LOG_DEBUG, "using persisted autonomous mode setting: '%s'", m_bSettingAutoEnabled ? "enabled" : "disabled");
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommands::RequestSettingCECVersion(void)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting CEC version setting");

  cec_datapacket response = RequestSetting(MSGCODE_GET_HDMI_VERSION);
  if (response.size == 1)
  {
    m_settingCecVersion = (cec_version)response[0];
    CLibCEC::AddLog(CEC_LOG_DEBUG, "using persisted CEC version setting: '%s'", CLibCEC::GetInstance()->ToString(m_settingCecVersion));
    return true;
  }
  return false;
}

uint32_t CUSBCECAdapterCommands::RequestBuildDate(void)
{
  if (m_iBuildDate == CEC_FW_BUILD_UNKNOWN)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting firmware build date");

    cec_datapacket response = RequestSetting(MSGCODE_GET_BUILDDATE);
    if (response.size == 4)
      m_iBuildDate = (uint32_t)response[0] << 24 | (uint32_t)response[1] << 16 | (uint32_t)response[2] << 8 | (uint32_t)response[3];
  }
  return m_iBuildDate;
}

bool CUSBCECAdapterCommands::RequestSettingDefaultLogicalAddress(void)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting default logical address setting");

  cec_datapacket response = RequestSetting(MSGCODE_GET_DEFAULT_LOGICAL_ADDRESS);
  if (response.size == 1)
  {
    m_persistedConfiguration.logicalAddresses.primary = (cec_logical_address)response[0];
    CLibCEC::AddLog(CEC_LOG_DEBUG, "using persisted logical address setting: '%s'", CLibCEC::GetInstance()->ToString(m_persistedConfiguration.logicalAddresses.primary));
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommands::RequestSettingDeviceType(void)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting device type setting");
  m_persistedConfiguration.deviceTypes.Clear();

  cec_datapacket response = RequestSetting(MSGCODE_GET_DEVICE_TYPE);
  if (response.size == 1)
  {
    m_persistedConfiguration.deviceTypes.Add((cec_device_type)response[0]);
    CLibCEC::AddLog(CEC_LOG_DEBUG, "using persisted device type setting: '%s'", CLibCEC::GetInstance()->ToString((cec_device_type)response[0]));
    return true;
  }
  CLibCEC::AddLog(CEC_LOG_DEBUG, "no persisted device type setting");
  return false;
}

bool CUSBCECAdapterCommands::RequestSettingLogicalAddressMask(void)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting logical address mask setting");

  cec_datapacket response = RequestSetting(MSGCODE_GET_LOGICAL_ADDRESS_MASK);
  if (response.size == 2)
  {
    m_iSettingLAMask = ((uint16_t)response[0] << 8) | ((uint16_t)response[1]);
    CLibCEC::AddLog(CEC_LOG_DEBUG, "using persisted logical address mask setting: '%x'", m_iSettingLAMask);
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommands::RequestSettingOSDName(void)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting OSD name setting");

  memset(m_persistedConfiguration.strDeviceName, 0, 13);
  cec_datapacket response = RequestSetting(MSGCODE_GET_OSD_NAME);
  if (response.size == 0)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "no persisted device name setting");
    return false;
  }

  char buf[14];
  for (uint8_t iPtr = 0; iPtr < response.size && iPtr < 13; iPtr++)
    buf[iPtr] = (char)response[iPtr];
  buf[response.size] = 0;

  snprintf(m_persistedConfiguration.strDeviceName, 13, "%s", buf);
  CLibCEC::AddLog(CEC_LOG_DEBUG, "using persisted device name setting: '%s'", buf);
  return true;
}

bool CUSBCECAdapterCommands::RequestSettingPhysicalAddress(void)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting physical address setting");

  cec_datapacket response = RequestSetting(MSGCODE_GET_PHYSICAL_ADDRESS);
  if (response.size == 2)
  {
    m_persistedConfiguration.iPhysicalAddress = ((uint16_t)response[0] << 8) | ((uint16_t)response[1]);
    CLibCEC::AddLog(CEC_LOG_DEBUG, "using persisted physical address setting: '%4x'", m_persistedConfiguration.iPhysicalAddress);
    return true;
  }
  CLibCEC::AddLog(CEC_LOG_DEBUG, "no persisted physical address setting");
  return false;
}

bool CUSBCECAdapterCommands::SetSettingAutoEnabled(bool enabled)
{
  bool bReturn(true);

  /* check whether this value was changed */
  if (m_bSettingAutoEnabled == enabled)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "autonomous mode setting unchanged (%s)", enabled ? "on" : "off");
    return bReturn;
  }

  m_bNeedsWrite = true;
  CLibCEC::AddLog(CEC_LOG_DEBUG, "turning autonomous mode %s", enabled ? "on" : "off");

  CCECAdapterMessage params;
  params.PushEscaped(enabled ? 1 : 0);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_AUTO_ENABLED, params);
  bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;

  if (bReturn)
    m_bSettingAutoEnabled = enabled;

  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingDeviceType(cec_device_type type)
{
  bool bReturn(true);

  /* check whether this value was changed */
  if (m_persistedConfiguration.deviceTypes.types[0] == type)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "device type setting unchanged (%X)", (uint8_t)type);
    return bReturn;
  }

  m_bNeedsWrite = true;
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the device type to %X (previous: %X)", (uint8_t)type, (uint8_t)m_persistedConfiguration.deviceTypes.types[0]);

  CCECAdapterMessage params;
  params.PushEscaped((uint8_t)type);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_DEVICE_TYPE, params);
  bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;

  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingDefaultLogicalAddress(cec_logical_address address)
{
  bool bReturn(true);

  /* check whether this value was changed */
  if (m_persistedConfiguration.logicalAddresses.primary == address)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "logical address setting unchanged (%X)", (uint8_t)address);
    return bReturn;
  }

  m_bNeedsWrite = true;
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the default logical address to %X (previous: %X)", (uint8_t)address, (uint8_t)m_persistedConfiguration.logicalAddresses.primary);

  CCECAdapterMessage params;
  params.PushEscaped((uint8_t)address);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_DEFAULT_LOGICAL_ADDRESS, params);
  bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;

  if (bReturn)
    m_persistedConfiguration.logicalAddresses.primary = address;

  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingLogicalAddressMask(uint16_t iMask)
{
  bool bReturn(true);

  /* check whether this value was changed */
  if (m_iSettingLAMask == iMask)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "logical address mask setting unchanged (%2X)", iMask);
    return bReturn;
  }

  m_bNeedsWrite = true;
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the logical address mask to %2X (previous: %2X)", iMask, m_iSettingLAMask);

  CCECAdapterMessage params;
  params.PushEscaped(iMask >> 8);
  params.PushEscaped((uint8_t)iMask);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_LOGICAL_ADDRESS_MASK, params);
  bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;

  if (bReturn)
    m_iSettingLAMask = iMask;

  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingPhysicalAddress(uint16_t iPhysicalAddress)
{
  bool bReturn(true);

  /* check whether this value was changed */
  if (m_persistedConfiguration.iPhysicalAddress == iPhysicalAddress)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "physical address setting unchanged (%04X)", iPhysicalAddress);
    return bReturn;
  }

  m_bNeedsWrite = true;
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the physical address to %04X (previous: %04X)", iPhysicalAddress, m_persistedConfiguration.iPhysicalAddress);

  CCECAdapterMessage params;
  params.PushEscaped(iPhysicalAddress >> 8);
  params.PushEscaped((uint8_t)iPhysicalAddress);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_PHYSICAL_ADDRESS, params);
  bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;

  if (bReturn)
    m_persistedConfiguration.iPhysicalAddress = iPhysicalAddress;

  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingCECVersion(cec_version version)
{
  bool bReturn(true);

  /* check whether this value was changed */
  if (m_settingCecVersion == version)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "CEC version setting unchanged (%s)", CLibCEC::GetInstance()->ToString(version));
    return bReturn;
  }

  m_bNeedsWrite = true;
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the CEC version to %s (previous: %s)", CLibCEC::GetInstance()->ToString(version), CLibCEC::GetInstance()->ToString(m_settingCecVersion));

  CCECAdapterMessage params;
  params.PushEscaped((uint8_t)version);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_HDMI_VERSION, params);
  bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;

  if (bReturn)
    m_settingCecVersion = version;

  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingOSDName(const char *strOSDName)
{
  bool bReturn(true);

  /* check whether this value was changed */
  if (!strcmp(m_persistedConfiguration.strDeviceName, strOSDName))
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "OSD name setting unchanged (%s)", strOSDName);
    return bReturn;
  }

  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the OSD name to %s (previous: %s)", strOSDName, m_persistedConfiguration.strDeviceName);

  CCECAdapterMessage params;
  for (size_t iPtr = 0; iPtr < strlen(strOSDName); iPtr++)
    params.PushEscaped(strOSDName[iPtr]);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_OSD_NAME, params);
  bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;

  if (bReturn)
    snprintf(m_persistedConfiguration.strDeviceName, 13, "%s", strOSDName);

  return bReturn;
}

bool CUSBCECAdapterCommands::WriteEEPROM(void)
{
  if (!m_bNeedsWrite)
    return true;

  CLibCEC::AddLog(CEC_LOG_DEBUG, "writing settings in the EEPROM");

  CCECAdapterMessage params;
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_WRITE_EEPROM, params);
  m_bNeedsWrite = !(message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED);
  delete message;
  return m_bNeedsWrite;
}

bool CUSBCECAdapterCommands::PersistConfiguration(libcec_configuration *configuration)
{
  if (m_persistedConfiguration.iFirmwareVersion < 2)
    return false;

  if (!RequestSettings())
    return false;

  bool bReturn(true);
  bReturn &= SetSettingAutoEnabled(true);
  bReturn &= SetSettingDeviceType(CLibCEC::GetType(configuration->logicalAddresses.primary));
  bReturn &= SetSettingDefaultLogicalAddress(configuration->logicalAddresses.primary);
  bReturn &= SetSettingLogicalAddressMask(CLibCEC::GetMaskForType(configuration->logicalAddresses.primary));
  bReturn &= SetSettingPhysicalAddress(configuration->iPhysicalAddress);
  bReturn &= SetSettingCECVersion(CEC_VERSION_1_3A);
  bReturn &= SetSettingOSDName(configuration->strDeviceName);
  bReturn &= WriteEEPROM();
  return bReturn;
}

bool CUSBCECAdapterCommands::RequestSettings(void)
{
  if (m_persistedConfiguration.iFirmwareVersion < 2)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - firmware version %d does not have any eeprom settings", __FUNCTION__, m_persistedConfiguration.iFirmwareVersion);
    // settings can only be persisted with firmware v2+
    return false;
  }

  if (m_bSettingsRetrieved)
    return true;

  bool bReturn(true);
  bReturn &= RequestSettingAutoEnabled();
  bReturn &= RequestSettingCECVersion();
  bReturn &= RequestSettingDefaultLogicalAddress();
  bReturn &= RequestSettingDeviceType();
  bReturn &= RequestSettingLogicalAddressMask();
  bReturn &= RequestSettingOSDName();
  bReturn &= RequestSettingPhysicalAddress();

  // don't read the following settings:
  // - auto enabled (always enabled)
  // - default logical address (autodetected)
  // - logical address mask (autodetected)
  // - CEC version (1.3a)

  // TODO to be added to the firmware:
  // - base device (4 bits)
  // - HDMI port number (4 bits)
  // - TV vendor id (12 bits)
  // - wake devices (8 bits)
  // - standby devices (8 bits)
  // - use TV menu language (1 bit)
  // - activate source (1 bit)
  // - power off screensaver (1 bit)
  // - power off on standby (1 bit)
  // - send inactive source (1 bit)

  m_bSettingsRetrieved = true;

  return bReturn;
}

bool CUSBCECAdapterCommands::GetConfiguration(libcec_configuration *configuration)
{
  // get the settings from the eeprom if needed
  if (!RequestSettings())
    return false;

  // copy the settings
  configuration->iFirmwareVersion = m_persistedConfiguration.iFirmwareVersion;
  configuration->deviceTypes      = m_persistedConfiguration.deviceTypes;
  configuration->iPhysicalAddress = m_persistedConfiguration.iPhysicalAddress;
  snprintf(configuration->strDeviceName, 13, "%s", m_persistedConfiguration.strDeviceName);

  return true;
}

bool CUSBCECAdapterCommands::PingAdapter(void)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "sending ping");

  CCECAdapterMessage params;
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_PING, params);
  bool bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;
  return bReturn;
}

bool CUSBCECAdapterCommands::SetAckMask(uint16_t iMask)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting ackmask to %2x", iMask);

  CCECAdapterMessage params;
  params.PushEscaped(iMask >> 8);
  params.PushEscaped((uint8_t)iMask);
  CCECAdapterMessage *message  = m_comm->SendCommand(MSGCODE_SET_ACK_MASK, params);
  bool bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;
  return bReturn;
}

bool CUSBCECAdapterCommands::StartBootloader(void)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "starting the bootloader");

  CCECAdapterMessage params;
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_START_BOOTLOADER, params);
  bool bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;
  return bReturn;
}

bool CUSBCECAdapterCommands::SetLineTimeout(uint8_t iTimeout)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the line timeout to %d", iTimeout);
  CCECAdapterMessage params;
  params.PushEscaped(iTimeout);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_TRANSMIT_IDLETIME, params);
  bool bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;
  return bReturn;
}

bool CUSBCECAdapterCommands::SetControlledMode(bool controlled)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "turning controlled mode %s", controlled ? "on" : "off");

  CCECAdapterMessage params;
  params.PushEscaped(controlled ? 1 : 0);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_CONTROLLED, params);
  bool bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;
  return bReturn;
}
