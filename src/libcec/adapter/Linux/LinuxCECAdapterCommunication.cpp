/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC Linux CEC Adapter is Copyright (C) 2017-2019 Jonas Karlman
 * based heavily on:
 * libCEC AOCEC Code is Copyright (C) 2016 Gerald Dachs
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

#if defined(HAVE_LINUX_API)
#include "LinuxCECAdapterCommunication.h"
#include "CECTypeUtils.h"
#include "LibCEC.h"
#include "p8-platform/util/buffer.h"
#include <linux/cec.h>

using namespace CEC;
using namespace P8PLATFORM;

#define LIB_CEC m_callback->GetLib()

// Required capabilities
#define CEC_LINUX_CAPABILITIES (CEC_CAP_LOG_ADDRS | CEC_CAP_TRANSMIT | CEC_CAP_PASSTHROUGH)

CLinuxCECAdapterCommunication::CLinuxCECAdapterCommunication(IAdapterCommunicationCallback *callback)
  : IAdapterCommunication(callback)
{
  m_fd = INVALID_SOCKET_VALUE;
}

CLinuxCECAdapterCommunication::~CLinuxCECAdapterCommunication(void)
{
  Close();
}

bool CLinuxCECAdapterCommunication::Open(uint32_t UNUSED(iTimeoutMs), bool UNUSED(bSkipChecks), bool bStartListening)
{
  if (IsOpen())
    Close();

  if ((m_fd = open(CEC_LINUX_PATH, O_RDWR)) >= 0)
  {
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "CLinuxCECAdapterCommunication::Open - m_fd=%d bStartListening=%d", m_fd, bStartListening);

    // Ensure the CEC device supports required capabilities
    struct cec_caps caps = {};
    if (ioctl(m_fd, CEC_ADAP_G_CAPS, &caps) || (caps.capabilities & CEC_LINUX_CAPABILITIES) != CEC_LINUX_CAPABILITIES)
    {
      LIB_CEC->AddLog(CEC_LOG_ERROR, "CLinuxCECAdapterCommunication::Open - ioctl CEC_ADAP_G_CAPS failed - capabilities=%02x errno=%d", caps.capabilities, errno);
      Close();
      return false;
    }

    if (!bStartListening)
    {
      Close();
      return true;
    }

    // This is an exclusive follower, in addition put the CEC device into passthrough mode
    uint32_t mode = CEC_MODE_INITIATOR | CEC_MODE_EXCL_FOLLOWER_PASSTHRU;
    if (ioctl(m_fd, CEC_S_MODE, &mode))
    {
      LIB_CEC->AddLog(CEC_LOG_ERROR, "CLinuxCECAdapterCommunication::Open - ioctl CEC_S_MODE failed - errno=%d", errno);
      Close();
      return false;
    }

    uint16_t addr;
    if (ioctl(m_fd, CEC_ADAP_G_PHYS_ADDR, &addr))
    {
      LIB_CEC->AddLog(CEC_LOG_ERROR, "CLinuxCECAdapterCommunication::Open - ioctl CEC_ADAP_G_PHYS_ADDR failed - errno=%d", errno);
      Close();
      return false;
    }

    LIB_CEC->AddLog(CEC_LOG_DEBUG, "CLinuxCECAdapterCommunication::Open - ioctl CEC_ADAP_G_PHYS_ADDR - addr=%04x", addr);

    if (addr == CEC_PHYS_ADDR_INVALID)
      LIB_CEC->AddLog(CEC_LOG_WARNING, "CLinuxCECAdapterCommunication::Open - physical address is invalid");

    // Clear existing logical addresses and set the CEC device to the unconfigured state
    struct cec_log_addrs log_addrs = {};
    if (ioctl(m_fd, CEC_ADAP_S_LOG_ADDRS, &log_addrs))
    {
      LIB_CEC->AddLog(CEC_LOG_ERROR, "CLinuxCECAdapterCommunication::Open - ioctl CEC_ADAP_S_LOG_ADDRS failed - errno=%d", errno);
      Close();
      return false;
    }

    LIB_CEC->AddLog(CEC_LOG_DEBUG, "CLinuxCECAdapterCommunication::Open - ioctl CEC_ADAP_S_LOG_ADDRS - log_addr_mask=%04x num_log_addrs=%u", log_addrs.log_addr_mask, log_addrs.num_log_addrs);

    // Set logical address to unregistered, without any logical address configured no messages is transmitted or received
    log_addrs = {};
    log_addrs.cec_version = CEC_OP_CEC_VERSION_1_4;
    log_addrs.vendor_id = CEC_VENDOR_PULSE_EIGHT;
    log_addrs.num_log_addrs = 1;
    log_addrs.flags = CEC_LOG_ADDRS_FL_ALLOW_UNREG_FALLBACK;
    log_addrs.log_addr[0] = CEC_LOG_ADDR_UNREGISTERED;
    log_addrs.primary_device_type[0] = CEC_OP_PRIM_DEVTYPE_SWITCH;
    log_addrs.log_addr_type[0] = CEC_LOG_ADDR_TYPE_UNREGISTERED;
    log_addrs.all_device_types[0] = CEC_OP_ALL_DEVTYPE_SWITCH;
    if (ioctl(m_fd, CEC_ADAP_S_LOG_ADDRS, &log_addrs))
    {
      LIB_CEC->AddLog(CEC_LOG_ERROR, "CLinuxCECAdapterCommunication::Open - ioctl CEC_ADAP_S_LOG_ADDRS failed - errno=%d", errno);
      Close();
      return false;
    }

    LIB_CEC->AddLog(CEC_LOG_DEBUG, "CLinuxCECAdapterCommunication::Open - ioctl CEC_ADAP_S_LOG_ADDRS - log_addr_mask=%04x num_log_addrs=%u", log_addrs.log_addr_mask, log_addrs.num_log_addrs);

    if (CreateThread())
      return true;

    Close();
  }

  return false;
}

void CLinuxCECAdapterCommunication::Close(void)
{
  StopThread(0);

  LIB_CEC->AddLog(CEC_LOG_DEBUG, "CLinuxCECAdapterCommunication::Close - m_fd=%d", m_fd);

  close(m_fd);
  m_fd = INVALID_SOCKET_VALUE;
}

bool CLinuxCECAdapterCommunication::IsOpen(void)
{
  return m_fd != INVALID_SOCKET_VALUE;
}

cec_adapter_message_state CLinuxCECAdapterCommunication::Write(const cec_command &data, bool &bRetry, uint8_t UNUSED(iLineTimeout), bool UNUSED(bIsReply))
{
  if (IsOpen())
  {
    struct cec_msg msg;
    cec_msg_init(&msg, data.initiator, data.destination);

    if (data.opcode_set)
    {
      msg.msg[msg.len++] = data.opcode;

      if (data.parameters.size)
      {
        memcpy(&msg.msg[msg.len], data.parameters.data, data.parameters.size);
        msg.len += data.parameters.size;
      }
    }

    if (ioctl(m_fd, CEC_TRANSMIT, &msg))
    {
      LIB_CEC->AddLog(CEC_LOG_ERROR, "CLinuxCECAdapterCommunication::Write - ioctl CEC_TRANSMIT failed - tx_status=%02x errno=%d", msg.tx_status, errno);
      return ADAPTER_MESSAGE_STATE_ERROR;
    }

    LIB_CEC->AddLog(CEC_LOG_DEBUG, "CLinuxCECAdapterCommunication::Write - ioctl CEC_TRANSMIT - tx_status=%02x len=%d addr=%02x opcode=%02x", msg.tx_status, msg.len, msg.msg[0], cec_msg_opcode(&msg));

    // The CEC driver will make re-transmission attempts
    bRetry = false;

    if (msg.tx_status & CEC_TX_STATUS_OK)
      return ADAPTER_MESSAGE_STATE_SENT_ACKED;

    if (msg.tx_status & CEC_TX_STATUS_NACK)
      return ADAPTER_MESSAGE_STATE_SENT_NOT_ACKED;

    return ADAPTER_MESSAGE_STATE_ERROR;
  }

  return ADAPTER_MESSAGE_STATE_UNKNOWN;
}

bool CLinuxCECAdapterCommunication::SetLogicalAddresses(const cec_logical_addresses &addresses)
{
  if (IsOpen())
  {
    struct cec_log_addrs log_addrs = {};
    if (ioctl(m_fd, CEC_ADAP_G_LOG_ADDRS, &log_addrs))
    {
      LIB_CEC->AddLog(CEC_LOG_ERROR, "CLinuxCECAdapterCommunication::SetLogicalAddresses - ioctl CEC_ADAP_G_LOG_ADDRS failed - errno=%d", errno);
      return false;
    }

    // TODO: Claiming a logical address will only work when CEC device has a valid physical address

    // Clear existing logical addresses and set the CEC device to the unconfigured state
    if (log_addrs.num_log_addrs)
    {
      log_addrs = {};
      if (ioctl(m_fd, CEC_ADAP_S_LOG_ADDRS, &log_addrs))
      {
        LIB_CEC->AddLog(CEC_LOG_ERROR, "CLinuxCECAdapterCommunication::SetLogicalAddresses - ioctl CEC_ADAP_S_LOG_ADDRS failed - errno=%d", errno);
        return false;
      }

      LIB_CEC->AddLog(CEC_LOG_DEBUG, "CLinuxCECAdapterCommunication::SetLogicalAddresses - ioctl CEC_ADAP_S_LOG_ADDRS - log_addr_mask=%04x num_log_addrs=%u", log_addrs.log_addr_mask, log_addrs.num_log_addrs);
    }

    if (!addresses.IsEmpty())
    {
      // NOTE: This can only be configured when num_log_addrs > 0
      //       and gets reset when num_log_addrs = 0
      log_addrs.cec_version = CEC_OP_CEC_VERSION_1_4;
      log_addrs.vendor_id = CEC_VENDOR_PULSE_EIGHT;

      // TODO: Support more then the primary logical address
      log_addrs.num_log_addrs = 1;
      log_addrs.log_addr[0] = addresses.primary;

      switch (addresses.primary)
      {
        case CECDEVICE_AUDIOSYSTEM:
          log_addrs.primary_device_type[0] = CEC_OP_PRIM_DEVTYPE_AUDIOSYSTEM;
          log_addrs.log_addr_type[0] = CEC_LOG_ADDR_TYPE_AUDIOSYSTEM;
          log_addrs.all_device_types[0] = CEC_OP_ALL_DEVTYPE_AUDIOSYSTEM;
          break;
        case CECDEVICE_PLAYBACKDEVICE1:
        case CECDEVICE_PLAYBACKDEVICE2:
        case CECDEVICE_PLAYBACKDEVICE3:
          log_addrs.primary_device_type[0] = CEC_OP_PRIM_DEVTYPE_PLAYBACK;
          log_addrs.log_addr_type[0] = CEC_LOG_ADDR_TYPE_PLAYBACK;
          log_addrs.all_device_types[0] = CEC_OP_ALL_DEVTYPE_PLAYBACK;
          break;
        case CECDEVICE_RECORDINGDEVICE1:
        case CECDEVICE_RECORDINGDEVICE2:
        case CECDEVICE_RECORDINGDEVICE3:
          log_addrs.primary_device_type[0] = CEC_OP_PRIM_DEVTYPE_RECORD;
          log_addrs.log_addr_type[0] = CEC_LOG_ADDR_TYPE_RECORD;
          log_addrs.all_device_types[0] = CEC_OP_ALL_DEVTYPE_RECORD;
          break;
        case CECDEVICE_TUNER1:
        case CECDEVICE_TUNER2:
        case CECDEVICE_TUNER3:
        case CECDEVICE_TUNER4:
          log_addrs.primary_device_type[0] = CEC_OP_PRIM_DEVTYPE_TUNER;
          log_addrs.log_addr_type[0] = CEC_LOG_ADDR_TYPE_TUNER;
          log_addrs.all_device_types[0] = CEC_OP_ALL_DEVTYPE_TUNER;
          break;
        case CECDEVICE_TV:
          log_addrs.primary_device_type[0] = CEC_OP_PRIM_DEVTYPE_TV;
          log_addrs.log_addr_type[0] = CEC_LOG_ADDR_TYPE_TV;
          log_addrs.all_device_types[0] = CEC_OP_ALL_DEVTYPE_TV;
          break;
        default:
          log_addrs.primary_device_type[0] = CEC_OP_PRIM_DEVTYPE_SWITCH;
          log_addrs.log_addr_type[0] = CEC_LOG_ADDR_TYPE_UNREGISTERED;
          log_addrs.all_device_types[0] = CEC_OP_ALL_DEVTYPE_SWITCH;
          break;
      }
    }
    else
      log_addrs.num_log_addrs = 0;

    if (ioctl(m_fd, CEC_ADAP_S_LOG_ADDRS, &log_addrs))
    {
      LIB_CEC->AddLog(CEC_LOG_ERROR, "CLinuxCECAdapterCommunication::SetLogicalAddresses - ioctl CEC_ADAP_S_LOG_ADDRS failed - errno=%d", errno);
      return false;
    }

    LIB_CEC->AddLog(CEC_LOG_DEBUG, "CLinuxCECAdapterCommunication::SetLogicalAddresses - ioctl CEC_ADAP_S_LOG_ADDRS - log_addr_mask=%04x num_log_addrs=%u", log_addrs.log_addr_mask, log_addrs.num_log_addrs);

    if (log_addrs.num_log_addrs && !log_addrs.log_addr_mask)
        return false;

    return true;
  }

  return false;
}

cec_logical_addresses CLinuxCECAdapterCommunication::GetLogicalAddresses(void) const
{
  cec_logical_addresses addresses;
  addresses.Clear();

  if (m_fd != INVALID_SOCKET_VALUE)
  {
    struct cec_log_addrs log_addrs = {};
    if (ioctl(m_fd, CEC_ADAP_G_LOG_ADDRS, &log_addrs))
    {
      LIB_CEC->AddLog(CEC_LOG_ERROR, "CLinuxCECAdapterCommunication::GetLogicalAddresses - ioctl CEC_ADAP_G_LOG_ADDRS failed - errno=%d", errno);
      return addresses;
    }

    for (int i = 0; i < log_addrs.num_log_addrs; i++)
      addresses.Set(cec_logical_address(log_addrs.log_addr[i]));
  }

  return addresses;
}

uint16_t CLinuxCECAdapterCommunication::GetPhysicalAddress(void)
{
  if (IsOpen())
  {
    uint16_t addr;
    if (!ioctl(m_fd, CEC_ADAP_G_PHYS_ADDR, &addr))
      return addr;

    LIB_CEC->AddLog(CEC_LOG_ERROR, "CLinuxCECAdapterCommunication::GetPhysicalAddress - ioctl CEC_ADAP_G_PHYS_ADDR failed - errno=%d", errno);
  }

  return CEC_INVALID_PHYSICAL_ADDRESS;
}

cec_vendor_id CLinuxCECAdapterCommunication::GetVendorId(void)
{
  if (IsOpen())
  {
    struct cec_log_addrs log_addrs = {};
    if (!ioctl(m_fd, CEC_ADAP_G_LOG_ADDRS, &log_addrs))
      return cec_vendor_id(log_addrs.vendor_id);

    LIB_CEC->AddLog(CEC_LOG_ERROR, "CLinuxCECAdapterCommunication::GetVendorId - ioctl CEC_ADAP_G_LOG_ADDRS failed - errno=%d", errno);
  }

  return CEC_VENDOR_UNKNOWN;
}

void *CLinuxCECAdapterCommunication::Process(void)
{
  CTimeout phys_addr_timeout;
  bool phys_addr_changed = false;
  uint16_t phys_addr = CEC_INVALID_PHYSICAL_ADDRESS;
  fd_set rd_fds;
  fd_set ex_fds;

  while (!IsStopped())
  {
    struct timeval timeval = {};
    timeval.tv_sec = 1;

    FD_ZERO(&rd_fds);
    FD_ZERO(&ex_fds);
    FD_SET(m_fd, &rd_fds);
    FD_SET(m_fd, &ex_fds);

    if (select(m_fd + 1, &rd_fds, NULL, &ex_fds, &timeval) < 0)
    {
      LIB_CEC->AddLog(CEC_LOG_ERROR, "CLinuxCECAdapterCommunication::Process - select failed - errno=%d", errno);
      break;
    }

    if (FD_ISSET(m_fd, &ex_fds))
    {
      struct cec_event ev = {};
      if (ioctl(m_fd, CEC_DQEVENT, &ev))
        LIB_CEC->AddLog(CEC_LOG_ERROR, "CLinuxCECAdapterCommunication::Process - ioctl CEC_DQEVENT failed - errno=%d", errno);
      else if (ev.event == CEC_EVENT_STATE_CHANGE)
      {
        LIB_CEC->AddLog(CEC_LOG_DEBUG, "CLinuxCECAdapterCommunication::Process - CEC_DQEVENT - CEC_EVENT_STATE_CHANGE - log_addr_mask=%04x phys_addr=%04x", ev.state_change.log_addr_mask, ev.state_change.phys_addr);

        // TODO: handle ev.state_change.log_addr_mask change

        phys_addr = ev.state_change.phys_addr;
        phys_addr_changed = true;

        if (ev.state_change.phys_addr == CEC_PHYS_ADDR_INVALID)
        {
          // Debounce change to invalid physical address with 2 seconds because
          // EDID refresh and other events may cause short periods of invalid physical address
          phys_addr_timeout.Init(2000);
        }
        else
        {
          // Debounce change to valid physical address with 500 ms when no logical address have been claimed
          phys_addr_timeout.Init(ev.state_change.log_addr_mask ? 0 : 500);
        }
      }
    }

    if (phys_addr_changed && !phys_addr_timeout.TimeLeft() && !IsStopped())
    {
      phys_addr_changed = false;
      m_callback->HandlePhysicalAddressChanged(phys_addr);
    }

    if (FD_ISSET(m_fd, &rd_fds))
    {
      struct cec_msg msg = {};
      if (ioctl(m_fd, CEC_RECEIVE, &msg))
        LIB_CEC->AddLog(CEC_LOG_ERROR, "CLinuxCECAdapterCommunication::Process - ioctl CEC_RECEIVE failed - rx_status=%02x errno=%d", msg.rx_status, errno);
      else if (msg.len > 0)
      {
        LIB_CEC->AddLog(CEC_LOG_DEBUG, "CLinuxCECAdapterCommunication::Process - ioctl CEC_RECEIVE - rx_status=%02x len=%d addr=%02x opcode=%02x", msg.rx_status, msg.len, msg.msg[0], cec_msg_opcode(&msg));

        cec_command cmd;
        cmd.PushArray(msg.len, msg.msg);

        if (!IsStopped())
          m_callback->OnCommandReceived(cmd);
      }
    }

    if (!IsStopped())
      Sleep(5);
  }

  LIB_CEC->AddLog(CEC_LOG_DEBUG, "CLinuxCECAdapterCommunication::Process - stopped - m_fd=%d", m_fd);
  return 0;
}

#endif
