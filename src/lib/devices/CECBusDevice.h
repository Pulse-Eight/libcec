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
#include "../platform/threads.h"
#include "../util/StdString.h"

namespace CEC
{
  class CCECProcessor;
  class CCECCommandHandler;

  class CCECBusDevice
  {
    friend class CCECProcessor;

  public:
    CCECBusDevice(CCECProcessor *processor, cec_logical_address address, uint16_t iPhysicalAddress = 0);
    virtual ~CCECBusDevice(void);

    virtual bool                MyLogicalAddressContains(cec_logical_address address) const;
    virtual cec_logical_address GetMyLogicalAddress(void) const;
    virtual uint16_t            GetMyPhysicalAddress(void) const;
    virtual const char *        GetVendorName(void) { return GetVendor().AsString(); }
    virtual cec_vendor_id       GetVendorId(void) { return GetVendor().vendor; };
    virtual const cec_vendor &  GetVendor(void);
    virtual uint8_t             GetVendorClass(void) const { return m_iVendorClass; }
    virtual uint64_t            GetLastActive(void) const { return m_iLastActive; }
    virtual cec_logical_address GetLogicalAddress(void) const { return m_iLogicalAddress; }
    virtual uint16_t            GetPhysicalAddress(void) const { return m_iPhysicalAddress; }
    virtual cec_version         GetCecVersion(bool bRefresh = true);
    virtual cec_menu_language & GetMenuLanguage(bool bRefresh = true);
    virtual cec_power_status    GetPowerStatus(bool bRefresh = true);

    virtual bool PowerOn(void);
    virtual bool Standby(void);
    virtual bool SetOSDString(cec_display_control duration, const char *strMessage);
    virtual void PollVendorId(void);

    virtual void SetPhysicalAddress(uint16_t iNewAddress, uint16_t iOldAddress = 0);
    virtual void SetCecVersion(const cec_version newVersion);
    virtual void SetMenuLanguage(const cec_menu_language &menuLanguage);
    virtual void SetVendorId(const cec_datapacket &data);
    virtual void SetVendorId(uint64_t iVendorId, uint8_t iVendorClass = 0);
    virtual void SetPowerStatus(const cec_power_status powerStatus);

    virtual bool HandleCommand(const cec_command &command);

    virtual void AddLog(cec_log_level level, const CStdString &strMessage);
    virtual CCECProcessor *GetProcessor() const { return m_processor; }
    virtual CCECCommandHandler *GetHandler(void) const { return m_handler; };

    virtual bool TransmitCECVersion(cec_logical_address dest);
    virtual bool TransmitDeckStatus(cec_logical_address dest);
    virtual bool TransmitMenuState(cec_logical_address dest);
    virtual bool TransmitOSDName(cec_logical_address dest);
    virtual bool TransmitPowerState(cec_logical_address dest);
    virtual bool TransmitPoll(cec_logical_address dest);
    virtual bool TransmitVendorID(cec_logical_address dest);

    virtual bool TransmitActiveView(void);
    virtual bool TransmitInactiveView(void);
    virtual bool TransmitPhysicalAddress(void);
    virtual bool TransmitActiveSource(void);

  protected:
    cec_device_type     m_type;
    CStdString          m_strDeviceName;
    uint16_t            m_iPhysicalAddress;
    cec_logical_address m_iLogicalAddress;
    cec_power_status    m_powerStatus;
    cec_menu_language   m_menuLanguage;
    CCECProcessor      *m_processor;
    CCECCommandHandler *m_handler;
    cec_vendor          m_vendor;
    bool                m_bMenuActive;
    uint8_t             m_iVendorClass;
    uint64_t            m_iLastActive;
    cec_version         m_cecVersion;
    CMutex              m_mutex;
    CCondition          m_condition;
  };
};
