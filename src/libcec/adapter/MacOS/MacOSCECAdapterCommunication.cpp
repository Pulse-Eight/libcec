/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC MacOS Code is Copyright (C) 2014 Valentin Manea
 * libCEC(R) is Copyright (C) 2011-2015 Pulse-Eight Limited.  All rights
 * reserved. libCEC(R) is an original work, containing original code.
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

#include <fcntl.h>
#include <stdlib.h>

#include <map>

#include "env.h"

#if defined(HAVE_MACOS_API)

#include "CECTypeUtils.h"
#include "LibCEC.h"
#include "MacOSCEC.h"
#include "MacOSCECAdapterCommunication.h"
#include "p8-platform/util/buffer.h"

using namespace CEC;
using namespace P8PLATFORM;

#define LIB_CEC m_callback->GetLib()

CMacOSCECAdapterCommunication::CMacOSCECAdapterCommunication(
    IAdapterCommunicationCallback *callback)
    : IAdapterCommunication(callback), m_dpAux() {
  CLockObject lock(m_mutex);
  m_logicalAddresses.Clear();
}

CMacOSCECAdapterCommunication::~CMacOSCECAdapterCommunication(void) { Close(); }

bool CMacOSCECAdapterCommunication::Open(uint32_t UNUSED(iTimeoutMs),
                                         bool UNUSED(bSkipChecks),
                                         bool bStartListening) {
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s", __func__);
  uint8_t val = DP_CEC_TUNNELING_ENABLE;
  m_dpAux.Write(DP_CEC_TUNNELING_CONTROL, &val, sizeof(val));
  if (!bStartListening || CreateThread()) {
    return true;
  }
  return true;
}

void CMacOSCECAdapterCommunication::Close(void) {
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s", __func__);
  {
    CLockObject lock(m_mutex);
    uint8_t val = 0;
    m_dpAux.Write(DP_CEC_TUNNELING_CONTROL, &val, sizeof(val));
  }
  StopThread(0);
}

std::string CMacOSCECAdapterCommunication::GetError(void) const {
  std::string strError(m_strError);
  return strError;
}

cec_adapter_message_state CMacOSCECAdapterCommunication::Write(
    const cec_command &data, bool &UNUSED(bRetry), uint8_t UNUSED(iLineTimeout),
    bool UNUSED(bIsReply)) {
  CLockObject lock(m_mutex);
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s", __func__);
  uint8_t buffer[CEC_MAX_FRAME_SIZE];
  int32_t size = 1;
  buffer[0] = (data.initiator << 4) | (data.destination & 0x0f);
  if (data.opcode_set) {
    buffer[1] = data.opcode;
    size++;

    memcpy(&buffer[size], data.parameters.data, data.parameters.size);
    size += data.parameters.size;
  }
  if ((size_t)data.parameters.size + data.opcode_set > sizeof(buffer)) {
    LIB_CEC->AddLog(CEC_LOG_ERROR, "%s: data size too large !", __func__);
    return ADAPTER_MESSAGE_STATE_ERROR;
  }
  m_dpAux.Write(DP_CEC_TX_MESSAGE_BUFFER, buffer, size);
  uint8_t info = (size & DP_CEC_TX_MESSAGE_LEN_MASK)
                     << DP_CEC_TX_MESSAGE_LEN_SHIFT |
                 DP_CEC_TX_MESSAGE_SEND;
  m_dpAux.Write(DP_CEC_TX_MESSAGE_INFO, &info, 1);
  auto ret = ADAPTER_MESSAGE_STATE_SENT_NOT_ACKED;
  uint8_t irq = 0;
  for (int i = 0; i < 5; i++) {
    if (!m_dpAux.Read(DP_CEC_TUNNELING_IRQ_FLAGS, &irq, 1)) {
      usleep(100000);
      continue;
    };
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s poll-flags %x", __func__, irq);
    if (irq & DP_CEC_TX_MESSAGE_SENT) {
      LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s out ack", __func__);
      ret = ADAPTER_MESSAGE_STATE_SENT_ACKED;
      break;
    }
    if (irq & (DP_CEC_TX_ADDRESS_NACK_ERROR | DP_CEC_TX_DATA_NACK_ERROR)) {
      LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s out nack", __func__);
      ret = ADAPTER_MESSAGE_STATE_SENT_NOT_ACKED;
      break;
    }
    usleep(100000);
  }
  if (irq) {
    irq &= ~DP_CEC_RX_MESSAGE_INFO_VALID;
    if (!m_dpAux.Write(DP_CEC_TUNNELING_IRQ_FLAGS, &irq, 1)) {
      LIB_CEC->AddLog(CEC_LOG_WARNING, "%s write irq fail", __func__);
    }
  }

  // TODO: Interrupt support?
  // https://developer.apple.com/documentation/kernel/ioframebuffer/1397721-registerforinterrupttype
  return ret;
}

uint16_t CMacOSCECAdapterCommunication::GetFirmwareVersion(void) { return 0; }

cec_vendor_id CMacOSCECAdapterCommunication::GetVendorId(void) {
  return cec_vendor_id(CEC_VENDOR_APPLE);
}

uint16_t CMacOSCECAdapterCommunication::GetPhysicalAddress(void) {
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s", __func__);
  return m_dpAux.GetPhysicalAddress();
}

cec_logical_addresses CMacOSCECAdapterCommunication::GetLogicalAddresses(
    void) const {
  CLockObject lock(m_mutex);
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s", __func__);
  return m_logicalAddresses;
}

bool CMacOSCECAdapterCommunication::SetLogicalAddresses(
    const cec_logical_addresses &addresses) {
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s", __func__);
  CLockObject lock(m_mutex);
  /* Bit 15 (logical address 15) should always be set */
  uint16_t la_mask = 1 << CECDEVICE_BROADCAST;
  la_mask |= (1 << addresses.primary);

  uint8_t mask[2];
  mask[0] = la_mask & 0xff;
  mask[1] = la_mask >> 8;
  m_logicalAddresses = addresses;
  return m_dpAux.Write(DP_CEC_LOGICAL_ADDRESS_MASK, mask, sizeof(mask));
}

void *CMacOSCECAdapterCommunication::Process(void) {
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s", __func__);
  uint8_t buffer[CEC_MAX_FRAME_SIZE];
  uint32_t size;
  cec_logical_address initiator, destination;

  uint8_t irq = 0;

  while (!IsStopped()) {
    {
      CLockObject lock(m_mutex);
      if (!m_dpAux.Read(DP_CEC_TUNNELING_IRQ_FLAGS, &irq, 1)) {
        LIB_CEC->AddLog(CEC_LOG_WARNING, "%s DisplayPortAux::Read fail",
                        __func__);
      }
    }
    if (irq) {
      LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s poll-flags %x", __func__, irq);
    }
    if (irq == 0xff) {
      usleep(10000);
      continue;
    }
    if (irq & DP_CEC_RX_MESSAGE_INFO_VALID) {
      uint8_t rx_info;
      CLockObject lock(m_mutex);
      m_dpAux.Read(DP_CEC_RX_MESSAGE_INFO, &rx_info, 1);
      if (!(rx_info & DP_CEC_RX_MESSAGE_ENDED)) {
        LIB_CEC->AddLog(CEC_LOG_WARNING, "%s Receive not ended", __func__);
        continue;
      }
      size = (rx_info & DP_CEC_RX_MESSAGE_LEN_MASK) + 1;

      if (size > 0) {
        m_dpAux.Read(DP_CEC_RX_MESSAGE_BUFFER, buffer, size);
        initiator = cec_logical_address(buffer[0] >> 4);
        destination = cec_logical_address(buffer[0] & 0x0f);

        cec_command cmd;

        cec_command::Format(
            cmd, initiator, destination,
            (size > 1) ? cec_opcode(buffer[1]) : CEC_OPCODE_NONE);

        for (uint8_t i = 2; i < size; i++) cmd.parameters.PushBack(buffer[i]);

        if (!IsStopped()) {
          lock.Unlock();
          m_callback->OnCommandReceived(cmd);
          lock.Lock();
        }
      }
      m_dpAux.Write(DP_CEC_TUNNELING_IRQ_FLAGS, &irq, 1);
    } else if (irq & DP_CEC_TX_MESSAGE_SENT) {
      CLockObject lock(m_mutex);
      LIB_CEC->AddLog(CEC_LOG_WARNING, "%s Spurious TX_MESSAGE_SENT", __func__);
      m_dpAux.Write(DP_CEC_TUNNELING_IRQ_FLAGS, &irq, 1);
    }
    usleep(100000);
  }

  return 0;
}

#endif  // HAVE_MACOS_API
