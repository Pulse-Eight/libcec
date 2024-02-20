/*
 * This file is part of the libCEC(R) library.
 *
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
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
#include "CECProcessor.h"

#include "adapter/AdapterFactory.h"
#include "devices/CECBusDevice.h"
#include "devices/CECAudioSystem.h"
#include "devices/CECPlaybackDevice.h"
#include "devices/CECRecordingDevice.h"
#include "devices/CECTuner.h"
#include "devices/CECTV.h"
#include "implementations/CECCommandHandler.h"
#include "LibCEC.h"
#include "CECClient.h"
#include "CECTypeUtils.h"
#include "p8-platform/util/timeutils.h"
#include "p8-platform/util/util.h"
#include <stdio.h>

using namespace CEC;
using namespace P8PLATFORM;

#define ACTIVE_SOURCE_CHECK_INTERVAL   500
#define TV_PRESENT_CHECK_INTERVAL      30000

#define ToString(x) CCECTypeUtils::ToString(x)

CCECStandbyProtection::CCECStandbyProtection(CCECProcessor* processor) :
    m_processor(processor) {}
CCECStandbyProtection::~CCECStandbyProtection(void) {}

void* CCECStandbyProtection::Process(void)
{
  int64_t last = GetTimeMs();
  int64_t next;
  while (!IsStopped())
  {
    P8PLATFORM::CEvent::Sleep(1000);

    next = GetTimeMs();

    // reset the connection if the clock changed
    if (next < last || next - last > 10000)
    {
      libcec_parameter param;
      param.paramData = NULL; param.paramType = CEC_PARAMETER_TYPE_UNKOWN;
      m_processor->GetLib()->Alert(CEC_ALERT_CONNECTION_LOST, param);
      break;
    }

    last = next;
  }
  return NULL;
}

CCECProcessor::CCECProcessor(CLibCEC *libcec) :
    m_bInitialised(false),
    m_communication(NULL),
    m_libcec(libcec),
    m_iStandardLineTimeout(3),
    m_iRetryLineTimeout(3),
    m_iLastTransmission(0),
    m_bMonitor(true),
    m_bRawTraffic(false),
    m_addrAllocator(NULL),
    m_bStallCommunication(false),
    m_connCheck(NULL)
{
  m_busDevices = new CCECDeviceMap(this);
}

CCECProcessor::~CCECProcessor(void)
{
  m_bStallCommunication = false;
  SAFE_DELETE(m_addrAllocator);
  Close();
  SAFE_DELETE(m_busDevices);
}

bool CCECProcessor::Start(const char *strPort, uint16_t iBaudRate /* = CEC_SERIAL_DEFAULT_BAUDRATE */, uint32_t iTimeoutMs /* = CEC_DEFAULT_CONNECT_TIMEOUT */)
{
  CLockObject lock(m_mutex);
  // open a connection
  if (!OpenConnection(strPort, iBaudRate, iTimeoutMs))
    return false;

  // create the processor thread
  if (!IsRunning())
  {
    if (!CreateThread())
    {
      m_libcec->AddLog(CEC_LOG_ERROR, "could not create a processor thread");
      return false;
    }
  }

  return true;
}

void CCECProcessor::Close(void)
{
  // mark as uninitialised
  SetCECInitialised(false);

  // stop the processor
  SAFE_DELETE(m_connCheck);
  StopThread(-1);
  m_inBuffer.Broadcast();
  StopThread();

  // close the connection
  CLockObject lock(m_mutex);
  SAFE_DELETE(m_communication);
}

void CCECProcessor::ResetMembers(void)
{
  // close the connection
  SAFE_DELETE(m_communication);

  // reset the other members to the initial state
  m_iStandardLineTimeout = 3;
  m_iRetryLineTimeout = 3;
  m_iLastTransmission = 0;
  m_busDevices->ResetDeviceStatus();
}

bool CCECProcessor::OpenConnection(const char *strPort, uint16_t iBaudRate, uint32_t iTimeoutMs, bool bStartListening /* = true */)
{
  bool bReturn(false);
  CTimeout timeout(iTimeoutMs > 0 ? iTimeoutMs : CEC_DEFAULT_TRANSMIT_WAIT);

  // ensure that a previous connection is closed
  if (m_communication)
    Close();

  // reset all member to the initial state
  ResetMembers();

  // check whether the Close() method deleted any previous connection
  if (m_communication)
  {
    m_libcec->AddLog(CEC_LOG_ERROR, "previous connection could not be closed");
    return bReturn;
  }

  // create a new connection
  m_communication = CAdapterFactory(this->m_libcec).GetInstance(strPort, iBaudRate);

  // open a new connection
  unsigned iConnectTry(0);
  while (timeout.TimeLeft() > 0 && (bReturn = m_communication->Open((timeout.TimeLeft() / CEC_CONNECT_TRIES), false, bStartListening)) == false)
  {
    m_libcec->AddLog(CEC_LOG_ERROR, "could not open a connection (try %d)", ++iConnectTry);
    m_communication->Close();
    CEvent::Sleep(CEC_DEFAULT_CONNECT_RETRY_WAIT);
  }

  m_communication->SetRawTrafficMode(m_bRawTraffic);

  m_libcec->AddLog(CEC_LOG_NOTICE, "connection opened");

  // mark as initialised
  SetCECInitialised(true);

  return bReturn;
}

bool CCECProcessor::CECInitialised(void)
{
  CLockObject lock(m_mutex);
  return m_bInitialised;
}

void CCECProcessor::SetCECInitialised(bool bSetTo /* = true */)
{
  {
    CLockObject lock(m_mutex);
    m_bInitialised = bSetTo;
  }
  if (!bSetTo)
    UnregisterClients();
}

bool CCECProcessor::TryLogicalAddress(cec_logical_address address, cec_version libCECSpecVersion /* = CEC_VERSION_1_4 */)
{
  // find the device
  CCECBusDevice *device = m_busDevices->At(address);
  if (device)
  {
    // check if it's already marked as present or used
    if (device->IsPresent() || device->IsHandledByLibCEC())
      return false;

    // poll the LA if not
    return device->TryLogicalAddress(libCECSpecVersion);
  }

  return false;
}

void CCECProcessor::ReplaceHandlers(void)
{
  CLockObject lock(m_mutex);
  if (!CECInitialised())
    return;

  // check each device
  for (CECDEVICEMAP::iterator it = m_busDevices->Begin(); it != m_busDevices->End(); ++it)
    it->second->ReplaceHandler(true);

  for (std::vector<device_type_change_t>::const_iterator it = m_deviceTypeChanges.begin(); it != m_deviceTypeChanges.end(); ++it)
    (*it).client->ChangeDeviceType((*it).from, (*it).to);
  m_deviceTypeChanges.clear();
}

void CCECProcessor::ChangeDeviceType(CECClientPtr client, cec_device_type from, cec_device_type to)
{
  CLockObject lock(m_mutex);
  if (!CECInitialised())
    return;
  device_type_change_t change = { client, from, to };
  m_deviceTypeChanges.push_back(change);
}

bool CCECProcessor::OnCommandReceived(const cec_command &command)
{
  return m_inBuffer.Push(command);
}

void *CCECProcessor::Process(void)
{
  uint16_t timeout = CEC_PROCESSOR_SIGNAL_WAIT_TIME;
  m_libcec->AddLog(CEC_LOG_DEBUG, "processor thread started");

  if (!m_connCheck)
    m_connCheck = new CCECStandbyProtection(this);
  m_connCheck->CreateThread();

  cec_command command; command.Clear();
  CTimeout activeSourceCheck(ACTIVE_SOURCE_CHECK_INTERVAL);
  CTimeout tvPresentCheck(TV_PRESENT_CHECK_INTERVAL);

  // as long as we're not being stopped and the connection is open
  while (!IsStopped() && m_communication->IsOpen())
  {
    // wait for a new incoming command, and process it
    if (m_inBuffer.Pop(command, timeout))
      ProcessCommand(command);

    if (CECInitialised() && !IsStopped())
    {
      // check clients for keypress timeouts
      timeout = m_libcec->CheckKeypressTimeout();

      // check if we need to replace handlers
      ReplaceHandlers();

      // check whether we need to activate a source, if it failed before
      if (activeSourceCheck.TimeLeft() == 0)
      {
        if (CECInitialised())
          TransmitPendingActiveSourceCommands();
        activeSourceCheck.Init(ACTIVE_SOURCE_CHECK_INTERVAL);
      }

      // check whether the TV is present and responding
      if (tvPresentCheck.TimeLeft() == 0)
      {
        CECClientPtr primary = GetPrimaryClient();
        // only check whether the tv responds to polls when a client is connected and not in monitoring mode
        if (primary && primary->GetConfiguration()->bMonitorOnly != 1)
        {
          if (!m_busDevices->At(CECDEVICE_TV)->IsPresent())
          {
            libcec_parameter param;
            param.paramType = CEC_PARAMETER_TYPE_STRING;
            param.paramData = (void*)"TV does not respond to CEC polls";
            primary->Alert(CEC_ALERT_TV_POLL_FAILED, param);
          }
        }
        tvPresentCheck.Init(TV_PRESENT_CHECK_INTERVAL);
      }
    }
    else
      timeout = CEC_PROCESSOR_SIGNAL_WAIT_TIME;
  }

  return NULL;
}

bool CCECProcessor::ActivateSource(uint16_t iStreamPath)
{
  bool bReturn(false);

  // find the device with the given PA
  CCECBusDevice *device = GetDeviceByPhysicalAddress(iStreamPath);
  // and make it the active source when found
  if (device)
    bReturn = device->ActivateSource();
  else
    m_libcec->AddLog(CEC_LOG_DEBUG, "device with PA '%04x' not found", iStreamPath);

  return bReturn;
}

#if CEC_LIB_VERSION_MAJOR >= 5
bool CCECProcessor::GetStats(struct cec_adapter_stats* stats)
{
  return !!m_communication ?
      m_communication->GetStats(stats) :
      false;
}
#endif

void CCECProcessor::SetActiveSource(bool bSetTo, bool bClientUnregistered)
{
  if (m_communication)
    m_communication->SetActiveSource(bSetTo, bClientUnregistered);
}

void CCECProcessor::SetStandardLineTimeout(uint8_t iTimeout)
{
  CLockObject lock(m_mutex);
  m_iStandardLineTimeout = iTimeout;
}

uint8_t CCECProcessor::GetStandardLineTimeout(void)
{
  CLockObject lock(m_mutex);
  return m_iStandardLineTimeout;
}

void CCECProcessor::SetRetryLineTimeout(uint8_t iTimeout)
{
  CLockObject lock(m_mutex);
  m_iRetryLineTimeout = iTimeout;
}

uint8_t CCECProcessor::GetRetryLineTimeout(void)
{
  CLockObject lock(m_mutex);
  return m_iRetryLineTimeout;
}

bool CCECProcessor::PhysicalAddressInUse(uint16_t iPhysicalAddress)
{
  CCECBusDevice *device = GetDeviceByPhysicalAddress(iPhysicalAddress);
  return device != NULL;
}

void CCECProcessor::LogOutput(const cec_command &data)
{
  // Send raw command
  if (m_bRawTraffic)
    m_libcec->AddCommand(data, true);

  std::string strTx;

  // initiator and destination
  strTx = StringUtils::Format("%s %02x", data.sent ? "<<" : ">>", ((uint8_t)data.initiator << 4) + (uint8_t)data.destination);

  // append the opcode
  if (data.opcode_set)
      strTx += StringUtils::Format(":%02x", (uint8_t)data.opcode);

  // append the parameters
  for (uint8_t iPtr = 0; iPtr < data.parameters.size; iPtr++)
    strTx += StringUtils::Format(":%02x", data.parameters[iPtr]);

  // and log it
  m_libcec->AddLog(CEC_LOG_TRAFFIC, strTx.c_str());
}

bool CCECProcessor::PollDevice(cec_logical_address iAddress)
{
  // try to find the primary device
  CCECBusDevice *primary = GetPrimaryDevice();
  // poll the destination, with the primary as source
  if (primary)
    return primary->TransmitPoll(iAddress, true);

  CCECBusDevice *device = m_busDevices->At(CECDEVICE_UNREGISTERED);
  if (device)
    return device->TransmitPoll(iAddress, true);

  return false;
}

CCECBusDevice *CCECProcessor::GetDeviceByPhysicalAddress(uint16_t iPhysicalAddress, bool bSuppressUpdate /* = true */)
{
  return m_busDevices ?
      m_busDevices->GetDeviceByPhysicalAddress(iPhysicalAddress, bSuppressUpdate) :
      NULL;
}

CCECBusDevice *CCECProcessor::GetDevice(cec_logical_address address) const
{
  return m_busDevices ?
      m_busDevices->At(address) :
      NULL;
}

cec_logical_address CCECProcessor::GetActiveSource(bool bRequestActiveSource /* = true */)
{
  // get the device that is marked as active source from the device map
  CCECBusDevice *activeSource = m_busDevices->GetActiveSource();
  if (activeSource)
    return activeSource->GetLogicalAddress();

  if (bRequestActiveSource)
  {
    // request the active source from the bus
    CCECBusDevice *primary = GetPrimaryDevice();
    if (primary)
    {
      primary->RequestActiveSource();
      return GetActiveSource(false);
    }
  }

  // unknown or none
  return CECDEVICE_UNKNOWN;
}

bool CCECProcessor::IsActiveSource(cec_logical_address iAddress)
{
  CCECBusDevice *device = m_busDevices->At(iAddress);
  return device && device->IsActiveSource();
}

bool CCECProcessor::Transmit(const cec_command &data, bool bIsReply)
{
  cec_command transmitData(data);
  uint8_t iMaxTries(0);
  bool bRetry(true);
  uint8_t iTries(0);
  bool result(false);

  // get the current timeout setting
  uint8_t iLineTimeout(GetStandardLineTimeout());

  // reset the state of this message to 'unknown'
  cec_adapter_message_state adapterState = ADAPTER_MESSAGE_STATE_UNKNOWN;

  if (data.initiator == CECDEVICE_UNKNOWN && data.destination == CECDEVICE_UNKNOWN)
    return false;

  CLockObject lock(m_mutex);
  if (!m_communication)
    return false;

  if (!m_communication->SupportsSourceLogicalAddress(transmitData.initiator))
  {
    if (transmitData.initiator == CECDEVICE_UNREGISTERED && m_communication->SupportsSourceLogicalAddress(CECDEVICE_FREEUSE))
    {
      m_libcec->AddLog(CEC_LOG_DEBUG, "initiator '%s' is not supported by the CEC adapter. using '%s' instead", ToString(transmitData.initiator), ToString(CECDEVICE_FREEUSE));
      transmitData.initiator = CECDEVICE_FREEUSE;
    }
    else
    {
      m_libcec->AddLog(CEC_LOG_DEBUG, "initiator '%s' is not supported by the CEC adapter", ToString(transmitData.initiator));
      return false;
    }
  }

  transmitData.sent = true;

  // find the initiator device
  CCECBusDevice *initiator = m_busDevices->At(transmitData.initiator);
  if (!initiator)
  {
    m_libcec->AddLog(CEC_LOG_WARNING, "invalid initiator");
    transmitData.ack = 1;
    transmitData.eom = 0;
    LogOutput(transmitData);
    return false;
  }

  // find the destination device, if it's not the broadcast address
  if (transmitData.destination != CECDEVICE_BROADCAST)
  {
    // check if the device is marked as handled by libCEC
    CCECBusDevice *destination = m_busDevices->At(transmitData.destination);
    if (destination && destination->IsHandledByLibCEC())
    {
      // and reject the command if it's trying to send data to a device that is handled by libCEC
      m_libcec->AddLog(CEC_LOG_WARNING, "not sending data to myself!");
      transmitData.ack = 1;
      transmitData.eom = 0;
      LogOutput(transmitData);
      return false;
    }
  }

  // wait until we finished allocating a new LA if it got lost if this is not a poll
  if (data.opcode_set)
  {
    lock.Unlock();
    while (m_bStallCommunication) Sleep(5);
    lock.Lock();
  }

  m_iLastTransmission = GetTimeMs();
  // set the number of tries
  iMaxTries = initiator->GetHandler()->GetTransmitRetries() + 1;
  initiator->MarkHandlerReady();

  // and try to send the command
  while (bRetry && ++iTries < iMaxTries)
  {
    if (initiator->IsUnsupportedFeature(transmitData.opcode)) {
      transmitData.ack = 1;
      transmitData.eom = 0;
      LogOutput(transmitData);
      return false;
    }

    adapterState = !IsStopped() && m_communication && m_communication->IsOpen() ?
        m_communication->Write(transmitData, bRetry, iLineTimeout, bIsReply) :
        ADAPTER_MESSAGE_STATE_ERROR;

    result = bIsReply ?
      adapterState == ADAPTER_MESSAGE_STATE_SENT_ACKED || adapterState == ADAPTER_MESSAGE_STATE_SENT || adapterState == ADAPTER_MESSAGE_STATE_WAITING_TO_BE_SENT :
      adapterState == ADAPTER_MESSAGE_STATE_SENT_ACKED;

    transmitData.ack = transmitData.destination == CECDEVICE_BROADCAST ? result : !result;
    transmitData.eom = adapterState != ADAPTER_MESSAGE_STATE_ERROR;
    LogOutput(transmitData);

    iLineTimeout = m_iRetryLineTimeout;
  }

  return result;
}

void CCECProcessor::TransmitAbort(cec_logical_address source, cec_logical_address destination, cec_opcode opcode, cec_abort_reason reason /* = CEC_ABORT_REASON_UNRECOGNIZED_OPCODE */)
{
  m_libcec->AddLog(CEC_LOG_DEBUG, "<< transmitting abort message");

  cec_command command;
  cec_command::Format(command, source, destination, CEC_OPCODE_FEATURE_ABORT);
  command.parameters.PushBack((uint8_t)opcode);
  command.parameters.PushBack((uint8_t)reason);

  Transmit(command, true);
}

void CCECProcessor::ProcessCommand(const cec_command &command)
{
  if (command.eom) {
    // log the command
    m_libcec->AddLog(CEC_LOG_TRAFFIC, ToString(command).c_str());

    if (m_bRawTraffic)
      m_libcec->AddCommand(command, true);

    // find the initiator
    CCECBusDevice *device = m_busDevices->At(command.initiator);

    if (device)
      device->HandleCommand(command);
  } else if (m_bRawTraffic) { // Error packet handling.
      m_libcec->AddLog(CEC_LOG_TRAFFIC, ToString(command).c_str());
      m_libcec->AddCommand(command, true);
  }
}

bool CCECProcessor::IsPresentDevice(cec_logical_address address)
{
  CCECBusDevice *device = m_busDevices->At(address);
  return device && device->GetStatus() == CEC_DEVICE_STATUS_PRESENT;
}

bool CCECProcessor::IsPresentDeviceType(cec_device_type type)
{
  CECDEVICEVEC devices;
  m_busDevices->GetByType(type, devices);
  CCECDeviceMap::FilterActive(devices);
  return !devices.empty();
}

uint16_t CCECProcessor::GetDetectedPhysicalAddress(void) const
{
  return m_communication ? m_communication->GetPhysicalAddress() : CEC_INVALID_PHYSICAL_ADDRESS;
}

bool CCECProcessor::ClearLogicalAddresses(void)
{
  cec_logical_addresses addresses; addresses.Clear();
  return SetLogicalAddresses(addresses);
}

bool CCECProcessor::SetLogicalAddresses(const cec_logical_addresses &addresses)
{
  return m_communication ? m_communication->SetLogicalAddresses(addresses) : false;
}

bool CCECProcessor::StandbyDevices(const cec_logical_address initiator, const CECDEVICEVEC &devices)
{
  bool bReturn(true);
  for (CECDEVICEVEC::const_iterator it = devices.begin(); it != devices.end(); it++)
    bReturn &= (*it)->Standby(initiator);
  return bReturn;
}

bool CCECProcessor::StandbyDevice(const cec_logical_address initiator, cec_logical_address address)
{
  CCECBusDevice *device = m_busDevices->At(address);
  return device ? device->Standby(initiator) : false;
}

bool CCECProcessor::PowerOnDevices(const cec_logical_address initiator, const CECDEVICEVEC &devices)
{
  bool bReturn(true);
  for (CECDEVICEVEC::const_iterator it = devices.begin(); it != devices.end(); it++)
    bReturn &= (*it)->PowerOn(initiator);
  return bReturn;
}

bool CCECProcessor::PowerOnDevice(const cec_logical_address initiator, cec_logical_address address)
{
  CCECBusDevice *device = m_busDevices->At(address);
  return device ? device->PowerOn(initiator) : false;
}

bool CCECProcessor::StartBootloader(const char *strPort /* = NULL */)
{
  bool bReturn(false);
  // open a connection if no connection has been opened
  if (!m_communication && strPort)
  {
    CAdapterFactory factory(this->m_libcec);
    IAdapterCommunication *comm = factory.GetInstance(strPort);
    CTimeout timeout(CEC_DEFAULT_CONNECT_TIMEOUT);
    int iConnectTry(0);
    while (timeout.TimeLeft() > 0 && (bReturn = comm->Open(timeout.TimeLeft() / CEC_CONNECT_TRIES, true)) == false)
    {
      m_libcec->AddLog(CEC_LOG_ERROR, "could not open a connection (try %d)", ++iConnectTry);
      comm->Close();
      Sleep(CEC_DEFAULT_TRANSMIT_RETRY_WAIT);
    }
    if (comm->IsOpen())
    {
      bReturn = comm->StartBootloader();
      SAFE_DELETE(comm);
    }
    return bReturn;
  }
  else
  {
    m_communication->StartBootloader();
    Close();
    bReturn = true;
  }

  return bReturn;
}

bool CCECProcessor::PingAdapter(void)
{
  return m_communication->PingAdapter();
}

void CCECProcessor::HandlePoll(cec_logical_address initiator, cec_logical_address destination)
{
  CCECBusDevice *device = m_busDevices->At(destination);
  if (device)
    device->HandlePollFrom(initiator);
}

bool CCECProcessor::HandleReceiveFailed(cec_logical_address initiator)
{
  CCECBusDevice *device = m_busDevices->At(initiator);
  return !device || !device->HandleReceiveFailed();
}

bool CCECProcessor::CanSaveConfiguration(void)
{
  return !!m_communication ?
    m_communication->GetFirmwareVersion() >= 2 :
    false;
}

bool CCECProcessor::SaveConfiguration(const libcec_configuration &configuration)
{
  libcec_configuration save_config = configuration;
  if (!CLibCEC::IsValidPhysicalAddress(configuration.iPhysicalAddress))
  {
    CCECBusDevice *device = GetPrimaryDevice();
    if (!!device)
      save_config.iPhysicalAddress = device->GetCurrentPhysicalAddress();
  }

  return !!m_communication ?
    m_communication->SaveConfiguration(save_config) :
    false;
}

bool CCECProcessor::SetAutoMode(bool automode)
{
  return !!m_communication ?
    m_communication->SetAutoMode(automode) :
    false;
}

void CCECProcessor::RescanActiveDevices(void)
{
  for (CECDEVICEMAP::iterator it = m_busDevices->Begin(); it != m_busDevices->End(); it++)
    it->second->GetStatus(true);
}

bool CCECProcessor::GetDeviceInformation(const char *strPort, libcec_configuration *config, uint32_t iTimeoutMs /* = CEC_DEFAULT_CONNECT_TIMEOUT */)
{
  if (!OpenConnection(strPort, CEC_SERIAL_DEFAULT_BAUDRATE, iTimeoutMs, false))
    return false;

  config->iFirmwareVersion   = m_communication->GetFirmwareVersion();
  config->iPhysicalAddress   = m_communication->GetPhysicalAddress();
  config->iFirmwareBuildDate = m_communication->GetFirmwareBuildDate();
  config->adapterType        = m_communication->GetAdapterType();

  Close();

  return true;
}

bool CCECProcessor::TransmitPendingActiveSourceCommands(void)
{
  bool bReturn(true);
  for (CECDEVICEMAP::iterator it = m_busDevices->Begin(); it != m_busDevices->End(); it++)
    bReturn &= it->second->TransmitPendingActiveSourceCommands();
  return bReturn;
}

CCECTV *CCECProcessor::GetTV(void) const
{
  return CCECBusDevice::AsTV(m_busDevices->At(CECDEVICE_TV));
}

CCECAudioSystem *CCECProcessor::GetAudioSystem(void) const
{
  return CCECBusDevice::AsAudioSystem(m_busDevices->At(CECDEVICE_AUDIOSYSTEM));
}

CCECPlaybackDevice *CCECProcessor::GetPlaybackDevice(cec_logical_address address) const
{
  return CCECBusDevice::AsPlaybackDevice(m_busDevices->At(address));
}

CCECRecordingDevice *CCECProcessor::GetRecordingDevice(cec_logical_address address) const
{
  return CCECBusDevice::AsRecordingDevice(m_busDevices->At(address));
}

CCECTuner *CCECProcessor::GetTuner(cec_logical_address address) const
{
  return CCECBusDevice::AsTuner(m_busDevices->At(address));
}

bool CCECProcessor::AllocateLogicalAddresses(CECClientPtr client)
{
  libcec_configuration &configuration = *client->GetConfiguration();

  // mark as unregistered
  client->SetRegistered(false);

  // unregister this client from the old addresses
  CECDEVICEVEC devices;
  m_busDevices->GetByLogicalAddresses(devices, configuration.logicalAddresses);
  for (CECDEVICEVEC::const_iterator it = devices.begin(); it != devices.end(); it++)
  {
    // remove client entry
    CLockObject lock(m_mutex);
    m_clients.erase((*it)->GetLogicalAddress());
  }

  // find logical addresses for this client
  if (!client->AllocateLogicalAddresses())
  {
    m_libcec->AddLog(CEC_LOG_ERROR, "failed to find a free logical address for the client");
    return false;
  }

  // refresh the address
  if (configuration.bAutodetectAddress)
    client->AutodetectPhysicalAddress();

  // register this client on the new addresses
  devices.clear();
  m_busDevices->GetByLogicalAddresses(devices, configuration.logicalAddresses);
  for (CECDEVICEVEC::const_iterator it = devices.begin(); it != devices.end(); it++)
  {
    // set the physical address of the device at this LA
    if (CLibCEC::IsValidPhysicalAddress(configuration.iPhysicalAddress))
      (*it)->SetPhysicalAddress(configuration.iPhysicalAddress);

    // replace a previous client
    CLockObject lock(m_mutex);
    m_clients.erase((*it)->GetLogicalAddress());
    m_clients.insert(make_pair((*it)->GetLogicalAddress(), client));
  }

  // set the new ackmask
  SetLogicalAddresses(GetLogicalAddresses());

  // resume outgoing communication
  m_bStallCommunication = false;

  return true;
}

uint16_t CCECProcessor::GetPhysicalAddressFromEeprom(void)
{
  libcec_configuration config; config.Clear();
  if (m_communication)
    m_communication->GetConfiguration(config);
  return config.iPhysicalAddress;
}

bool CCECProcessor::RegisterClient(CCECClient* client)
{
  for (std::map<cec_logical_address, CECClientPtr>::iterator it = m_clients.begin(); it != m_clients.end(); ++it)
  {
    if (it->second.get() == client)
      return RegisterClient(it->second);
  }
  return RegisterClient(CECClientPtr(client));
}

bool CCECProcessor::RegisterClient(CECClientPtr client)
{
  if (!client)
    return false;

  libcec_configuration &configuration = *client->GetConfiguration();
  if (configuration.bMonitorOnly == 1)
    return true;

  if (!CECInitialised())
  {
    m_libcec->AddLog(CEC_LOG_ERROR, "failed to register a new CEC client: CEC processor is not initialised");
    return false;
  }

  // unregister the client first if it's already been marked as registered
  if (client->IsRegistered())
    UnregisterClient(client);

  // ensure that controlled mode is enabled
  m_communication->SetControlledMode(true);
  m_bMonitor = false;

  // source logical address for requests
  cec_logical_address sourceAddress(CECDEVICE_UNREGISTERED);
  if (!m_communication->SupportsSourceLogicalAddress(CECDEVICE_UNREGISTERED))
  {
    if (m_communication->SupportsSourceLogicalAddress(CECDEVICE_FREEUSE))
      sourceAddress = CECDEVICE_FREEUSE;
    else
    {
      m_libcec->AddLog(CEC_LOG_ERROR, "failed to register a new CEC client: both unregistered and free use are not supported by the device");
      return false;
    }
  }

  // ensure that we know the vendor id of the TV
  CCECBusDevice *tv = GetTV();
  cec_vendor_id tvVendor(tv->GetVendorId(sourceAddress));

  // wait until the handler is replaced, to avoid double registrations
  if (tvVendor != CEC_VENDOR_UNKNOWN &&
      CCECCommandHandler::HasSpecificHandler(tvVendor))
  {
    while (!tv->ReplaceHandler(false))
      CEvent::Sleep(5);
  }

  // get the configuration from the client
  m_libcec->AddLog(CEC_LOG_DEBUG, "registering new CEC client - v%s", CCECTypeUtils::VersionToString(configuration.clientVersion).c_str());

  // get the current ackmask, so we can restore it if polling fails
  cec_logical_addresses previousMask = GetLogicalAddresses();

  // mark as uninitialised
  client->SetInitialised(false);

  // get the settings from eeprom
  {
    libcec_configuration config; config.Clear();
    m_communication->GetConfiguration(config);
    {
      CLockObject lock(m_mutex);
      if (configuration.bGetSettingsFromROM == 1)
      {
        if (!config.deviceTypes.IsEmpty())
          configuration.deviceTypes = config.deviceTypes;
        if (CLibCEC::IsValidPhysicalAddress(config.iPhysicalAddress))
          configuration.iPhysicalAddress = config.iPhysicalAddress;
        snprintf(configuration.strDeviceName, LIBCEC_OSD_NAME_SIZE, "%s", config.strDeviceName);
      }
#if CEC_LIB_VERSION_MAJOR >= 5
      // always load the configured auto power on setting
      configuration.bAutoPowerOn = config.bAutoPowerOn;
#endif
    }
    client->SetConfiguration(configuration);
  }

  // find logical addresses for this client
  if (!AllocateLogicalAddresses(client))
  {
    m_libcec->AddLog(CEC_LOG_ERROR, "failed to register the new CEC client - cannot allocate the requested device types");
    SetLogicalAddresses(previousMask);
    return false;
  }

  // set the firmware version and build date
  configuration.serverVersion      = LIBCEC_VERSION_CURRENT;
  configuration.iFirmwareVersion   = m_communication->GetFirmwareVersion();
  configuration.iFirmwareBuildDate = m_communication->GetFirmwareBuildDate();
  configuration.adapterType        = m_communication->GetAdapterType();

  // mark the client as registered
  client->SetRegistered(true);

  sourceAddress = client->GetPrimaryLogicalAddress();

  // initialise the client
  bool bReturn = client->OnRegister();

  // log the new registration
  std::string strLog;
  strLog = StringUtils::Format("%s: %s", bReturn ? "CEC client registered" : "failed to register the CEC client", client->GetConnectionInfo().c_str());
  m_libcec->AddLog(bReturn ? CEC_LOG_NOTICE : CEC_LOG_ERROR, strLog.c_str());

  // display a warning if the firmware can be upgraded
  if (bReturn && !IsRunningLatestFirmware())
  {
    const char *strUpgradeMessage = "The firmware of this adapter can be upgraded. Please visit http://blog.pulse-eight.com/ for more information.";
    m_libcec->AddLog(CEC_LOG_WARNING, strUpgradeMessage);
    libcec_parameter param;
    param.paramData = (void*)strUpgradeMessage; param.paramType = CEC_PARAMETER_TYPE_STRING;
    client->Alert(CEC_ALERT_SERVICE_DEVICE, param);
  }

  // ensure that the command handler for the TV is initialised
  if (bReturn)
  {
    CCECCommandHandler *handler = GetTV()->GetHandler();
    if (handler)
      handler->InitHandler();
    GetTV()->MarkHandlerReady();
  }

  // report our OSD name to the TV, since some TVs don't request it
  client->GetPrimaryDevice()->TransmitOSDName(CECDEVICE_TV, false);

  // request the power status of the TV
  tv->RequestPowerStatus(sourceAddress, true, true);
  return bReturn;
}

bool CCECProcessor::UnregisterClient(CCECClient* client)
{
  for (std::map<cec_logical_address, CECClientPtr>::iterator it = m_clients.begin(); it != m_clients.end(); ++it)
  {
    if (it->second.get() == client)
      return UnregisterClient(it->second);
  }
  return true;
}

bool CCECProcessor::UnregisterClient(CECClientPtr client)
{
  if (!client)
    return false;

  if (client->IsRegistered())
    m_libcec->AddLog(CEC_LOG_NOTICE, "unregistering client: %s", client->GetConnectionInfo().c_str());

  // notify the client that it will be unregistered
  client->OnUnregister();

  {
    CLockObject lock(m_mutex);
    // find all devices that match the LA's of this client
    CECDEVICEVEC devices;
    m_busDevices->GetByLogicalAddresses(devices, client->GetConfiguration()->logicalAddresses);
    for (CECDEVICEVEC::const_iterator it = devices.begin(); it != devices.end(); it++)
    {
      // find the client
      std::map<cec_logical_address, CECClientPtr>::iterator entry = m_clients.find((*it)->GetLogicalAddress());
      // unregister the client
      if (entry != m_clients.end())
        m_clients.erase(entry);

      // reset the device status
      (*it)->ResetDeviceStatus(true);
    }
  }

  // set the new ackmask
  cec_logical_addresses addresses = GetLogicalAddresses();
  if (SetLogicalAddresses(addresses))
  {
    // no more clients left, disable controlled mode
    if (addresses.IsEmpty() && !m_bMonitor)
      m_communication->SetControlledMode(false);

    return true;
  }

  return false;
}

void CCECProcessor::UnregisterClients(void)
{
  m_libcec->AddLog(CEC_LOG_DEBUG, "unregistering all CEC clients");

  std::vector<CECClientPtr> clients = m_libcec->GetClients();
  for (std::vector<CECClientPtr>::iterator it = clients.begin(); it != clients.end(); ++it)
    UnregisterClient(*it);
}

CECClientPtr CCECProcessor::GetClient(const cec_logical_address address)
{
  CLockObject lock(m_mutex);
  std::map<cec_logical_address, CECClientPtr>::const_iterator client = m_clients.find(address);
  if (client != m_clients.end())
    return client->second;
  return CECClientPtr();
}

CECClientPtr CCECProcessor::GetPrimaryClient(void)
{
  CLockObject lock(m_mutex);
  std::map<cec_logical_address, CECClientPtr>::const_iterator client = m_clients.begin();
  if (client != m_clients.end())
    return client->second;
  return CECClientPtr();
}

CCECBusDevice *CCECProcessor::GetPrimaryDevice(void)
{
  return m_busDevices->At(GetLogicalAddress());
}

cec_logical_address CCECProcessor::GetLogicalAddress(void)
{
  cec_logical_addresses addresses = GetLogicalAddresses();
  return addresses.primary;
}

cec_logical_addresses CCECProcessor::GetLogicalAddresses(void)
{
  CLockObject lock(m_mutex);
  cec_logical_addresses addresses;
  addresses.Clear();
  for (std::map<cec_logical_address, CECClientPtr>::const_iterator client = m_clients.begin(); client != m_clients.end(); client++)
    addresses.Set(client->first);

  return addresses;
}

bool CCECProcessor::IsHandledByLibCEC(const cec_logical_address address) const
{
  CCECBusDevice *device = GetDevice(address);
  return device && device->IsHandledByLibCEC();
}

bool CCECProcessor::IsRunningLatestFirmware(void)
{
  return m_communication && m_communication->IsOpen() ?
      m_communication->IsRunningLatestFirmware() :
      true;
}

void CCECProcessor::SwitchMonitoring(bool bSwitchTo)
{
  {
    CLockObject lock(m_mutex);
    m_bMonitor = bSwitchTo;
  }
  if (bSwitchTo)
    UnregisterClients();
}

void CCECProcessor::SwitchRawTraffic(bool bSwitchTo)
{
  {
    CLockObject lock(m_mutex);
    m_bRawTraffic = bSwitchTo;
    if (m_communication)
      m_communication->SetRawTrafficMode(bSwitchTo);
    m_inBuffer.SwitchRawTraffic(bSwitchTo);
  }
}

void CCECProcessor::HandleLogicalAddressLost(cec_logical_address oldAddress)
{
  m_libcec->AddLog(CEC_LOG_NOTICE, "logical address %x was taken by another device, allocating a new address", oldAddress);

  // stall outgoing messages until we know our new LA
  m_bStallCommunication = true;

  // reset the TV and the previous address
  GetTV()->SetDeviceStatus(CEC_DEVICE_STATUS_UNKNOWN);
  if (oldAddress < CECDEVICE_BROADCAST)
    m_busDevices->At(oldAddress)->SetDeviceStatus(CEC_DEVICE_STATUS_UNKNOWN);

  // try to detect the vendor id
  GetTV()->GetVendorId(CECDEVICE_UNREGISTERED);

  CECClientPtr client = GetClient(oldAddress);
  if (!client)
    client = GetPrimaryClient();
  if (client)
  {
    if (m_addrAllocator)
      while (m_addrAllocator->IsRunning()) Sleep(5);
    delete m_addrAllocator;

    m_addrAllocator = new CCECAllocateLogicalAddress(this, client);
    m_addrAllocator->CreateThread();
  }
}

void CCECProcessor::HandlePhysicalAddressChanged(uint16_t iNewAddress)
{
  if (!m_bStallCommunication) {
    CECClientPtr client = GetPrimaryClient();
    if (!!client)
      client->SetPhysicalAddress(iNewAddress);
  }
}

uint16_t CCECProcessor::GetAdapterVendorId(void) const
{
  return m_communication ? m_communication->GetAdapterVendorId() : 0;
}

uint16_t CCECProcessor::GetAdapterProductId(void) const
{
  return m_communication ? m_communication->GetAdapterProductId() : 0;
}

CCECAllocateLogicalAddress::CCECAllocateLogicalAddress(CCECProcessor* processor, CECClientPtr client) :
    m_processor(processor),
    m_client(client) { }

void* CCECAllocateLogicalAddress::Process(void)
{
  m_processor->AllocateLogicalAddresses(m_client);
  return NULL;
}
