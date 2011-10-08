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

#include "LibCEC.h"

using namespace CEC;
using namespace std;

/*!
 * C interface implementation
 */
//@{
ICECAdapter *cec_parser;

bool cec_init(const char *strDeviceName, cec_logical_address iLogicalAddress /* = CECDEVICE_PLAYBACKDEVICE1 */, uint8_t iPhysicalAddress /* = CEC_DEFAULT_PHYSICAL_ADDRESS */)
{
  cec_parser = (ICECAdapter *) CECCreate(strDeviceName, iLogicalAddress, iPhysicalAddress);
  return (cec_parser != NULL);
}

void cec_destroy(void)
{
  cec_close();
  CECDestroy(cec_parser);
  cec_parser = NULL;
}

bool cec_open(const char *strPort, uint32_t iTimeout)
{
  if (cec_parser)
    return cec_parser->Open(strPort, iTimeout);
  return false;
}

void cec_close(void)
{
  if (cec_parser)
    cec_parser->Close();
}

int8_t cec_find_adapters(cec_adapter *deviceList, uint8_t iBufSize, const char *strDevicePath /* = NULL */)
{
  if (cec_parser)
    return cec_parser->FindAdapters(deviceList, iBufSize, strDevicePath);
  return -1;
}

bool cec_ping_adapters(void)
{
  if (cec_parser)
    return cec_parser->PingAdapter();
  return false;
}

bool cec_start_bootloader(void)
{
  if (cec_parser)
    return cec_parser->StartBootloader();
  return false;
}

int8_t cec_get_min_version(void)
{
  if (cec_parser)
    return cec_parser->GetMinVersion();
  return -1;
}

int8_t cec_get_lib_version(void)
{
  if (cec_parser)
    return cec_parser->GetLibVersion();
  return -1;
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

bool cec_power_on_devices(cec_logical_address address /* = CECDEVICE_TV */)
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

//@}
