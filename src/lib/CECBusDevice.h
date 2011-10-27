#pragma once
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

#include <cectypes.h>
#include "platform/threads.h"
#include "util/StdString.h"

namespace CEC
{
  class CCECProcessor;
  class CCECCommandHandler;

  class CCECBusDevice
  {
  public:
    CCECBusDevice(CCECProcessor *processor, cec_logical_address address, uint16_t iPhysicalAddress = 0);
    virtual ~CCECBusDevice(void);

    virtual cec_logical_address GetLogicalAddress(void) const { return m_iLogicalAddress; }
    virtual uint16_t GetPhysicalAddress(void) const { return m_iPhysicalAddress; }

    virtual cec_logical_address GetMyLogicalAddress(void) const;
    virtual uint16_t GetMyPhysicalAddress(void) const;

    virtual void SetVendorId(uint64_t iVendorId, uint8_t iVendorClass = 0);
    virtual const char *GetVendorName(void) const { return CECVendorIdToString(m_iVendorId); }
    virtual uint64_t GetVendorId(void) const { return m_iVendorId; }
    virtual uint8_t GetVendorClass(void) const { return m_iVendorClass; }

    virtual uint64_t GetLastActive(void) const { return m_iLastActive; }

    virtual bool HandleCommand(const cec_command &command);

    virtual void AddLog(cec_log_level level, const CStdString &strMessage);
    virtual CCECProcessor *GetProcessor() const { return m_processor; }
    virtual CCECCommandHandler *GetHandler(void) const { return m_handler; };

    virtual void PollVendorId(void);

    static const char *CECVendorIdToString(const uint64_t iVendorId);

  protected:
    uint16_t            m_iPhysicalAddress;
    cec_logical_address m_iLogicalAddress;
    CCECProcessor      *m_processor;
    CCECCommandHandler *m_handler;
    uint64_t            m_iVendorId;
    uint8_t             m_iVendorClass;
    uint64_t            m_iLastActive;
    CMutex              m_mutex;
  };
};
