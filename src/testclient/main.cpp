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

using namespace CEC;
using namespace std;

#define CEC_TEST_CLIENT_VERSION 8

#include <cecloader.h>

int        g_cecLogLevel = CEC_LOG_ALL;
int        g_iLogicalAddress = CECDEVICE_PLAYBACKDEVICE1;
ofstream   g_logOutput;
bool       g_bShortLog = false;
CStdString g_strPort;

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

void flush_log(ICECAdapter *cecParser)
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

void list_devices(ICECAdapter *parser)
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

void show_help(const char* strExec)
{
  cout << endl <<
      strExec << " {-h|--help|-l|--list-devices|[COM PORT]}" << endl <<
      endl <<
      "parameters:" << endl <<
      "  -h --help                   Shows this help text" << endl <<
      "  -l --list-devices           List all devices on this system" << endl <<
      "  -la --logical-address {a}   The logical address to use." << endl <<
      "  -f --log-file {file}        Writes all libCEC log message to a file" << endl <<
      "  -sf --short-log-file {file} Writes all libCEC log message without timestamps" << endl <<
      "                              and log levels to a file." << endl <<
      "  -d --log-level {level}      Sets the log level. See cectypes.h for values." << endl <<
      "  [COM PORT]                  The com port to connect to. If no COM" << endl <<
      "                              port is given, the client tries to connect to the" << endl <<
      "                              first device that is detected." << endl <<
      endl <<
      "Type 'h' or 'help' and press enter after starting the client to display all " << endl <<
      "available commands" << endl;
}

void show_console_help(void)
{
  cout << endl <<
  "================================================================================" << endl <<
  "Available commands:" << endl <<
  endl <<
  "tx {bytes}                transfer bytes over the CEC line." << endl <<
  "txn {bytes}               transfer bytes but don't wait for transmission ACK." << endl <<
  "[tx 40 00 FF 11 22 33]    sends bytes 0x40 0x00 0xFF 0x11 0x22 0x33" << endl <<
  endl <<
  "on {address}              power on the device with the given logical address." << endl <<
  "[on 5]                    power on a connected audio system" << endl <<
  endl <<
  "standby {address}         put the device with the given address in standby mode." << endl <<
  "[standby 0]               powers off the TV" << endl <<
  endl <<
  "la {logical_address}      change the logical address of the CEC adapter." << endl <<
  "[la 4]                    logical address 4" << endl <<
  endl <<
  "pa {physical_address}     change the physical address of the CEC adapter." << endl <<
  "[pa 10 00]                physical address 1.0.0.0" << endl <<
  endl <<
  "osd {addr} {string}       set OSD message on the specified device." << endl <<
  "[osd 0 Test Message]      displays 'Test Message' on the TV" << endl <<
  endl <<
  "ver {addr}                get the CEC version of the specified device." << endl <<
  "[ver 0]                   get the CEC version of the TV" << endl <<
  endl <<
  "ven {addr}                get the vendor ID of the specified device." << endl <<
  "[ven 0]                   get the vendor ID of the TV" << endl <<
  endl <<
  "lang {addr}               get the menu language of the specified device." << endl <<
  "[lang 0]                  get the menu language of the TV" << endl <<
  endl <<
  "pow {addr}                get the power status of the specified device." << endl <<
  "[pow 0]                   get the power status of the TV" << endl <<
  endl <<
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

int main (int argc, char *argv[])
{
  ICECAdapter *parser = LoadLibCec("CECTester");
  if (!parser || parser->GetMinVersion() > CEC_TEST_CLIENT_VERSION)
  {
#ifdef __WINDOWS__
    cout << "Cannot load libcec.dll" << endl;
#else
    cout << "Cannot load libcec.so" << endl;
#endif
    return 1;
  }
  CStdString strLog;
  strLog.Format("CEC Parser created - libcec version %d", parser->GetLibVersion());
  cout << strLog.c_str() << endl;

  //make stdin non-blocking
#ifndef __WINDOWS__
  int flags = fcntl(0, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(0, F_SETFL, flags);
#endif

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
      else if (!strcmp(argv[iArgPtr], "-la") ||
               !strcmp(argv[iArgPtr], "--logical-address"))
      {
        if (argc >= iArgPtr + 2)
        {
          int iNewAddress = atoi(argv[iArgPtr + 1]);
          if (iNewAddress >= 0 && iNewAddress <= 15)
          {
            g_iLogicalAddress = iNewAddress;
            cout << "logical address set to " << argv[iArgPtr + 1] << endl;
          }
          else
          {
            cout << "== skipped logical-address parameter: invalid address '" << argv[iArgPtr + 1] << "' ==" << endl;
          }
          iArgPtr += 2;
        }
        else
        {
          cout << "== skipped logical-address parameter: no address given ==" << endl;
          ++iArgPtr;
        }
      }
      else if (!strcmp(argv[iArgPtr], "--list-devices") ||
               !strcmp(argv[iArgPtr], "-l"))
      {
        list_devices(parser);
        UnloadLibCec(parser);
        return 0;
      }
      else if (!strcmp(argv[iArgPtr], "--help") ||
               !strcmp(argv[iArgPtr], "-h"))
      {
        show_help(argv[0]);
        UnloadLibCec(parser);
        return 0;
      }
      else
      {
        g_strPort = argv[iArgPtr++];
      }
    }
  }

  if (g_strPort.IsEmpty())
  {
    cout << "no serial port given. trying autodetect: ";
    cec_adapter devices[10];
    uint8_t iDevicesFound = parser->FindAdapters(devices, 10, NULL);
    if (iDevicesFound <= 0)
    {
      cout << "FAILED" << endl;
      UnloadLibCec(parser);
      return 1;
    }
    else
    {
      cout << endl << " path:     " << devices[0].path << endl <<
          " com port: " << devices[0].comm << endl << endl;
      g_strPort = devices[0].comm;
    }
  }

  parser->SetLogicalAddress((cec_logical_address) g_iLogicalAddress);

  if (!parser->Open(g_strPort.c_str()))
  {
    cout << "unable to open the device on port " << g_strPort << endl;
    flush_log(parser);
    UnloadLibCec(parser);
    return 1;
  }

  cout << "cec device opened" << endl;

  parser->PowerOnDevices(CECDEVICE_TV);
  flush_log(parser);

  parser->SetActiveView();
  flush_log(parser);

  bool bContinue(true);
  cout << "waiting for input" << endl;
  while (bContinue)
  {
    flush_log(parser);

    /* just ignore the command buffer and clear it */
    cec_command dummy;
    while (parser && parser->GetNextCommand(&dummy)) {}

    string input;
    getline(cin, input);
    cin.clear();

    if (!input.empty())
    {
      string command;
      if (GetWord(input, command))
      {
        if (command == "tx" || command == "txn")
        {
          string strvalue;
          uint8_t ivalue;
          cec_command bytes;
          bytes.clear();

          while (GetWord(input, strvalue) && HexStrToInt(strvalue, ivalue))
            bytes.push_back(ivalue);

          if (command == "txn")
            bytes.transmit_timeout = 0;

          parser->Transmit(bytes);
        }
        else if (command == "on")
        {
          string strValue;
          uint8_t iValue = 0;
          if (GetWord(input, strValue) && HexStrToInt(strValue, iValue) && iValue <= 0xF)
          {
            parser->PowerOnDevices((cec_logical_address) iValue);
          }
          else
          {
            cout << "invalid destination" << endl;
          }
        }
        else if (command == "standby")
        {
          string strValue;
          uint8_t iValue = 0;
          if (GetWord(input, strValue) && HexStrToInt(strValue, iValue) && iValue <= 0xF)
          {
            parser->StandbyDevices((cec_logical_address) iValue);
          }
          else
          {
            cout << "invalid destination" << endl;
          }
        }
        else if (command == "la")
        {
          string strvalue;
          if (GetWord(input, strvalue))
          {
            parser->SetLogicalAddress((cec_logical_address) atoi(strvalue.c_str()));
          }
        }
        else if (command == "pa")
        {
          string strB1, strB2;
          uint8_t iB1, iB2;
          if (GetWord(input, strB1) && HexStrToInt(strB1, iB1) &&
              GetWord(input, strB2) && HexStrToInt(strB2, iB2))
          {
            uint16_t iPhysicalAddress = ((uint16_t)iB1 << 8) + iB2;
            parser->SetPhysicalAddress(iPhysicalAddress);
          }
        }
        else if (command == "osd")
        {
          bool bFirstWord(false);
          string strAddr, strMessage, strWord;
          uint8_t iAddr;
          if (GetWord(input, strAddr) && HexStrToInt(strAddr, iAddr) && iAddr < 0xF)
          {
            while (GetWord(input, strWord))
            {
              if (bFirstWord)
              {
                bFirstWord = false;
                strMessage.append(" ");
              }
              strMessage.append(strWord);
            }
            parser->SetOSDString((cec_logical_address) iAddr, CEC_DISPLAY_CONTROL_DISPLAY_FOR_DEFAULT_TIME, strMessage.c_str());
          }
        }
        else if (command == "ping")
        {
          parser->PingAdapter();
        }
        else if (command == "mon")
        {
          CStdString strEnable;
          if (GetWord(input, strEnable) && (strEnable.Equals("0") || strEnable.Equals("1")))
          {
            parser->SwitchMonitoring(strEnable.Equals("1"));
          }
        }
        else if (command == "bl")
        {
          parser->StartBootloader();
        }
        else if (command == "lang")
        {
          CStdString strDev;
          if (GetWord(input, strDev))
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
            }
          }
        }
        else if (command == "ven")
        {
          CStdString strDev;
          if (GetWord(input, strDev))
          {
            int iDev = atoi(strDev);
            if (iDev >= 0 && iDev < 15)
            {
              uint64_t iVendor = parser->GetDeviceVendorId((cec_logical_address) iDev);
              CStdString strLog;
              strLog.Format("vendor id: %06x", iVendor);
              cout << strLog.c_str() << endl;
            }
          }
        }
        else if (command == "ver")
        {
          CStdString strDev;
          if (GetWord(input, strDev))
          {
            int iDev = atoi(strDev);
            if (iDev >= 0 && iDev < 15)
            {
              cec_version iVersion = parser->GetDeviceCecVersion((cec_logical_address) iDev);
              switch (iVersion)
              {
              case CEC_VERSION_1_2:
                cout << "CEC version 1.2" << endl;
                break;
              case CEC_VERSION_1_2A:
                cout << "CEC version 1.2a" << endl;
                break;
              case CEC_VERSION_1_3:
                cout << "CEC version 1.3" << endl;
                break;
              case CEC_VERSION_1_3A:
                cout << "CEC version 1.3a" << endl;
                break;
              default:
                cout << "unknown CEC version" << endl;
                break;
              }
            }
          }
        }
        else if (command == "pow")
        {
          CStdString strDev;
          if (GetWord(input, strDev))
          {
            int iDev = atoi(strDev);
            if (iDev >= 0 && iDev < 15)
            {
              cec_power_status iPower = parser->GetDevicePowerStatus((cec_logical_address) iDev);
              switch (iPower)
              {
              case CEC_POWER_STATUS_ON:
                cout << "powered on" << endl;
                break;
              case CEC_POWER_STATUS_IN_TRANSITION_ON_TO_STANDBY:
                cout << "on -> standby" << endl;
                break;
              case CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON:
                cout << "standby -> on" << endl;
                break;
              case CEC_POWER_STATUS_STANDBY:
                cout << "standby" << endl;
                break;
              default:
                cout << "unknown power status" << endl;
                break;
              }
            }
          }
        }
        else if (command == "r")
        {
          cout << "closing the connection" << endl;
          parser->Close();
          flush_log(parser);

          cout << "opening a new connection" << endl;
          parser->Open(g_strPort.c_str());
          flush_log(parser);

          cout << "setting active view" << endl;
          parser->SetActiveView();
        }
        else if (command == "h" || command == "help")
        {
          show_console_help();
        }
        else if (command == "q" || command == "quit")
        {
          bContinue = false;
        }
        else if (command == "log")
        {
          CStdString strLevel;
          if (GetWord(input, strLevel))
          {
            int iNewLevel = atoi(strLevel);
            if (iNewLevel >= CEC_LOG_ERROR && iNewLevel <= CEC_LOG_ALL)
            {
              g_cecLogLevel = iNewLevel;
              cout << "log level changed to " << strLevel.c_str() << endl;
            }
          }
        }
      }
      if (bContinue)
        cout << "waiting for input" << endl;
    }
    CCondition::Sleep(50);
  }

  parser->StandbyDevices(CECDEVICE_BROADCAST);
  parser->Close();
  flush_log(parser);
  UnloadLibCec(parser);

  if (g_logOutput.is_open())
    g_logOutput.close();

  return 0;
}
