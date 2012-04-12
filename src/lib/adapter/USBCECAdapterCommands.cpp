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
  m_iFirmwareVersion = CEC_FW_VERSION_UNKNOWN;
  unsigned int iFwVersionTry(0);

  while (m_iFirmwareVersion == CEC_FW_VERSION_UNKNOWN && iFwVersionTry++ < 3)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting the firmware version");
    cec_datapacket response = RequestSetting(MSGCODE_FIRMWARE_VERSION);
    if (response.size == 2)
      m_iFirmwareVersion = (response[0] << 8 | response[1]);
    else
    {
      CLibCEC::AddLog(CEC_LOG_WARNING, "the adapter did not respond with a correct firmware version (try %d)", iFwVersionTry);
      CEvent::Sleep(500);
    }
  }

  if (m_iFirmwareVersion == CEC_FW_VERSION_UNKNOWN)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "defaulting to firmware version 1");
    m_iFirmwareVersion = 1;
  }

  return m_iFirmwareVersion;
}

bool CUSBCECAdapterCommands::RequestSettingAutoEnabled(bool &enabled)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting autonomous mode setting");

  cec_datapacket response = RequestSetting(MSGCODE_GET_AUTO_ENABLED);
  if (response.size == 1)
  {
    enabled = response[0] == 1;
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommands::RequestSettingCECVersion(cec_version &version)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting CEC version setting");

  cec_datapacket response = RequestSetting(MSGCODE_GET_HDMI_VERSION);
  if (response.size == 1)
  {
    version = (cec_version)response[0];
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommands::RequestSettingDefaultLogicalAddress(cec_logical_address &address)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting default logical address setting");

  cec_datapacket response = RequestSetting(MSGCODE_GET_DEFAULT_LOGICAL_ADDRESS);
  if (response.size == 1)
  {
    address = (cec_logical_address)response[0];
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommands::RequestSettingDeviceType(cec_device_type &value)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting device type setting");

  cec_datapacket response = RequestSetting(MSGCODE_GET_DEVICE_TYPE);
  if (response.size == 1)
  {
    value = (cec_device_type)response[0];
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommands::RequestSettingLogicalAddressMask(uint16_t &iMask)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting logical address mask setting");

  cec_datapacket response = RequestSetting(MSGCODE_GET_LOGICAL_ADDRESS_MASK);
  if (response.size == 2)
  {
    iMask = ((uint16_t)response[0] << 8) | ((uint16_t)response[1]);
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommands::RequestSettingOSDName(CStdString &strOSDName)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting OSD name setting");

  cec_datapacket response = RequestSetting(MSGCODE_GET_OSD_NAME);
  if (response.size == 0)
    return false;

  char buf[14];
  for (uint8_t iPtr = 0; iPtr < response.size && iPtr < 13; iPtr++)
    buf[iPtr] = (char)response[iPtr];
  buf[response.size] = 0;

  strOSDName.Format("%s", buf);
  return true;
}

bool CUSBCECAdapterCommands::RequestSettingPhysicalAddress(uint16_t &iPhysicalAddress)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting physical address setting");

  cec_datapacket response = RequestSetting(MSGCODE_GET_PHYSICAL_ADDRESS);
  if (response.size == 2)
  {
    iPhysicalAddress = ((uint16_t)response[0] << 8) | ((uint16_t)response[1]);
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommands::SetSettingAutoEnabled(bool enabled)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "turning autonomous mode %s", enabled ? "on" : "off");

  CCECAdapterMessage params;
  params.PushEscaped(enabled ? 1 : 0);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_AUTO_ENABLED, params);
  bool bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;
  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingDeviceType(cec_device_type type)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the device type to %1X", (uint8_t)type);

  CCECAdapterMessage params;
  params.PushEscaped((uint8_t)type);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_DEVICE_TYPE, params);
  bool bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;
  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingDefaultLogicalAddress(cec_logical_address address)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the default logical address to %1X", address);

  CCECAdapterMessage params;
  params.PushEscaped((uint8_t)address);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_DEFAULT_LOGICAL_ADDRESS, params);
  bool bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;
  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingLogicalAddressMask(uint16_t iMask)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the logical address mask to %2X", iMask);

  CCECAdapterMessage params;
  params.PushEscaped(iMask >> 8);
  params.PushEscaped((uint8_t)iMask);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_LOGICAL_ADDRESS_MASK, params);
  bool bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;
  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingPhysicalAddress(uint16_t iPhysicalAddress)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the physical address to %04X", iPhysicalAddress);

  CCECAdapterMessage params;
  params.PushEscaped(iPhysicalAddress >> 8);
  params.PushEscaped((uint8_t)iPhysicalAddress);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_PHYSICAL_ADDRESS, params);
  bool bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;
  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingCECVersion(cec_version version)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the CEC version to %s", CLibCEC::GetInstance()->ToString(version));

  CCECAdapterMessage params;
  params.PushEscaped((uint8_t)version);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_HDMI_VERSION, params);
  bool bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;
  return bReturn;
}

bool CUSBCECAdapterCommands::SetSettingOSDName(const char *strOSDName)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the OSD name to %s", strOSDName);

  CCECAdapterMessage params;
  for (size_t iPtr = 0; iPtr < strlen(strOSDName); iPtr++)
    params.PushEscaped(strOSDName[iPtr]);
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_SET_OSD_NAME, params);
  bool bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;
  return bReturn;
}

bool CUSBCECAdapterCommands::WriteEEPROM(void)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "writing settings in the EEPROM");

  CCECAdapterMessage params;
  CCECAdapterMessage *message = m_comm->SendCommand(MSGCODE_WRITE_EEPROM, params);
  bool bReturn = message->state == ADAPTER_MESSAGE_STATE_SENT_ACKED;
  delete message;
  return bReturn;
}

bool CUSBCECAdapterCommands::PersistConfiguration(libcec_configuration *configuration)
{
  if (m_iFirmwareVersion < 2)
    return false;

  bool bReturn(true);
  bReturn &= SetSettingAutoEnabled(true);
  bReturn &= SetSettingDeviceType(CLibCEC::GetType(configuration->logicalAddresses.primary));
  bReturn &= SetSettingDefaultLogicalAddress(configuration->logicalAddresses.primary);
  bReturn &= SetSettingLogicalAddressMask(CLibCEC::GetMaskForType(configuration->logicalAddresses.primary));
  bReturn &= SetSettingPhysicalAddress(configuration->iPhysicalAddress);
  bReturn &= SetSettingCECVersion(CEC_VERSION_1_3A);
  bReturn &= SetSettingOSDName(configuration->strDeviceName);
  if (bReturn)
    bReturn = WriteEEPROM();
  return bReturn;
}

bool CUSBCECAdapterCommands::GetConfiguration(libcec_configuration *configuration)
{
  configuration->iFirmwareVersion = m_iFirmwareVersion;
  if (m_iFirmwareVersion < 2)
    return false;

  bool bReturn(true);
  cec_device_type type;
  if (RequestSettingDeviceType(type))
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "using persisted device type setting %s", CLibCEC::GetInstance()->ToString(type));
    configuration->deviceTypes.Clear();
    configuration->deviceTypes.Add(type);
  }
  else
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "no persisted device type setting");
    bReturn = false;
  }

  if (RequestSettingPhysicalAddress(configuration->iPhysicalAddress))
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "using persisted physical address setting %4x", configuration->iPhysicalAddress);
  }
  else
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "no persisted physical address setting");
    bReturn = false;
  }

  CStdString strDeviceName;
  if (RequestSettingOSDName(strDeviceName))
  {
    snprintf(configuration->strDeviceName, 13, "%s", strDeviceName.c_str());
    CLibCEC::AddLog(CEC_LOG_DEBUG, "using persisted device name setting %s", configuration->strDeviceName);
  }
  else
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "no persisted device name setting");
    bReturn = false;
  }

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
  return bReturn;
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
