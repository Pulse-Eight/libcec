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

#include "../env.h"
#include "../include/cec.h"

#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "../lib/platform/threads/mutex.h"
#include "../lib/platform/util/timeutils.h"
#include "../lib/implementations/CECCommandHandler.h"
#include "../lib/platform/util/StdString.h"

using namespace CEC;
using namespace std;
using namespace PLATFORM;

#include <cecloader.h>

CMutex                g_outputMutex;

CEvent                g_responseEvent;
cec_opcode            g_lastCommand = CEC_OPCODE_NONE;

CEvent                g_keyEvent;
cec_user_control_code g_lastKey = CEC_USER_CONTROL_CODE_UNKNOWN;

ICECCallbacks         g_callbacks;
libcec_configuration  g_config;
ICECAdapter *         g_parser;

inline void PrintToStdOut(const char *strFormat, ...)
{
  CStdString strLog;

  va_list argList;
  va_start(argList, strFormat);
  strLog.FormatV(strFormat, argList);
  va_end(argList);

  CLockObject lock(g_outputMutex);
  cout << strLog << endl;
}

//get the first word (separated by whitespace) from string data and place that in word
//then remove that word from string data
bool GetWord(string& data, string& word)
{
  stringstream datastream(data);
  string end;

  datastream >> word;
  if (datastream.fail())
  {
    data.clear();
    return false;
  }

  size_t pos = data.find(word) + word.length();

  if (pos >= data.length())
  {
    data.clear();
    return true;
  }

  data = data.substr(pos);

  datastream.clear();
  datastream.str(data);

  datastream >> end;
  if (datastream.fail())
    data.clear();

  return true;
}

int CecLogMessage(void *UNUSED(cbParam), const cec_log_message &message)
{
  switch (message.level)
  {
  case CEC_LOG_ERROR:
    PrintToStdOut("ERROR:\t%s", message.message);
    break;
  case CEC_LOG_WARNING:
    PrintToStdOut("ERROR:\t%s", message.message);
    break;
  default:
    break;
  }

  return 0;
}

int CecKeyPress(void *UNUSED(cbParam), const cec_keypress &key)
{
  g_lastKey = key.keycode;
  g_keyEvent.Signal();
  return 0;
}

int CecCommand(void *UNUSED(cbParam), const cec_command &command)
{
  g_lastCommand = command.opcode;
  g_responseEvent.Signal();
  return 0;
}

bool ProcessConsoleCommand(string &input)
{
  if (!input.empty())
  {
    string command;
    if (GetWord(input, command))
    {
      if (command == "q" || command == "quit")
        return false;
    }
  }
  return true;
}

bool OpenConnection(cec_device_type type = CEC_DEVICE_TYPE_RECORDING_DEVICE)
{
  g_config.Clear();
  snprintf(g_config.strDeviceName, 13, "CEC-config");
  g_config.callbackParam      = NULL;
  g_config.clientVersion      = (uint32_t)CEC_CLIENT_VERSION_1_8_1;
  g_callbacks.CBCecLogMessage = &CecLogMessage;
  g_callbacks.CBCecKeyPress   = &CecKeyPress;
  g_callbacks.CBCecCommand    = &CecCommand;
  g_config.callbacks          = &g_callbacks;

  g_config.deviceTypes.add(type);

  g_parser = LibCecInitialise(&g_config);
  if (!g_parser)
    return false;

  // init video on targets that need this
  g_parser->InitVideoStandalone();

  CStdString strPort;
  cec_adapter devices[10];
  uint8_t iDevicesFound = g_parser->FindAdapters(devices, 10, NULL);
  if (iDevicesFound <= 0)
  {
    PrintToStdOut("autodetect FAILED");
    UnloadLibCec(g_parser);
    return false;
  }
  else
  {
    strPort = devices[0].comm;
  }

  PrintToStdOut("opening a connection to the CEC adapter...");
  if (!g_parser->Open(strPort.c_str()))
  {
    PrintToStdOut("unable to open the device on port %s", strPort.c_str());
    UnloadLibCec(g_parser);
    return false;
  }

  g_parser->GetCurrentConfiguration(&g_config);
  PrintToStdOut("CEC Parser created - libCEC version %s", g_parser->ToString((cec_server_version)g_config.serverVersion));

  return true;
}

int8_t FindPhysicalAddressPortNumber(void)
{
  PrintToStdOut("Enter the HDMI port number to which you connected your CEC adapter, followed by <enter>. Valid ports are in the range 1-15. Anything else will cancel this wizard.");
  string input;
  getline(cin, input);
  cin.clear();
  if (input.empty())
    return -1;

  int hdmiport = atoi(input.c_str());
  return (hdmiport < 1 || hdmiport > 15) ? -1 : (int8_t)hdmiport;
}

cec_logical_address FindPhysicalAddressBaseDevice(void)
{
  PrintToStdOut("Press 1 if your CEC adapter is connected to your TV or\nPress 2 if it's connected to an AVR, followed by <enter>. Anything else will cancel this wizard.");

  string input;
  getline(cin, input);
  cin.clear();
  if (input.empty() || (input != "1" && input != "2"))
  {
    PrintToStdOut("Exiting...");
    return CECDEVICE_UNKNOWN;
  }
  return (input == "2") ?
    CECDEVICE_AUDIOSYSTEM :
    CECDEVICE_TV;
}

uint16_t FindPhysicalAddress(void)
{
  PrintToStdOut("=== Physical Address Configuration ===\n");
  uint16_t iAddress(CEC_INVALID_PHYSICAL_ADDRESS);

  PrintToStdOut("Do you want to let libCEC try to autodetect the address (y/n)?");
  string input;
  getline(cin, input);
  cin.clear();
  if (input == "y" || input == "Y")
  {
    cec_logical_address baseDevice = FindPhysicalAddressBaseDevice();
    if (baseDevice == CECDEVICE_UNKNOWN)
      return iAddress;

    int8_t iPortNumber = FindPhysicalAddressPortNumber();
    if (iPortNumber == -1)
      return iAddress;

    PrintToStdOut("Trying to detect the physical address...");
    if (!g_parser->SetHDMIPort(baseDevice, iPortNumber))
      PrintToStdOut("Failed. Please enter the address manually, or restart this wizard and use different settings.");
    else
    {
      g_parser->GetCurrentConfiguration(&g_config);
      iAddress = g_parser->GetDevicePhysicalAddress(g_config.logicalAddresses.primary);
      if (iAddress == 0 || iAddress == CEC_INVALID_PHYSICAL_ADDRESS)
        PrintToStdOut("Failed. Please enter the address manually, or restart this wizard and use different settings.");
    }
  }

  if (iAddress == 0 || iAddress == CEC_INVALID_PHYSICAL_ADDRESS)
  {
    PrintToStdOut("Please enter the physical address (0001 - FFFE), followed by <enter>.");
    getline(cin, input);
    cin.clear();

    int iAddressTmp;
    if (sscanf(input.c_str(), "%x", &iAddressTmp) == 1)
    {
      if (iAddressTmp <= CEC_PHYSICAL_ADDRESS_TV || iAddressTmp > CEC_MAX_PHYSICAL_ADDRESS)
        iAddressTmp = CEC_INVALID_PHYSICAL_ADDRESS;
      iAddress = (uint16_t)iAddressTmp;
    }
  }

  if (iAddress != 0)
  {
    g_parser->SetPhysicalAddress(iAddress);
    g_parser->SetActiveSource(g_config.deviceTypes[0]);
  }

  return iAddress;
}

bool PowerOnTV(uint64_t iTimeout = 60000)
{
  cec_power_status currentTvPower(CEC_POWER_STATUS_UNKNOWN);
  uint64_t iNow = GetTimeMs();
  uint64_t iTarget = iNow + iTimeout;

  if (currentTvPower != CEC_POWER_STATUS_ON)
  {
    currentTvPower = g_parser->GetDevicePowerStatus(CECDEVICE_TV);
    if (currentTvPower != CEC_POWER_STATUS_ON)
    {
      PrintToStdOut("Sending 'power on' command to the TV\n=== Please wait ===");
      g_parser->PowerOnDevices(CECDEVICE_TV);
      while (iTarget > iNow)
      {
        g_responseEvent.Wait((uint32_t)(iTarget - iNow));
        if (g_lastCommand == CEC_OPCODE_REQUEST_ACTIVE_SOURCE)
          break;
        iNow = GetTimeMs();
      }
    }
  }

  currentTvPower = g_parser->GetDevicePowerStatus(CECDEVICE_TV);

  if (currentTvPower != CEC_POWER_STATUS_ON)
    PrintToStdOut("Failed to power on the TV, or the TV does not respond properly");

  return currentTvPower == CEC_POWER_STATUS_ON;
}

int main (int UNUSED(argc), char *UNUSED(argv[]))
{
  g_callbacks.Clear();
  g_config.Clear();
  PrintToStdOut("=== USB-CEC Adapter Configuration ===\n");
  if (!OpenConnection())
    return 1;

  if (!PowerOnTV())
    return 1;

  bool bAddressOk(false);
  while (!bAddressOk)
  {
    uint16_t iAddress = FindPhysicalAddress();

    PrintToStdOut("Physical address: %4X", iAddress);
    PrintToStdOut("Is this correct (y/n)?");
    string input;
    getline(cin, input);
    cin.clear();
    bAddressOk = (input == "y" || input == "Y");
  }

  g_parser->GetCurrentConfiguration(&g_config);

  {
    cec_menu_language lang;
    if (g_parser->GetDeviceMenuLanguage(CECDEVICE_TV, &lang))
    {
      PrintToStdOut("TV menu language: %s", lang.language);
      PrintToStdOut("Do you want the application to use the menu language of the TV (y/n)?");
      string input;
      getline(cin, input);
      cin.clear();
      g_config.bUseTVMenuLanguage = (input == "y" || input == "Y") ? 1 : 0;
    }
    else
    {
      PrintToStdOut("The TV did not respond properly to the menu language request.");
    }
  }

  {
    PrintToStdOut("Do you want to make the CEC adapter the active source when starting the application (y/n)?");
    string input;
    getline(cin, input);
    cin.clear();
    g_config.bActivateSource = (input == "y" || input == "Y") ? 1 : 0;
  }

  {
    PrintToStdOut("Do you want to power on the TV when starting the application (y/n)?");
    string input;
    getline(cin, input);
    cin.clear();
    if (input == "y" || input == "Y")
      g_config.wakeDevices.Set(CECDEVICE_TV);
  }

  {
    PrintToStdOut("Do you want to power off CEC devices when closing the application (y/n)?");
    string input;
    getline(cin, input);
    cin.clear();
    if (input == "y" || input == "Y")
      g_config.powerOffDevices.Set(CECDEVICE_TV);
  }

  {
    PrintToStdOut("Do you want to power off CEC devices when the screensaver is activated (y/n)?");
    string input;
    getline(cin, input);
    cin.clear();
    g_config.bPowerOffScreensaver = (input == "y" || input == "Y") ? 1 : 0;
  }

  {
    PrintToStdOut("Do you want to put the PC in standby when the TV is put in standby mode (y/n)?");
    string input;
    getline(cin, input);
    cin.clear();
    g_config.bPowerOffOnStandby = (input == "y" || input == "Y") ? 1 : 0;
  }

  {
    PrintToStdOut("Do you want to send an inactive source message when stopping the application (y/n)?");
    string input;
    getline(cin, input);
    cin.clear();
    g_config.bSendInactiveSource = (input == "y" || input == "Y") ? 1 : 0;
  }

  PrintToStdOut("\n\n=== USB-CEC Adapter Configuration Summary ===");
  PrintToStdOut("HDMI port number:                                        %d", g_config.iHDMIPort);
  PrintToStdOut("Connected to HDMI device:                                %X", (uint8_t)g_config.baseDevice);
  PrintToStdOut("Physical address:                                        %4X", g_config.iPhysicalAddress);
  PrintToStdOut("Use the TV's language setting:                           %s", g_config.bUseTVMenuLanguage ? "yes" : "no");
  PrintToStdOut("Make the adapter the active source when starting XBMC:   %s", g_config.bActivateSource ? "yes" : "no");
  PrintToStdOut("Power on the TV when starting XBMC:                      %s", g_config.wakeDevices.IsSet(CECDEVICE_BROADCAST) ? "yes" : "no");
  PrintToStdOut("Power off devices when stopping XBMC:                    %s", g_config.powerOffDevices.IsSet(CECDEVICE_BROADCAST) ? "yes" : "no");
  PrintToStdOut("Put devices in standby mode when activating screensaver: %s", g_config.bPowerOffScreensaver ? "yes" : "no");
  PrintToStdOut("Put this PC in standby mode when the TV is switched off: %s", g_config.bPowerOffOnStandby ? "yes" : "no");
  PrintToStdOut("Send an inactive source message when stopping XBMC:      %s\n\n", g_config.bSendInactiveSource ? "yes" : "no");

  PrintToStdOut("Storing settings ...");
  if (g_parser->PersistConfiguration(&g_config))
    PrintToStdOut("Settings stored.");
  else
    PrintToStdOut("The settings could not be stored");

  ofstream configOutput;
  configOutput.open("usb_2548_1001.xml");
  if (configOutput.is_open())
  {
    CStdString strWakeDevices;
    for (uint8_t iPtr = 0; iPtr < 16; iPtr++)
      if (g_config.wakeDevices[iPtr])
        strWakeDevices.AppendFormat(" %d", iPtr);
    CStdString strStandbyDevices;
    for (uint8_t iPtr = 0; iPtr < 16; iPtr++)
      if (g_config.powerOffDevices[iPtr])
        strStandbyDevices.AppendFormat(" %d", iPtr);

    configOutput <<
        "<settings>\n" <<
          "\t<setting id=\"enabled\" value=\"1\" />\n" <<
          "\t<setting id=\"activate_source\" value=\"" << (int)g_config.bActivateSource << "\" />\n" <<
          "\t<setting id=\"wake_devices\" value=\"" << strWakeDevices.c_str() << "\" />\n" <<
          "\t<setting id=\"standby_devices\" value=\"" << strStandbyDevices.c_str() << "\" />\n" <<
          "\t<setting id=\"cec_standby_screensaver\" value=\"" << (int)g_config.bPowerOffScreensaver << "\" />\n" <<
          "\t<setting id=\"standby_pc_on_tv_standby\" value=\"" << (int)g_config.bPowerOffOnStandby << "\" />\n" <<
          "\t<setting id=\"use_tv_menu_language\" value=\"" << (int)g_config.bUseTVMenuLanguage << "\" />\n" <<
          "\t<setting id=\"physical_address\" value=\"" << hex << g_config.iPhysicalAddress << "\" />\n" <<
          "\t<setting id=\"cec_hdmi_port\" value=\"" << g_config.iHDMIPort << "\" />\n" <<
          "\t<setting id=\"connected_device\" value=\"" << (int)g_config.baseDevice << "\" />\n" <<
          "\t<setting id=\"port\" value=\"\" />\n" <<
          "\t<setting id=\"send_inactive_source\" value=\"" << (int)g_config.bSendInactiveSource << "\" />\n" <<
        "</settings>";
    configOutput.close();

    PrintToStdOut("The configuration has been stored in 'usb_2548_1001.xml'. Copy this file to ~/.userdata/peripheral_data to use it in XBMC");
  }

  g_parser->StandbyDevices();
  g_parser->Close();
  UnloadLibCec(g_parser);

  PrintToStdOut("Press enter to close this wizard.");
  string input;
  getline(cin, input);
  return 0;
}
