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

#include "CECParser.h"

using namespace CEC;
using namespace std;

/*!
 * C interface implementation
 */
//@{
ICECDevice *cec_parser;

bool cec_init(const char *strDeviceName, cec_logical_address iLogicalAddress /* = CECDEVICE_PLAYBACKDEVICE1 */, int iPhysicalAddress /* = CEC_DEFAULT_PHYSICAL_ADDRESS */)
{
  cec_parser = (ICECDevice *) CECCreate(strDeviceName, iLogicalAddress, iPhysicalAddress);
  return (cec_parser != NULL);
}

bool cec_open(const char *strPort, int iTimeout)
{
  if (cec_parser)
    return cec_parser->Open(strPort, iTimeout);
  return false;
}

bool cec_close(int iTimeout)
{
  bool bReturn = false;
  if (cec_parser)
    bReturn = cec_parser->Close(iTimeout);

  delete cec_parser;
  cec_parser = NULL;
  return bReturn;
}

bool cec_ping(void)
{
  if (cec_parser)
    return cec_parser->Ping();
  return false;
}

bool cec_start_bootloader(void)
{
  if (cec_parser)
    return cec_parser->StartBootloader();
  return false;
}

bool cec_power_off_devices(cec_logical_address address /* = CECDEVICE_BROADCAST */)
{
  if (cec_parser)
    return cec_parser->PowerOffDevices(address);
  return false;
}

bool cec_power_on_devices(cec_logical_address address /* = CECDEVICE_BROADCAST */)
{
  if (cec_parser)
    return cec_parser->PowerOnDevices(address);
  return false;
}

bool cec_standby_devices(cec_logical_address address /* = CECDEVICE_BROADCAST */)
{
  if (cec_parser)
    return cec_parser->StandbyDevices(address);
  return false;
}

bool cec_set_active_view(void)
{
  if (cec_parser)
    return cec_parser->SetActiveView();
  return false;
}

bool cec_set_inactive_view(void)
{
  if (cec_parser)
    return cec_parser->SetInactiveView();
  return false;
}

bool cec_get_next_log_message(cec_log_message *message)
{
  if (cec_parser)
    return cec_parser->GetNextLogMessage(message);
  return false;
}

bool cec_get_next_keypress(cec_keypress *key)
{
  if (cec_parser)
    return cec_parser->GetNextKeypress(key);
  return false;
}

bool cec_get_next_command(cec_command *command)
{
  if (cec_parser)
    return cec_parser->GetNextCommand(command);
  return false;
}

bool cec_transmit(const CEC::cec_frame &data, bool bWaitForAck /* = true */)
{
  if (cec_parser)
    return cec_parser->Transmit(data, bWaitForAck);
  return false;
}

bool cec_set_logical_address(cec_logical_address iLogicalAddress)
{
  if (cec_parser)
    return cec_parser->SetLogicalAddress(iLogicalAddress);
  return false;
}

bool cec_set_ack_mask(uint16_t iMask)
{
  if (cec_parser)
    return cec_parser->SetAckMask(iMask);
  return false;
}

int cec_get_min_version(void)
{
  if (cec_parser)
    return cec_parser->GetMinVersion();
  return -1;
}

int cec_get_lib_version(void)
{
  if (cec_parser)
    return cec_parser->GetLibVersion();
  return -1;
}

int cec_find_devices(vector<cec_device> &deviceList, const char *strDevicePath /* = NULL */)
{
  if (cec_parser)
    return cec_parser->FindDevices(deviceList, strDevicePath);
  return -1;
}

//@}
