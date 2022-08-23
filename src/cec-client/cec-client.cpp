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
#include "cec.h"

#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <signal.h>
#include <stdlib.h>
#include "p8-platform/os.h"
#include "p8-platform/util/StringUtils.h"
#include "p8-platform/threads/threads.h"
#if defined(HAVE_CURSES_API)
  #include "curses/CursesControl.h"
#endif

using namespace CEC;
using namespace P8PLATFORM;

#include "cecloader.h"

static void PrintToStdOut(const char *strFormat, ...);

ICECCallbacks         g_callbacks;
libcec_configuration  g_config;
int                   g_cecLogLevel(-1);
int                   g_cecDefaultLogLevel(CEC_LOG_ALL);
std::ofstream         g_logOutput;
bool                  g_bShortLog(false);
std::string           g_strPort;
bool                  g_bSingleCommand(false);
volatile sig_atomic_t g_bExit(0);
bool                  g_bHardExit(false);
CMutex                g_outputMutex;
ICECAdapter*          g_parser;
#if defined(HAVE_CURSES_API)
bool                  g_cursesEnable(false);
CCursesControl        g_cursesControl("1", "0");
#endif

class CReconnect : public P8PLATFORM::CThread
{
public:
  static CReconnect& Get(void)
  {
    static CReconnect _instance;
    return _instance;
  }

  virtual ~CReconnect(void) {}

  void* Process(void)
  {
    if (g_parser)
    {
      g_parser->Close();
      if (!g_parser->Open(g_strPort.c_str()))
      {
        PrintToStdOut("Failed to reconnect\n");
        g_bExit = 1;
      }
    }
    return NULL;
  }

private:
  CReconnect(void) {}
};

static void PrintToStdOut(const char *strFormat, ...)
{
  std::string strLog;

  va_list argList;
  va_start(argList, strFormat);
  strLog = StringUtils::FormatV(strFormat, argList);
  va_end(argList);

  CLockObject lock(g_outputMutex);
  std::cout << strLog << std::endl;
}

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
static bool GetWord(std::string& data, std::string& word)
{
  std::stringstream datastream(data);
  std::string end;

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

static cec_logical_address GetAddressFromInput(std::string& arguments)
{
  std::string strDev;
  if (GetWord(arguments, strDev))
  {
      unsigned long iDev = strtoul(strDev.c_str(), NULL, 16);
      if ((iDev >= CECDEVICE_TV) && (iDev <= CECDEVICE_BROADCAST))
        return (cec_logical_address)iDev;
  }
  return CECDEVICE_UNKNOWN;
}

void CecLogMessage(void *UNUSED(cbParam), const cec_log_message* message)
{
  if ((message->level & g_cecLogLevel) == message->level)
  {
    std::string strLevel;
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

    std::string strFullLog;
    strFullLog = StringUtils::Format("%s[%16lld]\t%s", strLevel.c_str(), message->time, message->message);
    PrintToStdOut(strFullLog.c_str());

    if (g_logOutput.is_open())
    {
      if (g_bShortLog)
        g_logOutput << message->message << std::endl;
      else
        g_logOutput << strFullLog.c_str() << std::endl;
    }
  }
}

void CecKeyPress(void *UNUSED(cbParam), const cec_keypress* UNUSED(key))
{
}

void CecCommand(void *UNUSED(cbParam), const cec_command* UNUSED(command))
{
}

void CecAlert(void *UNUSED(cbParam), const libcec_alert type, const libcec_parameter UNUSED(param))
{
  switch (type)
  {
  case CEC_ALERT_CONNECTION_LOST:
    if (!CReconnect::Get().IsRunning())
    {
      PrintToStdOut("Connection lost - trying to reconnect\n");
      CReconnect::Get().CreateThread(false);
    }
    break;
  default:
    break;
  }
}

void ListDevices(ICECAdapter *parser)
{
  cec_adapter_descriptor devices[10];
  std::string strMessage = StringUtils::Format("libCEC version: %s, %s",
                                               parser->VersionToString(g_config.serverVersion).c_str(),
                                               parser->GetLibInfo());
  PrintToStdOut(strMessage.c_str());

  int8_t iDevicesFound = parser->DetectAdapters(devices, 10, NULL);
  if (iDevicesFound <= 0)
  {
    PrintToStdOut("Found devices: NONE");
  }
  else
  {
    PrintToStdOut("Found devices: %d\n", iDevicesFound);

    for (int8_t iDevicePtr = 0; iDevicePtr < iDevicesFound; iDevicePtr++)
    {
      PrintToStdOut("device:              %d", iDevicePtr + 1);
      PrintToStdOut("com port:            %s", devices[iDevicePtr].strComName);
      PrintToStdOut("vendor id:           %04x", devices[iDevicePtr].iVendorId);
      PrintToStdOut("product id:          %04x", devices[iDevicePtr].iProductId);
      PrintToStdOut("firmware version:    %d", devices[iDevicePtr].iFirmwareVersion);

      if (devices[iDevicePtr].iFirmwareBuildDate != CEC_FW_BUILD_UNKNOWN)
      {
        time_t buildTime = (time_t)devices[iDevicePtr].iFirmwareBuildDate;
        std::string strDeviceInfo;
        strDeviceInfo = StringUtils::Format("firmware build date: %s", asctime(gmtime(&buildTime)));
        strDeviceInfo = StringUtils::Left(strDeviceInfo, strDeviceInfo.length() > 1 ? (unsigned)(strDeviceInfo.length() - 1) : 0); // strip \n added by asctime
        strDeviceInfo.append(" +0000");
        PrintToStdOut(strDeviceInfo.c_str());
      }

      if (devices[iDevicePtr].adapterType != ADAPTERTYPE_UNKNOWN)
      {
        PrintToStdOut("type:                %s", parser->ToString(devices[iDevicePtr].adapterType));
      }

      PrintToStdOut("");
    }
  }
}

void ShowHelpCommandLine(const char* strExec)
{
  CLockObject lock(g_outputMutex);
  std::cout << std::endl <<
      strExec << " {-h|--help|-l|--list-devices|[COM PORT]}" << std::endl <<
      std::endl <<
      "parameters:" << std::endl <<
      "  -h --help                   Shows this help text" << std::endl <<
      "  -l --list-devices           List all devices on this system" << std::endl <<
      "  -t --type {p|r|t|a}         The device type to use. More than one is possible." << std::endl <<
      "  -p --port {int}             The HDMI port to use as active source." << std::endl <<
      "  -dp --default-port {int}    The default HDMI port to fall back to when resetting." << std::endl <<
      "  -b --base {int}             The logical address of the device to which this " << std::endl <<
      "                              adapter is connected." << std::endl <<
      "  -f --log-file {file}        Writes all libCEC log message to a file" << std::endl <<
      "  -r --rom                    Read saved settings from the EEPROM" << std::endl <<
      "  -sf --short-log-file {file} Writes all libCEC log message without timestamps" << std::endl <<
      "                              and log levels to a file." << std::endl <<
      "  -d --log-level {level}      Sets the log level. See cectypes.h for values." << std::endl <<
      "  -s --single-command         Execute a single command and exit. Does not power" << std::endl <<
      "                              on devices on startup and power them off on exit." << std::endl <<
      "  -o --osd-name {osd name}    Use a custom osd name." << std::endl <<
      "  -m --monitor                Start a monitor-only client." << std::endl <<
      "  -i --info                   Shows information about how libCEC was compiled." << std::endl <<
      "  [COM PORT]                  The com port to connect to. If no COM" << std::endl <<
      "                              port is given, the client tries to connect to the" << std::endl <<
      "                              first device that is detected." << std::endl <<
      std::endl <<
      "Type 'h' or 'help' and press enter after starting the client to display all " << std::endl <<
      "available commands" << std::endl;
}

void ShowHelpConsole(void)
{
  CLockObject lock(g_outputMutex);
  std::cout << std::endl <<
  "================================================================================" << std::endl <<
  "Available commands:" << std::endl <<
  std::endl <<
  "[tx] {bytes}              transfer bytes over the CEC line." << std::endl <<
  "[txn] {bytes}             transfer bytes but don't wait for transmission ACK." << std::endl <<
  "[on] {address}            power on the device with the given logical address." << std::endl <<
  "[standby] {address}       put the device with the given address in standby mode." << std::endl <<
  "[la] {logical address}    change the logical address of the CEC adapter." << std::endl <<
  "[p] {device} {port}       change the HDMI port number of the CEC adapter." << std::endl <<
  "[pa] {physical address}   change the physical address of the CEC adapter." << std::endl <<
  "[as]                      make the CEC adapter the active source." << std::endl <<
  "[is]                      mark the CEC adapter as inactive source." << std::endl <<
  "[osd] {addr} {string}     set OSD message on the specified device." << std::endl <<
  "[ver] {addr}              get the CEC version of the specified device." << std::endl <<
  "[ven] {addr}              get the vendor ID of the specified device." << std::endl <<
  "[lang] {addr}             get the menu language of the specified device." << std::endl <<
  "[pow] {addr}              get the power status of the specified device." << std::endl <<
  "[name] {addr}             get the OSD name of the specified device." << std::endl <<
  "[poll] {addr}             poll the specified device." << std::endl <<
  "[lad]                     lists active devices on the bus" << std::endl <<
  "[ad] {addr}               checks whether the specified device is active." << std::endl <<
  "[at] {type}               checks whether the specified device type is active." << std::endl <<
  "[sp] {addr}               makes the specified physical address active." << std::endl <<
  "[spl] {addr}              makes the specified logical address active." << std::endl <<
  "[volup]                   send a volume up command to the amp if present" << std::endl <<
  "[voldown]                 send a volume down command to the amp if present" << std::endl <<
  "[mute]                    send a mute/unmute command to the amp if present" << std::endl <<
  "[self]                    show the list of addresses controlled by libCEC" << std::endl <<
  "[scan]                    scan the CEC bus and display device info" << std::endl <<
  "[mon] {1|0}               enable or disable CEC bus monitoring." << std::endl <<
  "[log] {1 - 31}            change the log level. see cectypes.h for values." << std::endl <<
  "[ping]                    send a ping command to the CEC adapter." << std::endl <<
  "[bl]                      to let the adapter enter the bootloader, to upgrade" << std::endl <<
  "                          the flash rom." << std::endl <<
  "[r]                       reconnect to the CEC adapter." << std::endl <<
  "[h] or [help]             show this help." << std::endl <<
  "[q] or [quit]             to quit the CEC test client and switch off all" << std::endl <<
  "                          connected CEC devices." << std::endl <<
  "================================================================================" << std::endl;
}

bool ProcessCommandSELF(ICECAdapter *parser, const std::string &command, std::string & UNUSED(arguments))
{
  if (command == "self")
  {
    cec_logical_addresses addr = parser->GetLogicalAddresses();
    std::string strOut = "Addresses controlled by libCEC: ";
    bool bFirst(true);
    for (uint8_t iPtr = 0; iPtr <= 15; iPtr++)
    {
      if (addr[iPtr])
      {
        strOut += StringUtils::Format((bFirst ? "%d%s" : ", %d%s"), iPtr, parser->IsActiveSource((cec_logical_address)iPtr) ? "*" : "");
        bFirst = false;
      }
    }
    PrintToStdOut(strOut.c_str());
    return true;
  }

  return false;
}

bool ProcessCommandSP(ICECAdapter *parser, const std::string &command, std::string &arguments)
{
  if (command == "sp")
  {
    std::string strAddress;
    int iAddress;
    if (GetWord(arguments, strAddress))
    {
      sscanf(strAddress.c_str(), "%x", &iAddress);
      if (iAddress >= 0 && iAddress <= CEC_INVALID_PHYSICAL_ADDRESS)
        parser->SetStreamPath((uint16_t)iAddress);
      return true;
    }
  }

  return false;
}

bool ProcessCommandSPL(ICECAdapter *parser, const std::string &command, std::string &arguments)
{
  if (command == "spl")
  {
    cec_logical_address addr = GetAddressFromInput(arguments);
    if ((addr != CECDEVICE_UNKNOWN) && (addr != CECDEVICE_BROADCAST))
    {
      parser->SetStreamPath(addr);
      return true;
    }
  }

  return false;
}

bool ProcessCommandTX(ICECAdapter *parser, const std::string &command, std::string &arguments)
{
  if (command == "tx" || command == "txn")
  {
    std::string strvalue;
    cec_command bytes = parser->CommandFromString(arguments.c_str());

    if (command == "txn")
      bytes.transmit_timeout = 0;

    parser->Transmit(bytes);

    return true;
  }

  return false;
}

bool ProcessCommandON(ICECAdapter *parser, const std::string &command, std::string &arguments)
{
  if (command == "on")
  {
    std::string strValue;
    uint8_t iValue = 0;
    if (GetWord(arguments, strValue) && HexStrToInt(strValue, iValue) && iValue <= 0xF)
    {
      parser->PowerOnDevices((cec_logical_address) iValue);
      return true;
    }
    else
    {
      PrintToStdOut("invalid destination");
    }
  }

  return false;
}

bool ProcessCommandSTANDBY(ICECAdapter *parser, const std::string &command, std::string &arguments)
{
  if (command == "standby")
  {
    std::string strValue;
    uint8_t iValue = 0;
    if (GetWord(arguments, strValue) && HexStrToInt(strValue, iValue) && iValue <= 0xF)
    {
      parser->StandbyDevices((cec_logical_address) iValue);
      return true;
    }
    else
    {
      PrintToStdOut("invalid destination");
    }
  }

  return false;
}

bool ProcessCommandPOLL(ICECAdapter *parser, const std::string &command, std::string &arguments)
{
  if (command == "poll")
  {
    std::string strValue;
    uint8_t iValue = 0;
    if (GetWord(arguments, strValue) && HexStrToInt(strValue, iValue) && iValue <= 0xF)
    {
      if (parser->PollDevice((cec_logical_address) iValue))
        PrintToStdOut("POLL message sent");
      else
        PrintToStdOut("POLL message not sent");
      return true;
    }
    else
    {
      PrintToStdOut("invalid destination");
    }
  }

  return false;
}

bool ProcessCommandLA(ICECAdapter *parser, const std::string &command, std::string &arguments)
{
  if (command == "la")
  {
    cec_logical_address addr = GetAddressFromInput(arguments);
    if ((addr != CECDEVICE_UNKNOWN) && (addr != CECDEVICE_BROADCAST))
    {
      parser->SetLogicalAddress(addr);
      return true;
    }
  }

  return false;
}

bool ProcessCommandP(ICECAdapter *parser, const std::string &command, std::string &arguments)
{
  if (command == "p")
  {
    std::string strPort;
    cec_logical_address addr = GetAddressFromInput(arguments);
    if ((addr != CECDEVICE_UNKNOWN) && (addr != CECDEVICE_BROADCAST) &&
        GetWord(arguments, strPort))
    {
      parser->SetHDMIPort(addr, (uint8_t)atoi(strPort.c_str()));
      return true;
    }
  }

  return false;
}

bool ProcessCommandPA(ICECAdapter *parser, const std::string &command, std::string &arguments)
{
  if (command == "pa")
  {
    std::string strB1, strB2;
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

bool ProcessCommandOSD(ICECAdapter *parser, const std::string &command, std::string &arguments)
{
  if (command == "osd")
  {
    bool bFirstWord(false);
    std::string strAddr, strMessage, strWord;
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

bool ProcessCommandAS(ICECAdapter *parser, const std::string &command, std::string & UNUSED(arguments))
{
  if (command == "as")
  {
    parser->SetActiveSource();
    // wait for the source switch to finish for 15 seconds tops
    if (g_bSingleCommand)
    {
      CTimeout timeout(15000);
      bool bActiveSource(false);
      while (timeout.TimeLeft() > 0 && !bActiveSource)
      {
        bActiveSource = parser->IsLibCECActiveSource();
        if (!bActiveSource)
          CEvent::Sleep(100);
      }
    }
    return true;
  }

  return false;
}

bool ProcessCommandIS(ICECAdapter *parser, const std::string &command, std::string & UNUSED(arguments))
{
  if (command == "is")
    return parser->SetInactiveView();

  return false;
}

bool ProcessCommandPING(ICECAdapter *parser, const std::string &command, std::string & UNUSED(arguments))
{
  if (command == "ping")
  {
    parser->PingAdapter();
    return true;
  }

  return false;
}

bool ProcessCommandVOLUP(ICECAdapter *parser, const std::string &command, std::string & UNUSED(arguments))
{
  if (command == "volup")
  {
    PrintToStdOut("volume up: %2X", parser->VolumeUp());
    return true;
  }

  return false;
}

bool ProcessCommandVOLDOWN(ICECAdapter *parser, const std::string &command, std::string & UNUSED(arguments))
{
  if (command == "voldown")
  {
    PrintToStdOut("volume down: %2X", parser->VolumeDown());
    return true;
  }

  return false;
}

bool ProcessCommandMUTE(ICECAdapter *parser, const std::string &command, std::string & UNUSED(arguments))
{
  if (command == "mute")
  {
    PrintToStdOut("mute: %2X", parser->AudioToggleMute());
    return true;
  }

  return false;
}

bool ProcessCommandMON(ICECAdapter *parser, const std::string &command, std::string &arguments)
{
  if (command == "mon")
  {
    std::string strEnable;
    if (GetWord(arguments, strEnable) && (strEnable == "0" || strEnable == "1"))
    {
      parser->SwitchMonitoring(strEnable == "1");
      return true;
    }
  }

  return false;
}

bool ProcessCommandBL(ICECAdapter *parser, const std::string &command, std::string & UNUSED(arguments))
{
  if (command == "bl")
  {
    if (parser->StartBootloader())
    {
      PrintToStdOut("entered bootloader mode. exiting cec-client");
      g_bExit = 1;
      g_bHardExit = true;
    }
    return true;
  }

  return false;
}

bool ProcessCommandLANG(ICECAdapter *parser, const std::string &command, std::string &arguments)
{
  if (command == "lang")
  {
    cec_logical_address addr = GetAddressFromInput(arguments);
    if ((addr != CECDEVICE_UNKNOWN) && (addr != CECDEVICE_BROADCAST))
    {
      std::string strLog;
      strLog = StringUtils::Format("menu language '%s'", parser->GetDeviceMenuLanguage(addr).c_str());
      PrintToStdOut(strLog.c_str());
      return true;
    }
  }

  return false;
}

bool ProcessCommandVEN(ICECAdapter *parser, const std::string &command, std::string &arguments)
{
  if (command == "ven")
  {
    cec_logical_address addr = GetAddressFromInput(arguments);
    if ((addr != CECDEVICE_UNKNOWN) && (addr != CECDEVICE_BROADCAST))
    {
      uint64_t iVendor = parser->GetDeviceVendorId(addr);
      PrintToStdOut("vendor id: %06llx", iVendor);
      return true;
    }
  }

  return false;
}

bool ProcessCommandVER(ICECAdapter *parser, const std::string &command, std::string &arguments)
{
  if (command == "ver")
  {
    cec_logical_address addr = GetAddressFromInput(arguments);
    if ((addr != CECDEVICE_UNKNOWN) && (addr != CECDEVICE_BROADCAST))
    {
      cec_version iVersion = parser->GetDeviceCecVersion(addr);
      PrintToStdOut("CEC version %s", parser->ToString(iVersion));
      return true;
    }
  }

  return false;
}

bool ProcessCommandPOW(ICECAdapter *parser, const std::string &command, std::string &arguments)
{
  if (command == "pow")
  {
    cec_logical_address addr = GetAddressFromInput(arguments);
    if ((addr != CECDEVICE_UNKNOWN) && (addr != CECDEVICE_BROADCAST))
    {
      cec_power_status iPower = parser->GetDevicePowerStatus(addr);
      PrintToStdOut("power status: %s", parser->ToString(iPower));
      return true;
    }
  }

  return false;
}

bool ProcessCommandNAME(ICECAdapter *parser, const std::string &command, std::string &arguments)
{
  if (command == "name")
  {
    cec_logical_address addr = GetAddressFromInput(arguments);
    if ((addr != CECDEVICE_UNKNOWN) && (addr != CECDEVICE_BROADCAST))
    {
      std::string name = parser->GetDeviceOSDName(addr);
      PrintToStdOut("OSD name of device %d is '%s'", addr, name.c_str());
      return true;
    }
  }

  return false;
}

bool ProcessCommandLAD(ICECAdapter *parser, const std::string &command, std::string & UNUSED(arguments))
{
  if (command == "lad")
  {
    PrintToStdOut("listing active devices:");
    cec_logical_addresses addresses = parser->GetActiveDevices();
    for (uint8_t iPtr = 0; iPtr <= 11; iPtr++)
      if (addresses[iPtr])
      {
        PrintToStdOut("logical address %X", (int)iPtr);
      }
    return true;
  }

  return false;
}

bool ProcessCommandAD(ICECAdapter *parser, const std::string &command, std::string &arguments)
{
  if (command == "ad")
  {
    cec_logical_address addr = GetAddressFromInput(arguments);
    if ((addr != CECDEVICE_UNKNOWN) && (addr != CECDEVICE_BROADCAST))
    {
      PrintToStdOut("logical address %X is %s", addr,
                    (parser->IsActiveDevice(addr) ? "active" : "not active"));
    }
  }

  return false;
}

bool ProcessCommandAT(ICECAdapter *parser, const std::string &command, std::string &arguments)
{
  if (command == "at")
  {
    std::string strType;
    if (GetWord(arguments, strType))
    {
      cec_device_type type = CEC_DEVICE_TYPE_TV;
      if (strType == "a")
        type = CEC_DEVICE_TYPE_AUDIO_SYSTEM;
      else if (strType == "p")
        type = CEC_DEVICE_TYPE_PLAYBACK_DEVICE;
      else if (strType == "r")
        type = CEC_DEVICE_TYPE_RECORDING_DEVICE;
      else if (strType == "t")
        type = CEC_DEVICE_TYPE_TUNER;

      PrintToStdOut("device %d is %s", type, (parser->IsActiveDeviceType(type) ? "active" : "not active"));
      return true;
    }
  }

  return false;
}

bool ProcessCommandR(ICECAdapter *parser, const std::string &command, std::string & UNUSED(arguments))
{
  if (command == "r")
  {
    bool bReactivate = parser->IsLibCECActiveSource();

    PrintToStdOut("closing the connection");
    parser->Close();

    PrintToStdOut("opening a new connection");
    parser->Open(g_strPort.c_str());

    if (bReactivate)
    {
      PrintToStdOut("setting active source");
      parser->SetActiveSource();
    }
    return true;
  }

  return false;
}

bool ProcessCommandH(ICECAdapter * UNUSED(parser), const std::string &command, std::string & UNUSED(arguments))
{
  if (command == "h" || command == "help")
  {
    ShowHelpConsole();
    return true;
  }

  return false;
}

bool ProcessCommandLOG(ICECAdapter * UNUSED(parser), const std::string &command, std::string &arguments)
{
  if (command == "log")
  {
    std::string strLevel;
    if (GetWord(arguments, strLevel))
    {
      int iNewLevel = atoi(strLevel.c_str());
      if (iNewLevel >= CEC_LOG_ERROR && iNewLevel <= CEC_LOG_ALL)
      {
        g_cecLogLevel = iNewLevel;

        PrintToStdOut("log level changed to %s", strLevel.c_str());
        return true;
      }
    }
  }

  return false;
}

bool ProcessCommandSCAN(ICECAdapter *parser, const std::string &command, std::string & UNUSED(arguments))
{
  if (command == "scan")
  {
    std::string strLog;
    PrintToStdOut("requesting CEC bus information ...");

    strLog.append("CEC bus information\n===================\n");
    cec_logical_addresses addresses = parser->GetActiveDevices();
    cec_logical_address activeSource = parser->GetActiveSource();
    for (uint8_t iPtr = 0; iPtr < 16; iPtr++)
    {
      if (addresses[iPtr])
      {
        uint64_t iVendorId        = parser->GetDeviceVendorId((cec_logical_address)iPtr);
        uint16_t iPhysicalAddress = parser->GetDevicePhysicalAddress((cec_logical_address)iPtr);
        bool     bActive          = parser->IsActiveSource((cec_logical_address)iPtr);
        cec_version iCecVersion   = parser->GetDeviceCecVersion((cec_logical_address)iPtr);
        cec_power_status power    = parser->GetDevicePowerStatus((cec_logical_address)iPtr);
        std::string osdName       = parser->GetDeviceOSDName((cec_logical_address)iPtr);
        std::string strAddr;
        strAddr = StringUtils::Format("%x.%x.%x.%x", (iPhysicalAddress >> 12) & 0xF, (iPhysicalAddress >> 8) & 0xF, (iPhysicalAddress >> 4) & 0xF, iPhysicalAddress & 0xF);
        std::string lang          = parser->GetDeviceMenuLanguage((cec_logical_address)iPtr);

        strLog += StringUtils::Format("device #%X: %s\n", (int)iPtr, parser->ToString((cec_logical_address)iPtr));
        strLog += StringUtils::Format("address:       %s\n", strAddr.c_str());
        strLog += StringUtils::Format("active source: %s\n", (bActive ? "yes" : "no"));
        strLog += StringUtils::Format("vendor:        %s\n", parser->ToString((cec_vendor_id)iVendorId));
        strLog += StringUtils::Format("osd string:    %s\n", osdName.c_str());
        strLog += StringUtils::Format("CEC version:   %s\n", parser->ToString(iCecVersion));
        strLog += StringUtils::Format("power status:  %s\n", parser->ToString(power));
        strLog += StringUtils::Format("language:      %s\n", lang.c_str());
        strLog.append("\n\n");
      }
    }

    activeSource = parser->GetActiveSource();
    strLog += StringUtils::Format("currently active source: %s (%d)", parser->ToString(activeSource), (int)activeSource);

    PrintToStdOut(strLog.c_str());
    return true;
  }

  return false;
}

#if CEC_LIB_VERSION_MAJOR >= 5
bool ProcessCommandSTATS(ICECAdapter *parser, const std::string &command, std::string & UNUSED(arguments))
{
  if (command == "stats")
  {
    cec_adapter_stats stats;
    if (parser->GetStats(&stats))
    {
      std::string strLog;
      strLog += StringUtils::Format("tx acked:  %u\n", stats.tx_ack);
      strLog += StringUtils::Format("tx nacked: %u\n", stats.tx_nack);
      strLog += StringUtils::Format("tx error:  %u\n", stats.tx_error);
      strLog += StringUtils::Format("rx total:  %u\n", stats.rx_total);
      strLog += StringUtils::Format("rx error:  %u\n", stats.rx_error);
      PrintToStdOut(strLog.c_str());
    }
    else
    {
        PrintToStdOut("not supported\n");
    }
    return true;
  }
  return false;
}
#endif

bool ProcessConsoleCommand(ICECAdapter *parser, std::string &input)
{
  if (!input.empty())
  {
    std::string command;
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
      ProcessCommandAS(parser, command, input) ||
      ProcessCommandIS(parser, command, input) ||
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
      ProcessCommandSCAN(parser, command, input) ||
      ProcessCommandSP(parser, command, input) ||
      ProcessCommandSPL(parser, command, input) ||
      ProcessCommandSELF(parser, command, input)
#if CEC_LIB_VERSION_MAJOR >= 5
   || ProcessCommandSTATS(parser, command, input)
#endif
      ;
    }
  }
  return true;
}

bool ProcessCommandLineArguments(int argc, char *argv[])
{
  bool bReturn(true);
  int iArgPtr = 1;
  while (iArgPtr < argc && bReturn)
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
          std::cout << "== skipped log-file parameter: no file given ==" << std::endl;
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
              std::cout << "log level set to " << argv[iArgPtr + 1] << std::endl;
          }
          else
          {
            std::cout << "== skipped log-level parameter: invalid level '" << argv[iArgPtr + 1] << "' ==" << std::endl;
          }
          iArgPtr += 2;
        }
        else
        {
          std::cout << "== skipped log-level parameter: no level given ==" << std::endl;
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
              std::cout << "== using device type 'playback device'" << std::endl;
            g_config.deviceTypes.Add(CEC_DEVICE_TYPE_PLAYBACK_DEVICE);
          }
          else if (!strcmp(argv[iArgPtr + 1], "r"))
          {
            if (!g_bSingleCommand)
              std::cout << "== using device type 'recording device'" << std::endl;
            g_config.deviceTypes.Add(CEC_DEVICE_TYPE_RECORDING_DEVICE);
          }
          else if (!strcmp(argv[iArgPtr + 1], "t"))
          {
            if (!g_bSingleCommand)
              std::cout << "== using device type 'tuner'" << std::endl;
            g_config.deviceTypes.Add(CEC_DEVICE_TYPE_TUNER);
          }
          else if (!strcmp(argv[iArgPtr + 1], "a"))
          {
            if (!g_bSingleCommand)
              std::cout << "== using device type 'audio system'" << std::endl;
            g_config.deviceTypes.Add(CEC_DEVICE_TYPE_AUDIO_SYSTEM);
          }
          else if (!strcmp(argv[iArgPtr + 1], "x"))
          {
            if (!g_bSingleCommand)
              std::cout << "== using device type 'tv'" << std::endl;
            g_config.deviceTypes.Add(CEC_DEVICE_TYPE_TV);
          }
          else
          {
            std::cout << "== skipped invalid device type '" << argv[iArgPtr + 1] << "'" << std::endl;
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
        ICECAdapter *parser = LibCecInitialise(&g_config);
        if (parser)
        {
          std::string strMessage;
          strMessage = StringUtils::Format("libCEC version: %s, %s",
                                           parser->VersionToString(g_config.serverVersion).c_str(),
                                           parser->GetLibInfo());
          PrintToStdOut(strMessage.c_str());
          UnloadLibCec(parser);
          parser = NULL;
        }
        bReturn = false;
      }
      else if (!strcmp(argv[iArgPtr], "--list-devices") ||
               !strcmp(argv[iArgPtr], "-l"))
      {
        if (g_cecLogLevel == -1)
          g_cecLogLevel = CEC_LOG_WARNING + CEC_LOG_ERROR;
        ICECAdapter *parser = LibCecInitialise(&g_config);
        if (parser)
        {
          ListDevices(parser);
          UnloadLibCec(parser);
          parser = NULL;
        }
        bReturn = false;
      }
      else if (!strcmp(argv[iArgPtr], "--bootloader"))
      {
        LibCecBootloader();
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
        if (g_cecLogLevel == -1)
          g_cecLogLevel = CEC_LOG_WARNING + CEC_LOG_ERROR;

        ShowHelpCommandLine(argv[0]);
        return 0;
      }
      else if (!strcmp(argv[iArgPtr], "-b") ||
               !strcmp(argv[iArgPtr], "--base"))
      {
        if (argc >= iArgPtr + 2)
        {
          g_config.baseDevice = (cec_logical_address)atoi(argv[iArgPtr + 1]);
          std::cout << "using base device '" << (int)g_config.baseDevice << "'" << std::endl;
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
          std::cout << "using HDMI port '" << (int)g_config.iHDMIPort << "'" << std::endl;
          ++iArgPtr;
        }
        ++iArgPtr;
      }
      else if (!strcmp(argv[iArgPtr], "-dp") ||
               !strcmp(argv[iArgPtr], "--default-port"))
      {
        if (argc >= iArgPtr + 2)
        {
          uint8_t hdmiport = (int8_t)atoi(argv[iArgPtr + 1]);
          if (hdmiport < 1)
              hdmiport = 1;
          if (hdmiport > 15)
              hdmiport = 15;
          g_config.iDefaultHDMIPort = hdmiport;
          std::cout << "using default HDMI port '" << (int)g_config.iDefaultHDMIPort << "'" << std::endl;
          ++iArgPtr;
        }
        ++iArgPtr;
      }
      else if (!strcmp(argv[iArgPtr], "-r") ||
               !strcmp(argv[iArgPtr], "--rom"))
      {
        std::cout << "using settings from EEPROM" << std::endl;
        g_config.bGetSettingsFromROM = 1;
        ++iArgPtr;
      }
      else if (!strcmp(argv[iArgPtr], "-o") ||
               !strcmp(argv[iArgPtr], "--osd-name"))
      {
        if (argc >= iArgPtr + 2)
        {
          snprintf(g_config.strDeviceName, LIBCEC_OSD_NAME_SIZE, "%s", argv[iArgPtr + 1]);
          std::cout << "using osd name " << g_config.strDeviceName << std::endl;
          ++iArgPtr;
        }
        ++iArgPtr;
      }
      else if (!strcmp(argv[iArgPtr], "-m") ||
               !strcmp(argv[iArgPtr], "--monitor"))
      {
        std::cout << "starting a monitor-only client. use 'mon 0' to switch to normal mode" << std::endl;
        g_config.bMonitorOnly = 1;
        ++iArgPtr;
      }
#if defined(HAVE_CURSES_API)
      else if (!strcmp(argv[iArgPtr], "-c"))
      {
        g_cursesEnable = true;
        if (argc >= iArgPtr + 2)
        {
          std::string input = std::string(argv[iArgPtr + 1]);
          if (input.size() > 2)
          {
            PrintToStdOut("== using default: 10 == ");
          }
          else
          {
            std::string g_in(1, input[0]);
            std::string g_out(1, input[1]);
            g_cursesControl.SetInput(g_in);
            g_cursesControl.SetOutput(g_out);
          }
          iArgPtr += 2;
        }
        else
        {
          PrintToStdOut("== using default: 10 == ");
          ++iArgPtr;
        }
      }
#endif
#if CEC_LIB_VERSION_MAJOR >= 5
      else if (!strcmp(argv[iArgPtr], "-aw") ||
               !strcmp(argv[iArgPtr], "--autowake"))
      {
        if (argc >= iArgPtr + 2)
        {
          bool wake = (*argv[iArgPtr + 1] == '1');
          if (wake)
          {
            std::cout << "enabling auto-wake" << std::endl;
            g_config.bAutoPowerOn = 1;
          }
          else
          {
            std::cout << "disabling auto-wake" << std::endl;
            g_config.bAutoPowerOn = 0;
          }
          ++iArgPtr;
        }
        ++iArgPtr;
      }
#endif
      else
      {
        g_strPort = argv[iArgPtr++];
      }
    }
  }

  return bReturn;
}

void sighandler(int iSignal)
{
  PrintToStdOut("signal caught: %d - exiting", iSignal);
  g_bExit = 1;
}

int main (int argc, char *argv[])
{
  if (signal(SIGINT, sighandler) == SIG_ERR)
  {
    PrintToStdOut("can't register sighandler");
    return -1;
  }

  g_config.Clear();
  g_callbacks.Clear();
  snprintf(g_config.strDeviceName, LIBCEC_OSD_NAME_SIZE, "CECTester");
  g_config.clientVersion      = LIBCEC_VERSION_CURRENT;
  g_config.bActivateSource    = 0;
  g_callbacks.logMessage      = &CecLogMessage;
  g_callbacks.keyPress        = &CecKeyPress;
  g_callbacks.commandReceived = &CecCommand;
  g_callbacks.alert           = &CecAlert;
  g_config.callbacks          = &g_callbacks;

  if (!ProcessCommandLineArguments(argc, argv))
    return 0;

  if (g_cecLogLevel == -1)
    g_cecLogLevel = g_cecDefaultLogLevel;

  if (g_config.deviceTypes.IsEmpty())
  {
    if (!g_bSingleCommand)
      std::cout << "No device type given. Using 'recording device'" << std::endl;
    g_config.deviceTypes.Add(CEC_DEVICE_TYPE_RECORDING_DEVICE);
  }

  g_parser = LibCecInitialise(&g_config);
  if (!g_parser)
  {
#ifdef __WINDOWS__
    std::cout << "Cannot load cec.dll" << std::endl;
#else
    std::cout << "Cannot load libcec.so" << std::endl;
#endif

    if (g_parser)
      UnloadLibCec(g_parser);

    return 1;
  }

  // init video on targets that need this
  g_parser->InitVideoStandalone();

  if (!g_bSingleCommand)
  {
    std::string strLog;
    strLog = StringUtils::Format("CEC Parser created - libCEC version %s", g_parser->VersionToString(g_config.serverVersion).c_str());
    std::cout << strLog.c_str() << std::endl;

    //make stdin non-blocking
  #ifndef __WINDOWS__
    int flags = fcntl(0, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(0, F_SETFL, flags);
  #endif
  }

  if (g_strPort.empty())
  {
    if (!g_bSingleCommand)
      std::cout << "no serial port given. trying autodetect: ";
    cec_adapter_descriptor devices[10];
    uint8_t iDevicesFound = g_parser->DetectAdapters(devices, 10, NULL, true);
    if (iDevicesFound <= 0)
    {
      if (g_bSingleCommand)
        std::cout << "autodetect ";
      std::cout << "FAILED" << std::endl;
      UnloadLibCec(g_parser);
      return 1;
    }
    else
    {
      if (!g_bSingleCommand)
      {
        std::cout << std::endl << " path:     " << devices[0].strComPath << std::endl <<
            " com port: " << devices[0].strComName << std::endl << std::endl;
      }
      g_strPort = devices[0].strComName;
    }
  }

  PrintToStdOut("opening a connection to the CEC adapter...");

  if (!g_parser->Open(g_strPort.c_str()))
  {
    PrintToStdOut("unable to open the device on port %s", g_strPort.c_str());
    UnloadLibCec(g_parser);
    return 1;
  }

#if defined(HAVE_CURSES_API)
  if (g_cursesEnable)
    g_cursesControl.Init();
#endif

  if (!g_bSingleCommand)
    PrintToStdOut("waiting for input");

  while (!g_bExit && !g_bHardExit)
  {
    std::string input;
#if defined(HAVE_CURSES_API)
    if (!g_cursesEnable) {
      getline(std::cin, input);
      std::cin.clear();
    }
    else
    {
      input = g_cursesControl.ParseCursesKey();
    }
#else
    getline(std::cin, input);
    std::cin.clear();
#endif

    if (ProcessConsoleCommand(g_parser, input) && !g_bSingleCommand && !g_bExit && !g_bHardExit)
    {
      if (!input.empty())
        PrintToStdOut("waiting for input");
    }
    else
    {
#if defined(HAVE_CURSES_API)
      if (g_cursesEnable)
        g_cursesControl.End();
#endif
      g_bExit = 1;
    }

    if (!g_bExit && !g_bHardExit)
      CEvent::Sleep(50);
  }

  g_parser->Close();
  UnloadLibCec(g_parser);

  if (g_logOutput.is_open())
    g_logOutput.close();

#if defined(HAVE_CURSES_API)
  if (g_cursesEnable)
    g_cursesControl.End();
#endif

  return 0;
}
