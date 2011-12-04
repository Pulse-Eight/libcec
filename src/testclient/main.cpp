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

#include <cec.h>

#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "../lib/platform/threads.h"
#include "../lib/util/StdString.h"
#include "../lib/implementations/CECCommandHandler.h"

using namespace CEC;
using namespace std;

#define CEC_TEST_CLIENT_VERSION 1

#include <cecloader.h>

int                  g_cecLogLevel(CEC_LOG_ALL);
ofstream             g_logOutput;
bool                 g_bShortLog(false);
CStdString           g_strPort;
int8_t               g_iHDMIPort(-1);
cec_device_type_list g_typeList;
bool                 g_bSingleCommand(false);


inline bool HexStrToInt(const std::string& data, uint8_t& value)
{
  int iTmp(0);
  if (sscanf(data.c_str(), "%x", &iTmp) == 1)
  {
    if (iTmp > 256)
      value = 255;
	  else if (iTmp < 0)
      value = 0;
    else
      value = (uint8_t) iTmp;

    return true;
  }

  return false;
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

void FlushLog(ICECAdapter *cecParser)
{
  cec_log_message message;
  while (cecParser && cecParser->GetNextLogMessage(&message))
  {
    if ((message.level & g_cecLogLevel) == message.level)
    {
      CStdString strLevel;
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

      CStdString strFullLog;
      strFullLog.Format("%s[%16lld]\t%s", strLevel.c_str(), message.time, message.message);
      cout << strFullLog.c_str() << endl;

      if (g_logOutput.is_open())
      {
        if (g_bShortLog)
          g_logOutput << message.message << endl;
        else
          g_logOutput << strFullLog.c_str() << endl;
      }
    }
  }
}

void ListDevices(ICECAdapter *parser)
{
  cec_adapter *devices = new cec_adapter[10];
  uint8_t iDevicesFound = parser->FindAdapters(devices, 10, NULL);
  if (iDevicesFound <= 0)
  {
    cout << "Found devices: NONE" << endl;
  }
  else
  {
    CStdString strLog;
    strLog.Format("Found devices: %d", iDevicesFound);
    cout << strLog.c_str() << endl;
    for (unsigned int iDevicePtr = 0; iDevicePtr < iDevicesFound; iDevicePtr++)
    {
      CStdString strDevice;
      strDevice.Format("device:        %d\npath:          %s\ncom port:      %s", iDevicePtr + 1, devices[iDevicePtr].path, devices[iDevicePtr].comm);
      cout << endl << strDevice.c_str() << endl;
    }
  }
}

void ShowHelpCommandLine(const char* strExec)
{
  cout << endl <<
      strExec << " {-h|--help|-l|--list-devices|[COM PORT]}" << endl <<
      endl <<
      "parameters:" << endl <<
      "  -h --help                   Shows this help text" << endl <<
      "  -l --list-devices           List all devices on this system" << endl <<
      "  -t --type {p|r|t|a}         The device type to use. More than one is possible." << endl <<
      "  -p --port {int}             The HDMI port to use as active source." << endl <<
      "  -f --log-file {file}        Writes all libCEC log message to a file" << endl <<
      "  -sf --short-log-file {file} Writes all libCEC log message without timestamps" << endl <<
      "                              and log levels to a file." << endl <<
      "  -d --log-level {level}      Sets the log level. See cectypes.h for values." << endl <<
      "  -s --single-command         Execute a single command and exit. Does not power" << endl <<
      "                              on devices on startup and power them off on exit." << endl <<
      "  [COM PORT]                  The com port to connect to. If no COM" << endl <<
      "                              port is given, the client tries to connect to the" << endl <<
      "                              first device that is detected." << endl <<
      endl <<
      "Type 'h' or 'help' and press enter after starting the client to display all " << endl <<
      "available commands" << endl;
}

ICECAdapter *CreateParser(cec_device_type_list typeList)
{
  ICECAdapter *parser = LibCecInit("CECTester", typeList);
  if (!parser || parser->GetMinLibVersion() > CEC_TEST_CLIENT_VERSION)
  {
  #ifdef __WINDOWS__
    cout << "Cannot load libcec.dll" << endl;
  #else
    cout << "Cannot load libcec.so" << endl;
  #endif
    return NULL;
  }

  CStdString strLog;
  strLog.Format("CEC Parser created - libcec version %d.%d", parser->GetLibVersionMajor(), parser->GetLibVersionMinor());
  cout << strLog.c_str() << endl;

  return parser;
}

void ShowHelpConsole(void)
{
  cout << endl <<
  "================================================================================" << endl <<
  "Available commands:" << endl <<
  endl <<
  "[tx] {bytes}              transfer bytes over the CEC line." << endl <<
  "[txn] {bytes}             transfer bytes but don't wait for transmission ACK." << endl <<
  "[on] {address}            power on the device with the given logical address." << endl <<
  "[standby] {address}       put the device with the given address in standby mode." << endl <<
  "[la] {logical address}    change the logical address of the CEC adapter." << endl <<
  "[p] {device} {port}       change the HDMI port number of the CEC adapter." << endl <<
  "[pa] {physical address}   change the physical address of the CEC adapter." << endl <<
  "[osd] {addr} {string}     set OSD message on the specified device." << endl <<
  "[ver] {addr}              get the CEC version of the specified device." << endl <<
  "[ven] {addr}              get the vendor ID of the specified device." << endl <<
  "[lang] {addr}             get the menu language of the specified device." << endl <<
  "[pow] {addr}              get the power status of the specified device." << endl <<
  "[name] {addr}             get the OSD name of the specified device." << endl <<
  "[poll] {addr}             poll the specified device." << endl <<
  "[lad]                     lists active devices on the bus" << endl <<
  "[ad] {addr}               checks whether the specified device is active." << endl <<
  "[at] {type}               checks whether the specified device type is active." << endl <<
  "[volup]                   send a volume up command to the amp if present" << endl <<
  "[voldown]                 send a volume down command to the amp if present" << endl <<
  "[mute]                    send a mute/unmute command to the amp if present" << endl <<
  "[scan]                    scan the CEC bus and display device info" << endl <<
  "[mon] {1|0}               enable or disable CEC bus monitoring." << endl <<
  "[log] {1 - 31}            change the log level. see cectypes.h for values." << endl <<
  "[ping]                    send a ping command to the CEC adapter." << endl <<
  "[bl]                      to let the adapter enter the bootloader, to upgrade" << endl <<
  "                          the flash rom." << endl <<
  "[r]                       reconnect to the CEC adapter." << endl <<
  "[h] or [help]             show this help." << endl <<
  "[q] or [quit]             to quit the CEC test client and switch off all" << endl <<
  "                          connected CEC devices." << endl <<
  "================================================================================" << endl;
}

bool ProcessCommandTX(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "tx" || command == "txn")
  {
    string strvalue;
    uint8_t ivalue;
    cec_command bytes;
    bytes.Clear();

    while (GetWord(arguments, strvalue) && HexStrToInt(strvalue, ivalue))
      bytes.PushBack(ivalue);

    if (command == "txn")
      bytes.transmit_timeout = 0;

    parser->Transmit(bytes);

    return true;
  }

  return false;
}

bool ProcessCommandON(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "on")
  {
    string strValue;
    uint8_t iValue = 0;
    if (GetWord(arguments, strValue) && HexStrToInt(strValue, iValue) && iValue <= 0xF)
    {
      parser->PowerOnDevices((cec_logical_address) iValue);
      return true;
    }
    else
    {
      cout << "invalid destination" << endl;
    }
  }

  return false;
}

bool ProcessCommandSTANDBY(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "standby")
  {
    string strValue;
    uint8_t iValue = 0;
    if (GetWord(arguments, strValue) && HexStrToInt(strValue, iValue) && iValue <= 0xF)
    {
      parser->StandbyDevices((cec_logical_address) iValue);
      return true;
    }
    else
    {
      cout << "invalid destination" << endl;
    }
  }

  return false;
}

bool ProcessCommandPOLL(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "poll")
  {
    string strValue;
    uint8_t iValue = 0;
    if (GetWord(arguments, strValue) && HexStrToInt(strValue, iValue) && iValue <= 0xF)
    {
      if (parser->PollDevice((cec_logical_address) iValue))
        cout << "POLL message sent" << endl;
      else
        cout << "POLL message not sent" << endl;
      return true;
    }
    else
    {
      cout << "invalid destination" << endl;
    }
  }

  return false;
}

bool ProcessCommandLA(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "la")
  {
    string strvalue;
    if (GetWord(arguments, strvalue))
    {
      parser->SetLogicalAddress((cec_logical_address) atoi(strvalue.c_str()));
      return true;
    }
  }

  return false;
}

bool ProcessCommandP(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "p")
  {
    string strPort, strDevice;
    if (GetWord(arguments, strDevice) && GetWord(arguments, strPort))
    {
      parser->SetHDMIPort((cec_logical_address)atoi(strDevice.c_str()), (uint8_t)atoi(strPort.c_str()));
      return true;
    }
  }

  return false;
}

bool ProcessCommandPA(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "pa")
  {
    string strB1, strB2;
    uint8_t iB1, iB2;
    if (GetWord(arguments, strB1) && HexStrToInt(strB1, iB1) &&
        GetWord(arguments, strB2) && HexStrToInt(strB2, iB2))
    {
      uint16_t iPhysicalAddress = ((uint16_t)iB1 << 8) + iB2;
      parser->SetPhysicalAddress(iPhysicalAddress);
      return true;
    }
  }

  return false;
}

bool ProcessCommandOSD(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "osd")
  {
    bool bFirstWord(false);
    string strAddr, strMessage, strWord;
    uint8_t iAddr;
    if (GetWord(arguments, strAddr) && HexStrToInt(strAddr, iAddr) && iAddr < 0xF)
    {
      while (GetWord(arguments, strWord))
      {
        if (bFirstWord)
        {
          bFirstWord = false;
          strMessage.append(" ");
        }
        strMessage.append(strWord);
      }
      parser->SetOSDString((cec_logical_address) iAddr, CEC_DISPLAY_CONTROL_DISPLAY_FOR_DEFAULT_TIME, strMessage.c_str());
      return true;
    }
  }

  return false;
}

bool ProcessCommandPING(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "ping")
  {
    parser->PingAdapter();
    return true;
  }

  return false;
}

bool ProcessCommandVOLUP(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "volup")
  {
    CStdString strLog;
    strLog.Format("volume up: %2X", parser->VolumeUp());
    cout << strLog.c_str() << endl;
    return true;
  }

  return false;
}

bool ProcessCommandVOLDOWN(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "voldown")
  {
    CStdString strLog;
    strLog.Format("volume up: %2X", parser->VolumeDown());
    cout << strLog.c_str() << endl;
    return true;
  }

  return false;
}

bool ProcessCommandMUTE(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "mute")
  {
    CStdString strLog;
    strLog.Format("mute: %2X", parser->MuteAudio());
    cout << strLog.c_str() << endl;
    return true;
  }

  return false;
}

bool ProcessCommandMON(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "mon")
  {
    CStdString strEnable;
    if (GetWord(arguments, strEnable) && (strEnable.Equals("0") || strEnable.Equals("1")))
    {
      parser->SwitchMonitoring(strEnable.Equals("1"));
      return true;
    }
  }

  return false;
}

bool ProcessCommandBL(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "bl")
  {
    parser->StartBootloader();
    return true;
  }

  return false;
}

bool ProcessCommandLANG(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "lang")
  {
    CStdString strDev;
    if (GetWord(arguments, strDev))
    {
      int iDev = atoi(strDev);
      if (iDev >= 0 && iDev < 15)
      {
        CStdString strLog;
        cec_menu_language language;
        if (parser->GetDeviceMenuLanguage((cec_logical_address) iDev, &language))
          strLog.Format("menu language '%s'", language.language);
        else
          strLog = "failed!";
        cout << strLog.c_str() << endl;
        return true;
      }
    }
  }

  return false;
}

bool ProcessCommandVEN(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "ven")
  {
    CStdString strDev;
    if (GetWord(arguments, strDev))
    {
      int iDev = atoi(strDev);
      if (iDev >= 0 && iDev < 15)
      {
        uint64_t iVendor = parser->GetDeviceVendorId((cec_logical_address) iDev);
        CStdString strLog;
        strLog.Format("vendor id: %06x", iVendor);
        cout << strLog.c_str() << endl;
        return true;
      }
    }
  }

  return false;
}

bool ProcessCommandVER(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "ver")
  {
    CStdString strDev;
    if (GetWord(arguments, strDev))
    {
      int iDev = atoi(strDev);
      if (iDev >= 0 && iDev < 15)
      {
        cec_version iVersion = parser->GetDeviceCecVersion((cec_logical_address) iDev);
        cout << "CEC version " << parser->ToString(iVersion) << endl;
        return true;
      }
    }
  }

  return false;
}

bool ProcessCommandPOW(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "pow")
  {
    CStdString strDev;
    if (GetWord(arguments, strDev))
    {
      int iDev = atoi(strDev);
      if (iDev >= 0 && iDev < 15)
      {
        cec_power_status iPower = parser->GetDevicePowerStatus((cec_logical_address) iDev);
        cout << "power status: " << parser->ToString(iPower) << endl;

        return true;
      }
    }
  }

  return false;
}

bool ProcessCommandNAME(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "name")
  {
    CStdString strDev;
    if (GetWord(arguments, strDev))
    {
      int iDev = atoi(strDev);
      if (iDev >= 0 && iDev < 15)
      {
        cec_osd_name name = parser->GetOSDName((cec_logical_address)iDev);
        cout << "OSD name of device " << iDev << " is '" << name.name << "'" << endl;
      }
      return true;
    }
  }

  return false;
}

bool ProcessCommandLAD(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "lad")
  {
    cout << "listing active devices:" << endl;
    cec_logical_addresses addresses = parser->GetActiveDevices();
    for (uint8_t iPtr = 0; iPtr < 16; iPtr++)
      if (addresses[iPtr])
        cout << "logical address " << (int)iPtr << endl;
    return true;
  }

  return false;
}

bool ProcessCommandAD(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "ad")
  {
    CStdString strDev;
    if (GetWord(arguments, strDev))
    {
      int iDev = atoi(strDev);
      if (iDev >= 0 && iDev < 15)
        cout << "logical address " << iDev << " is " << (parser->IsActiveDevice((cec_logical_address)iDev) ? "active" : "not active") << endl;
    }
  }

  return false;
}

bool ProcessCommandAT(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "at")
  {
    CStdString strType;
    if (GetWord(arguments, strType))
    {
      cec_device_type type = CEC_DEVICE_TYPE_TV;
      if (strType.Equals("a"))
        type = CEC_DEVICE_TYPE_AUDIO_SYSTEM;
      else if (strType.Equals("p"))
        type = CEC_DEVICE_TYPE_PLAYBACK_DEVICE;
      else if (strType.Equals("r"))
        type = CEC_DEVICE_TYPE_RECORDING_DEVICE;
      else if (strType.Equals("t"))
        type = CEC_DEVICE_TYPE_TUNER;
      cout << "device " << type << " is " << (parser->IsActiveDeviceType(type) ? "active" : "not active") << endl;
      return true;
    }
  }

  return false;
}

bool ProcessCommandR(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "r")
  {
    cout << "closing the connection" << endl;
    parser->Close();
    FlushLog(parser);

    cout << "opening a new connection" << endl;
    parser->Open(g_strPort.c_str());
    FlushLog(parser);

    cout << "setting active source" << endl;
    parser->SetActiveSource();
    return true;
  }

  return false;
}

bool ProcessCommandH(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "h" || command == "help")
  {
    ShowHelpConsole();
    return true;
  }

  return false;
}

bool ProcessCommandLOG(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "log")
  {
    CStdString strLevel;
    if (GetWord(arguments, strLevel))
    {
      int iNewLevel = atoi(strLevel);
      if (iNewLevel >= CEC_LOG_ERROR && iNewLevel <= CEC_LOG_ALL)
      {
        g_cecLogLevel = iNewLevel;
        cout << "log level changed to " << strLevel.c_str() << endl;
        return true;
      }
    }
  }

  return false;
}

bool ProcessCommandSCAN(ICECAdapter *parser, const string &command, string &arguments)
{
  if (command == "scan")
  {
    cout << "CEC bus information" << endl;
    cout << "===================" << endl;
    cec_logical_addresses addresses = parser->GetActiveDevices();
    for (uint8_t iPtr = 0; iPtr < 16; iPtr++)
    {
      if (addresses[iPtr])
      {
        uint64_t iVendorId      = parser->GetDeviceVendorId((cec_logical_address)iPtr);
        cec_version iCecVersion = parser->GetDeviceCecVersion((cec_logical_address)iPtr);
        cec_power_status power  = parser->GetDevicePowerStatus((cec_logical_address)iPtr);
        cec_osd_name osdName    = parser->GetOSDName((cec_logical_address)iPtr);
        cec_menu_language lang;
        lang.device = CECDEVICE_UNKNOWN;
        parser->GetDeviceMenuLanguage((cec_logical_address)iPtr, &lang);

        cout << "device #" << (int)iPtr << ": " << parser->ToString((cec_logical_address)iPtr) << endl;
        cout << "vendor:       " << parser->ToString((cec_vendor_id)iVendorId) << endl;
        cout << "osd string:   " << osdName.name << endl;
        cout << "CEC version:  " << parser->ToString(iCecVersion) << endl;
        cout << "power status: " << parser->ToString(power) << endl;
        if ((uint8_t)lang.device == iPtr)
          cout << "language:     " << lang.language << endl;
        cout << endl;
      }
    }
    return true;
  }

  return false;
}

bool ProcessConsoleCommand(ICECAdapter *parser, string &input)
{
  if (!input.empty())
  {
    string command;
    if (GetWord(input, command))
    {
      if (command == "q" || command == "quit")
        return false;

      ProcessCommandTX(parser, command, input) ||
      ProcessCommandON(parser, command, input) ||
      ProcessCommandSTANDBY(parser, command, input) ||
      ProcessCommandPOLL(parser, command, input) ||
      ProcessCommandLA(parser, command, input) ||
      ProcessCommandP(parser, command, input) ||
      ProcessCommandPA(parser, command, input) ||
      ProcessCommandOSD(parser, command, input) ||
      ProcessCommandPING(parser, command, input) ||
      ProcessCommandVOLUP(parser, command, input) ||
      ProcessCommandVOLDOWN(parser, command, input) ||
      ProcessCommandMUTE(parser, command, input) ||
      ProcessCommandMON(parser, command, input) ||
      ProcessCommandBL(parser, command, input) ||
      ProcessCommandLANG(parser, command, input) ||
      ProcessCommandVEN(parser, command, input) ||
      ProcessCommandVER(parser, command, input) ||
      ProcessCommandPOW(parser, command, input) ||
      ProcessCommandNAME(parser, command, input) ||
      ProcessCommandLAD(parser, command, input) ||
      ProcessCommandAD(parser, command, input) ||
      ProcessCommandAT(parser, command, input) ||
      ProcessCommandR(parser, command, input) ||
      ProcessCommandH(parser, command, input) ||
      ProcessCommandLOG(parser, command, input) ||
      ProcessCommandSCAN(parser, command, input);
    }
  }
  return true;
}

bool ProcessCommandLineArguments(int argc, char *argv[])
{
  bool bReturn(true);
  int iArgPtr = 1;
  while (iArgPtr < argc)
  {
    if (argc >= iArgPtr + 1)
    {
      if (!strcmp(argv[iArgPtr], "-f") ||
          !strcmp(argv[iArgPtr], "--log-file") ||
          !strcmp(argv[iArgPtr], "-sf") ||
          !strcmp(argv[iArgPtr], "--short-log-file"))
      {
        if (argc >= iArgPtr + 2)
        {
          g_logOutput.open(argv[iArgPtr + 1]);
          g_bShortLog = (!strcmp(argv[iArgPtr], "-sf") || !strcmp(argv[iArgPtr], "--short-log-file"));
          iArgPtr += 2;
        }
        else
        {
          cout << "== skipped log-file parameter: no file given ==" << endl;
          ++iArgPtr;
        }
      }
      else if (!strcmp(argv[iArgPtr], "-d") ||
          !strcmp(argv[iArgPtr], "--log-level"))
      {
        if (argc >= iArgPtr + 2)
        {
          int iNewLevel = atoi(argv[iArgPtr + 1]);
          if (iNewLevel >= CEC_LOG_ERROR && iNewLevel <= CEC_LOG_ALL)
          {
            g_cecLogLevel = iNewLevel;
            if (!g_bSingleCommand)
              cout << "log level set to " << argv[iArgPtr + 1] << endl;
          }
          else
          {
            cout << "== skipped log-level parameter: invalid level '" << argv[iArgPtr + 1] << "' ==" << endl;
          }
          iArgPtr += 2;
        }
        else
        {
          cout << "== skipped log-level parameter: no level given ==" << endl;
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
              cout << "== using device type 'playback device'" << endl;
            g_typeList.add(CEC_DEVICE_TYPE_PLAYBACK_DEVICE);
          }
          else if (!strcmp(argv[iArgPtr + 1], "r"))
          {
            if (!g_bSingleCommand)
              cout << "== using device type 'recording device'" << endl;
            g_typeList.add(CEC_DEVICE_TYPE_RECORDING_DEVICE);
          }
          else if (!strcmp(argv[iArgPtr + 1], "t"))
          {
            if (!g_bSingleCommand)
              cout << "== using device type 'tuner'" << endl;
            g_typeList.add(CEC_DEVICE_TYPE_TUNER);
          }
          else if (!strcmp(argv[iArgPtr + 1], "a"))
          {
            if (!g_bSingleCommand)
              cout << "== using device type 'audio system'" << endl;
            g_typeList.add(CEC_DEVICE_TYPE_AUDIO_SYSTEM);
          }
          else
          {
            cout << "== skipped invalid device type '" << argv[iArgPtr + 1] << "'" << endl;
          }
          ++iArgPtr;
        }
        ++iArgPtr;
      }
      else if (!strcmp(argv[iArgPtr], "--list-devices") ||
               !strcmp(argv[iArgPtr], "-l"))
      {
        ICECAdapter *parser = CreateParser(g_typeList);
        if (parser)
        {
          ListDevices(parser);
          UnloadLibCec(parser);
        }
        bReturn = false;
      }
      else if (!strcmp(argv[iArgPtr], "--single-command") ||
          !strcmp(argv[iArgPtr], "-s"))
      {
        g_bSingleCommand = true;
        ++iArgPtr;
      }
      else if (!strcmp(argv[iArgPtr], "--help") ||
               !strcmp(argv[iArgPtr], "-h"))
      {
        ShowHelpCommandLine(argv[0]);
        return 0;
      }
      else if (!strcmp(argv[iArgPtr], "-p") ||
               !strcmp(argv[iArgPtr], "--port"))
      {
        if (argc >= iArgPtr + 2)
        {
          g_iHDMIPort = (int8_t)atoi(argv[iArgPtr + 1]);
          cout << "using HDMI port '" << g_iHDMIPort << "'" << endl;
          ++iArgPtr;
        }
        ++iArgPtr;
      }
      else
      {
        g_strPort = argv[iArgPtr++];
      }
    }
  }

  return bReturn;
}

int main (int argc, char *argv[])
{
  g_typeList.clear();

  if (!ProcessCommandLineArguments(argc, argv))
    return 0;

  if (g_typeList.IsEmpty())
  {
    if (!g_bSingleCommand)
      cout << "No device type given. Using 'recording device'" << endl;
    g_typeList.add(CEC_DEVICE_TYPE_RECORDING_DEVICE);
  }

  ICECAdapter *parser = LibCecInit("CECTester", g_typeList);
  if (!parser || parser->GetMinLibVersion() > CEC_TEST_CLIENT_VERSION)
  {
#ifdef __WINDOWS__
    cout << "Cannot load libcec.dll" << endl;
#else
    cout << "Cannot load libcec.so" << endl;
#endif
    return 1;
  }

  if (!g_bSingleCommand)
  {
    CStdString strLog;
    strLog.Format("CEC Parser created - libcec version %d.%d", parser->GetLibVersionMajor(), parser->GetLibVersionMinor());
    cout << strLog.c_str() << endl;

    //make stdin non-blocking
  #ifndef __WINDOWS__
    int flags = fcntl(0, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(0, F_SETFL, flags);
  #endif
  }

  if (g_strPort.IsEmpty())
  {
    if (!g_bSingleCommand)
      cout << "no serial port given. trying autodetect: ";
    cec_adapter devices[10];
    uint8_t iDevicesFound = parser->FindAdapters(devices, 10, NULL);
    if (iDevicesFound <= 0)
    {
      if (g_bSingleCommand)
        cout << "autodetect ";
      cout << "FAILED" << endl;
      UnloadLibCec(parser);
      return 1;
    }
    else
    {
      if (!g_bSingleCommand)
      {
        cout << endl << " path:     " << devices[0].path << endl <<
            " com port: " << devices[0].comm << endl << endl;
      }
      g_strPort = devices[0].comm;
    }
  }

  if (g_iHDMIPort > 0)
  {
    parser->SetHDMIPort((cec_logical_address)CEC_DEFAULT_BASE_DEVICE, (uint8_t)g_iHDMIPort);
    FlushLog(parser);
  }

  cout << "scanning the CEC bus..." << endl;

  if (!parser->Open(g_strPort.c_str()))
  {
    cout << "unable to open the device on port " << g_strPort << endl;
    FlushLog(parser);
    UnloadLibCec(parser);
    return 1;
  }

  if (!g_bSingleCommand)
  {
    FlushLog(parser);
    cout << "cec device opened" << endl;

    parser->PowerOnDevices(CECDEVICE_TV);
    FlushLog(parser);

    parser->SetActiveSource();
    FlushLog(parser);

    cout << "waiting for input" << endl;
  }

  bool bContinue(true);
  while (bContinue)
  {
    FlushLog(parser);

    /* just ignore the command buffer and clear it */
    cec_command dummy;
    while (parser && parser->GetNextCommand(&dummy)) {}

    string input;
    getline(cin, input);
    cin.clear();

    if (ProcessConsoleCommand(parser, input) && !g_bSingleCommand)
    {
      if (!input.empty())
        cout << "waiting for input" << endl;
    }
    else
      bContinue = false;

    if (bContinue)
      CCondition::Sleep(50);
  }

  if (!g_bSingleCommand)
    parser->StandbyDevices(CECDEVICE_BROADCAST);

  parser->Close();
  FlushLog(parser);
  UnloadLibCec(parser);

  if (g_logOutput.is_open())
    g_logOutput.close();

  return 0;
}
