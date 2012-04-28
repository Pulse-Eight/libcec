#pragma once
/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2012 Pulse-Eight Limited.  All rights reserved.
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

#include "../../../include/cectypes.h"
#include <set>
#include "../platform/threads/mutex.h"
#include "../platform/util/StdString.h"

namespace CEC
{
  class CCECProcessor;
  class CCECCommandHandler;

  class CCECBusDevice
  {
    friend class CCECProcessor;

  public:
    CCECBusDevice(CCECProcessor *processor, cec_logical_address address, uint16_t iPhysicalAddress = CEC_INVALID_PHYSICAL_ADDRESS);
    virtual ~CCECBusDevice(void);

    virtual bool HandleCommand(const cec_command &command);
    virtual bool PowerOn(void);
    virtual bool Standby(void);

    virtual cec_version           GetCecVersion(bool bUpdate = false);
    virtual CCECCommandHandler *  GetHandler(void) const { return m_handler; };
    virtual uint64_t              GetLastActive(void) const { return m_iLastActive; }
    virtual cec_logical_address   GetLogicalAddress(void) const { return m_iLogicalAddress; }
    virtual const char*           GetLogicalAddressName(void) const;
    virtual cec_menu_language &   GetMenuLanguage(bool bUpdate = false);
    virtual cec_menu_state        GetMenuState(void);
    virtual cec_logical_address   GetMyLogicalAddress(void) const;
    virtual uint16_t              GetMyPhysicalAddress(void) const;
    virtual CStdString            GetOSDName(bool bUpdate = false);
    virtual uint16_t              GetPhysicalAddress(bool bSuppressUpdate = true);
    virtual cec_power_status      GetPowerStatus(bool bUpdate = false);
    virtual CCECProcessor *       GetProcessor(void) const { return m_processor; }
    virtual cec_device_type       GetType(void) const { return m_type; }
    virtual cec_vendor_id         GetVendorId(bool bUpdate = false);
    virtual const char *          GetVendorName(bool bUpdate = false);
    virtual bool                  MyLogicalAddressContains(cec_logical_address address) const;
    virtual cec_bus_device_status GetStatus(bool bForcePoll = false, bool bSuppressPoll = false);
    virtual bool                  IsActiveSource(void) const { return m_bActiveSource; }
    virtual bool                  IsUnsupportedFeature(cec_opcode opcode);
    virtual void                  SetUnsupportedFeature(cec_opcode opcode);
    virtual void                  HandlePoll(cec_logical_address destination);
    virtual void                  HandlePollFrom(cec_logical_address initiator);
    virtual bool                  HandleReceiveFailed(void);

    virtual void SetInactiveSource(void);
    virtual void SetActiveSource(void);
    virtual bool TryLogicalAddress(void);
    virtual bool ActivateSource(void);

    virtual void SetDeviceStatus(const cec_bus_device_status newStatus);
    virtual void SetPhysicalAddress(uint16_t iNewAddress);
    virtual void SetStreamPath(uint16_t iNewAddress, uint16_t iOldAddress = CEC_INVALID_PHYSICAL_ADDRESS);
    virtual void SetCecVersion(const cec_version newVersion);
    virtual void SetMenuLanguage(const cec_menu_language &menuLanguage);
    virtual void SetOSDName(CStdString strName);
    virtual void SetMenuState(const cec_menu_state state);
    virtual bool SetVendorId(uint64_t iVendorId);
    virtual void SetPowerStatus(const cec_power_status powerStatus);

    virtual bool TransmitActiveSource(void);
    virtual bool TransmitCECVersion(cec_logical_address dest);
    virtual bool TransmitImageViewOn(void);
    virtual bool TransmitInactiveSource(void);
    virtual bool TransmitMenuState(cec_logical_address dest);
    virtual bool TransmitOSDName(cec_logical_address dest);
    virtual bool TransmitOSDString(cec_logical_address dest, cec_display_control duration, const char *strMessage);
    virtual bool TransmitPhysicalAddress(void);
    virtual bool TransmitSetMenuLanguage(cec_logical_address dest);
    virtual bool TransmitPowerState(cec_logical_address dest);
    virtual bool TransmitPoll(cec_logical_address dest);
    virtual bool TransmitVendorID(cec_logical_address dest, bool bSendAbort = true);
    virtual bool TransmitKeypress(cec_user_control_code key, bool bWait = true);
    virtual bool TransmitKeyRelease(bool bWait = true);

    bool ReplaceHandler(bool bActivateSource = true);
    virtual bool TransmitPendingActiveSourceCommands(void);

    virtual bool RequestActiveSource(bool bWaitForResponse = true);

  protected:
    void ResetDeviceStatus(void);
    void CheckVendorIdRequested(void);
    void MarkBusy(void);
    void MarkReady(void);

    bool RequestCecVersion(bool bWaitForResponse = true);
    bool RequestMenuLanguage(bool bWaitForResponse = true);
    bool RequestPowerStatus(bool bWaitForResponse = true);
    bool RequestVendorId(bool bWaitForResponse = true);
    bool RequestPhysicalAddress(bool bWaitForResponse = true);
    bool RequestOSDName(bool bWaitForResponse = true);

    bool NeedsPoll(void);

    cec_device_type       m_type;
    CStdString            m_strDeviceName;
    uint16_t              m_iPhysicalAddress;
    uint16_t              m_iStreamPath;
    cec_logical_address   m_iLogicalAddress;
    cec_power_status      m_powerStatus;
    cec_menu_language     m_menuLanguage;
    CCECProcessor      *  m_processor;
    CCECCommandHandler *  m_handler;
    cec_vendor_id         m_vendor;
    bool                  m_bReplaceHandler;
    cec_menu_state        m_menuState;
    bool                  m_bActiveSource;
    uint64_t              m_iLastActive;
    uint64_t              m_iLastPowerStateUpdate;
    cec_version           m_cecVersion;
    cec_bus_device_status m_deviceStatus;
    std::set<cec_opcode>  m_unsupportedFeatures;
    PLATFORM::CMutex      m_mutex;
    PLATFORM::CMutex      m_handlerMutex;
    PLATFORM::CEvent      m_replacing;
    unsigned              m_iHandlerUseCount;
    bool                  m_bAwaitingReceiveFailed;
    bool                  m_bVendorIdRequested;
  };
};
