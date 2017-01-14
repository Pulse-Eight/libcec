/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC RainShadow Code Copyright (C) 2017 Gerald Dachs
 * based heavily on:
 * libCEC Exynos Code Copyright (C) 2014 Valentin Manea
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
#include <fcntl.h>
#include <sys/ioctl.h>

#if defined(HAVE_RAINSHADOW_API)
#include "Rain.h"
#include "RainAdapterCommunication.h"
#include "CECTypeUtils.h"
#include "LibCEC.h"
#include "platform/sockets/serialport.h"
#include <p8-platform/util/buffer.h>
#include <p8-platform/util/util.h>
#include "platform/util/edid.h"
#include "platform/drm/drm-edid.h"

using namespace CEC;
using namespace P8PLATFORM;

#define LIB_CEC m_callback->GetLib()

CRainAdapterCommunication::CRainAdapterCommunication(IAdapterCommunicationCallback *callback, const char *strPort, uint32_t iBaudRate /* = CEC_RAINSHADOW_SERIAL_DEFAULT_BAUDRATE */) :
    IAdapterCommunication(callback),
    m_port(NULL),
    m_gotResponse(false),
    m_bLogicalAddressChanged(false),
    m_osdNameRequestState(OSD_NAME_REQUEST_NEEDED)
{ 
  CLockObject lock(m_mutex);
  m_logicalAddresses.Clear();
  m_port = new CSerialPort(strPort, iBaudRate);
}

CRainAdapterCommunication::~CRainAdapterCommunication(void)
{
  Close();
  SAFE_DELETE(m_port);
}

bool CRainAdapterCommunication::IsOpen(void)
{
  /* thread is not being stopped, the port is open and the thread is running */
  return !IsStopped() && m_port->IsOpen() && IsRunning();
}

bool CRainAdapterCommunication::Open(uint32_t iTimeoutMs /* = CEC_DEFAULT_CONNECT_TIMEOUT */, bool UNUSED(bSkipChecks) /* = false */, bool bStartListening /* = true */)
{
  bool bConnectionOpened(false);
  CLockObject lock(m_mutex);

  /* we need the port settings here */
  if (!m_port)
  {
    LIB_CEC->AddLog(CEC_LOG_ERROR, "port is NULL");
    return bConnectionOpened;
  }

  /* return true when the port is already open */
  if (IsOpen())
  {
    LIB_CEC->AddLog(CEC_LOG_WARNING, "port is already open");
    return true;
  }

  /* try to open the connection */
  std::string strError;
  CTimeout timeout(iTimeoutMs);
  while (!bConnectionOpened && timeout.TimeLeft() > 0)
  {
    if ((bConnectionOpened = m_port->Open(timeout.TimeLeft())) == false)
    {
      strError = StringUtils::Format("error opening serial port '%s': %s", m_port->GetName().c_str(), m_port->GetError().c_str());
      Sleep(250);
    }
    /* and retry every 250ms until the timeout passed */
  }

  /* return false when we couldn't connect */
  if (!bConnectionOpened)
  {
    LIB_CEC->AddLog(CEC_LOG_ERROR, strError.c_str());

    if (m_port->GetErrorNumber() == EACCES)
    {
      libcec_parameter param;
      param.paramType = CEC_PARAMETER_TYPE_STRING;
      param.paramData = (void*)"No permission to open the device";
      LIB_CEC->Alert(CEC_ALERT_PERMISSION_ERROR, param);
    }
    else if (m_port->GetErrorNumber() == EBUSY)
    {
      libcec_parameter param;
      param.paramType = CEC_PARAMETER_TYPE_STRING;
      param.paramData = (void*)"The serial port is busy. Only one program can access the device directly.";
      LIB_CEC->Alert(CEC_ALERT_PORT_BUSY, param);
    }
    return false;
  }

  LIB_CEC->AddLog(CEC_LOG_DEBUG, "connection opened, clearing any previous input and waiting for active transmissions to end before starting");

  m_response[0] = '\0';
  m_gotResponse = true;
  m_condition.Signal();

  // always start by setting the ackmask to 0, to clear previous values
  cec_logical_addresses addresses; addresses.Clear();
  SetLogicalAddresses(addresses);

  if (!CreateThread())
  {
    bConnectionOpened = false;
    LIB_CEC->AddLog(CEC_LOG_ERROR, "could not create a communication thread");
  }

  if (!bConnectionOpened || !bStartListening)
    StopThread(0);

  lock.Unlock();

  while(!SetAdapterPhysicalAddress());

  SetAdapterConfigurationBits();

  return bConnectionOpened;
}

void CRainAdapterCommunication::Close(void)
{
  /* stop the reader thread */
  StopThread(0);

  CLockObject lock(m_mutex);

  /* set the ackmask to 0 before closing the connection */
  if (IsOpen() && m_port->GetErrorNumber() == 0)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - closing the connection", __FUNCTION__);
    cec_logical_addresses addresses; addresses.Clear();
    SetLogicalAddresses(addresses);
  }

  /* close and delete the com port connection */
  if (m_port)
    m_port->Close();
}

/**
 * hex_to_bin - convert a hex digit to its real value
 * @ch: ascii character represents hex digit
 *
 * hex_to_bin() converts one hex digit to its actual value or -1 in case of bad
 * input.
 */
int CRainAdapterCommunication::hex_to_bin(char ch)
{
    if ((ch >= '0') && (ch <= '9'))
        return ch - '0';
    ch = tolower(ch);
    if ((ch >= 'a') && (ch <= 'f'))
        return ch - 'a' + 10;
    return -1;
}

/**
 * hex2bin - convert an ascii hexadecimal string to its binary representation
 * @dst: binary result
 * @src: ascii hexadecimal string
 * @count: result length
 *
 * Return 0 on success, -1 in case of bad input.
 */
int CRainAdapterCommunication::hex2bin(uint8_t *dst, const char *src, size_t count)
{
    while (count--)
    {
        int hi = hex_to_bin(*src++);
        int lo = hex_to_bin(*src++);

        if ((hi < 0) || (lo < 0))
            return -1;

        *dst++ = (hi << 4) | lo;
    }
    return 0;
}

bool CRainAdapterCommunication::SetAdapterPhysicalAddress()
{
  char command[DATA_SIZE];

  CLockObject lock(m_mutex);

  if (!IsOpen())
    return false;

  snprintf(command, sizeof(command), "!P %04x~", GetPhysicalAddress());

  return WriteAdapterCommand(command, "PHY");
}

bool CRainAdapterCommunication::SetAdapterConfigurationBits()
{
  char command[DATA_SIZE];
  uint16_t adapterConfigurationBits = 5;  // Higher level functions and Host wakeup
  CLockObject lock(m_mutex);

  if (!IsOpen())
    return false;

  snprintf(command, sizeof(command), "!C %04x~", adapterConfigurationBits);

  return WriteAdapterCommand(command, "CFG");
}

bool CRainAdapterCommunication::SetAdapterOsdName(const cec_datapacket &packet)
{
  char command[DATA_SIZE] = "!O";
  char *cmd_ptr = command + strlen(command);

  CLockObject lock(m_mutex);

  if (!IsOpen())
    return false;

  for (int i = 0; i < packet.size; ++i)
  {
    *cmd_ptr++ = packet.At(i);
  }

  *cmd_ptr++ = '~';
  *cmd_ptr++ = '\0';

  return WriteAdapterCommand(command, "OSD");
}

bool CRainAdapterCommunication::WriteAdapterCommand(char *command,
    const char *response)
{
  bool ret;

  LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - Write command to adapter: %s", __FUNCTION__, command);

  if (m_port->Write(command, strlen(command)) != (ssize_t) strlen(command))
  {
    return false;
  }

  m_condition.Wait(m_mutex, m_gotResponse);
  m_gotResponse = false;

  ret = !strncmp(m_response, response, strlen(response));

  LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - Response for command %s %s received", __FUNCTION__, command, ret ? "": "not ");
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - Response was %s", __FUNCTION__, m_response);

  return ret;
}

std::string CRainAdapterCommunication::GetError(void) const
{
  return m_port->GetError();
}

cec_adapter_message_state CRainAdapterCommunication::Write(
    const cec_command &data, bool &UNUSED(bRetry), uint8_t UNUSED(iLineTimeout),
    bool UNUSED(bIsReply))
{
  char buffer[DATA_SIZE];
  CLockObject lock(m_mutex);

  if (!IsOpen())
    return ADAPTER_MESSAGE_STATE_SENT_NOT_ACKED;

  if (!data.opcode_set)
  {
    snprintf(buffer, sizeof(buffer), "!X%x~", data.destination);
  }
  else if (m_osdNameRequestState == OSD_NAME_REQUEST_SENT
      && data.initiator == m_logicalAddresses.primary
      && data.destination == CECDEVICE_BROADCAST
      && data.opcode == CEC_OPCODE_SET_OSD_NAME)
  {
    SetAdapterOsdName(data.parameters);

    m_osdNameRequestState = OSD_NAME_REQUEST_DONE;

    return ADAPTER_MESSAGE_STATE_SENT_ACKED;
  }
  else
  {
    char hex[4];

    snprintf(buffer, sizeof(buffer), "!X%x %02x ", data.destination,
        data.opcode);
    for (int i = 0; i < data.parameters.size; i++)
    {
      snprintf(hex, sizeof(hex), "%02x ", data.parameters[i]);
      strncat(buffer, hex, sizeof(buffer) - 1);
    }
    buffer[strlen(buffer) - 1] = '~';
  }

  LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - Write message to adapter: %s", __FUNCTION__, buffer);

  if (m_port->Write(buffer, strlen(buffer)) == (ssize_t) strlen(buffer))
  {
    m_condition.Wait(m_mutex, m_gotResponse);
    m_gotResponse = false;

    LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - got response %s for message %s", __FUNCTION__, m_response, buffer);

    if (!strncmp(m_response, "STA", 3) && m_response[strlen(m_response) - 1] == '1')
    {
      return ADAPTER_MESSAGE_STATE_SENT_ACKED;
    }
  }
  return ADAPTER_MESSAGE_STATE_SENT_NOT_ACKED;
}

uint16_t CRainAdapterCommunication::GetFirmwareVersion(void)
{
  return 0;
}

cec_vendor_id CRainAdapterCommunication::GetVendorId(void)
{
  return cec_vendor_id(CEC_VENDOR_UNKNOWN);
}

std::string CRainAdapterCommunication::GetPortName(void)
{
  return m_port->GetName();
}

uint16_t CRainAdapterCommunication::GetPhysicalAddress(void)
{
  uint16_t iPA(0);

  // try to get the PA from ADL
#if defined(HAS_ADL_EDID_PARSER)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - trying to get the physical address via ADL", __FUNCTION__);
    CADLEdidParser adl;
    iPA = adl.GetPhysicalAddress();
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - ADL returned physical address %04x", __FUNCTION__, iPA);
  }
#endif

  // try to get the PA from the nvidia driver
#if defined(HAS_NVIDIA_EDID_PARSER)
  if (iPA == 0)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - trying to get the physical address via nvidia driver", __FUNCTION__);
    CNVEdidParser nv;
    iPA = nv.GetPhysicalAddress();
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - nvidia driver returned physical address %04x", __FUNCTION__, iPA);
  }
#endif

// try to get the PA from the intel driver
#if defined(HAVE_DRM_EDID_PARSER)
  if (iPA == 0)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - trying to get the physical address via drm files", __FUNCTION__);
    CDRMEdidParser drm;
    iPA = drm.GetPhysicalAddress();
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - drm files returned physical address %04x", __FUNCTION__, iPA);
  }
#endif

  // try to get the PA from the OS
  if (iPA == 0)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - trying to get the physical address from the OS", __FUNCTION__);
    iPA = CEDIDParser::GetPhysicalAddress();
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - OS returned physical address %04x", __FUNCTION__, iPA);
  }

  return iPA;
}

cec_logical_addresses CRainAdapterCommunication::GetLogicalAddresses(void)
{
  CLockObject lock(m_mutex);
  return m_logicalAddresses;
}

bool CRainAdapterCommunication::InternalSetLogicalAddresses(const unsigned int log_addr)
{
  char command[DATA_SIZE];

  CLockObject lock(m_mutex);

  if (!IsOpen())
    return false;

  snprintf(command, sizeof(command), "!A %x~", log_addr);

  return WriteAdapterCommand(command, "ADR");
}

bool CRainAdapterCommunication::SetLogicalAddresses(const cec_logical_addresses &addresses)
{
  if (InternalSetLogicalAddresses(addresses.primary))
  {
    m_logicalAddresses = addresses;
    m_bLogicalAddressChanged = true;

    return true;
  }
  return false;
}

void CRainAdapterCommunication::HandleLogicalAddressLost(cec_logical_address UNUSED(oldAddress))
{
  InternalSetLogicalAddresses(CECDEVICE_BROADCAST);
}

void CRainAdapterCommunication::ProcessMessage(char *buffer)
{
  unsigned char msg[CEC_MAX_FRAME_SIZE];
  unsigned int len = 0;
  int stat = -1;

  for (char *data = buffer + 3; *data; data++)
  {
    if (!isxdigit(*data))
      continue;
    if (isxdigit(data[0]) && isxdigit(data[1]))
    {
      if (len == sizeof(msg))
        break;
      if (hex2bin(msg + len, data, 1))
        continue;
      len++;
      data++;
      continue;
    }
    if (!data[1])
      stat = hex_to_bin(data[0]);
    break;
  }

  if (len > 0 && (stat == 1 || stat == 2))
  {
    cec_command cmd;
    cec_logical_address initiator, destination;

    initiator = cec_logical_address(msg[0] >> 4);
    destination = cec_logical_address(msg[0] & 0x0f);

    cec_command::Format(cmd, initiator, destination,
        (len > 1) ? cec_opcode(msg[1]) : CEC_OPCODE_NONE);

    for (uint8_t i = 2; i < len; i++)
      cmd.parameters.PushBack(msg[i]);

    if (!IsStopped())
    {
      m_callback->OnCommandReceived(cmd);
    }
  }
}

void *CRainAdapterCommunication::Process(void)
{
  char buf[DATA_SIZE];
  unsigned int idx = 0;
  bool started = false;
  unsigned char data;

  if (!IsOpen())
    return 0;

  while (!IsStopped())
  {
    if (m_osdNameRequestState == OSD_NAME_REQUEST_NEEDED
        && m_logicalAddresses.primary != CECDEVICE_BROADCAST)
    {
      cec_command cmd;

      cec_command::Format(cmd, CECDEVICE_BROADCAST, m_logicalAddresses.primary,
          CEC_OPCODE_GIVE_OSD_NAME);

      m_callback->OnCommandReceived(cmd);
      m_osdNameRequestState = OSD_NAME_REQUEST_SENT;

      LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - osd name request sent", __FUNCTION__);
    }

    do
    {
      /* retry Read() if it was interrupted */
      m_port->Read(&data, sizeof(uint8_t), 50);
    } while (m_port->GetErrorNumber() == EINTR);

    if (m_port->GetErrorNumber())
    {
      libcec_parameter param;
      param.paramType = CEC_PARAMETER_TYPE_STRING;
      param.paramData = (void*)"No permission to open the device";
      LIB_CEC->Alert(CEC_ALERT_CONNECTION_LOST, param);

      break;
    }

    if (!started && data != '?')
      continue;
    if (data == '\r')
    {
      buf[idx] = '\0';

      if (!strncmp(buf, "REC", 3))
        ProcessMessage(buf);
      else
      {
        strncpy(m_response, buf, sizeof(m_response));

        m_gotResponse = true;
        m_condition.Signal();
      }
      idx = 0;
      started = false;
      continue;
    }
    else if (data == '?')
    {
      idx = 0;
      started = true;
      continue;
    }

    if (data == '\n')
    {
      idx = 0;
      started = false;
      continue;
    }
    if (idx >= DATA_SIZE - 1)
    {
      idx = 0;
    }
    buf[idx++] = data;
    continue;
  }

  return 0;
}

#endif	// HAVE_RAINSHADOW_API
