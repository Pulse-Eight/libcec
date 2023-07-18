/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2013 Pulse-Eight Limited.  All rights reserved.
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
 *     http://www.pulse-eight.com/CTegraCECAdapterCommunication
 *     http://www.pulse-eight.net/
 */

#include "env.h"

#if defined(HAVE_TEGRA_API)
#include "TegraCECAdapterCommunication.h"
#include "TegraCECDev.h"

#include "CECTypeUtils.h"
#include "LibCEC.h"
#include "p8-platform/sockets/cdevsocket.h"
#include "p8-platform/util/StdString.h"
#include "p8-platform/util/buffer.h"

using namespace std;
using namespace CEC;
using namespace P8PLATFORM;

#include "AdapterMessageQueue.h"

#define LIB_CEC m_callback->GetLib()

TegraCECAdapterCommunication::TegraCECAdapterCommunication(IAdapterCommunicationCallback *callback) :
    IAdapterCommunication(callback),
    m_bLogicalAddressChanged(false)
{
  CLockObject lock(m_mutex);
  LIB_CEC->AddLog(CEC_LOG_ERROR, "%s: Creating Adaptor", __func__);
  m_iNextMessage = 0;
  m_logicalAddresses.Clear();
}

TegraCECAdapterCommunication::~TegraCECAdapterCommunication(void)
{
  Close();
  CLockObject lock(m_mutex);
  delete m_dev;
  m_dev = 0;
}

bool TegraCECAdapterCommunication::IsOpen(void)
{
  return devOpen;
}

bool TegraCECAdapterCommunication::Open(uint32_t iTimeoutMs, bool UNUSED(bSkipChecks), bool bStartListening)
{

  fd = open(TEGRA_CEC_DEV_PATH, O_RDWR);

  if (fd < 0){
    LIB_CEC->AddLog(CEC_LOG_ERROR, "%s: Failed To Open Tegra CEC Device", __func__);
    return false;
  }

  fdAddr = open(TEGRA_ADDR_PATH, O_RDWR);

  if (fdAddr < 0){
    LIB_CEC->AddLog(CEC_LOG_ERROR, "%s: Failed To Open Tegra Logical Address Node", __func__);
    close(fd);
    return false;
  }
  if (!bStartListening)
  {
    Close();
    return true;
  }


  if (CreateThread()){
    devOpen = true;
    return true;
  }

  return false;
}

void TegraCECAdapterCommunication::Close(void)
{
  StopThread(0);
  close(fdAddr);
  close(fd);
  devOpen = false;
}

std::string TegraCECAdapterCommunication::GetError(void) const
{
  std::string strError(m_strError);
  return strError;
}

cec_adapter_message_state TegraCECAdapterCommunication::Write(
  const cec_command &data, bool &UNUSED(bRetry), uint8_t UNUSED(iLineTimeout), bool UNUSED(bIsReply))
{

  int size = 0;
  unsigned char cmdData[TEGRA_CEC_FRAME_MAX_LENGTH];
  unsigned char addr = (data.initiator << 4) | (data.destination & 0x0f);

  if (data.initiator == data.destination){
    return ADAPTER_MESSAGE_STATE_SENT_NOT_ACKED;
  }

  cmdData[size] = addr;
  size++;

  if (data.opcode_set){
     cmdData[size] = data.opcode;
     size++;
  }

  for (int i = 0; i < data.parameters.size; i++){
    cmdData[size] = data.parameters.data[i];
    size++;

    if (size > TEGRA_CEC_FRAME_MAX_LENGTH){
      LIB_CEC->AddLog(CEC_LOG_ERROR, "%s: Command Longer Than %i Bytes", __func__,TEGRA_CEC_FRAME_MAX_LENGTH);
      return ADAPTER_MESSAGE_STATE_ERROR;
    }
  }

  int status = write(fd,cmdData,size);

  if (status < 0){

    if(errno == ECONNRESET || errno == EHOSTUNREACH){
      LIB_CEC->AddLog(CEC_LOG_TRAFFIC, "%s: Write OK But Not ACKED (%s)", __func__, strerror(errno));
      return ADAPTER_MESSAGE_STATE_SENT_NOT_ACKED;
    } else {
      LIB_CEC->AddLog(CEC_LOG_ERROR, "%s: Write Error (%s)", __func__, strerror(errno));
      return ADAPTER_MESSAGE_STATE_ERROR;
    }

  } else {
    LIB_CEC->AddLog(CEC_LOG_TRAFFIC, "%s: Write OK And ACKED", __func__);
    return ADAPTER_MESSAGE_STATE_SENT_ACKED;
  }

}

uint16_t TegraCECAdapterCommunication::GetFirmwareVersion(void)
{
  return 0;
}

cec_vendor_id TegraCECAdapterCommunication::GetVendorId(void)
{
   return CEC_VENDOR_LG;
}

uint16_t TegraCECAdapterCommunication::GetPhysicalAddress(void)
{
  uint16_t iPA;

  LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - trying to get the physical address from the OS", __FUNCTION__);
  iPA = CEDIDParser::GetPhysicalAddress();
  LIB_CEC->AddLog(CEC_LOG_DEBUG, "%s - OS returned physical address %04x", __FUNCTION__, iPA);

  return iPA;
}

cec_logical_addresses TegraCECAdapterCommunication::GetLogicalAddresses(void) const
{
  cec_logical_addresses addresses;
  char logical_str[5] = {0};
  int logical_int = 0;

  addresses.Clear();

  if (lseek(fdAddr, 0, SEEK_SET) < 0)
      goto failed;

  if (read(fdAddr, logical_str, sizeof(logical_str) - 1) <= 0)
      goto failed;

  logical_int = strtol(logical_str, NULL, 16);
  for (int i = CECDEVICE_TV; i < CECDEVICE_BROADCAST; i++)
  {
      if (logical_int & (1<<i))
          addresses.Set(cec_logical_address(i));
  }

failed:
  return addresses;
}

bool TegraCECAdapterCommunication::SetLogicalAddresses(const cec_logical_addresses &addresses)
{
  if (!IsOpen())
  	  return false;

  uint16_t mask = 0x0;
  char buf [8] = {0};

  for (int i = CECDEVICE_TV; i < CECDEVICE_BROADCAST; i++)
  {
    if (addresses.IsSet(cec_logical_address(i))){
      mask |= 0x1 << i;
    }
  }

  sprintf(buf, "0x%x", mask);

  lseek(fdAddr, 0, SEEK_SET);

  if(write(fdAddr,&buf,sizeof(buf)) < 0){
    LIB_CEC->AddLog(CEC_LOG_ERROR, "%s: Failed write to logical address node (%s) (%i)", __func__,strerror(errno),mask);
    return false;
  }

  m_logicalAddresses = addresses;
  m_bLogicalAddressChanged = true;
  return true;
}

void TegraCECAdapterCommunication::HandleLogicalAddressLost(cec_logical_address UNUSED(oldAddress))
{
  //Tegra always listens on broadcast so no need to handle this
}

void *TegraCECAdapterCommunication::Process(void)
{
  unsigned char opcode;
  cec_logical_address initiator, destination;

  while (!IsStopped())
  {

    int8_t isNotEndOfData = 1;
    unsigned char buffer[2] = {0,0};
    cec_command cmd;

    if (read(fd,buffer,2) < 0){
        LIB_CEC->AddLog(CEC_LOG_ERROR, "%s: Failed To Read From Tegra CEC Device", __func__);
    }

    initiator = cec_logical_address(buffer[0] >> 4);
    destination = cec_logical_address(buffer[0] & 0x0f);

    if ((buffer[1] & 0x01) > 0){
      isNotEndOfData = 0;
    }

    if (isNotEndOfData > 0){

      if (read(fd,buffer,2) < 0){
        LIB_CEC->AddLog(CEC_LOG_ERROR, "%s: Failed To Read From Tegra CEC Device", __func__);
      }

      opcode = buffer[0];
      cec_command::Format(cmd, initiator, destination, cec_opcode(opcode));
      if ((buffer[1] & 0x01) > 0){
        isNotEndOfData = 0;
      }
    } else {
      cec_command::Format(cmd, initiator, destination, CEC_OPCODE_NONE);
    }

    while (isNotEndOfData > 0){

      if (read(fd,buffer,2) < 0){
        LIB_CEC->AddLog(CEC_LOG_ERROR, "%s: Failed To Read From Tegra CEC Device", __func__);
      }

      cmd.parameters.PushBack(buffer[0]);

      if ((buffer[1] & 0x01) > 0){
        isNotEndOfData = 0;
      }

    }

    //LIB_CEC->AddLog(CEC_LOG_TRAFFIC, "%s: Reading Data Len : %i", __func__, cmd.parameters.size);
    if (!IsStopped())
      m_callback->OnCommandReceived(cmd);
  }

  return 0;
}

#endif
