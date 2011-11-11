/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011 Pulse-Eight Limited.  All rights reserved.
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

#include "CECPlaybackDevice.h"
#include "../implementations/CECCommandHandler.h"
#include "../CECProcessor.h"

using namespace CEC;

CCECPlaybackDevice::CCECPlaybackDevice(CCECProcessor *processor, cec_logical_address address, uint16_t iPhysicalAddress /* = 0 */) :
    CCECBusDevice(processor, address, iPhysicalAddress),
    m_deckStatus(CEC_DECK_INFO_STOP),
    m_deckControlMode(CEC_DECK_CONTROL_MODE_STOP)
{
  m_type = CEC_DEVICE_TYPE_PLAYBACK_DEVICE;
}

void CCECPlaybackDevice::SetDeckStatus(cec_deck_info deckStatus)
{
  if (m_deckStatus != deckStatus)
  {
    CStdString strLog;
    strLog.Format(">> %s (%X): deck status changed from '%s' to '%s'", GetLogicalAddressName(), m_iLogicalAddress, CCECCommandHandler::ToString(m_deckStatus), CCECCommandHandler::ToString(deckStatus));
    AddLog(CEC_LOG_DEBUG, strLog.c_str());

    m_deckStatus = deckStatus;
  }
}

void CCECPlaybackDevice::SetDeckControlMode(cec_deck_control_mode mode)
{
  if (m_deckControlMode != mode)
  {
    CStdString strLog;
    strLog.Format(">> %s (%X): deck control mode changed from '%s' to '%s'", GetLogicalAddressName(), m_iLogicalAddress, CCECCommandHandler::ToString(m_deckControlMode), CCECCommandHandler::ToString(mode));
    AddLog(CEC_LOG_DEBUG, strLog.c_str());

    m_deckControlMode = mode;
  }
}

bool CCECPlaybackDevice::TransmitDeckStatus(cec_logical_address dest)
{
  CStdString strLog;
  strLog.Format("<< %s (%X) -> %s (%X): deck status '%s'", GetLogicalAddressName(), m_iLogicalAddress, CCECCommandHandler::ToString(dest), dest, CCECCommandHandler::ToString(m_deckStatus));
  AddLog(CEC_LOG_NOTICE, strLog);

  cec_command command;
  cec_command::format(command, m_iLogicalAddress, dest, CEC_OPCODE_GIVE_DECK_STATUS);
  command.push_back((uint8_t)m_deckStatus);

  return m_processor->Transmit(command);
}
