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

#include "CECAudioSystem.h"
#include "../CECProcessor.h"
#include "../implementations/CECCommandHandler.h"

using namespace CEC;

#define ToString(p) m_processor->ToString(p)

CCECAudioSystem::CCECAudioSystem(CCECProcessor *processor, cec_logical_address address, uint16_t iPhysicalAddress /* = 0 */) :
    CCECBusDevice(processor, address, iPhysicalAddress),
    m_systemAudioStatus(CEC_SYSTEM_AUDIO_STATUS_ON),
    m_audioStatus(CEC_AUDIO_MUTE_STATUS_MASK)
{
  m_type = CEC_DEVICE_TYPE_AUDIO_SYSTEM;
}

bool CCECAudioSystem::SetAudioStatus(uint8_t status)
{
  CLockObject lock(&m_writeMutex);
  if (m_audioStatus != status)
  {
    CStdString strLog;
    strLog.Format(">> %s (%X): audio status changed from %2x to %2x", GetLogicalAddressName(), m_iLogicalAddress, m_audioStatus, status);
    AddLog(CEC_LOG_DEBUG, strLog.c_str());

    m_audioStatus = status;
    return true;
  }

  return false;
}

bool CCECAudioSystem::SetSystemAudioModeStatus(const cec_system_audio_status mode)
{
  CLockObject lock(&m_writeMutex);
  if (m_systemAudioStatus != mode)
  {
    CStdString strLog;
    strLog.Format(">> %s (%X): system audio mode status changed from %s to %s", GetLogicalAddressName(), m_iLogicalAddress, ToString(m_systemAudioStatus), ToString(mode));
    AddLog(CEC_LOG_DEBUG, strLog.c_str());

    m_systemAudioStatus = mode;
    return true;
  }

  return false;
}

bool CCECAudioSystem::TransmitAudioStatus(cec_logical_address dest)
{
  uint8_t state;
  {
    CLockObject lock(&m_writeMutex);
    CStdString strLog;
    strLog.Format("<< %x -> %x: audio status '%2x'", m_iLogicalAddress, dest, m_audioStatus);
    AddLog(CEC_LOG_NOTICE, strLog);
    state = m_audioStatus;
  }

  return m_handler->TransmitAudioStatus(m_iLogicalAddress, dest, state);
}

bool CCECAudioSystem::TransmitSetSystemAudioMode(cec_logical_address dest)
{
  cec_system_audio_status state;
  {
    CLockObject lock(&m_writeMutex);
    CStdString strLog;
    strLog.Format("<< %x -> %x: set system audio mode '%2x'", m_iLogicalAddress, dest, m_audioStatus);
    AddLog(CEC_LOG_NOTICE, strLog);
    state = m_systemAudioStatus;
  }

  return m_handler->TransmitSetSystemAudioMode(m_iLogicalAddress, dest, state);
}

bool CCECAudioSystem::TransmitSystemAudioModeStatus(cec_logical_address dest)
{
  cec_system_audio_status state;
  {
    CLockObject lock(&m_writeMutex);
    CStdString strLog;
    strLog.Format("<< %x -> %x: system audio mode '%s'", m_iLogicalAddress, dest, ToString(m_systemAudioStatus));
    AddLog(CEC_LOG_NOTICE, strLog);
    state = m_systemAudioStatus;
  }

  return m_handler->TransmitSystemAudioModeStatus(m_iLogicalAddress, dest, state);
}

uint8_t CCECAudioSystem::VolumeUp(void)
{
  if (SendKeypress(CEC_USER_CONTROL_CODE_VOLUME_UP))
    SendKeyRelease();

  CLockObject lock(&m_mutex);
  return m_audioStatus;
}

uint8_t CCECAudioSystem::VolumeDown(void)
{
  if (SendKeypress(CEC_USER_CONTROL_CODE_VOLUME_DOWN))
    SendKeyRelease();

  CLockObject lock(&m_mutex);
  return m_audioStatus;
}

uint8_t CCECAudioSystem::MuteAudio(void)
{
  if (SendKeypress(CEC_USER_CONTROL_CODE_MUTE))
    SendKeyRelease();

  CLockObject lock(&m_mutex);
  return m_audioStatus;
}
