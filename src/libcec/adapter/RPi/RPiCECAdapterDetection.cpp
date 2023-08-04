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

#if defined(HAVE_RPI_API)
#include "RPiCECAdapterDetection.h"

extern "C" {
#include <interface/vmcs_host/vc_cecservice.h>
#include <interface/vchiq_arm/vchiq_if.h>
#include <bcm_host.h>
}

using namespace CEC;

bool CRPiCECAdapterDetection::FindAdapter(void)
{
  uint8_t iResult;

  VCHI_INSTANCE_T vchiq_instance;
  if ((iResult = vchi_initialise(&vchiq_instance)) != VCHIQ_SUCCESS)
    return false;

  if ((iResult = vchi_connect(NULL, 0, vchiq_instance)) != VCHIQ_SUCCESS)
    return false;

  bcm_host_init();
  iResult = vc_cec_set_passive(true);
  if (iResult != VCHIQ_SUCCESS)
    return false;

  return true;
}

#endif
