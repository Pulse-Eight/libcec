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
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

static void cb_cec_log_message(void* lib, const cec_log_message* message);

#if defined(__WINDOWS__)
#include <Windows.h>
static void usleep(__int64 usec)
{
  HANDLE timer;
  LARGE_INTEGER ft;
  ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative value indicates relative time
  timer = CreateWaitableTimer(NULL, TRUE, NULL);
  SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
  WaitForSingleObject(timer, INFINITE);
  CloseHandle(timer);
}

#define sleep(x) usleep(1000000 * x)
#define PRId64 "%lld"
#endif

static ICECCallbacks        g_callbacks = {
    .logMessage           = cb_cec_log_message,
    .keyPress             = NULL,
    .commandReceived      = NULL,
    .configurationChanged = NULL,
    .alert                = NULL,
    .menuStateChanged     = NULL,
    .sourceActivated      = NULL
};

static libcec_configuration  g_config;
static int                   g_cecLogLevel = -1;
static int                   g_cecDefaultLogLevel = CEC_LOG_ALL;
static char                  g_strPort[50] = { 0 };
static int                   g_bSingleCommand = 0;
static volatile sig_atomic_t g_bExit = 0;
static int                   g_bHardExit = 0;
static libcec_interface_t    g_iface;

static void sighandler(int iSignal)
{
  printf("signal caught: %d - exiting\n", iSignal);
  g_bExit = 1;
}

static void cb_cec_log_message(void* lib, const cec_log_message* message)
{
  if ((message->level & g_cecLogLevel) == message->level)
  {
    const char* strLevel;
    switch (message->level)
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

    printf("%s[%" PRId64 "]\t%s\n", strLevel, message->time, message->message);
  }
}

static void cec_list_devices(void)
{
  //TODO
}

static int cec_process_command_line_arguments(int argc, char *argv[])
{
  int bReturn = 1;
  int iArgPtr = 1;
  while (iArgPtr < argc && bReturn)
  {
    if (argc >= iArgPtr + 1)
    {
      if (!strcmp(argv[iArgPtr], "-d") ||
          !strcmp(argv[iArgPtr], "--log-level"))
      {
        if (argc >= iArgPtr + 2)
        {
          int iNewLevel = atoi(argv[iArgPtr + 1]);
          if (iNewLevel >= CEC_LOG_ERROR && iNewLevel <= CEC_LOG_ALL)
          {
            g_cecLogLevel = iNewLevel;
            if (!g_bSingleCommand)
              printf("log level set to %s\n", argv[iArgPtr + 1]);
          }
          else
          {
            printf("== skipped log-level parameter: invalid level %s' ==\n", argv[iArgPtr + 1]);
          }
          iArgPtr += 2;
        }
        else
        {
          printf("== skipped log-level parameter: no level given ==\n");
          ++iArgPtr;
        }
      }
      else if (!strcmp(argv[iArgPtr], "-t") ||
               !strcmp(argv[iArgPtr], "--type"))
      {
        if (argc >= iArgPtr + 2)
        {
          if (!strcmp(argv[iArgPtr + 1], "p"))
          {
            if (!g_bSingleCommand)
              printf("== using device type 'playback device'\n");
            g_config.deviceTypes.types[0] = CEC_DEVICE_TYPE_PLAYBACK_DEVICE;
          }
          else if (!strcmp(argv[iArgPtr + 1], "r"))
          {
            if (!g_bSingleCommand)
              printf("== using device type 'recording device'\n");
            g_config.deviceTypes.types[0] = CEC_DEVICE_TYPE_RECORDING_DEVICE;
          }
          else if (!strcmp(argv[iArgPtr + 1], "t"))
          {
            if (!g_bSingleCommand)
              printf("== using device type 'tuner'\n");
            g_config.deviceTypes.types[0] = CEC_DEVICE_TYPE_TUNER;
          }
          else if (!strcmp(argv[iArgPtr + 1], "a"))
          {
            if (!g_bSingleCommand)
              printf("== using device type 'audio system'\n");
            g_config.deviceTypes.types[0] = CEC_DEVICE_TYPE_AUDIO_SYSTEM;
          }
          else if (!strcmp(argv[iArgPtr + 1], "x"))
          {
            if (!g_bSingleCommand)
              printf("== using device type 'tv'\n");
            g_config.deviceTypes.types[0] = CEC_DEVICE_TYPE_TV;
          }
          else
          {
            printf("== skipped invalid device type %s'\n", argv[iArgPtr + 1]);
          }
          ++iArgPtr;
        }
        ++iArgPtr;
      }
      else if (!strcmp(argv[iArgPtr], "--info") ||
               !strcmp(argv[iArgPtr], "-i"))
      {
        if (g_cecLogLevel == -1)
          g_cecLogLevel = CEC_LOG_WARNING + CEC_LOG_ERROR;
        if (libcecc_initialise(&g_config, &g_iface, NULL) == 1)
        {
          char verbuf[10];
          g_iface.version_to_string(g_config.serverVersion, verbuf, sizeof(verbuf));
          printf("libCEC version: %s %s\n", verbuf, g_iface.get_lib_info(g_iface.connection));
          libcecc_destroy(&g_iface);
        }
        bReturn = 0;
      }
      else if (!strcmp(argv[iArgPtr], "--list-devices") ||
               !strcmp(argv[iArgPtr], "-l"))
      {
        if (g_cecLogLevel == -1)
          g_cecLogLevel = CEC_LOG_WARNING + CEC_LOG_ERROR;
        if (libcecc_initialise(&g_config, &g_iface, NULL) == 1)
        {
          cec_list_devices();
          libcecc_destroy(&g_iface);
        }
        bReturn = 0;
      }
      else if (!strcmp(argv[iArgPtr], "--single-command") ||
          !strcmp(argv[iArgPtr], "-s"))
      {
        g_bSingleCommand = 1;
        ++iArgPtr;
      }
      else if (!strcmp(argv[iArgPtr], "--help") ||
               !strcmp(argv[iArgPtr], "-h"))
      {
        if (g_cecLogLevel == -1)
          g_cecLogLevel = CEC_LOG_WARNING + CEC_LOG_ERROR;

//        TODO ShowHelpCommandLine(argv[0]);
        return 0;
      }
      else if (!strcmp(argv[iArgPtr], "-b") ||
               !strcmp(argv[iArgPtr], "--base"))
      {
        if (argc >= iArgPtr + 2)
        {
          g_config.baseDevice = (cec_logical_address)atoi(argv[iArgPtr + 1]);
          printf("using base device '%d'\n", (int)g_config.baseDevice);
          ++iArgPtr;
        }
        ++iArgPtr;
      }
      else if (!strcmp(argv[iArgPtr], "-p") ||
               !strcmp(argv[iArgPtr], "--port"))
      {
        if (argc >= iArgPtr + 2)
        {
          uint8_t hdmiport = (int8_t)atoi(argv[iArgPtr + 1]);
          if (hdmiport < 1)
              hdmiport = 1;
          if (hdmiport > 15)
              hdmiport = 15;
          g_config.iHDMIPort = hdmiport;
          printf("using HDMI port '%d'\n", (int)g_config.iHDMIPort);
          ++iArgPtr;
        }
        ++iArgPtr;
      }
      else if (!strcmp(argv[iArgPtr], "-r") ||
               !strcmp(argv[iArgPtr], "--rom"))
      {
        printf("using settings from EEPROM\n");
        g_config.bGetSettingsFromROM = 1;
        ++iArgPtr;
      }
      else if (!strcmp(argv[iArgPtr], "-o") ||
               !strcmp(argv[iArgPtr], "--osd-name"))
      {
        if (argc >= iArgPtr + 2)
        {
          snprintf(g_config.strDeviceName, 13, "%s", argv[iArgPtr + 1]);
          printf("using osd name '%s'\n", g_config.strDeviceName);
          ++iArgPtr;
        }
        ++iArgPtr;
      }
      else if (!strcmp(argv[iArgPtr], "-m") ||
               !strcmp(argv[iArgPtr], "--monitor"))
      {
        printf("starting a monitor-only client. use 'mon 0' to switch to normal mode\n");
        g_config.bMonitorOnly = 1;
        ++iArgPtr;
      }
      else
      {
        strcpy(g_strPort, argv[iArgPtr++]);
      }
    }
  }

  return bReturn;
}

static int cec_process_command_as(const char* data)
{
  if (strncmp(data, "as", 2) == 0)
  {
    g_iface.set_active_source(g_iface.connection, g_config.deviceTypes.types[0]);
    // wait for the source switch to finish for 15 seconds tops
    if (g_bSingleCommand)
    {
      int isactive = 0;
      int timeout = 15;
      while (timeout-- > 0)
      {
        isactive = g_iface.is_libcec_active_source(g_iface.connection);
        if (!isactive)
          sleep(1);
      }
    }
    return 1;
  }

  return 0;
}

static int cec_process_command_scan(const char* data)
{
  if (strncmp(data, "scan", 4) == 0)
  {
    char buffer[10000] = { 0 };
    char tmpbuf[50];
    int bufferpos = 0;
    cec_logical_addresses addresses;
    cec_logical_address activeSource;
    uint8_t iPtr;

    printf("requesting CEC bus information ...\n");

    bufferpos += snprintf(buffer + bufferpos, sizeof(buffer) - bufferpos, "CEC bus information\n===================\n");
    addresses = g_iface.get_active_devices(g_iface.connection);
    activeSource = g_iface.get_active_source(g_iface.connection);
    for (iPtr = 0; iPtr < 16; iPtr++)
    {
      if (addresses.addresses[iPtr])
      {
        cec_menu_language lang;
        cec_osd_name osdName;
        uint64_t iVendorId        = g_iface.get_device_vendor_id(g_iface.connection, (cec_logical_address)iPtr);
        uint16_t iPhysicalAddress = g_iface.get_device_physical_address(g_iface.connection, (cec_logical_address)iPtr);
        int      bActive          = g_iface.is_active_source(g_iface.connection, (cec_logical_address)iPtr);
        cec_version iCecVersion   = g_iface.get_device_cec_version(g_iface.connection, (cec_logical_address)iPtr);
        cec_power_status power    = g_iface.get_device_power_status(g_iface.connection, (cec_logical_address)iPtr);

        g_iface.logical_address_to_string(iPtr, tmpbuf, sizeof(tmpbuf));
        bufferpos += snprintf(buffer + bufferpos, sizeof(buffer) - bufferpos, "device #%X: %s\n", (int)iPtr, tmpbuf);
        bufferpos += snprintf(buffer + bufferpos, sizeof(buffer) - bufferpos, "address:       %x.%x.%x.%x\n", (iPhysicalAddress >> 12) & 0xF, (iPhysicalAddress >> 8) & 0xF, (iPhysicalAddress >> 4) & 0xF, iPhysicalAddress & 0xF);
        bufferpos += snprintf(buffer + bufferpos, sizeof(buffer) - bufferpos, "active source: %s\n", (bActive ? "yes" : "no"));
        g_iface.vendor_id_to_string(iVendorId, tmpbuf, sizeof(tmpbuf));
        bufferpos += snprintf(buffer + bufferpos, sizeof(buffer) - bufferpos, "vendor:        %s\n", tmpbuf);
        g_iface.get_device_osd_name(g_iface.connection, (cec_logical_address)iPtr, osdName);
        bufferpos += snprintf(buffer + bufferpos, sizeof(buffer) - bufferpos, "osd string:    %s\n", osdName);
        g_iface.cec_version_to_string(iCecVersion, tmpbuf, sizeof(tmpbuf));
        bufferpos += snprintf(buffer + bufferpos, sizeof(buffer) - bufferpos, "CEC version:   %s\n", tmpbuf);
        g_iface.power_status_to_string(power, tmpbuf, sizeof(tmpbuf));
        bufferpos += snprintf(buffer + bufferpos, sizeof(buffer) - bufferpos, "power status:  %s\n", tmpbuf);
        g_iface.get_device_menu_language(g_iface.connection, iPtr, lang);
        bufferpos += snprintf(buffer + bufferpos, sizeof(buffer) - bufferpos, "language:      %s\n", lang);
        bufferpos += snprintf(buffer + bufferpos, sizeof(buffer) - bufferpos, "\n\n");
      }
    }

    activeSource = g_iface.get_active_source(g_iface.connection);
    g_iface.logical_address_to_string(activeSource, tmpbuf, sizeof(tmpbuf));
    bufferpos += snprintf(buffer + bufferpos, sizeof(buffer) - bufferpos, "currently active source: %s (%d)", tmpbuf, (int)activeSource);

    printf("%s\n", buffer);
    return 1;
  }

  return 0;
}

static int cec_process_console_command(const char* buffer)
{
  size_t buflen;
  buflen = strlen(buffer);

  if (strncmp(buffer, "q", 1) == 0 || strncmp(buffer, "quit", 4) == 0)
    return 0;

  cec_process_command_as(buffer) ||
  cec_process_command_scan(buffer);
  //TODO

  return 1;
}

int main(int argc, char *argv[])
{
  char buffer[100];
  if (signal(SIGINT, sighandler) == SIG_ERR)
  {
    printf("can't register sighandler\n");
    return -1;
  }

  libcecc_reset_configuration(&g_config);
  g_config.clientVersion        = LIBCEC_VERSION_CURRENT;
  g_config.bActivateSource      = 0;
  g_config.callbacks            = &g_callbacks;
  snprintf(g_config.strDeviceName, sizeof(g_config.strDeviceName), "CEC tester");

  if (!cec_process_command_line_arguments(argc, argv))
    return 0;

  if (g_cecLogLevel == -1)
    g_cecLogLevel = g_cecDefaultLogLevel;

  if (g_config.deviceTypes.types[0] == CEC_DEVICE_TYPE_RESERVED)
  {
    if (!g_bSingleCommand)
      printf("No device type given. Using 'recording device'\n");
    g_config.deviceTypes.types[0] = CEC_DEVICE_TYPE_RECORDING_DEVICE;
  }

  if (libcecc_initialise(&g_config, &g_iface, NULL) != 1)
  {
    printf("can't initialise libCEC\n");
    return -1;
  }

  // init video on targets that need this
  g_iface.init_video_standalone(g_iface.connection);

  if (!g_bSingleCommand)
  {
    g_iface.version_to_string(g_config.serverVersion, buffer, sizeof(buffer));
    printf("CEC Parser created - libCEC version %s\n", buffer);

    //make stdin non-blocking
  #ifndef __WINDOWS__
    int flags = fcntl(0, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(0, F_SETFL, flags);
  #endif
  }

  if (g_strPort[0] == 0)
  {
    cec_adapter devices[10];
    int8_t iDevicesFound;
    if (!g_bSingleCommand)
      printf("no serial port given. trying autodetect: ");

    iDevicesFound = g_iface.find_adapters(g_iface.connection, devices, sizeof(devices) / sizeof(devices), NULL);
    if (iDevicesFound <= 0)
    {
      if (g_bSingleCommand)
        printf("autodetect ");
      printf("FAILED\n");
      libcecc_destroy(&g_iface);
      return 1;
    }
    else
    {
      if (!g_bSingleCommand)
      {
        printf("\n path:     %s\n com port: %s\n\n", devices[0].path, devices[0].comm);
      }
      strcpy(g_strPort, devices[0].comm);
    }
  }

  printf("opening a connection to the CEC adapter...\n");

  if (!g_iface.open(g_iface.connection, g_strPort, 5000))
  {
    printf("unable to open the device on port %s\n", g_strPort);
    libcecc_destroy(&g_iface);
    return 1;
  }

  if (!g_bSingleCommand)
    printf("waiting for input\n");

  while (!g_bExit && !g_bHardExit)
  {
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), stdin);

    if (cec_process_console_command(buffer) && !g_bSingleCommand && !g_bExit && !g_bHardExit)
    {
      if (buffer[0] != 0 && buffer[0] != '\n' && buffer[0] != '\r')
        printf("waiting for input\n");
    }
    else
    {
      g_bExit = 1;
    }

    if (!g_bExit && !g_bHardExit)
      usleep(50000);
  }

  libcecc_destroy(&g_iface);
  return 0;
}
