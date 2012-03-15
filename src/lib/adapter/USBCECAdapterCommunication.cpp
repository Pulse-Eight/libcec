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

#include "USBCECAdapterCommunication.h"
#include "../platform/sockets/serialport.h"
#include "../platform/util/timeutils.h"
#include "../LibCEC.h"
#include "../CECProcessor.h"

using namespace std;
using namespace CEC;
using namespace PLATFORM;

#define CEC_ADAPTER_PING_TIMEOUT 15000

void *CUSBCECAdapterProcessor::Process(void)
{
  cec_command command;
  while (!IsStopped())
  {
    if (m_inBuffer.Pop(command))
      m_callback->OnCommandReceived(command);
    Sleep(5);
  }

  return NULL;
}

void CUSBCECAdapterProcessor::AddCommand(cec_command command)
{
  m_inBuffer.Push(command);
}

CUSBCECAdapterCommunication::CUSBCECAdapterCommunication(CCECProcessor *processor, const char *strPort, uint16_t iBaudRate /* = 38400 */) :
    m_port(NULL),
    m_processor(processor),
    m_bHasData(false),
    m_iLineTimeout(0),
    m_iFirmwareVersion(CEC_FW_VERSION_UNKNOWN),
    m_lastInitiator(CECDEVICE_UNKNOWN),
    m_bNextIsEscaped(false),
    m_bGotStart(false),
    m_messageProcessor(NULL),
    m_bInitialised(false)
{
  m_port = new CSerialPort(strPort, iBaudRate);
}

CUSBCECAdapterCommunication::~CUSBCECAdapterCommunication(void)
{
  Close();
}

bool CUSBCECAdapterCommunication::CheckAdapter(uint32_t iTimeoutMs /* = 10000 */)
{
  bool bReturn(false);
  uint64_t iNow = GetTimeMs();
  uint64_t iTarget = iTimeoutMs > 0 ? iNow + iTimeoutMs : iNow + CEC_DEFAULT_TRANSMIT_WAIT;

  /* try to ping the adapter */
  bool bPinged(false);
  unsigned iPingTry(0);
  while (iNow < iTarget && (bPinged = PingAdapter()) == false)
  {
    CLibCEC::AddLog(CEC_LOG_ERROR, "the adapter did not respond correctly to a ping (try %d)", ++iPingTry);
    CEvent::Sleep(500);
    iNow = GetTimeMs();
  }

  /* try to read the firmware version */
  m_iFirmwareVersion = CEC_FW_VERSION_UNKNOWN;
  unsigned iFwVersionTry(0);
  while (bPinged && iNow < iTarget && (m_iFirmwareVersion = GetFirmwareVersion()) == CEC_FW_VERSION_UNKNOWN && iFwVersionTry < 3)
  {
    CLibCEC::AddLog(CEC_LOG_WARNING, "the adapter did not respond with a correct firmware version (try %d)", ++iFwVersionTry);
    CEvent::Sleep(500);
    iNow = GetTimeMs();
  }

  if (m_iFirmwareVersion == CEC_FW_VERSION_UNKNOWN)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "defaulting to firmware version 1");
    m_iFirmwareVersion = 1;
  }

  if (m_iFirmwareVersion >= 2)
  {
    /* try to set controlled mode */
    unsigned iControlledTry(0);
    bool bControlled(false);
    while (iNow < iTarget && (bControlled = SetControlledMode(true)) == false)
    {
      CLibCEC::AddLog(CEC_LOG_ERROR, "the adapter did not respond correctly to setting controlled mode (try %d)", ++iControlledTry);
      CEvent::Sleep(500);
      iNow = GetTimeMs();
    }
    bReturn = bControlled;
  }
  else
    bReturn = true;

  {
    CLockObject lock(m_mutex);
    m_bInitialised = bReturn;
  }

  return bReturn;
}

bool CUSBCECAdapterCommunication::Open(IAdapterCommunicationCallback *cb, uint32_t iTimeoutMs /* = 10000 */, bool bSkipChecks /* = false */)
{
  uint64_t iNow = GetTimeMs();
  uint64_t iTimeout = iNow + iTimeoutMs;

  {
    CLockObject lock(m_mutex);

    if (!m_port)
    {
      CLibCEC::AddLog(CEC_LOG_ERROR, "port is NULL");
      return false;
    }

    if (IsOpen())
    {
      CLibCEC::AddLog(CEC_LOG_ERROR, "port is already open");
      return true;
    }

    m_callback = cb;
    CStdString strError;
    bool bConnected(false);
    while (!bConnected && iNow < iTimeout)
    {
      if ((bConnected = m_port->Open(iTimeout)) == false)
      {
        strError.Format("error opening serial port '%s': %s", m_port->GetName().c_str(), m_port->GetError().c_str());
        Sleep(250);
        iNow = GetTimeMs();
      }
    }

    if (!bConnected)
    {
      CLibCEC::AddLog(CEC_LOG_ERROR, strError);
      return false;
    }

    CLibCEC::AddLog(CEC_LOG_DEBUG, "connection opened, clearing any previous input and waiting for active transmissions to end before starting");

    if (!bSkipChecks)
    {
      //clear any input bytes
      uint8_t buff[1024];
      while (m_port->Read(buff, 1024, 100) > 0)
      {
        CLibCEC::AddLog(CEC_LOG_DEBUG, "data received, clearing it");
        Sleep(250);
      }
    }
  }

  if (!bSkipChecks && !CheckAdapter())
  {
    CLibCEC::AddLog(CEC_LOG_ERROR, "the adapter failed to pass basic checks");
    return false;
  }
  else
  {
    if (CreateThread())
    {
      CLibCEC::AddLog(CEC_LOG_DEBUG, "communication thread started");
      return true;
    }
    else
    {
      CLibCEC::AddLog(CEC_LOG_ERROR, "could not create a communication thread");
    }
  }

  return false;
}

void CUSBCECAdapterCommunication::Close(void)
{
  StopThread();
}

void *CUSBCECAdapterCommunication::Process(void)
{
  m_messageProcessor = new CUSBCECAdapterProcessor(m_callback);
  m_messageProcessor->CreateThread();

  cec_command command;
  command.Clear();
  bool bCommandReceived(false);
  CTimeout pingTimeout(CEC_ADAPTER_PING_TIMEOUT);
  while (!IsStopped())
  {
    {
      CLockObject lock(m_mutex);
      ReadFromDevice(50);
      bCommandReceived = m_callback && Read(command, 0) && m_bInitialised;
    }

    /* push the next command to the callback method if there is one */
    if (!IsStopped() && bCommandReceived)
      m_messageProcessor->AddCommand(command);

    /* ping the adapter every 15 seconds */
    if (pingTimeout.TimeLeft() == 0)
    {
      pingTimeout.Init(CEC_ADAPTER_PING_TIMEOUT);
      PingAdapter();
    }

    if (!IsStopped())
    {
      Sleep(5);
      WriteNextCommand();
    }
  }

  /* stop the message processor */
  m_messageProcessor->StopThread();
  delete m_messageProcessor;

  /* notify all threads that are waiting on messages to be sent */
  CCECAdapterMessage *msg(NULL);
  while (m_outBuffer.Pop(msg))
    msg->event.Broadcast();

  /* set the ackmask to 0 before closing the connection */
  SetAckMaskInternal(0, true);

  if (m_iFirmwareVersion >= 2)
    SetControlledMode(false);

  if (m_port)
  {
    delete m_port;
    m_port = NULL;
  }

  return NULL;
}

cec_adapter_message_state CUSBCECAdapterCommunication::Write(const cec_command &data, uint8_t iMaxTries, uint8_t iLineTimeout /* = 3 */, uint8_t iRetryLineTimeout /* = 3 */)
{
  cec_adapter_message_state retVal(ADAPTER_MESSAGE_STATE_UNKNOWN);
  if (!IsRunning())
    return retVal;

  CCECAdapterMessage *output = new CCECAdapterMessage(data);

  /* set the number of retries */
  if (data.opcode == CEC_OPCODE_NONE) //TODO
    output->maxTries = 1;
  else if (data.initiator != CECDEVICE_BROADCAST)
    output->maxTries = iMaxTries;

  output->lineTimeout = iLineTimeout;
  output->retryTimeout = iRetryLineTimeout;
  output->tries = 0;

  bool bRetry(true);
  while (bRetry && ++output->tries < output->maxTries)
  {
    bRetry = (!Write(output) || output->NeedsRetry()) && output->transmit_timeout > 0;
    if (bRetry)
      Sleep(CEC_DEFAULT_TRANSMIT_RETRY_WAIT);
  }
  retVal = output->state;

  delete output;
  return retVal;
}

bool CUSBCECAdapterCommunication::Write(CCECAdapterMessage *data)
{
  data->state = ADAPTER_MESSAGE_STATE_WAITING_TO_BE_SENT;
  m_outBuffer.Push(data);
  data->event.Wait(5000);

  if ((data->expectControllerAck && data->state != ADAPTER_MESSAGE_STATE_SENT_ACKED) ||
      (!data->expectControllerAck && data->state != ADAPTER_MESSAGE_STATE_SENT))
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "command was not %s", data->state == ADAPTER_MESSAGE_STATE_SENT_NOT_ACKED ? "acked" : "sent");
    return false;
  }

  return true;
}

bool CUSBCECAdapterCommunication::Read(cec_command &command, uint32_t iTimeout)
{
  if (!IsRunning())
    return false;

  CCECAdapterMessage msg;
  if (Read(msg, iTimeout))
  {
    if (ParseMessage(msg))
    {
      command = m_currentframe;
      m_currentframe.Clear();
      return true;
    }
  }
  return false;
}

bool CUSBCECAdapterCommunication::Read(CCECAdapterMessage &msg, uint32_t iTimeout)
{
  CLockObject lock(m_mutex);

  msg.Clear();
  CCECAdapterMessage *buf(NULL);

  if (!m_inBuffer.Pop(buf))
  {
    if (iTimeout == 0 || !m_rcvCondition.Wait(m_mutex, m_bHasData, iTimeout))
      return false;
    m_inBuffer.Pop(buf);
    m_bHasData = !m_inBuffer.IsEmpty();
  }

  if (buf)
  {
    msg.packet = buf->packet;
    msg.state = ADAPTER_MESSAGE_STATE_INCOMING;
    delete buf;
    return true;
  }
  return false;
}

CStdString CUSBCECAdapterCommunication::GetError(void) const
{
  CStdString strError;
  strError = m_port->GetError();
  return strError;
}

bool CUSBCECAdapterCommunication::StartBootloader(void)
{
  bool bReturn(false);
  if (!IsRunning())
    return bReturn;

  CLibCEC::AddLog(CEC_LOG_DEBUG, "starting the bootloader");

  CCECAdapterMessage params;
  return SendCommand(MSGCODE_START_BOOTLOADER, params, false);
}

bool CUSBCECAdapterCommunication::PingAdapter(void)
{
  CLockObject lock(m_mutex);
  CLibCEC::AddLog(CEC_LOG_DEBUG, "sending ping");

  CCECAdapterMessage params;
  return SendCommand(MSGCODE_PING, params);
}

bool CUSBCECAdapterCommunication::ParseMessage(const CCECAdapterMessage &msg)
{
  bool bEom(false);
  bool bIsError(msg.IsError());

  if (msg.IsEmpty())
    return bEom;

  CLockObject adapterLock(m_mutex);
  switch(msg.Message())
  {
  case MSGCODE_FRAME_START:
    {
      m_currentframe.Clear();
      if (msg.Size() >= 2)
      {
        m_currentframe.initiator   = msg.Initiator();
        m_currentframe.destination = msg.Destination();
        m_currentframe.ack         = msg.IsACK();
        m_currentframe.eom         = msg.IsEOM();
      }
      if (m_currentframe.ack == 0x1)
      {
        m_lastInitiator = m_currentframe.initiator;
        m_processor->HandlePoll(m_currentframe.initiator, m_currentframe.destination);
      }
    }
    break;
  case MSGCODE_RECEIVE_FAILED:
    {
      m_currentframe.Clear();
      if (m_lastInitiator != CECDEVICE_UNKNOWN)
        bIsError = m_processor->HandleReceiveFailed(m_lastInitiator);
    }
    break;
  case MSGCODE_FRAME_DATA:
    {
      if (msg.Size() >= 2)
      {
        m_currentframe.PushBack(msg[1]);
        m_currentframe.eom = msg.IsEOM();
      }
    }
    break;
  default:
    break;
  }

  CLibCEC::AddLog(bIsError ? CEC_LOG_WARNING : CEC_LOG_DEBUG, msg.ToString());
  return msg.IsEOM();
}

uint16_t CUSBCECAdapterCommunication::GetFirmwareVersion(void)
{
  uint16_t iReturn(m_iFirmwareVersion);

  if (iReturn == CEC_FW_VERSION_UNKNOWN)
  {
    CLockObject lock(m_mutex);
    CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting the firmware version");
    cec_datapacket response = GetSetting(MSGCODE_FIRMWARE_VERSION, 2);
    if (response.size == 2)
    {
      m_iFirmwareVersion = (response[0] << 8 | response[1]);
      iReturn = m_iFirmwareVersion;
      CLibCEC::AddLog(CEC_LOG_DEBUG, "firmware version %d", m_iFirmwareVersion);
    }
  }

  return iReturn;
}

bool CUSBCECAdapterCommunication::SetLineTimeout(uint8_t iTimeout)
{
  m_iLineTimeout = iTimeout;
  bool bReturn(m_iLineTimeout != iTimeout);

  if (!bReturn)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the line timeout to %d", iTimeout);
    CCECAdapterMessage params;
    params.PushEscaped(iTimeout);
    bReturn = SendCommand(MSGCODE_TRANSMIT_IDLETIME, params);
  }

  return bReturn;
}

bool CUSBCECAdapterCommunication::SetAckMask(uint16_t iMask)
{
  return SetAckMaskInternal(iMask, IsRunning());
}

bool CUSBCECAdapterCommunication::SetAckMaskInternal(uint16_t iMask, bool bWriteDirectly /* = false */)
{
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting ackmask to %2x", iMask);

  CCECAdapterMessage params;
  params.PushEscaped(iMask >> 8);
  params.PushEscaped((uint8_t)iMask);
  return SendCommand(MSGCODE_SET_ACK_MASK, params, true, false, bWriteDirectly);
}

bool CUSBCECAdapterCommunication::PersistConfiguration(libcec_configuration *configuration)
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

bool CUSBCECAdapterCommunication::GetConfiguration(libcec_configuration *configuration)
{
  if (m_iFirmwareVersion < 2)
    return false;

  bool bReturn(true);
  cec_device_type type;
  if (GetSettingDeviceType(type))
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "using persisted device type setting %s", m_processor->ToString(type));
    configuration->deviceTypes.Clear();
    configuration->deviceTypes.Add(type);
  }
  else
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "no persisted device type setting");
    bReturn = false;
  }

  if (GetSettingPhysicalAddress(configuration->iPhysicalAddress))
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "using persisted physical address setting %4x", configuration->iPhysicalAddress);
  }
  else
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "no persisted physical address setting");
    bReturn = false;
  }

  CStdString strDeviceName;
  if (GetSettingOSDName(strDeviceName))
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

bool CUSBCECAdapterCommunication::SetControlledMode(bool controlled)
{
  CLockObject lock(m_mutex);
  CLibCEC::AddLog(CEC_LOG_DEBUG, "turning controlled mode %s", controlled ? "on" : "off");

  CCECAdapterMessage params;
  params.PushEscaped(controlled ? 1 : 0);
  return SendCommand(MSGCODE_SET_CONTROLLED, params);
}

bool CUSBCECAdapterCommunication::SetSettingAutoEnabled(bool enabled)
{
  CLockObject lock(m_mutex);
  CLibCEC::AddLog(CEC_LOG_DEBUG, "turning autonomous mode %s", enabled ? "on" : "off");

  CCECAdapterMessage params;
  params.PushEscaped(enabled ? 1 : 0);
  return SendCommand(MSGCODE_SET_AUTO_ENABLED, params);
}

bool CUSBCECAdapterCommunication::GetSettingAutoEnabled(bool &enabled)
{
  CLockObject lock(m_mutex);
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting autonomous mode setting");

  cec_datapacket response = GetSetting(MSGCODE_GET_AUTO_ENABLED, 1);
  if (response.size == 1)
  {
    enabled = response[0] == 1;
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommunication::SetSettingDeviceType(cec_device_type type)
{
  CLockObject lock(m_mutex);
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the device type to %1X", (uint8_t)type);

  CCECAdapterMessage params;
  params.PushEscaped((uint8_t)type);
  return SendCommand(MSGCODE_SET_DEVICE_TYPE, params);
}

bool CUSBCECAdapterCommunication::GetSettingDeviceType(cec_device_type &value)
{
  CLockObject lock(m_mutex);
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting device type setting");

  cec_datapacket response = GetSetting(MSGCODE_GET_DEVICE_TYPE, 1);
  if (response.size == 1)
  {
    value = (cec_device_type)response[0];
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommunication::SetSettingDefaultLogicalAddress(cec_logical_address address)
{
  CLockObject lock(m_mutex);
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the default logical address to %1X", address);

  CCECAdapterMessage params;
  params.PushEscaped((uint8_t)address);
  return SendCommand(MSGCODE_SET_DEFAULT_LOGICAL_ADDRESS, params);
}

bool CUSBCECAdapterCommunication::GetSettingDefaultLogicalAddress(cec_logical_address &address)
{
  CLockObject lock(m_mutex);
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting default logical address setting");

  cec_datapacket response = GetSetting(MSGCODE_GET_DEFAULT_LOGICAL_ADDRESS, 1);
  if (response.size == 1)
  {
    address = (cec_logical_address)response[0];
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommunication::SetSettingLogicalAddressMask(uint16_t iMask)
{
  CLockObject lock(m_mutex);
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the logical address mask to %2X", iMask);

  CCECAdapterMessage params;
  params.PushEscaped(iMask >> 8);
  params.PushEscaped((uint8_t)iMask);
  return SendCommand(MSGCODE_SET_LOGICAL_ADDRESS_MASK, params);
}

bool CUSBCECAdapterCommunication::GetSettingLogicalAddressMask(uint16_t &iMask)
{
  CLockObject lock(m_mutex);
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting logical address mask setting");

  cec_datapacket response = GetSetting(MSGCODE_GET_LOGICAL_ADDRESS_MASK, 2);
  if (response.size == 2)
  {
    iMask = ((uint16_t)response[0] << 8) | ((uint16_t)response[1]);
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommunication::SetSettingPhysicalAddress(uint16_t iPhysicalAddress)
{
  CLockObject lock(m_mutex);
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the physical address to %2X", iPhysicalAddress);

  CCECAdapterMessage params;
  params.PushEscaped(iPhysicalAddress >> 8);
  params.PushEscaped((uint8_t)iPhysicalAddress);
  return SendCommand(MSGCODE_SET_PHYSICAL_ADDRESS, params);
}

bool CUSBCECAdapterCommunication::GetSettingPhysicalAddress(uint16_t &iPhysicalAddress)
{
  CLockObject lock(m_mutex);
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting physical address setting");

  cec_datapacket response = GetSetting(MSGCODE_GET_PHYSICAL_ADDRESS, 2);
  if (response.size == 2)
  {
    iPhysicalAddress = ((uint16_t)response[0] << 8) | ((uint16_t)response[1]);
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommunication::SetSettingCECVersion(cec_version version)
{
  CLockObject lock(m_mutex);
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the CEC version to %s", CLibCEC::GetInstance()->ToString(version));

  CCECAdapterMessage params;
  params.PushEscaped((uint8_t)version);
  return SendCommand(MSGCODE_SET_HDMI_VERSION, params);
}

bool CUSBCECAdapterCommunication::GetSettingCECVersion(cec_version &version)
{
  CLockObject lock(m_mutex);
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting CEC version setting");

  cec_datapacket response = GetSetting(MSGCODE_GET_HDMI_VERSION, 1);
  if (response.size == 1)
  {
    version = (cec_version)response[0];
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommunication::SetSettingOSDName(const char *strOSDName)
{
  CLockObject lock(m_mutex);
  CLibCEC::AddLog(CEC_LOG_DEBUG, "setting the OSD name to %s", strOSDName);

  CCECAdapterMessage params;
  for (size_t iPtr = 0; iPtr < strlen(strOSDName); iPtr++)
    params.PushEscaped(strOSDName[iPtr]);
  return SendCommand(MSGCODE_SET_OSD_NAME, params);
}

bool CUSBCECAdapterCommunication::GetSettingOSDName(CStdString &strOSDName)
{
  CLockObject lock(m_mutex);
  CLibCEC::AddLog(CEC_LOG_DEBUG, "requesting OSD name setting");

  cec_datapacket response = GetSetting(MSGCODE_GET_OSD_NAME, 13);
  if (response.size == 0)
    return false;

  char buf[14];
  for (uint8_t iPtr = 0; iPtr < response.size && iPtr < 13; iPtr++)
    buf[iPtr] = (char)response[iPtr];
  buf[response.size] = 0;

  strOSDName.Format("%s", buf);
  return true;
}

bool CUSBCECAdapterCommunication::WriteEEPROM(void)
{
  CLockObject lock(m_mutex);
  CLibCEC::AddLog(CEC_LOG_DEBUG, "writing settings in the EEPROM");

  CCECAdapterMessage params;
  return SendCommand(MSGCODE_WRITE_EEPROM, params);
}

bool CUSBCECAdapterCommunication::IsOpen(void)
{
  return !IsStopped() && m_port->IsOpen() && IsRunning();
}

bool CUSBCECAdapterCommunication::WaitForAck(CCECAdapterMessage &message)
{
  bool bError(false);
  bool bTransmitSucceeded(false);
  uint8_t iPacketsLeft(message.isTransmission ? message.Size() / 4 : 1);

  int64_t iNow = GetTimeMs();
  int64_t iTargetTime = iNow + (message.transmit_timeout <= 5 ? CEC_DEFAULT_TRANSMIT_WAIT : message.transmit_timeout);

  while (!bTransmitSucceeded && !bError && iNow < iTargetTime)
  {
    ReadFromDevice(50);
    CCECAdapterMessage msg;
    if (!Read(msg, 0))
    {
      iNow = GetTimeMs();
      continue;
    }

    if (msg.Message() == MSGCODE_FRAME_START && msg.IsACK())
    {
      m_processor->HandlePoll(msg.Initiator(), msg.Destination());
      m_lastInitiator = msg.Initiator();
      iNow = GetTimeMs();
      continue;
    }

    if (msg.Message() == MSGCODE_RECEIVE_FAILED &&
        m_lastInitiator != CECDEVICE_UNKNOWN &&
        m_processor->HandleReceiveFailed(m_lastInitiator))
    {
      iNow = GetTimeMs();
      continue;
    }

    bError = msg.IsError();
    if (bError)
    {
      message.reply = msg.Message();
      CLibCEC::AddLog(CEC_LOG_DEBUG, msg.ToString());
    }
    else
    {
      switch(msg.Message())
      {
      case MSGCODE_COMMAND_ACCEPTED:
        if (iPacketsLeft > 0)
          iPacketsLeft--;
        if (!message.isTransmission && iPacketsLeft == 0)
          bTransmitSucceeded = true;
        CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - waiting for %d more", msg.ToString().c_str(), iPacketsLeft);
        break;
      case MSGCODE_TRANSMIT_SUCCEEDED:
        CLibCEC::AddLog(CEC_LOG_DEBUG, msg.ToString());
        bTransmitSucceeded = (iPacketsLeft == 0);
        bError = !bTransmitSucceeded;
        message.reply = MSGCODE_TRANSMIT_SUCCEEDED;
        break;
      default:
        // ignore other data while waiting
        break;
      }

      iNow = GetTimeMs();
    }
  }

  message.state = bTransmitSucceeded && !bError ?
      ADAPTER_MESSAGE_STATE_SENT_ACKED :
      ADAPTER_MESSAGE_STATE_SENT_NOT_ACKED;

  return bTransmitSucceeded && !bError;
}

void CUSBCECAdapterCommunication::AddData(uint8_t *data, size_t iLen)
{
  CLockObject lock(m_mutex);
  for (size_t iPtr = 0; iPtr < iLen; iPtr++)
  {
    if (!m_bGotStart)
    {
      if (data[iPtr] == MSGSTART)
        m_bGotStart = true;
    }
    else if (data[iPtr] == MSGSTART) //we found a msgstart before msgend, this is not right, remove
    {
      if (m_currentAdapterMessage.Size() > 0)
        CLibCEC::AddLog(CEC_LOG_WARNING, "received MSGSTART before MSGEND, removing previous buffer contents");
      m_currentAdapterMessage.Clear();
      m_bGotStart = true;
    }
    else if (data[iPtr] == MSGEND)
    {
      CCECAdapterMessage *newMessage = new CCECAdapterMessage;
      newMessage->packet = m_currentAdapterMessage.packet;
      m_inBuffer.Push(newMessage);
      m_currentAdapterMessage.Clear();
      m_bGotStart = false;
      m_bNextIsEscaped = false;
      m_bHasData = true;
      m_rcvCondition.Broadcast();
    }
    else if (m_bNextIsEscaped)
    {
      m_currentAdapterMessage.PushBack(data[iPtr] + (uint8_t)ESCOFFSET);
      m_bNextIsEscaped = false;
    }
    else if (data[iPtr] == MSGESC)
    {
      m_bNextIsEscaped = true;
    }
    else
    {
      m_currentAdapterMessage.PushBack(data[iPtr]);
    }
  }
}

bool CUSBCECAdapterCommunication::ReadFromDevice(uint32_t iTimeout, size_t iSize /* = 256 */)
{
  ssize_t iBytesRead;
  uint8_t buff[256];
  if (!m_port)
    return false;
  if (iSize > 256)
    iSize = 256;

  CLockObject lock(m_mutex);
  iBytesRead = m_port->Read(buff, sizeof(uint8_t) * iSize, iTimeout);
  if (iBytesRead < 0 || iBytesRead > 256)
  {
    CLibCEC::AddLog(CEC_LOG_ERROR, "error reading from serial port: %s", m_port->GetError().c_str());
    StopThread(false);
    return false;
  }
  else if (iBytesRead > 0)
  {
    AddData(buff, iBytesRead);
  }

  return iBytesRead > 0;
}

void CUSBCECAdapterCommunication::SendMessageToAdapter(CCECAdapterMessage *msg)
{
  CLockObject adapterLock(m_mutex);
  if (!m_port->IsOpen())
  {
    CLibCEC::AddLog(CEC_LOG_ERROR, "error writing to serial port: the connection is closed");
    msg->state = ADAPTER_MESSAGE_STATE_ERROR;
    return;
  }

  if (msg->isTransmission && (msg->Size() < 2 || msg->At(1) != MSGCODE_TRANSMIT_IDLETIME))
  {
    if (msg->tries == 1)
      SetLineTimeout(msg->lineTimeout);
    else
      SetLineTimeout(msg->retryTimeout);
  }

  if (m_port->Write(msg->packet.data, msg->Size()) != (ssize_t) msg->Size())
  {
    CLibCEC::AddLog(CEC_LOG_ERROR, "error writing to serial port: %s", m_port->GetError().c_str());
    msg->state = ADAPTER_MESSAGE_STATE_ERROR;
  }
  else
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "command sent");
    msg->state = ADAPTER_MESSAGE_STATE_SENT;

    if (msg->expectControllerAck)
    {
      if (!WaitForAck(*msg))
        CLibCEC::AddLog(CEC_LOG_DEBUG, "did not receive ack");
    }
  }
  msg->event.Signal();
}

void CUSBCECAdapterCommunication::WriteNextCommand(void)
{
  CCECAdapterMessage *msg(NULL);
  if (m_outBuffer.Pop(msg))
    SendMessageToAdapter(msg);
}

CStdString CUSBCECAdapterCommunication::GetPortName(void)
{
  CStdString strName;
  strName = m_port->GetName();
  return strName;
}

bool CUSBCECAdapterCommunication::SendCommand(cec_adapter_messagecode msgCode, CCECAdapterMessage &params, bool bExpectAck /* = true */, bool bIsTransmission /* = false */, bool bSendDirectly /* = true */, bool bIsRetry /* = false */)
{
  CLockObject lock(m_mutex);

  CCECAdapterMessage *output = new CCECAdapterMessage;

  output->PushBack(MSGSTART);
  output->PushEscaped((uint8_t)msgCode);
  output->Append(params);
  output->PushBack(MSGEND);
  output->isTransmission = bIsTransmission;
  output->expectControllerAck = bExpectAck;

  if (bSendDirectly)
    SendMessageToAdapter(output);
  else
    Write(output);

  bool bWriteOk = output->state == (output->expectControllerAck ? ADAPTER_MESSAGE_STATE_SENT_ACKED : ADAPTER_MESSAGE_STATE_SENT);
  if (!bWriteOk)
  {
    CLibCEC::AddLog(CEC_LOG_ERROR, "'%s' failed", CCECAdapterMessage::ToString(msgCode));
    delete output;

    if (!bIsRetry && output->reply == MSGCODE_COMMAND_REJECTED && msgCode != MSGCODE_SET_CONTROLLED)
    {
      CLibCEC::AddLog(CEC_LOG_DEBUG, "setting controlled mode and retrying");
      if (SetControlledMode(true))
        return SendCommand(msgCode, params, bExpectAck, bIsTransmission, bSendDirectly, true);
    }
    return false;
  }

  delete output;
  return true;
}

cec_datapacket CUSBCECAdapterCommunication::GetSetting(cec_adapter_messagecode msgCode, uint8_t iResponseLength)
{
  cec_datapacket retVal;
  retVal.Clear();

  CCECAdapterMessage params;
  if (!SendCommand(msgCode, params, false))
  {
    CLibCEC::AddLog(CEC_LOG_ERROR, "%s failed", CCECAdapterMessage::ToString(msgCode));
    return retVal;
  }

  Sleep(250); // TODO ReadFromDevice() isn't waiting for the timeout to pass on win32
  ReadFromDevice(CEC_DEFAULT_TRANSMIT_WAIT, iResponseLength + 3 /* start + msgcode + iResponseLength + end */);
  CCECAdapterMessage input;
  if (Read(input, 0))
  {
    if (input.Message() != msgCode)
      CLibCEC::AddLog(CEC_LOG_ERROR, "invalid response to %s received (%s)", CCECAdapterMessage::ToString(msgCode), CCECAdapterMessage::ToString(input.Message()));
    else
    {
      for (uint8_t iPtr = 1; iPtr < input.Size(); iPtr++)
        retVal.PushBack(input[iPtr]);
    }
  }
  else
  {
    CLibCEC::AddLog(CEC_LOG_ERROR, "no response to %s received", CCECAdapterMessage::ToString(msgCode));
  }

  return retVal;
}
