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

#include "env.h"
#include "PHCommandHandler.h"

#include "lib/devices/CECBusDevice.h"
#include "lib/CECProcessor.h"
#include "lib/LibCEC.h"
#include "lib/CECClient.h"

using namespace CEC;

#define LIB_CEC     m_busDevice->GetProcessor()->GetLib()
#define ToString(p) LIB_CEC->ToString(p)

CPHCommandHandler::CPHCommandHandler(CCECBusDevice *busDevice,
                                     int32_t iTransmitTimeout /* = CEC_DEFAULT_TRANSMIT_TIMEOUT */,
                                     int32_t iTransmitWait /* = CEC_DEFAULT_TRANSMIT_WAIT */,
                                     int8_t iTransmitRetries /* = CEC_DEFAULT_TRANSMIT_RETRIES */,
                                     int64_t iActiveSourcePending /* = 0 */) :
    CCECCommandHandler(busDevice, iTransmitTimeout, iTransmitWait, iTransmitRetries, iActiveSourcePending),
    m_iLastKeyCode(CEC_USER_CONTROL_CODE_UNKNOWN)
{
  m_vendorId = CEC_VENDOR_PHILIPS;
}

int CPHCommandHandler::HandleUserControlPressed(const cec_command& command)
{
  // tv keeps sending these until a button is pressed
  if (command.parameters[0] == CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION &&
      m_iLastKeyCode == CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION)
    return COMMAND_HANDLED;

  m_iLastKeyCode = command.parameters[0];

  return CCECCommandHandler::HandleUserControlPressed(command);
}

int CPHCommandHandler::HandleUserControlRelease(const cec_command& command)
{
  m_iLastKeyCode = CEC_USER_CONTROL_CODE_UNKNOWN;

  return CCECCommandHandler::HandleUserControlRelease(command);
}
