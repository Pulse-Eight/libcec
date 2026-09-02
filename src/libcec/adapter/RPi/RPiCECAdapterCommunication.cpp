/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2026 Pulse-Eight Limited.  All rights reserved.
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

#if defined(HAVE_RPI_API)
#include "RPiCECAdapterCommunication.h"

extern "C" {
#include <bcm_host.h>
}

#include <unistd.h>

#include "CECTypeUtils.h"
#include "LibCEC.h"
#include "RPiCECAdapterMessageQueue.h"

using namespace CEC;

#define LIB_CEC m_callback->GetLib()

static bool g_bHostInited = false;

// how long the dispatch thread blocks on the callback queue before it rechecks
// whether it has been asked to stop
#define CEC_RPI_CALLBACK_POLL_MS 500

// callback for the RPi CEC service
void rpi_cec_callback(void *callback_data, uint32_t header, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3)
{
  if (callback_data)
    static_cast<CRPiCECAdapterCommunication *>(callback_data)->QueueCallback(false, header, p0, p1, p2, p3);
}

// callback for the TV service
void rpi_tv_callback(void *callback_data, uint32_t reason, uint32_t p0, uint32_t p1)
{
  if (callback_data)
    static_cast<CRPiCECAdapterCommunication *>(callback_data)->QueueCallback(true, reason, p0, p1, 0, 0);
}

CRPiCECAdapterCommunication::CRPiCECAdapterCommunication(IAdapterCommunicationCallback *callback) :
    IAdapterCommunication(callback),
    m_bInitialised(false),
    m_logicalAddress(CECDEVICE_UNKNOWN),
    m_bLogicalAddressChanged(false),
    m_previousLogicalAddress(CECDEVICE_FREEUSE),
    m_bLogicalAddressRegistered(false),
    m_bDisableCallbacks(false),
    m_iDroppedCallbacks(0),
    m_busChanges(this)
{
  m_queue = new CRPiCECAdapterMessageQueue(this);
}

CRPiCECAdapterCommunication::~CRPiCECAdapterCommunication(void)
{
  // everything here depends on what is still up when it runs:
  // - the LA release is confirmed by a callback, so it needs the callbacks
  //   registered and the dispatch thread draining them
  // - vc_cec_set_passive() needs the videocore host, which Close() ends
  // - OnDataReceived() touches m_queue, so it is freed once Close() has
  //   stopped the thread
  UnregisterLogicalAddress();
  vc_cec_set_passive(false);
  Close();
  delete(m_queue);
}

const char *ToString(const VC_CEC_ERROR_T error)
{
  switch(error)
  {
  case VC_CEC_SUCCESS:
    return "success";
  case VC_CEC_ERROR_NO_ACK:
    return "no ack";
  case VC_CEC_ERROR_SHUTDOWN:
    return "shutdown";
  case VC_CEC_ERROR_BUSY:
    return "device is busy";
  case VC_CEC_ERROR_NO_LA:
    return "no logical address";
  case VC_CEC_ERROR_NO_PA:
    return "no physical address";
  case VC_CEC_ERROR_NO_TOPO:
    return "no topology";
  case VC_CEC_ERROR_INVALID_FOLLOWER:
    return "invalid follower";
  case VC_CEC_ERROR_INVALID_ARGUMENT:
    return "invalid arg";
  default:
    return "unknown";
  }
}

bool CRPiCECAdapterCommunication::IsInitialised(void)
{
  CLockObject lock(m_mutex);
  return m_bInitialised;
}

/*!
 * @brief Copy one VideoCore userland callback and hand it to the dispatch thread.
 *
 * The copy is all that may happen here. The thread VideoCore delivers callbacks
 * on has a 4KiB stack, and formatting a single log message needs some 1.6KiB
 * more than this does. vcos asks for that size (VCOS_DEFAULT_STACK_SIZE) without
 * checking the result, so glibc, whose PTHREAD_STACK_MIN is far larger, refuses
 * and leaves the thread with the 8MiB default, while musl grants it.
 *
 * Callbacks are dropped rather than queued while they are disabled: that is what
 * keeps Write()'s POLL sequence from reporting the LA changes it causes itself,
 * and what stops a callback racing Close() from reaching an m_callback that is
 * already gone.
 */
void CRPiCECAdapterCommunication::QueueCallback(bool bTVService, uint32_t header, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3)
{
  {
    CLockObject lock(m_mutex);
    if (m_bDisableCallbacks)
      return;
  }

  rpi_callback callback = { bTVService, header, p0, p1, p2, p3 };
  if (!m_callbacks.Push(callback))
    ++m_iDroppedCallbacks;
}

void *CRPiCECAdapterBusChangeThread::Process(void)
{
  rpi_bus_change change;

  while (!IsStopped())
  {
    if (!m_changes.Pop(change, CEC_RPI_CALLBACK_POLL_MS))
      continue;

    if (change.bAddressLost)
      m_com->m_callback->HandleLogicalAddressLost((cec_logical_address)change.address);
    else
      m_com->m_callback->HandlePhysicalAddressChanged(change.address);
  }

  return NULL;
}

void *CRPiCECAdapterCommunication::Process(void)
{
  rpi_callback callback;

  while (!IsStopped())
  {
    if (m_callbacks.Pop(callback, CEC_RPI_CALLBACK_POLL_MS))
    {
      if (callback.bTVService)
        OnTVServiceCallback(callback.header, callback.p0, callback.p1);
      else
        OnDataReceived(callback.header, callback.p0, callback.p1, callback.p2, callback.p3);
    }

    const uint32_t iDropped = m_iDroppedCallbacks.exchange(0);
    if (iDropped > 0)
      LIB_CEC->AddLog(CEC_LOG_WARNING, "dropped %u callback(s) from VideoCore: the handler is not keeping up", iDropped);
  }

  return NULL;
}

void CRPiCECAdapterCommunication::OnTVServiceCallback(uint32_t reason, uint32_t UNUSED(p0), uint32_t UNUSED(p1))
{
  switch(reason)
  {
  case VC_HDMI_ATTACHED:
  {
    // vc_cec_get_physical_address() is a plain read, so it stays here; reporting
    // the address is what transmits, and that goes to the bus change thread
    rpi_bus_change change = { false, GetPhysicalAddress() };
    if (!m_busChanges.Queue(change))
      LIB_CEC->AddLog(CEC_LOG_WARNING, "dropped a physical address change: the handler is not keeping up");
    break;
  }
  case VC_HDMI_UNPLUGGED:
  case VC_HDMI_DVI:
  case VC_HDMI_HDMI:
  case VC_HDMI_HDCP_UNAUTH:
  case VC_HDMI_HDCP_AUTH:
  case VC_HDMI_HDCP_KEY_DOWNLOAD:
  case VC_HDMI_HDCP_SRM_DOWNLOAD:
  default:
     break;
  }
}

void CRPiCECAdapterCommunication::OnDataReceived(uint32_t header, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3)
{
  VC_CEC_NOTIFY_T reason = (VC_CEC_NOTIFY_T)CEC_CB_REASON(header);

#ifdef CEC_DEBUGGING
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "received data: header:%08X p0:%08X p1:%08X p2:%08X p3:%08X reason:%x", header, p0, p1, p2, p3, reason);
#endif

  switch (reason)
  {
  case VC_CEC_RX:
    // CEC data received
    {
      // translate into a VC_CEC_MESSAGE_T
      VC_CEC_MESSAGE_T message;
      vc_cec_param2message(header, p0, p1, p2, p3, &message);

      // translate to a cec_command
      cec_command command;
      cec_command::Format(command,
          (cec_logical_address)message.initiator,
          (cec_logical_address)message.follower,
          (cec_opcode)CEC_CB_OPCODE(p0));

      // copy parameters. videocore sets the uint32_t message.length to
      // CEC_CB_MSG_LENGTH(header) - 1 without clamping it, so a reported length
      // of 0 underflows to 0xFFFFFFFF and wraps the uint8_t index below. bound
      // the loop to payload[] so a bogus length can't spin or read past it
      for (uint8_t iPtr = 1; iPtr < message.length && iPtr < sizeof(message.payload); iPtr++)
        command.PushBack(message.payload[iPtr]);

      // send to libCEC
      m_callback->OnCommandReceived(command);
    }
    break;
  case VC_CEC_TX:
    {
      // handle response to a command that was sent earlier
      m_queue->MessageReceived((cec_opcode)CEC_CB_OPCODE(p0), (cec_logical_address)CEC_CB_INITIATOR(p0), (cec_logical_address)CEC_CB_FOLLOWER(p0), CEC_CB_RC(header));
    }
    break;
  case VC_CEC_BUTTON_PRESSED:
  case VC_CEC_REMOTE_PRESSED:
    {
      // translate into a cec_command
      cec_command command;
      cec_command::Format(command,
                          (cec_logical_address)CEC_CB_INITIATOR(p0),
                          (cec_logical_address)CEC_CB_FOLLOWER(p0),
                          reason == VC_CEC_BUTTON_PRESSED ? CEC_OPCODE_USER_CONTROL_PRESSED : CEC_OPCODE_VENDOR_REMOTE_BUTTON_DOWN);
      command.parameters.PushBack((uint8_t)CEC_CB_OPERAND1(p0));

      // send to libCEC
      m_callback->OnCommandReceived(command);
    }
    break;
  case VC_CEC_BUTTON_RELEASE:
  case VC_CEC_REMOTE_RELEASE:
    {
      // translate into a cec_command
      cec_command command;
      cec_command::Format(command,
                          (cec_logical_address)CEC_CB_INITIATOR(p0),
                          (cec_logical_address)CEC_CB_FOLLOWER(p0),
                          reason == VC_CEC_BUTTON_RELEASE ? CEC_OPCODE_USER_CONTROL_RELEASE : CEC_OPCODE_VENDOR_REMOTE_BUTTON_UP);
      command.parameters.PushBack((uint8_t)CEC_CB_OPERAND1(p0));

      // send to libCEC
      m_callback->OnCommandReceived(command);
    }
    break;
  case VC_CEC_LOGICAL_ADDR:
    {
      CLockObject lock(m_mutex);
      m_previousLogicalAddress = m_logicalAddress;
      if (CEC_CB_RC(header) == VCHIQ_SUCCESS)
      {
        m_bLogicalAddressChanged = true;
        m_logicalAddress = (cec_logical_address)(p0 & 0xF);
        m_bLogicalAddressRegistered = true;
        LIB_CEC->AddLog(CEC_LOG_DEBUG, "logical address changed to %s (%x)", LIB_CEC->ToString(m_logicalAddress), m_logicalAddress);
      }
      else
      {
        m_logicalAddress = CECDEVICE_FREEUSE;
        LIB_CEC->AddLog(CEC_LOG_DEBUG, "failed to change the logical address, reset to %s (%x)", LIB_CEC->ToString(m_logicalAddress), m_logicalAddress);
      }
      m_logicalAddressCondition.Signal();
    }
    break;
  case VC_CEC_LOGICAL_ADDR_LOST:
    {
      LIB_CEC->AddLog(CEC_LOG_DEBUG, "logical %s (%x) address lost", LIB_CEC->ToString(m_logicalAddress), m_logicalAddress);
      // the logical address was taken by another device
      cec_logical_address previousAddress = m_logicalAddress == CECDEVICE_FREEUSE ? m_previousLogicalAddress : m_logicalAddress;
      m_logicalAddress = CECDEVICE_UNKNOWN;

      // notify libCEC that we lost our LA when the connection was initialised
      bool bNotify(false);
      {
        CLockObject lock(m_mutex);
        bNotify = m_bInitialised && m_bLogicalAddressRegistered;
      }
      if (bNotify)
      {
        rpi_bus_change change = { true, (uint16_t)previousAddress };
        if (!m_busChanges.Queue(change))
          LIB_CEC->AddLog(CEC_LOG_WARNING, "dropped a lost logical address: the handler is not keeping up");
      }
    }
    break;
  case VC_CEC_TOPOLOGY:
    break;
  default:
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "ignoring unknown reason %x", reason);
    break;
  }
}

bool CRPiCECAdapterCommunication::Open(uint32_t iTimeoutMs /* = CEC_DEFAULT_CONNECT_TIMEOUT */, bool UNUSED(bSkipChecks) /* = false */, bool bStartListening)
{
  Close();

  InitHost();

  if (bStartListening)
  {
    SetDisableCallback(false);

    // enable passive mode
    vc_cec_set_passive(true);

    // start draining both queues before anything can be put on them
    if (!CreateThread())
    {
      LIB_CEC->AddLog(CEC_LOG_ERROR, "%s - could not start the callback dispatch thread", __FUNCTION__);
      return false;
    }

    if (!m_busChanges.CreateThread())
    {
      LIB_CEC->AddLog(CEC_LOG_ERROR, "%s - could not start the bus change thread", __FUNCTION__);
      return false;
    }

    // register the callbacks
    vc_cec_register_callback(rpi_cec_callback, (void*)this);
    vc_tv_register_callback(rpi_tv_callback, (void*)this);

    // register LA "freeuse"
    if (RegisterLogicalAddress(CECDEVICE_FREEUSE, iTimeoutMs))
    {
      LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - vc_cec initialised", __FUNCTION__);
      CLockObject lock(m_mutex);
      m_bInitialised = true;
    }
    else
    {
      LIB_CEC->AddLog(CEC_LOG_ERROR, "%s - vc_cec could not be initialised", __FUNCTION__);
      return false;
    }
  }

  return true;
}

uint16_t CRPiCECAdapterCommunication::GetPhysicalAddress(void)
{
  uint16_t iPA(CEC_INVALID_PHYSICAL_ADDRESS);
  if (!IsInitialised())
    return iPA;

  if (vc_cec_get_physical_address(&iPA) == VCHIQ_SUCCESS)
    LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - physical address = %04x", __FUNCTION__, iPA);
  else
  {
    LIB_CEC->AddLog(CEC_LOG_WARNING, "%s - failed to get the physical address", __FUNCTION__);
    iPA = CEC_INVALID_PHYSICAL_ADDRESS;
  }

  return iPA;
}

void CRPiCECAdapterCommunication::Close(void)
{
  // first, while the dispatch thread is still running: a change being reported
  // may be blocked on a transmit that only that thread can confirm
  m_busChanges.StopThread();
  m_busChanges.Clear();

  if (m_bInitialised) {
    // clear the CEC callback so VideoCore stops holding a pointer to this object
    vc_cec_register_callback(NULL, NULL);
    vc_tv_unregister_callback(rpi_tv_callback);
    m_bInitialised = false;
    // reject any callback that races the unregister above
    SetDisableCallback(true);
  }

  // no more callbacks can be queued, so stop the dispatch thread and drop
  // whatever it had not handled yet. Open() runs this before it starts a new
  // thread, so this also runs when the connection never came up
  StopThread();
  m_callbacks.Clear();

  if (g_bHostInited)
  {
    g_bHostInited = false;
    bcm_host_deinit();
  }
}

std::string CRPiCECAdapterCommunication::GetError(void) const
{
  std::string strError(m_strError);
  return strError;
}

void CRPiCECAdapterCommunication::SetDisableCallback(const bool disable)
{
  CLockObject lock(m_mutex);
  m_bDisableCallbacks = disable;
}

cec_adapter_message_state CRPiCECAdapterCommunication::Write(const cec_command &data, bool &bRetry, uint8_t iLineTimeout, bool bIsReply)
{
  VC_CEC_ERROR_T vcAnswer;
  uint32_t iTimeout = (data.transmit_timeout ? data.transmit_timeout : iLineTimeout*1000);
  cec_adapter_message_state rc;

  // to send a real POLL (dest & source LA the same - eg 11), VC
  // needs us to be in passivemode(we are) and with no actual LA
  // registered
  // libCEC sends 'true' POLLs only when at LA choosing process.
  // any other POLLing of devices happens with regular 'empty'
  // msg (just header, no OPCODE) with actual LA as source to X.
  // for us this means, that libCEC already registered tmp LA
  // (0xf, 0xe respectively) before it calls us for LA POLLing.
  //
  // that means - unregistering any A from adapter, _while_
  // ignoring callbacks (and especially not reporting the
  // subsequent actions generated from VC layer - like
  // LA change to 0xf ...)
  //
  // calling vc_cec_release_logical_address() over and over is
  // fine.
  // once libCEC gets NACK on tested A, it calls RegisterLogicalAddress()
  // on it's own - so we don't need to take care of re-registering
  if (!data.opcode_set && data.initiator == data.destination)
  {
    SetDisableCallback(true);

    vc_cec_release_logical_address();
    // accept nothing else than NACK or ACK, repeat until this happens
    while (ADAPTER_MESSAGE_STATE_WAITING_TO_BE_SENT ==
          (rc = m_queue->Write(data, bRetry, iTimeout, bIsReply, vcAnswer)));

    SetDisableCallback(false);
    return rc;
  }

  rc = m_queue->Write(data, bRetry, iTimeout, bIsReply, vcAnswer);
#ifdef CEC_DEBUGGING
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "sending data: result %s", ToString(vcAnswer));
#endif
  return rc;
}

uint16_t CRPiCECAdapterCommunication::GetFirmwareVersion(void)
{
  return VC_CECSERVICE_VER;
}

cec_logical_address CRPiCECAdapterCommunication::GetLogicalAddress(void) const
{
  CLockObject lock(m_mutex);

  return m_logicalAddress;
}

bool CRPiCECAdapterCommunication::UnregisterLogicalAddress(void)
{
  CLockObject lock(m_mutex);
  if (!m_bInitialised)
    return true;

  LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - releasing previous logical address", __FUNCTION__);
  {
    CLockObject lock(m_mutex);
    m_bLogicalAddressRegistered = false;
    m_bLogicalAddressChanged    = false;
  }

  vc_cec_release_logical_address();

  return m_logicalAddressCondition.Wait(lock, m_bLogicalAddressChanged);
}

bool CRPiCECAdapterCommunication::RegisterLogicalAddress(const cec_logical_address address, uint32_t iTimeoutMs)
{
  {
    CLockObject lock(m_mutex);
    if ((m_logicalAddress == address) && m_bLogicalAddressRegistered)
      return true;
  }

  m_bLogicalAddressChanged = false;

  // register the new LA
  int iRetval = vc_cec_set_logical_address((CEC_AllDevices_T)address, (CEC_DEVICE_TYPE_T)CCECTypeUtils::GetType(address), CEC_VENDOR_ID_BROADCOM);
  if (iRetval != VCHIQ_SUCCESS)
  {
    LIB_CEC->AddLog(CEC_LOG_WARNING, "%s - vc_cec_set_logical_address(%X) returned %s (%d)", __FUNCTION__, address, ToString((VC_CEC_ERROR_T)iRetval), iRetval);
    if (iRetval == VC_CEC_ERROR_INVALID_ARGUMENT)
      LIB_CEC->AddLog(CEC_LOG_ERROR, "%s - CEC is being used by another application. Run \"tvservice --off\" and try again.", __FUNCTION__);
    UnregisterLogicalAddress();
  }
  else
  {
    // the mutex has to be held to wait on the condition. it was not held here
    // before, which left the wait reading m_bLogicalAddressChanged unguarded
    CLockObject lock(m_mutex);
    if (m_logicalAddressCondition.Wait(lock, m_bLogicalAddressChanged, iTimeoutMs))
      return true;
  }
  return false;
}

cec_logical_addresses CRPiCECAdapterCommunication::GetLogicalAddresses(void) const
{
  CLockObject lock(m_mutex);
  cec_logical_addresses addresses; addresses.Clear();
  if (m_bLogicalAddressRegistered)
    addresses.primary = GetLogicalAddress();

  return addresses;
}

bool CRPiCECAdapterCommunication::SetLogicalAddresses(const cec_logical_addresses &addresses)
{
  // the current generation RPi only supports 1 LA, so just ensure that the primary address is registered
  return SupportsSourceLogicalAddress(addresses.primary) &&
      RegisterLogicalAddress(addresses.primary);
}

void CRPiCECAdapterCommunication::InitHost(void)
{
  // InitVideoStandalone() reaches this without going through adapter detection,
  // so guard here too: bcm_host_init() calls exit(-1) on a non-Pi
  if (!g_bHostInited && access("/dev/vchiq", F_OK) == 0)
  {
    g_bHostInited = true;
    bcm_host_init();
  }
}

#endif
