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
#include <vector>
#include <map>
#include "../platform/threads/mutex.h"
#include "../platform/util/StdString.h"

namespace CEC
{
  class CCECProcessor;
  class CCECBusDevice;

  class CResponse
  {
  public:
    CResponse(cec_opcode opcode) :
        m_opcode(opcode){}
    ~CResponse(void)
    {
      Broadcast();
    }

    bool Wait(uint32_t iTimeout)
    {
      return m_event.Wait(iTimeout);
    }

    void Broadcast(void)
    {
      m_event.Broadcast();
    }

  private:
    cec_opcode       m_opcode;
    PLATFORM::CEvent m_event;
  };

  class CWaitForResponse
  {
  public:
    CWaitForResponse(void) {}
    ~CWaitForResponse(void)
    {
      PLATFORM::CLockObject lock(m_mutex);
      m_waitingFor.clear();
    }

    bool Wait(cec_opcode opcode, uint32_t iTimeout = CEC_DEFAULT_TRANSMIT_WAIT)
    {
      CResponse *response = GetEvent(opcode);
      return response ? response->Wait(iTimeout) : false;
    }

    void Received(cec_opcode opcode)
    {
      CResponse *response = GetEvent(opcode);
      if (response)
        response->Broadcast();
    }

  private:
    CResponse *GetEvent(cec_opcode opcode)
    {
      CResponse *retVal(NULL);
      {
        PLATFORM::CLockObject lock(m_mutex);
        std::map<cec_opcode, CResponse*>::iterator it = m_waitingFor.find(opcode);
        if (it != m_waitingFor.end())
        {
          retVal = it->second;
        }
        else
        {
          retVal = new CResponse(opcode);
          m_waitingFor[opcode] = retVal;
        }
        return retVal;
      }
    }

    PLATFORM::CMutex                 m_mutex;
    std::map<cec_opcode, CResponse*> m_waitingFor;
  };

  class CCECCommandHandler
  {
  public:
    CCECCommandHandler(CCECBusDevice *busDevice);
    virtual ~CCECCommandHandler(void);

    virtual bool HandleCommand(const cec_command &command);
    virtual cec_vendor_id GetVendorId(void) { return m_vendorId; };
    virtual void SetVendorId(cec_vendor_id vendorId) { m_vendorId = vendorId; }
    static bool HasSpecificHandler(cec_vendor_id vendorId) { return vendorId == CEC_VENDOR_LG || vendorId == CEC_VENDOR_SAMSUNG || vendorId == CEC_VENDOR_PANASONIC;}

    virtual bool InitHandler(void) { return true; }
    virtual bool ActivateSource(void);
    virtual uint8_t GetTransmitRetries(void) const { return m_iTransmitRetries; }

    virtual bool PowerOn(const cec_logical_address iInitiator, const cec_logical_address iDestination);
    virtual bool TransmitImageViewOn(const cec_logical_address iInitiator, const cec_logical_address iDestination);
    virtual bool TransmitStandby(const cec_logical_address iInitiator, const cec_logical_address iDestination);
    virtual bool TransmitRequestActiveSource(const cec_logical_address iInitiator, bool bWaitForResponse = true);
    virtual bool TransmitRequestCecVersion(const cec_logical_address iInitiator, const cec_logical_address iDestination, bool bWaitForResponse = true);
    virtual bool TransmitRequestMenuLanguage(const cec_logical_address iInitiator, const cec_logical_address iDestination, bool bWaitForResponse = true);
    virtual bool TransmitRequestOSDName(const cec_logical_address iInitiator, const cec_logical_address iDestination, bool bWaitForResponse = true);
    virtual bool TransmitRequestPhysicalAddress(const cec_logical_address iInitiator, const cec_logical_address iDestination, bool bWaitForResponse = true);
    virtual bool TransmitRequestPowerStatus(const cec_logical_address iInitiator, const cec_logical_address iDestination, bool bWaitForResponse = true);
    virtual bool TransmitRequestVendorId(const cec_logical_address iInitiator, const cec_logical_address iDestination, bool bWaitForResponse = true);
    virtual bool TransmitActiveSource(const cec_logical_address iInitiator, uint16_t iPhysicalAddress);
    virtual bool TransmitCECVersion(const cec_logical_address iInitiator, const cec_logical_address iDestination, cec_version cecVersion);
    virtual bool TransmitInactiveSource(const cec_logical_address iInitiator, uint16_t iPhysicalAddress);
    virtual bool TransmitMenuState(const cec_logical_address iInitiator, const cec_logical_address iDestination, cec_menu_state menuState);
    virtual bool TransmitOSDName(const cec_logical_address iInitiator, const cec_logical_address iDestination, CStdString strDeviceName);
    virtual bool TransmitOSDString(const cec_logical_address iInitiator, const cec_logical_address iDestination, cec_display_control duration, const char *strMessage);
    virtual bool TransmitPhysicalAddress(const cec_logical_address iInitiator, uint16_t iPhysicalAddress, cec_device_type type);
    virtual bool TransmitSetMenuLanguage(const cec_logical_address iInitiator, const char lang[3]);
    virtual bool TransmitPoll(const cec_logical_address iInitiator, const cec_logical_address iDestination);
    virtual bool TransmitPowerState(const cec_logical_address iInitiator, const cec_logical_address iDestination, cec_power_status state);
    virtual bool TransmitVendorID(const cec_logical_address iInitiator, uint64_t iVendorId);
    virtual bool TransmitAudioStatus(const cec_logical_address iInitiator, const cec_logical_address iDestination, uint8_t state);
    virtual bool TransmitSetSystemAudioMode(const cec_logical_address iInitiator, const cec_logical_address iDestination, cec_system_audio_status state);
    virtual bool TransmitSystemAudioModeStatus(const cec_logical_address iInitiator, const cec_logical_address iDestination, cec_system_audio_status state);
    virtual bool TransmitDeckStatus(const cec_logical_address iInitiator, const cec_logical_address iDestination, cec_deck_info state);
    virtual bool TransmitKeypress(const cec_logical_address iInitiator, const cec_logical_address iDestination, cec_user_control_code key, bool bWait = true);
    virtual bool TransmitKeyRelease(const cec_logical_address iInitiator, const cec_logical_address iDestination, bool bWait = true);
    virtual bool TransmitSetStreamPath(uint16_t iStreamPath);
    virtual bool SendDeckStatusUpdateOnActiveSource(void) const { return m_bOPTSendDeckStatusUpdateOnActiveSource; };
    virtual bool TransmitPendingActiveSourceCommands(void) { return true; }

    virtual void SignalOpcode(cec_opcode opcode);

  protected:
    virtual bool HandleActiveSource(const cec_command &command);
    virtual bool HandleDeckControl(const cec_command &command);
    virtual bool HandleDeviceCecVersion(const cec_command &command);
    virtual bool HandleDeviceVendorCommandWithId(const cec_command &command);
    virtual bool HandleDeviceVendorId(const cec_command &command);
    virtual bool HandleFeatureAbort(const cec_command &command);
    virtual bool HandleGetCecVersion(const cec_command &command);
    virtual bool HandleGiveAudioStatus(const cec_command &command);
    virtual bool HandleGiveDeckStatus(const cec_command &command);
    virtual bool HandleGiveDevicePowerStatus(const cec_command &command);
    virtual bool HandleGiveDeviceVendorId(const cec_command &command);
    virtual bool HandleGiveOSDName(const cec_command &command);
    virtual bool HandleGivePhysicalAddress(const cec_command &command);
    virtual bool HandleGiveMenuLanguage(const cec_command &command);
    virtual bool HandleGiveSystemAudioModeStatus(const cec_command &command);
    virtual bool HandleImageViewOn(const cec_command &command);
    virtual bool HandleMenuRequest(const cec_command &command);
    virtual bool HandlePoll(const cec_command &command);
    virtual bool HandleReportAudioStatus(const cec_command &command);
    virtual bool HandleReportPhysicalAddress(const cec_command &command);
    virtual bool HandleReportPowerStatus(const cec_command &command);
    virtual bool HandleRequestActiveSource(const cec_command &command);
    virtual bool HandleRoutingChange(const cec_command &command);
    virtual bool HandleRoutingInformation(const cec_command &command);
    virtual bool HandleSetMenuLanguage(const cec_command &command);
    virtual bool HandleSetOSDName(const cec_command &command);
    virtual bool HandleSetStreamPath(const cec_command &command);
    virtual bool HandleSystemAudioModeRequest(const cec_command &command);
    virtual bool HandleStandby(const cec_command &command);
    virtual bool HandleSystemAudioModeStatus(const cec_command &command);
    virtual bool HandleSetSystemAudioMode(const cec_command &command);
    virtual bool HandleTextViewOn(const cec_command &command);
    virtual bool HandleUserControlPressed(const cec_command &command);
    virtual bool HandleUserControlRelease(const cec_command &command);
    virtual bool HandleVendorCommand(const cec_command &command);
    virtual void UnhandledCommand(const cec_command &command);

    virtual size_t GetMyDevices(std::vector<CCECBusDevice *> &devices) const;
    virtual CCECBusDevice *GetDevice(cec_logical_address iLogicalAddress) const;
    virtual CCECBusDevice *GetDeviceByPhysicalAddress(uint16_t iPhysicalAddress) const;
    virtual CCECBusDevice *GetDeviceByType(cec_device_type type) const;

    virtual bool SetVendorId(const cec_command &command);
    virtual void SetPhysicalAddress(cec_logical_address iAddress, uint16_t iNewAddress);

    virtual bool Transmit(cec_command &command, bool bSuppressWait = false);

    CCECBusDevice *                       m_busDevice;
    CCECProcessor *                       m_processor;
    int32_t                               m_iTransmitTimeout;
    int32_t                               m_iTransmitWait;
    int8_t                                m_iTransmitRetries;
    bool                                  m_bHandlerInited;
    bool                                  m_bOPTSendDeckStatusUpdateOnActiveSource;
    cec_vendor_id                         m_vendorId;
    CWaitForResponse                     *m_waitForResponse;
  };
};
