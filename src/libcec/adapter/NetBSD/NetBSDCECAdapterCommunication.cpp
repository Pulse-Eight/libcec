/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC NetBSD Code is Copyright (C) 2015 Jared McNeill, based on
 * libCEC Exynos Code is Copyright (C) 2014 Valentin Manea
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


#if defined(HAVE_NETBSD_API)
#include "NetBSDCECAdapterCommunication.h"

#include "CECTypeUtils.h"
#include "LibCEC.h"
#include "platform/util/buffer.h"

#include <dev/hdmicec/hdmicecio.h>

#define CEC_MAX_FRAME_SIZE	16
#define CEC_DEFAULT_PADDR	0x1000

using namespace CEC;
using namespace PLATFORM;

#define LIB_CEC m_callback->GetLib()


CNetBSDCECAdapterCommunication::CNetBSDCECAdapterCommunication(IAdapterCommunicationCallback *callback) :
    IAdapterCommunication(callback),
    m_bLogicalAddressChanged(false)
{ 
  CLockObject lock(m_mutex);

  m_logicalAddresses.Clear();
  m_fd = INVALID_SOCKET_VALUE;
}


CNetBSDCECAdapterCommunication::~CNetBSDCECAdapterCommunication(void)
{
  Close();
}


bool CNetBSDCECAdapterCommunication::IsOpen(void)
{
  return IsInitialised() && m_fd != INVALID_SOCKET_VALUE;
}


bool CNetBSDCECAdapterCommunication::Open(uint32_t UNUSED(iTimeoutMs), bool UNUSED(bSkipChecks), bool bStartListening)
{
  if (m_fd != INVALID_SOCKET_VALUE)
    close(m_fd);

  if ((m_fd = open(CEC_NETBSD_PATH, O_RDWR)) > 0)
  {
    if (!bStartListening || CreateThread()) {
        return true;
    }
    close(m_fd);
  }
  return false;
}


void CNetBSDCECAdapterCommunication::Close(void)
{
  StopThread(0);

  close(m_fd);
  m_fd = INVALID_SOCKET_VALUE;
}


std::string CNetBSDCECAdapterCommunication::GetError(void) const
{
  std::string strError(m_strError);
  return strError;
}


cec_adapter_message_state CNetBSDCECAdapterCommunication::Write(
  const cec_command &data, bool &UNUSED(bRetry), uint8_t UNUSED(iLineTimeout), bool UNUSED(bIsReply))
{
  uint8_t buffer[CEC_MAX_FRAME_SIZE];
  int32_t size = 1;
  cec_adapter_message_state rc = ADAPTER_MESSAGE_STATE_ERROR;

  if (!IsOpen())
    return rc;

  if ((size_t)data.parameters.size + data.opcode_set > sizeof(buffer))
  {
    LIB_CEC->AddLog(CEC_LOG_ERROR, "%s: data size too large !", __func__);
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
  else if (errno == ECONNREFUSED)
  {
    rc = ADAPTER_MESSAGE_STATE_SENT_NOT_ACKED;
  }
  else
  {
    LIB_CEC->AddLog(CEC_LOG_ERROR, "%s: write failed !", __func__);
  }

  return rc;
}


uint16_t CNetBSDCECAdapterCommunication::GetFirmwareVersion(void)
{
  return 0;
}


cec_vendor_id CNetBSDCECAdapterCommunication::GetVendorId(void)
{
  uint16_t vendor_id;

  if (ioctl(m_fd, CEC_GET_VENDOR_ID, &vendor_id) == -1) {
    LIB_CEC->AddLog(CEC_LOG_ERROR, "%s: CEC_GET_VENDOR_ID failed !", __func__);
    return cec_vendor_id(CEC_VENDOR_UNKNOWN);
  }

  return cec_vendor_id(vendor_id);
}


uint16_t CNetBSDCECAdapterCommunication::GetPhysicalAddress(void)
{
  uint16_t phys_addr;

  if (ioctl(m_fd, CEC_GET_PHYS_ADDR, &phys_addr) == -1) {
    LIB_CEC->AddLog(CEC_LOG_ERROR, "%s: CEC_GET_PHYS_ADDR failed !", __func__);
    phys_addr = CEC_DEFAULT_PADDR;
  }

  return phys_addr;
}


cec_logical_addresses CNetBSDCECAdapterCommunication::GetLogicalAddresses(void)
{
  uint16_t logical_addresses;
  int la;

  CLockObject lock(m_mutex);

  if (m_bLogicalAddressChanged || m_logicalAddresses.IsEmpty()) {
    m_logicalAddresses.Clear();
    if (ioctl(m_fd, CEC_GET_LOG_ADDRS, &logical_addresses) == -1) {
      LIB_CEC->AddLog(CEC_LOG_ERROR, "%s: CEC_GET_LOG_ADDRS failed !", __func__);
    } else {
      while ((la = ffs(logical_addresses)) > 0) {
        la--;
        m_logicalAddresses.Set(cec_logical_address(la));
        logical_addresses &= ~(1 << la);
      }
    }
    m_bLogicalAddressChanged = false;
  }

  return m_logicalAddresses;
}


bool CNetBSDCECAdapterCommunication::SetLogicalAddresses(const cec_logical_addresses &addresses)
{
  uint16_t logical_addresses = addresses.AckMask();

  CLockObject lock(m_mutex);

  if (!IsOpen())
    return false;

  if (ioctl(m_fd, CEC_SET_LOG_ADDRS, &logical_addresses))
  {
    LIB_CEC->AddLog(CEC_LOG_ERROR, "%s: CEC_SET_LOG_ADDRS failed !", __func__);
    return false;
  }
  m_logicalAddresses = addresses;
  m_bLogicalAddressChanged = true;

  return true;
}


void CNetBSDCECAdapterCommunication::HandleLogicalAddressLost(cec_logical_address UNUSED(oldAddress))
{
  uint16_t logical_addresses = 1 << CECDEVICE_BROADCAST;
  if (ioctl(m_fd, CEC_SET_LOG_ADDRS, &logical_addresses))
  {
    LIB_CEC->AddLog(CEC_LOG_ERROR, "%s: CEC_SET_LOG_ADDRS failed !", __func__);
  }
}


void *CNetBSDCECAdapterCommunication::Process(void)
{
  uint8_t buffer[CEC_MAX_FRAME_SIZE];
  uint32_t size;
  fd_set rfds;
  cec_logical_address initiator, destination;

  if (!IsOpen())
    return 0;

  FD_ZERO(&rfds);
  FD_SET(m_fd, &rfds);

  while (!IsStopped())
  {
    if (select(m_fd + 1, &rfds, NULL, NULL, NULL) >= 0 )
    {
      size = read(m_fd, buffer, CEC_MAX_FRAME_SIZE);

      if (size > 0)
      {
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

#endif	// HAVE_NETBSD_API
