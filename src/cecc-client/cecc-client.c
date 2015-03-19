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
#include "ceccloader.h"

#include <stdio.h>
#include <signal.h>

static int cb_cec_log_message(void* lib, const cec_log_message message);

static int                  g_bExit = 0;
static libcec_configuration g_configuration;
static libcec_interface_t   g_iface;
static ICECCallbacks        g_callbacks = {
    .CBCecLogMessage = cb_cec_log_message
};

static void sighandler(int iSignal)
{
  printf("signal caught: %d - exiting\n", iSignal);
  g_bExit = 1;
}

static int cb_cec_log_message(void* lib, const cec_log_message message)
{
  const char* strLevel;
  switch (message.level)
  {
  case CEC_LOG_ERROR:
    strLevel = "ERROR:   ";
    break;
  case CEC_LOG_WARNING:
    strLevel = "WARNING: ";
    break;
  case CEC_LOG_NOTICE:
    strLevel = "NOTICE:  ";
    break;
  case CEC_LOG_TRAFFIC:
    strLevel = "TRAFFIC: ";
    break;
  case CEC_LOG_DEBUG:
    strLevel = "DEBUG:   ";
    break;
  default:
    break;
  }

  printf("%s[%16lld]\t%s\n", strLevel, message.time, message.message);
}

int main(int argc, char *argv[])
{
  char logbuf[100];
  cec_adapter adapters[3];
  int8_t nbAdapters;
  if (signal(SIGINT, sighandler) == SIG_ERR)
  {
    printf("can't register sighandler\n");
    return -1;
  }

  libcecc_reset_configuration(&g_configuration);
  g_configuration.clientVersion        = LIBCEC_VERSION_CURRENT;
  g_configuration.deviceTypes.types[0] = CEC_DEVICE_TYPE_RECORDING_DEVICE;
  g_configuration.baseDevice           = CECDEVICE_TV;
  g_configuration.iHDMIPort            = 1;
  g_configuration.callbacks            = &g_callbacks;
  snprintf(g_configuration.strDeviceName, sizeof(g_configuration.strDeviceName), "CEC tester");

  if (libcecc_initialise(&g_configuration, &g_iface, NULL) != 1)
  {
    printf("can't initialise libCEC\n");
    return -1;
  }

  g_iface.version_to_string(g_configuration.serverVersion, logbuf, sizeof(logbuf));
  printf("libCEC %s loaded\n", logbuf);

  nbAdapters = g_iface.find_adapters(g_iface.connection, adapters, sizeof(adapters) / sizeof(cec_adapter), NULL);
  if (nbAdapters > 0)
  {
    if (g_iface.open(g_iface.connection, adapters[0].comm, 5000) != 1)
    {
      printf("failed to open a connection\n");
      libcecc_destroy(&g_iface);
      return -1;
    }
    printf("connection opened\n");
    sleep(10);
  }
  else
  {
    printf("failed to open a connection: no adapters found\n");
  }

  libcecc_destroy(&g_iface);
  return 0;
}
