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
#include <string>
#include <sstream>
#include "../lib/platform/threads.h"
#include "../lib/util/StdString.h"

using namespace CEC;
using namespace std;

#define CEC_TEST_CLIENT_VERSION 8

#include <cecloader.h>

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
    switch (message.level)
    {
    case CEC_LOG_ERROR:
      cout << "ERROR:   ";
      break;
    case CEC_LOG_WARNING:
      cout << "WARNING: ";
      break;
    case CEC_LOG_NOTICE:
      cout << "NOTICE:  ";
      break;
    case CEC_LOG_TRAFFIC:
      cout << "TRAFFIC: ";
      break;
    case CEC_LOG_DEBUG:
      cout << "DEBUG:   ";
      break;
    }

    CStdString strMessageTmp;
    strMessageTmp.Format("[%16lld]\t%s", message.time, message.message);
    cout << strMessageTmp.c_str() << endl;
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
      "\t-h --help            Shows this help text" << endl <<
      "\t-l --list-devices    List all devices on this system" << endl <<
      "\t[COM PORT]           The com port to connect to. If no COM port is given, the client tries to connect to the first device that is detected" << endl <<
      endl <<
      "Type 'h' or 'help' and press enter after starting the client to display all available commands" << endl;
}

void show_console_help(void)
{
  cout << endl <<
  "================================================================================" << endl <<
  "Available commands:" << endl <<
  endl <<
  "tx {bytes}                transfer bytes over the CEC line." << endl <<
  "txn {bytes}               transfer bytes and don't wait for an ACK reply." << endl <<
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
  "[mon] {1|0}               enable or disable CEC bus monitoring." << endl <<
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

  string strPort;
  if (argc < 2)
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
      strPort = devices[0].comm;
    }
  }
  else if (!strcmp(argv[1], "--list-devices") || !strcmp(argv[1], "-l"))
  {
    list_devices(parser);
    UnloadLibCec(parser);
    return 0;
  }
  else if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))
  {
    show_help(argv[0]);
    return 0;
  }
  else
  {
    strPort = argv[1];
  }

  if (!parser->Open(strPort.c_str()))
  {
    cout << "unable to open the device on port " << strPort << endl;
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

          parser->Transmit(bytes, command == "tx");
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
        else if (command == "r")
        {
          cout << "closing the connection" << endl;
          parser->Close();
          flush_log(parser);

          cout << "opening a new connection" << endl;
          parser->Open(strPort.c_str());
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
  return 0;
}
