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

#include "VLCommandHandler.h"
#include "../devices/CECBusDevice.h"
#include "../util/StdString.h"

using namespace CEC;

CVLCommandHandler::CVLCommandHandler(CCECBusDevice *busDevice) :
    CCECCommandHandler(busDevice)
{
}

bool CVLCommandHandler::HandleSetStreamPath(const cec_command &command)
{
  if (command.parameters.size >= 2)
  {
    int streamaddr = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
    CStdString strLog;
    strLog.Format(">> %i requests stream path from physical address %04x", command.initiator, streamaddr);
    m_busDevice->AddLog(CEC_LOG_DEBUG, strLog.c_str());
    if (streamaddr == m_busDevice->GetMyPhysicalAddress())
    {
      CCECBusDevice *device = GetDevice(command.destination);
      if (device)
      {
        return device->TransmitActiveSource() &&
               device->TransmitActiveView() &&
               device->TransmitMenuState(command.initiator);
      }
      return false;
    }
  }
  return true;
}
