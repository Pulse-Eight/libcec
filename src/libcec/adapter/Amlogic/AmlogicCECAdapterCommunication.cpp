/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC Amlogic Code Copyright (C) 2016 Gerald Dachs
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


#if defined(HAVE_AMLOGIC_API)
#include "AmlogicCEC.h"
#include "AmlogicCECAdapterCommunication.h"

#include "CECTypeUtils.h"
#include "LibCEC.h"
#include <p8-platform/util/buffer.h>

using namespace CEC;
using namespace P8PLATFORM;

#define LIB_CEC m_callback->GetLib()


CAmlogicCECAdapterCommunication::CAmlogicCECAdapterCommunication(IAdapterCommunicationCallback *callback) :
    IAdapterCommunication(callback),
    m_bLogicalAddressChanged(false)
{
  CLockObject lock(m_mutex);

  m_logicalAddresses.Clear();
  m_fd = INVALID_SOCKET_VALUE;
}

CAmlogicCECAdapterCommunication::~CAmlogicCECAdapterCommunication(void)
{
  Close();
}

bool CAmlogicCECAdapterCommunication::IsOpen(void)
{
  CLockObject lock(m_mutex);
  return IsInitialised() && m_fd != INVALID_SOCKET_VALUE;
}

bool CAmlogicCECAdapterCommunication::Open(uint32_t UNUSED(iTimeoutMs), bool UNUSED(bSkipChecks), bool bStartListening)
{
  CLockObject lock(m_mutex);

  if (IsOpen())
    Close();

  if ((m_fd = open(CEC_AMLOGIC_PATH, O_RDWR)) > 0)
  {
    if (!bStartListening || CreateThread()) {
        return true;
    }
    close(m_fd);
    m_fd = INVALID_SOCKET_VALUE;
  }
  return false;
}

void CAmlogicCECAdapterCommunication::Close(void)
{
  StopThread(0);

  CLockObject lock(m_mutex);

  close(m_fd);
  m_fd = INVALID_SOCKET_VALUE;
}

std::string CAmlogicCECAdapterCommunication::GetError(void) const
{
  std::string strError(m_strError);
  return strError;
}

cec_adapter_message_state CAmlogicCECAdapterCommunication::Write(
  const cec_command &data, bool &UNUSED(bRetry), uint8_t UNUSED(iLineTimeout), bool UNUSED(bIsReply))
{
  uint8_t buffer[CEC_MAX_FRAME_SIZE];
  int32_t size = 1;
  cec_adapter_message_state rc = ADAPTER_MESSAGE_STATE_SENT_NOT_ACKED;

  CLockObject lock(m_mutex);

  if (!IsOpen())
    return rc;

  if ((size_t)data.parameters.size + data.opcode_set > sizeof(buffer))
  {
    LIB_CEC->AddLog(CEC_LOG_WARNING, "%s: buffer too small for data", __func__);
    return ADAPTER_MESSAGE_STATE_ERROR;
  }

  buffer[0] = (data.initiator << 4) | (data.destination & 0x0f);

  if (data.opcode_set)
  {
    buffer[1] = data.opcode;
    size++;

    memcpy(&buffer[size], data.parameters.data, data.parameters.size);
    size += data.parameters.size;
  }

  if (write(m_fd, (void *)buffer, size) == size)
  {
    rc = ADAPTER_MESSAGE_STATE_SENT_ACKED;
  }
  else
  {
    LIB_CEC->AddLog(CEC_LOG_WARNING, "%s: write failed",  __func__);
  }

  return rc;
}

uint16_t CAmlogicCECAdapterCommunication::GetFirmwareVersion(void)
{
  return 0;
}

cec_vendor_id CAmlogicCECAdapterCommunication::GetVendorId(void)
{
  return cec_vendor_id(CEC_VENDOR_UNKNOWN);
}

uint16_t CAmlogicCECAdapterCommunication::GetPhysicalAddress(void)
{
  int phys_addr = CEC_INVALID_PHYSICAL_ADDRESS;

  CLockObject lock(m_mutex);

  if (!IsOpen())
    return (uint16_t)phys_addr;

  if ((phys_addr = ioctl(m_fd, CEC_IOC_GETPADDR)) < 0)
  {
    LIB_CEC->AddLog(CEC_LOG_WARNING, "%s: ioctl(CEC_IOC_GETPADDR) failed", __func__);
    phys_addr = CEC_INVALID_PHYSICAL_ADDRESS;
  }
  return (uint16_t)phys_addr;
}

cec_logical_addresses CAmlogicCECAdapterCommunication::GetLogicalAddresses(void)
{
  CLockObject lock(m_mutex);
  return m_logicalAddresses;
}

bool CAmlogicCECAdapterCommunication::SetLogicalAddresses(const cec_logical_addresses &addresses)
{
  CLockObject lock(m_mutex);

  unsigned int log_addr = addresses.primary;

  if (!IsOpen())
    return false;

  if (ioctl(m_fd, CEC_IOC_SETLADDR, &log_addr))
  {
    LIB_CEC->AddLog(CEC_LOG_WARNING, "%s: ioctl(CEC_IOC_SETLADDR) failed", __func__);
    return false;
  }
  m_logicalAddresses = addresses;
  m_bLogicalAddressChanged = true;

  return true;
}

void CAmlogicCECAdapterCommunication::HandleLogicalAddressLost(cec_logical_address UNUSED(oldAddress))
{
  unsigned int log_addr = CECDEVICE_BROADCAST;

  CLockObject lock(m_mutex);

  if (!IsOpen())
    return;

  if (ioctl(m_fd, CEC_IOC_SETLADDR, &log_addr))
  {
    LIB_CEC->AddLog(CEC_LOG_WARNING, "%s: ioctl(CEC_IOC_SETLADDR) failed", __func__);
  }
}

void *CAmlogicCECAdapterCommunication::Process(void)
{
  uint8_t buffer[CEC_MAX_FRAME_SIZE];
  uint32_t size;
  fd_set rfds;
  cec_logical_address initiator, destination;
  struct timeval tv;

  if (!IsOpen())
    return 0;

  while (!IsStopped())
  {
    if (m_fd == INVALID_SOCKET_VALUE)
    {
      Sleep(250);
      continue;
    }

    FD_ZERO(&rfds);
    FD_SET(m_fd, &rfds);

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    if (select(m_fd + 1, &rfds, NULL, NULL, &tv) >= 0 )
    {

      if (!FD_ISSET(m_fd, &rfds))
	  continue;

      size = read(m_fd, buffer, CEC_MAX_FRAME_SIZE);

      if (size > 0)
      {
          if (buffer[0] == 0xff) // driver wants us to reread the physical address
          {
              if (!IsStopped())
              {
                  uint16_t iNewAddress = GetPhysicalAddress();
                  m_callback->HandlePhysicalAddressChanged(iNewAddress);
              }
              continue;
          }

          initiator = cec_logical_address(buffer[0] >> 4);
          destination = cec_logical_address(buffer[0] & 0x0f);

          cec_command cmd;

          cec_command::Format(
            cmd, initiator, destination,
            ( size > 1 ) ? cec_opcode(buffer[1]) : CEC_OPCODE_NONE);

          for( uint8_t i = 2; i < size; i++ )
            cmd.parameters.PushBack(buffer[i]);

          if (!IsStopped())
            m_callback->OnCommandReceived(cmd);
      }
    }
  }

  return 0;
}

#endif	// HAVE_AMLOGIC_API
