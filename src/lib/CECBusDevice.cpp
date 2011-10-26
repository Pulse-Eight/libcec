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

#include "CECBusDevice.h"
#include "CECProcessor.h"
#include "implementations/ANCommandHandler.h"
#include "implementations/CECCommandHandler.h"
#include "implementations/SLCommandHandler.h"

using namespace CEC;

CCECBusDevice::CCECBusDevice(CCECProcessor *processor, cec_logical_address iLogicalAddress, uint16_t iPhysicalAddress) :
  m_iPhysicalAddress(iPhysicalAddress),
  m_iLogicalAddress(iLogicalAddress),
  m_processor(processor),
  m_iVendorId(0),
  m_iVendorClass(0)
{
  m_handler = new CCECCommandHandler(this);
}

CCECBusDevice::~CCECBusDevice(void)
{
  delete m_handler;
}

cec_logical_address CCECBusDevice::GetMyLogicalAddress(void) const
{
  return m_processor->GetLogicalAddress();
}

uint16_t CCECBusDevice::GetMyPhysicalAddress(void) const
{
  return m_processor->GetPhysicalAddress();
}

void CCECBusDevice::AddLog(cec_log_level level, const CStdString &strMessage)
{
  m_processor->AddLog(level, strMessage);
}

void CCECBusDevice::SetVendorId(uint16_t iVendorId, uint8_t iVendorClass /* = 0 */)
{
  delete m_handler;
  m_iVendorId = iVendorId;
  m_iVendorClass = iVendorClass;

  switch (iVendorId)
  {
  case CEC_VENDOR_SAMSUNG:
    m_handler = new CANCommandHandler(this);
    break;
  case CEC_VENDOR_LG:
    m_handler = new CSLCommandHandler(this);
    break;
  default:
    m_handler = new CCECCommandHandler(this);
    break;
  }

  CStdString strLog;
  strLog.Format("device %d: vendor = %s (%04x) class = %2x", m_iLogicalAddress, CECVendorIdToString(iVendorId), iVendorId, iVendorClass);
  m_processor->AddLog(CEC_LOG_DEBUG, strLog.c_str());
}

bool CCECBusDevice::HandleCommand(const cec_command &command)
{
  CLockObject lock(&m_mutex);
  m_handler->HandleCommand(command);
  return true;
}

const char *CCECBusDevice::CECVendorIdToString(const uint64_t iVendorId)
{
  switch (iVendorId)
  {
  case CEC_VENDOR_SAMSUNG:
    return "Samsung";
  case CEC_VENDOR_LG:
      return "LG";
  default:
    return "Unknown";
  }
}
