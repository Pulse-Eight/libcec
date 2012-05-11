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

#include "CECProcessor.h"

#include "adapter/USBCECAdapterCommunication.h"
#include "devices/CECBusDevice.h"
#include "devices/CECAudioSystem.h"
#include "devices/CECPlaybackDevice.h"
#include "devices/CECRecordingDevice.h"
#include "devices/CECTuner.h"
#include "devices/CECTV.h"
#include "implementations/CECCommandHandler.h"
#include "LibCEC.h"
#include "CECClient.h"
#include "platform/util/timeutils.h"

using namespace CEC;
using namespace std;
using namespace PLATFORM;

#define CEC_PROCESSOR_SIGNAL_WAIT_TIME 1000

#define ToString(x) m_libcec->ToString(x)

CCECProcessor::CCECProcessor(CLibCEC *libcec) :
    m_bInitialised(false),
    m_communication(NULL),
    m_libcec(libcec),
    m_bMonitor(false),
    m_iPreviousAckMask(0),
    m_iStandardLineTimeout(3),
    m_iRetryLineTimeout(3),
    m_iLastTransmission(0)
{
  m_busDevices = new CCECDeviceMap(this);
}

CCECProcessor::~CCECProcessor(void)
{
  Close();
  delete m_busDevices;
}

bool CCECProcessor::Start(const char *strPort, uint16_t iBaudRate /* = CEC_SERIAL_DEFAULT_BAUDRATE */, uint32_t iTimeoutMs /* = CEC_DEFAULT_CONNECT_TIMEOUT */)
{
  CLockObject lock(m_mutex);
  if (!OpenConnection(strPort, iBaudRate, iTimeoutMs))
    return false;

  /* create the processor thread */
  if (!IsRunning())
  {
    if (CreateThread())
      m_libcec->AddLog(CEC_LOG_DEBUG, "processor thread started");
    else
    {
      m_libcec->AddLog(CEC_LOG_ERROR, "could not create a processor thread");
      return false;
    }
  }

  SetCECInitialised(true);

  return true;
}

void CCECProcessor::Close(void)
{
  SetCECInitialised(false);
  StopThread();

  if (m_communication)
  {
    delete m_communication;
    m_communication = NULL;
  }

  m_bMonitor = false;
  m_iPreviousAckMask = 0;
  m_iStandardLineTimeout = 3;
  m_iRetryLineTimeout = 3;
  m_iLastTransmission = 0;
  m_busDevices->ResetDeviceStatus();
}

bool CCECProcessor::OpenConnection(const char *strPort, uint16_t iBaudRate, uint32_t iTimeoutMs, bool bStartListening /* = true */)
{
  bool bReturn(false);
  Close();

  {
    CLockObject lock(m_mutex);
    if (m_communication && m_communication->IsOpen())
    {
      m_libcec->AddLog(CEC_LOG_ERROR, "connection already opened");
      return true;
    }
    else if (!m_communication)
      m_communication = new CUSBCECAdapterCommunication(this, strPort, iBaudRate);
  }

  CTimeout timeout(iTimeoutMs > 0 ? iTimeoutMs : CEC_DEFAULT_TRANSMIT_WAIT);

  /* open a new connection */
  unsigned iConnectTry(0);
  while (timeout.TimeLeft() > 0 && (bReturn = m_communication->Open((timeout.TimeLeft() / CEC_CONNECT_TRIES), false, bStartListening)) == false)
  {
    m_libcec->AddLog(CEC_LOG_ERROR, "could not open a connection (try %d)", ++iConnectTry);
    m_communication->Close();
    CEvent::Sleep(CEC_DEFAULT_CONNECT_RETRY_WAIT);
  }

  m_libcec->AddLog(CEC_LOG_NOTICE, "connection opened");

  return bReturn;
}

bool CCECProcessor::CECInitialised(void)
{
  CLockObject lock(m_threadMutex);
  return m_bInitialised;
}

void CCECProcessor::SetCECInitialised(bool bSetTo /* = true */)
{
  CLockObject lock(m_mutex);
  m_bInitialised = bSetTo;
  if (!bSetTo)
    UnregisterClients();
}

bool CCECProcessor::TryLogicalAddress(cec_logical_address address)
{
  CCECBusDevice *device = m_busDevices->At(address);
  if (device)
  {
    if (device->IsPresent() || device->IsHandledByLibCEC())
      return false;

    SetAckMask(0);
    return device->TryLogicalAddress();
  }

  return false;
}

void CCECProcessor::ReplaceHandlers(void)
{
  if (!CECInitialised())
    return;

  for (CECDEVICEMAP::iterator it = m_busDevices->Begin(); it != m_busDevices->End(); it++)
    it->second->ReplaceHandler(true);
}

bool CCECProcessor::OnCommandReceived(const cec_command &command)
{
  return m_inBuffer.Push(command);
}

void *CCECProcessor::Process(void)
{
  m_libcec->AddLog(CEC_LOG_DEBUG, "processor thread started");

  cec_command command;
  command.Clear();

  while (!IsStopped() && m_communication->IsOpen())
  {
    if (m_inBuffer.Pop(command, CEC_PROCESSOR_SIGNAL_WAIT_TIME))
      ParseCommand(command);

    if (CECInitialised())
    {
      ReplaceHandlers();

      m_libcec->CheckKeypressTimeout();
    }
  }

  return NULL;
}

bool CCECProcessor::SetActiveSource(uint16_t iStreamPath)
{
  bool bReturn(false);

  CCECBusDevice *device = GetDeviceByPhysicalAddress(iStreamPath);
  if (device)
    bReturn = device->ActivateSource();
  else
    m_libcec->AddLog(CEC_LOG_DEBUG, "device with PA '%04x' not found", iStreamPath);

  return bReturn;
}

void CCECProcessor::SetStandardLineTimeout(uint8_t iTimeout)
{
  CLockObject lock(m_mutex);
  m_iStandardLineTimeout = iTimeout;
}

void CCECProcessor::SetRetryLineTimeout(uint8_t iTimeout)
{
  CLockObject lock(m_mutex);
  m_iRetryLineTimeout = iTimeout;
}

bool CCECProcessor::PhysicalAddressInUse(uint16_t iPhysicalAddress)
{
  CCECBusDevice *device = GetDeviceByPhysicalAddress(iPhysicalAddress);
  return device != NULL;
}

void CCECProcessor::LogOutput(const cec_command &data)
{
  CStdString strTx;
  strTx.Format("<< %02x", ((uint8_t)data.initiator << 4) + (uint8_t)data.destination);
  if (data.opcode_set)
      strTx.AppendFormat(":%02x", (uint8_t)data.opcode);

  for (uint8_t iPtr = 0; iPtr < data.parameters.size; iPtr++)
    strTx.AppendFormat(":%02x", data.parameters[iPtr]);
  m_libcec->AddLog(CEC_LOG_TRAFFIC, strTx.c_str());
}

bool CCECProcessor::SwitchMonitoring(bool bEnable)
{
  m_libcec->AddLog(CEC_LOG_NOTICE, "== %s monitoring mode ==", bEnable ? "enabling" : "disabling");

  {
    CLockObject lock(m_mutex);
    m_bMonitor = bEnable;
    m_iPreviousAckMask = m_communication->GetAckMask();
  }

  if (bEnable)
    return SetAckMask(0);
  else
    return SetAckMask(m_iPreviousAckMask);
}

bool CCECProcessor::PollDevice(cec_logical_address iAddress)
{
  CCECBusDevice *device = m_busDevices->At(iAddress);
  CCECBusDevice *primary = GetPrimaryDevice();
  if (device)
  {
    return primary ?
        primary->TransmitPoll(iAddress) :
        device->TransmitPoll(iAddress);
  }
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

bool CCECProcessor::Transmit(const cec_command &data)
{
  CCECBusDevice *initiator = m_busDevices->At(data.initiator);
  if (!initiator)
  {
    m_libcec->AddLog(CEC_LOG_WARNING, "invalid initiator");
    return false;
  }

  if (data.destination != CECDEVICE_BROADCAST)
  {
    CCECBusDevice *destination = m_busDevices->At(data.destination);
    if (destination && destination->IsHandledByLibCEC())
    {
      m_libcec->AddLog(CEC_LOG_WARNING, "not sending data to myself!");
      return false;
    }
  }

  uint8_t iMaxTries(0);
  {
    CLockObject lock(m_mutex);
    if (IsStopped())
      return false;
    LogOutput(data);
    m_iLastTransmission = GetTimeMs();
    if (!m_communication || !m_communication->IsOpen())
    {
      m_libcec->AddLog(CEC_LOG_ERROR, "cannot transmit command: connection closed");
      return false;
    }
    iMaxTries = initiator->GetHandler()->GetTransmitRetries() + 1;
  }

  bool bRetry(true);
  uint8_t iTries(0);
  uint8_t iLineTimeout = m_iStandardLineTimeout;
  cec_adapter_message_state adapterState = ADAPTER_MESSAGE_STATE_UNKNOWN;

  while (bRetry && ++iTries < iMaxTries)
  {
    if (initiator->IsUnsupportedFeature(data.opcode))
      return false;

    adapterState = m_communication->Write(data, bRetry, iLineTimeout);
    iLineTimeout = m_iRetryLineTimeout;
  }

  return adapterState == ADAPTER_MESSAGE_STATE_SENT_ACKED;
}

void CCECProcessor::TransmitAbort(cec_logical_address source, cec_logical_address destination, cec_opcode opcode, cec_abort_reason reason /* = CEC_ABORT_REASON_UNRECOGNIZED_OPCODE */)
{
  m_libcec->AddLog(CEC_LOG_DEBUG, "<< transmitting abort message");

  cec_command command;
  cec_command::Format(command, source, destination, CEC_OPCODE_FEATURE_ABORT);
  command.parameters.PushBack((uint8_t)opcode);
  command.parameters.PushBack((uint8_t)reason);

  Transmit(command);
}

void CCECProcessor::ParseCommand(const cec_command &command)
{
  CStdString dataStr;
  dataStr.Format(">> %1x%1x", command.initiator, command.destination);
  if (command.opcode_set == 1)
    dataStr.AppendFormat(":%02x", command.opcode);
  for (uint8_t iPtr = 0; iPtr < command.parameters.size; iPtr++)
    dataStr.AppendFormat(":%02x", (unsigned int)command.parameters[iPtr]);
  m_libcec->AddLog(CEC_LOG_TRAFFIC, dataStr.c_str());

  if (!m_bMonitor)
  {
    CCECBusDevice *device = m_busDevices->At(command.initiator);
    if (device)
      device->HandleCommand(command);
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

bool CCECProcessor::SetAckMask(uint16_t iMask)
{
  return m_communication ? m_communication->SetAckMask(iMask) : false;
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
  if (!m_communication && strPort)
  {
    IAdapterCommunication *comm = new CUSBCECAdapterCommunication(this, strPort);
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
      delete comm;
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

bool CCECProcessor::SetStreamPath(uint16_t iPhysicalAddress)
{
  // stream path changes are sent by the TV
  return GetTV()->GetHandler()->TransmitSetStreamPath(iPhysicalAddress);
}

bool CCECProcessor::CanPersistConfiguration(void)
{
  return m_communication ? m_communication->GetFirmwareVersion() >= 2 : false;
}

bool CCECProcessor::PersistConfiguration(libcec_configuration *configuration)
{
  return m_communication ? m_communication->PersistConfiguration(configuration) : false;
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

bool CCECProcessor::RegisterClient(CCECClient *client)
{
  if (!client)
    return false;

  libcec_configuration &configuration = *client->GetConfiguration();
  m_libcec->AddLog(CEC_LOG_NOTICE, "registering new CEC client - v%s", ToString((cec_client_version)configuration.clientVersion));

  client->SetRegistered(false);
  client->SetInitialised(false);

  uint16_t iPreviousMask(m_communication->GetAckMask());

  // find logical addresses for this client
  if (!client->FindLogicalAddresses())
  {
    SetAckMask(iPreviousMask);
    return false;
  }

  // register this client on the new addresses
  CECDEVICEVEC devices;
  m_busDevices->GetByLogicalAddresses(devices, configuration.logicalAddresses);
  for (CECDEVICEVEC::const_iterator it = devices.begin(); it != devices.end(); it++)
  {
    CLockObject lock(m_mutex);
    m_clients.erase((*it)->GetLogicalAddress());
    m_clients.insert(make_pair<cec_logical_address, CCECClient *>((*it)->GetLogicalAddress(), client));
    client->SetRegistered(true);
  }

  // get the settings from the rom
  if (configuration.bGetSettingsFromROM == 1)
  {
    libcec_configuration config;
    m_communication->GetConfiguration(&config);

    CLockObject lock(m_mutex);
    if (!config.deviceTypes.IsEmpty())
      configuration.deviceTypes = config.deviceTypes;
    if (CLibCEC::IsValidPhysicalAddress(config.iPhysicalAddress))
      configuration.iPhysicalAddress = config.iPhysicalAddress;
    snprintf(configuration.strDeviceName, 13, "%s", config.strDeviceName);
  }

  // set the new ack mask
  bool bReturn = SetAckMask(GetLogicalAddresses().AckMask()) &&
      client->Initialise();

  // set the firmware version and build date
  configuration.serverVersion      = LIBCEC_VERSION_CURRENT;
  configuration.iFirmwareVersion   = m_communication->GetFirmwareVersion();
  configuration.iFirmwareBuildDate = m_communication->GetFirmwareBuildDate();

  CStdString strLog;
  if (bReturn)
    strLog = "CEC client registered.";
  else
    strLog = "failed to register the CEC client.";
  strLog.AppendFormat(" libCEC version = %s, client version = %s, firmware version = %d", ToString((cec_server_version)configuration.serverVersion), ToString((cec_client_version)configuration.clientVersion), configuration.iFirmwareVersion);
  if (configuration.iFirmwareBuildDate != CEC_FW_BUILD_UNKNOWN)
  {
    time_t buildTime = (time_t)configuration.iFirmwareBuildDate;
    strLog.AppendFormat(", firmware build date: %s", asctime(gmtime(&buildTime)));
    strLog = strLog.Left((int)strLog.length() - 1); // strip \n added by asctime
    strLog.append(" +0000");
  }

  m_libcec->AddLog(bReturn ? CEC_LOG_NOTICE : CEC_LOG_ERROR, strLog);

  if (bReturn)
  {
    strLog = "Using logical address(es): ";
    CECDEVICEVEC devices;
    m_busDevices->GetByLogicalAddresses(devices, configuration.logicalAddresses);
    for (CECDEVICEVEC::iterator it = devices.begin(); it != devices.end(); it++)
      strLog.AppendFormat("%s (%X) ", (*it)->GetLogicalAddressName(), (*it)->GetLogicalAddress());
    m_libcec->AddLog(CEC_LOG_NOTICE, strLog);
  }

  // display a warning if the firmware can be upgraded
  if (!m_communication->IsRunningLatestFirmware())
  {
    const char *strUpgradeMessage = "The firmware of this adapter can be upgraded. Please visit http://blog.pulse-eight.com/ for more information.";
    m_libcec->AddLog(CEC_LOG_WARNING, strUpgradeMessage);
    libcec_parameter param;
    param.paramData = (void*)strUpgradeMessage; param.paramType = CEC_PARAMETER_TYPE_STRING;
    client->Alert(CEC_ALERT_SERVICE_DEVICE, param);
  }
  else
  {
    m_libcec->AddLog(CEC_LOG_DEBUG, "the adapter is using the latest (known) firmware version");
  }

  if (bReturn)
  {
    /* get the vendor id from the TV, so we are using the correct handler */
    GetTV()->GetVendorId(configuration.logicalAddresses.primary);
  }

  return bReturn;
}

void CCECProcessor::UnregisterClient(CCECClient *client)
{
  CLockObject lock(m_mutex);
  CECDEVICEVEC devices;
  m_busDevices->GetByLogicalAddresses(devices, client->GetConfiguration()->logicalAddresses);
  for (CECDEVICEVEC::const_iterator it = devices.begin(); it != devices.end(); it++)
  {
    map<cec_logical_address, CCECClient *>::iterator entry = m_clients.find((*it)->GetLogicalAddress());
    if (entry != m_clients.end())
      m_clients.erase(entry);
  }
}

void CCECProcessor::UnregisterClients(void)
{
  CLockObject lock(m_mutex);
  for (map<cec_logical_address, CCECClient *>::iterator client = m_clients.begin(); client != m_clients.end(); client++)
    client->second->OnUnregister();
  m_clients.clear();
}

CCECClient *CCECProcessor::GetClient(const cec_logical_address address)
{
  CLockObject lock(m_mutex);
  map<cec_logical_address, CCECClient *>::const_iterator client = m_clients.find(address);
  if (client != m_clients.end())
    return client->second;
  return NULL;
}

CCECClient *CCECProcessor::GetPrimaryClient(void)
{
  CLockObject lock(m_mutex);
  map<cec_logical_address, CCECClient *>::const_iterator client = m_clients.begin();
  if (client != m_clients.end())
    return client->second;
  return NULL;
}

CCECBusDevice *CCECProcessor::GetPrimaryDevice(void) const
{
  return m_busDevices->At(GetLogicalAddress());
}

cec_logical_address CCECProcessor::GetLogicalAddress(void) const
{
  cec_logical_addresses addresses = GetLogicalAddresses();
  return addresses.primary;
}

cec_logical_addresses CCECProcessor::GetLogicalAddresses(void) const
{
  cec_logical_addresses addresses;
  addresses.Clear();
  for (map<cec_logical_address, CCECClient *>::const_iterator client = m_clients.begin(); client != m_clients.end(); client++)
    addresses.Set(client->first);

  return addresses;
}

bool CCECProcessor::IsHandledByLibCEC(const cec_logical_address address) const
{
  CCECBusDevice *device = GetDevice(address);
  return device && device->IsHandledByLibCEC();
}
