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

#include "../../include/CECExports.h"
#include "../lib/util/threads.h"
#include "../lib/util/misc.h"
#include "../lib/util/StdString.h"
#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <string>

using namespace CEC;
using namespace std;

#define CEC_TEST_CLIENT_VERSION 3

void flush_log(ICECAdapter *cecParser)
{
  cec_log_message message;
  while (cecParser && cecParser->GetNextLogMessage(&message))
  {
    switch (message.level)
    {
    case CEC_LOG_ERROR:
      cout << "ERROR:   " << message.message.c_str() << endl;
      break;
    case CEC_LOG_WARNING:
      cout << "WARNING: " << message.message.c_str() << endl;
      break;
    case CEC_LOG_NOTICE:
      cout << "NOTICE:  " << message.message.c_str() << endl;
      break;
    case CEC_LOG_DEBUG:
      cout << "DEBUG:   " << message.message.c_str() << endl;
      break;
    }
  }
}

void list_devices(ICECAdapter *parser)
{
  cout << "Found devices: ";
  vector<cec_adapter> devices;
  int iDevicesFound = parser->FindAdapters(devices);
  if (iDevicesFound <= 0)
  {
#ifdef __WINDOWS__
    cout << "Not supported yet, sorry!" << endl;
#else
    cout << "NONE" << endl;
#endif
  }
  else
  {
    cout << devices.size() << endl;
    for (unsigned int iDevicePtr = 0; iDevicePtr < devices.size(); iDevicePtr++)
    {
      CStdString strDevice;
      strDevice.Format("device:        %d\npath:          %s\ncom port:      %s", iDevicePtr, devices[iDevicePtr].path.c_str(), devices[0].comm.c_str());
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
  "[tx 40 00 FF 11 22 33]    sends bytes 0x40 0x00 0xFF 0x11 0x22 0x33" << endl <<
  endl <<
  "la {logical_address}      change the logical address of the CEC adapter." << endl <<
  "[la 4]                    logical address 4" << endl <<
  endl <<
  "[ping]                    send a ping command to the CEC adapter." << endl <<
  "[bl]                      to let the adapter enter the bootloader, to upgrade the flash rom." << endl <<
  "[h] or [help]             show this help." << endl <<
  "[q] or [quit]             to quit the CEC test client and switch off all connected CEC devices." << endl <<
  "================================================================================" << endl;
}

int main (int argc, char *argv[])
{
  ICECAdapter *parser = LoadLibCec("CEC Tester");
  if (!parser && parser->GetMinVersion() > CEC_TEST_CLIENT_VERSION)
  {
    cout << "Unable to create parser. Is libcec.dll present?" << endl;
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
    vector<cec_adapter> devices;
    int iDevicesFound = parser->FindAdapters(devices);
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
  usleep(CEC_SETTLE_DOWN_TIME);

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
        if (command == "tx")
        {
          string strvalue;
          int    ivalue;
          vector<uint8_t> bytes;
          while (GetWord(input, strvalue) && HexStrToInt(strvalue, ivalue))
          bytes.push_back(ivalue);

          parser->Transmit(bytes);
        }
        else if (command == "la")
        {
          string strvalue;
          if (GetWord(input, strvalue))
          {
            parser->SetLogicalAddress((cec_logical_address) atoi(strvalue.c_str()));
          }
        }
        else if (command == "ping")
        {
          parser->PingAdapter();
        }
        else if (command == "bl")
        {
          parser->StartBootloader();
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
