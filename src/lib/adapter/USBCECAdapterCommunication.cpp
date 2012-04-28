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

#include "USBCECAdapterCommunication.h"
#include "USBCECAdapterCommands.h"
#include "USBCECAdapterMessageQueue.h"
#include "../platform/sockets/serialport.h"
#include "../platform/util/timeutils.h"
#include "../LibCEC.h"
#include "../CECProcessor.h"

using namespace std;
using namespace CEC;
using namespace PLATFORM;

#define CEC_ADAPTER_PING_TIMEOUT 15000

CUSBCECAdapterCommunication::CUSBCECAdapterCommunication(IAdapterCommunicationCallback *callback, const char *strPort, uint16_t iBaudRate /* = CEC_SERIAL_DEFAULT_BAUDRATE */) :
    IAdapterCommunication(callback),
    m_port(NULL),
    m_iLineTimeout(0),
    m_lastPollDestination(CECDEVICE_UNKNOWN),
    m_bInitialised(false),
    m_pingThread(NULL),
    m_commands(NULL),
    m_adapterMessageQueue(NULL)
{
  for (unsigned int iPtr = CECDEVICE_TV; iPtr < CECDEVICE_BROADCAST; iPtr++)
    m_bWaitingForAck[iPtr] = false;
  m_port = new CSerialPort(strPort, iBaudRate);
}

CUSBCECAdapterCommunication::~CUSBCECAdapterCommunication(void)
{
  Close();
  delete m_commands;
  delete m_adapterMessageQueue;
  delete m_port;
}

bool CUSBCECAdapterCommunication::Open(uint32_t iTimeoutMs /* = CEC_DEFAULT_CONNECT_TIMEOUT */, bool bSkipChecks /* = false */, bool bStartListening /* = true */)
{
  bool bConnectionOpened(false);
  {
    CLockObject lock(m_mutex);

    /* we need the port settings here */
    if (!m_port)
    {
      CLibCEC::AddLog(CEC_LOG_ERROR, "port is NULL");
      return bConnectionOpened;
    }

    /* return true when the port is already open */
    if (IsOpen())
    {
      CLibCEC::AddLog(CEC_LOG_WARNING, "port is already open");
      return true;
    }

    /* adapter commands */
    if (!m_commands)
      m_commands = new CUSBCECAdapterCommands(this);

    if (!m_adapterMessageQueue)
    {
      m_adapterMessageQueue = new CCECAdapterMessageQueue(this);
      m_adapterMessageQueue->CreateThread();
    }

    /* try to open the connection */
    CStdString strError;
    CTimeout timeout(iTimeoutMs);
    while (!bConnectionOpened && timeout.TimeLeft() > 0)
    {
      if ((bConnectionOpened = m_port->Open(timeout.TimeLeft())) == false)
      {
        strError.Format("error opening serial port '%s': %s", m_port->GetName().c_str(), m_port->GetError().c_str());
        Sleep(250);
      }
      /* and retry every 250ms until the timeout passed */
    }

    /* return false when we couldn't connect */
    if (!bConnectionOpened)
    {
      CLibCEC::AddLog(CEC_LOG_ERROR, strError);
      return false;
    }

    CLibCEC::AddLog(CEC_LOG_DEBUG, "connection opened, clearing any previous input and waiting for active transmissions to end before starting");
    ClearInputBytes();
  }

  if (!CreateThread())
  {
    bConnectionOpened = false;
    CLibCEC::AddLog(CEC_LOG_ERROR, "could not create a communication thread");
  }
  else if (!bSkipChecks && !CheckAdapter())
  {
    bConnectionOpened = false;
    CLibCEC::AddLog(CEC_LOG_ERROR, "the adapter failed to pass basic checks");
  }
  else if (bStartListening)
  {
    /* start a ping thread, that will ping the adapter every 15 seconds
       if it doesn't receive any ping for 30 seconds, it'll switch to auto mode */
    m_pingThread = new CAdapterPingThread(this, CEC_ADAPTER_PING_TIMEOUT);
    if (m_pingThread->CreateThread())
    {
      bConnectionOpened = true;
    }
    else
    {
      bConnectionOpened = false;
      CLibCEC::AddLog(CEC_LOG_ERROR, "could not create a ping thread");
    }
  }

  if (!bConnectionOpened || !bStartListening)
    StopThread(0);

  return bConnectionOpened;
}

void CUSBCECAdapterCommunication::Close(void)
{
  /* stop the reader thread */
  StopThread(0);

  CLockObject lock(m_mutex);

  /* set the ackmask to 0 before closing the connection */
  if (IsRunning() && m_port->IsOpen() && m_port->GetErrorNumber() == 0)
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "%s - closing the connection", __FUNCTION__);
    SetAckMask(0);
    if (m_commands->GetFirmwareVersion() >= 2)
      SetControlledMode(false);
  }

  m_adapterMessageQueue->Clear();

  /* stop and delete the ping thread */
  if (m_pingThread)
    m_pingThread->StopThread(0);
  delete m_pingThread;
  m_pingThread = NULL;

  /* close and delete the com port connection */
  if (m_port)
    m_port->Close();

  libcec_parameter param;
  CLibCEC::Alert(CEC_ALERT_CONNECTION_LOST, param);
}

cec_adapter_message_state CUSBCECAdapterCommunication::Write(const cec_command &data, bool &bRetry, uint8_t iLineTimeout)
{
  cec_adapter_message_state retVal(ADAPTER_MESSAGE_STATE_UNKNOWN);
  if (!IsRunning())
    return retVal;

  CCECAdapterMessage *output = new CCECAdapterMessage(data, iLineTimeout);

  /* mark as waiting for an ack from the destination */
  MarkAsWaiting(data.destination);

  /* send the message */
  bRetry = (!m_adapterMessageQueue->Write(output) || output->NeedsRetry()) && output->transmit_timeout > 0;
  if (bRetry)
    Sleep(CEC_DEFAULT_TRANSMIT_RETRY_WAIT);
  retVal = output->state;

  delete output;
  return retVal;
}

void *CUSBCECAdapterCommunication::Process(void)
{
  CCECAdapterMessage msg;
  CLibCEC::AddLog(CEC_LOG_DEBUG, "communication thread started");

  while (!IsStopped())
  {
    /* read from the serial port */
    if (!ReadFromDevice(50, 5))
      break;

    /* TODO sleep 5 ms so other threads can get a lock */
    Sleep(5);
  }

  m_adapterMessageQueue->Clear();
  CLibCEC::AddLog(CEC_LOG_DEBUG, "communication thread ended");
  return NULL;
}

bool CUSBCECAdapterCommunication::HandlePoll(const CCECAdapterMessage &msg)
{
  bool bIsError(msg.IsError());
  cec_adapter_messagecode messageCode(msg.Message());
  CLockObject lock(m_mutex);

  if (messageCode == MSGCODE_FRAME_START && msg.IsACK())
  {
    m_lastPollDestination = msg.Destination();
    if (msg.Destination() < CECDEVICE_BROADCAST)
    {
      if (!m_bWaitingForAck[msg.Destination()] && !msg.IsEOM())
      {
        if (m_callback)
          m_callback->HandlePoll(msg.Initiator(), msg.Destination());
      }
      else
        m_bWaitingForAck[msg.Destination()] = false;
    }
  }
  else if (messageCode == MSGCODE_RECEIVE_FAILED)
  {
    /* hack to suppress warnings when an LG is polling */
    if (m_lastPollDestination != CECDEVICE_UNKNOWN)
      bIsError = m_callback->HandleReceiveFailed(m_lastPollDestination);
  }

  return bIsError;
}

void CUSBCECAdapterCommunication::MarkAsWaiting(const cec_logical_address dest)
{
  /* mark as waiting for an ack from the destination */
  if (dest < CECDEVICE_BROADCAST)
  {
    CLockObject lock(m_mutex);
    m_bWaitingForAck[dest] = true;
  }
}

void CUSBCECAdapterCommunication::ClearInputBytes(uint32_t iTimeout /* = CEC_CLEAR_INPUT_DEFAULT_WAIT */)
{
  CTimeout timeout(iTimeout);
  uint8_t buff[1024];
  ssize_t iBytesRead(0);
  bool bGotMsgEnd(true);

  while (timeout.TimeLeft() > 0 && ((iBytesRead = m_port->Read(buff, 1024, 5)) > 0 || !bGotMsgEnd))
  {
    bGotMsgEnd = false;
    /* if something was received, wait for MSGEND */
    for (ssize_t iPtr = 0; iPtr < iBytesRead; iPtr++)
      bGotMsgEnd = buff[iPtr] == MSGEND;
  }
}

bool CUSBCECAdapterCommunication::SetLineTimeout(uint8_t iTimeout)
{
  bool bReturn(true);
  bool bChanged(false);

  /* only send the command if the timeout changed */
  {
    CLockObject lock(m_mutex);
    bChanged = (m_iLineTimeout != iTimeout);
    m_iLineTimeout = iTimeout;
  }

  if (bChanged)
    bReturn = m_commands->SetLineTimeout(iTimeout);

  return bReturn;
}

bool CUSBCECAdapterCommunication::WriteToDevice(CCECAdapterMessage *message)
{
  CLockObject adapterLock(m_mutex);
  if (!m_port->IsOpen())
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "error writing command '%s' to serial port '%s': the connection is closed", CCECAdapterMessage::ToString(message->Message()), m_port->GetName().c_str());
    message->state = ADAPTER_MESSAGE_STATE_ERROR;
    return false;
  }

  /* write the message */
  if (m_port->Write(message->packet.data, message->Size()) != (ssize_t) message->Size())
  {
    CLibCEC::AddLog(CEC_LOG_DEBUG, "error writing command '%s' to serial port '%s': %s", CCECAdapterMessage::ToString(message->Message()), m_port->GetName().c_str(), m_port->GetError().c_str());
    message->state = ADAPTER_MESSAGE_STATE_ERROR;
    Close();
    return false;
  }

  CLibCEC::AddLog(CEC_LOG_DEBUG, "command '%s' sent", message->IsTranmission() ? "CEC transmission" : CCECAdapterMessage::ToString(message->Message()));
  message->state = ADAPTER_MESSAGE_STATE_SENT;
  return true;
}

bool CUSBCECAdapterCommunication::ReadFromDevice(uint32_t iTimeout, size_t iSize /* = 256 */)
{
  ssize_t iBytesRead(0);
  uint8_t buff[256];
  if (iSize > 256)
    iSize = 256;

  /* read from the serial port */
  {
    CLockObject lock(m_mutex);
    if (!m_port || !m_port->IsOpen())
      return false;

    iBytesRead = m_port->Read(buff, sizeof(uint8_t) * iSize, iTimeout);

    if (m_port->GetErrorNumber())
    {
      CLibCEC::AddLog(CEC_LOG_ERROR, "error reading from serial port: %s", m_port->GetError().c_str());
      m_port->Close();
      return false;
    }
  }

  if (iBytesRead < 0 || iBytesRead > 256)
    return false;
  else if (iBytesRead > 0)
  {
    /* add the data to the current frame */
    m_adapterMessageQueue->AddData(buff, iBytesRead);
  }

  return true;
}

CCECAdapterMessage *CUSBCECAdapterCommunication::SendCommand(cec_adapter_messagecode msgCode, CCECAdapterMessage &params, bool bIsRetry /* = false */)
{
  if (!m_port || !m_port->IsOpen() ||
      !m_adapterMessageQueue)
    return NULL;

  /* create the adapter message for this command */
  CCECAdapterMessage *output = new CCECAdapterMessage;
  output->PushBack(MSGSTART);
  output->PushEscaped((uint8_t)msgCode);
  output->Append(params);
  output->PushBack(MSGEND);

  /* write the command */
  if (!m_adapterMessageQueue->Write(output))
  {
    if (output->state == ADAPTER_MESSAGE_STATE_ERROR)
      Close();
    return output;
  }
  else
  {
    if (!bIsRetry && output->Reply() == MSGCODE_COMMAND_REJECTED && msgCode != MSGCODE_SET_CONTROLLED &&
        msgCode != MSGCODE_GET_BUILDDATE /* same messagecode value had a different meaning in older fw builds */)
    {
      /* if the controller reported that the command was rejected, and we didn't send the command
         to set controlled mode, then the controller probably switched to auto mode. set controlled
         mode and retry */
      CLibCEC::AddLog(CEC_LOG_DEBUG, "setting controlled mode and retrying");
      delete output;
      if (SetControlledMode(true))
        return SendCommand(msgCode, params, true);
    }
  }

  return output;
}

bool CUSBCECAdapterCommunication::CheckAdapter(uint32_t iTimeoutMs /* = CEC_DEFAULT_CONNECT_TIMEOUT */)
{
  bool bReturn(false);
  CTimeout timeout(iTimeoutMs > 0 ? iTimeoutMs : CEC_DEFAULT_TRANSMIT_WAIT);

  /* try to ping the adapter */
  bool bPinged(false);
  unsigned iPingTry(0);
  while (timeout.TimeLeft() > 0 && (bPinged = PingAdapter()) == false)
  {
    CLibCEC::AddLog(CEC_LOG_ERROR, "the adapter did not respond correctly to a ping (try %d)", ++iPingTry);
    CEvent::Sleep(500);
  }

  /* try to read the firmware version */
  if (bPinged && timeout.TimeLeft() > 0 && m_commands->RequestFirmwareVersion() >= 2)
  {
    /* try to set controlled mode for v2+ firmwares */
    unsigned iControlledTry(0);
    bool bControlled(false);
    while (timeout.TimeLeft() > 0 && (bControlled = SetControlledMode(true)) == false)
    {
      CLibCEC::AddLog(CEC_LOG_ERROR, "the adapter did not respond correctly to setting controlled mode (try %d)", ++iControlledTry);
      CEvent::Sleep(500);
    }
    bReturn = bControlled;
  }
  else
    bReturn = true;

  /* try to read the build date */
  m_commands->RequestBuildDate();

  SetInitialised(bReturn);
  return bReturn;
}

bool CUSBCECAdapterCommunication::IsOpen(void)
{
  /* thread is not being stopped, the port is open and the thread is running */
  return !IsStopped() && m_port->IsOpen() && IsRunning();
}

CStdString CUSBCECAdapterCommunication::GetError(void) const
{
  return m_port->GetError();
}

void CUSBCECAdapterCommunication::SetInitialised(bool bSetTo /* = true */)
{
  CLockObject lock(m_mutex);
  m_bInitialised = bSetTo;
}

bool CUSBCECAdapterCommunication::IsInitialised(void)
{
  CLockObject lock(m_mutex);
  return m_bInitialised;
}

bool CUSBCECAdapterCommunication::StartBootloader(void)
{
  if (m_port->IsOpen() && m_commands->StartBootloader())
  {
    Close();
    return true;
  }
  return false;
}

bool CUSBCECAdapterCommunication::SetAckMask(uint16_t iMask)
{
  return m_port->IsOpen() ? m_commands->SetAckMask(iMask) : false;
}

bool CUSBCECAdapterCommunication::PingAdapter(void)
{
  return m_port->IsOpen() ? m_commands->PingAdapter() : false;
}

uint16_t CUSBCECAdapterCommunication::GetFirmwareVersion(void)
{
  return m_commands->GetFirmwareVersion();
}

uint32_t CUSBCECAdapterCommunication::GetFirmwareBuildDate(void)
{
  return m_commands->RequestBuildDate();
}

bool CUSBCECAdapterCommunication::PersistConfiguration(libcec_configuration *configuration)
{
  return m_port->IsOpen() ? m_commands->PersistConfiguration(configuration) : false;
}

bool CUSBCECAdapterCommunication::GetConfiguration(libcec_configuration *configuration)
{
  return m_port->IsOpen() ? m_commands->GetConfiguration(configuration) : false;
}

CStdString CUSBCECAdapterCommunication::GetPortName(void)
{
  return m_port->GetName();
}

bool CUSBCECAdapterCommunication::SetControlledMode(bool controlled)
{
  return m_port->IsOpen() ? m_commands->SetControlledMode(controlled) : false;
}

void *CAdapterPingThread::Process(void)
{
  while (!IsStopped())
  {
    if (m_timeout.TimeLeft() == 0)
    {
      /* reinit the timeout */
      m_timeout.Init(CEC_ADAPTER_PING_TIMEOUT);

      /* send a ping to the adapter */
      bool bPinged(false);
      int iFailedCounter(0);
      while (!bPinged && iFailedCounter < 3)
      {
        if (!m_com->PingAdapter())
        {
          /* sleep and retry */
          Sleep(CEC_DEFAULT_TRANSMIT_RETRY_WAIT);
          ++iFailedCounter;
        }
        else
        {
          bPinged = true;
        }
      }

      if (iFailedCounter == 3)
      {
        /* failed to ping the adapter 3 times in a row. something must be wrong with the connection */
        CLibCEC::AddLog(CEC_LOG_ERROR, "failed to ping the adapter 3 times in a row. closing the connection.");
        m_com->StopThread(false);
        break;
      }
    }

    Sleep(500);
  }
  return NULL;
}
