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

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace CecSharpClient
{
  class CecSharpClient
  {
    public CecSharpClient()
    {
      CecDeviceTypeList types = new CecDeviceTypeList();
      types.Types[0] = CecDeviceType.PlaybackDevice;

      Lib = new LibCecSharp("CEC Tester", types);
      LogLevel = (int) CecLogLevel.All;

      Console.WriteLine("CEC Parser created - libcec version " + Lib.GetLibVersionMajor() + "." + Lib.GetLibVersionMinor());
    }

    void FlushLog()
    {
      CecLogMessage message = Lib.GetNextLogMessage();
      bool bGotMessage = !message.Empty;
      while (bGotMessage)
      {
        if (((int)message.Level & LogLevel) == (int)message.Level)
        {
          string strLevel = "";
          switch (message.Level)
          {
            case CecLogLevel.Error:
              strLevel = "ERROR:   ";
              break;
            case CecLogLevel.Warning:
              strLevel = "WARNING: ";
              break;
            case CecLogLevel.Notice:
              strLevel = "NOTICE:  ";
              break;
            case CecLogLevel.Traffic:
              strLevel = "TRAFFIC: ";
              break;
            case CecLogLevel.Debug:
              strLevel = "DEBUG:   ";
              break;
            default:
              break;
          }
          string strLog = string.Format("{0} {1,16} {2}", strLevel, message.Time, message.Message);
          Console.WriteLine(strLog);
        }

        message = Lib.GetNextLogMessage();
        bGotMessage = !message.Empty;
      }
    }

    public bool Connect(int timeout)
    {
      CecAdapter[] adapters = Lib.FindAdapters(string.Empty);
      if (adapters.Length > 0)
        return Connect(adapters[0].ComPort, timeout);
      else
      {
        Console.WriteLine("Did not find any CEC adapters");
        return false;
      }
    }

    public bool Connect(string port, int timeout)
    {
      return Lib.Open(port, timeout);
    }

    public void Close()
    {
      Lib.Close();
    }

    public void ListDevices()
    {
      int iAdapter = 0;
      foreach (CecAdapter adapter in Lib.FindAdapters(string.Empty))
      {
        Console.WriteLine("Adapter:  " + iAdapter++);
        Console.WriteLine("Path:     " + adapter.Path);
        Console.WriteLine("Com port: " + adapter.ComPort);
      }
    }

    void ShowConsoleHelp()
    {
      Console.WriteLine(
        "================================================================================" + System.Environment.NewLine +
        "Available commands:" + System.Environment.NewLine +
        System.Environment.NewLine +
        "tx {bytes}                transfer bytes over the CEC line." + System.Environment.NewLine +
        "txn {bytes}               transfer bytes but don't wait for transmission ACK." + System.Environment.NewLine +
        "[tx 40 00 FF 11 22 33]    sends bytes 0x40 0x00 0xFF 0x11 0x22 0x33" + System.Environment.NewLine +
        System.Environment.NewLine +
        "on {address}              power on the device with the given logical address." + System.Environment.NewLine +
        "[on 5]                    power on a connected audio system" + System.Environment.NewLine +
        System.Environment.NewLine +
        "standby {address}         put the device with the given address in standby mode." + System.Environment.NewLine +
        "[standby 0]               powers off the TV" + System.Environment.NewLine +
        System.Environment.NewLine +
        "la {logical_address}      change the logical address of the CEC adapter." + System.Environment.NewLine +
        "[la 4]                    logical address 4" + System.Environment.NewLine +
        System.Environment.NewLine +
        "pa {physical_address}     change the physical address of the CEC adapter." + System.Environment.NewLine +
        "[pa 1000]                 physical address 1.0.0.0" + System.Environment.NewLine +
        System.Environment.NewLine +
        "osd {addr} {string}       set OSD message on the specified device." + System.Environment.NewLine +
        "[osd 0 Test Message]      displays 'Test Message' on the TV" + System.Environment.NewLine +
        System.Environment.NewLine +
        "ver {addr}                get the CEC version of the specified device." + System.Environment.NewLine +
        "[ver 0]                   get the CEC version of the TV" + System.Environment.NewLine +
        System.Environment.NewLine +
        "ven {addr}                get the vendor ID of the specified device." + System.Environment.NewLine +
        "[ven 0]                   get the vendor ID of the TV" + System.Environment.NewLine +
        System.Environment.NewLine +
        "lang {addr}               get the menu language of the specified device." + System.Environment.NewLine +
        "[lang 0]                  get the menu language of the TV" + System.Environment.NewLine +
        System.Environment.NewLine +
        "pow {addr}                get the power status of the specified device." + System.Environment.NewLine +
        "[pow 0]                   get the power status of the TV" + System.Environment.NewLine +
        System.Environment.NewLine +
        "poll {addr}               poll the specified device." + System.Environment.NewLine +
        "[poll 0]                  sends a poll message to the TV" + System.Environment.NewLine +
        System.Environment.NewLine +
        "[mon] {1|0}               enable or disable CEC bus monitoring." + System.Environment.NewLine +
        "[log] {1 - 31}            change the log level. see cectypes.h for values." + System.Environment.NewLine +
        System.Environment.NewLine +
        "[ping]                    send a ping command to the CEC adapter." + System.Environment.NewLine +
        "[bl]                      to let the adapter enter the bootloader, to upgrade" + System.Environment.NewLine +
        "                          the flash rom." + System.Environment.NewLine +
        "[r]                       reconnect to the CEC adapter." + System.Environment.NewLine +
        "[h] or [help]             show this help." + System.Environment.NewLine +
        "[q] or [quit]             to quit the CEC test client and switch off all" + System.Environment.NewLine +
        "                          connected CEC devices." + System.Environment.NewLine +
        "================================================================================");
    }

    public void MainLoop()
    {
      Lib.PowerOnDevices(CecLogicalAddress.Tv);
      FlushLog();

      Lib.SetActiveSource(CecDeviceType.PlaybackDevice);
      FlushLog();

      bool bContinue = true;
      string command;
      while (bContinue)
      {
        Console.WriteLine("waiting for input");

        command = Console.ReadLine();
        if (command.Length == 0)
          continue;
        string[] splitCommand = command.Split(' ');
        if (splitCommand[0] == "tx" || splitCommand[0] == "txn")
        {
          CecCommand bytes = new CecCommand();
          for (int iPtr = 1; iPtr < splitCommand.Length; iPtr++)
          {
            bytes.PushBack(byte.Parse(splitCommand[iPtr], System.Globalization.NumberStyles.HexNumber));
          }

          if (command == "txn")
            bytes.TransmitTimeout = 0;

          Lib.Transmit(bytes);
        }
        else if (splitCommand[0] == "on")
        {
          if (splitCommand.Length > 1)
            Lib.PowerOnDevices((CecLogicalAddress)byte.Parse(splitCommand[1], System.Globalization.NumberStyles.HexNumber));
          else
            Lib.PowerOnDevices(CecLogicalAddress.Broadcast);
        }
        else if (splitCommand[0] == "standby")
        {
          if (splitCommand.Length > 1)
            Lib.StandbyDevices((CecLogicalAddress)byte.Parse(splitCommand[1], System.Globalization.NumberStyles.HexNumber));
          else
            Lib.StandbyDevices(CecLogicalAddress.Broadcast);
        }
        else if (splitCommand[0] == "poll")
        {
          bool bSent = false;
          if (splitCommand.Length > 1)
            bSent = Lib.PollDevice((CecLogicalAddress)byte.Parse(splitCommand[1], System.Globalization.NumberStyles.HexNumber));
          else
            bSent = Lib.PollDevice(CecLogicalAddress.Broadcast);
          if (bSent)
            Console.WriteLine("POLL message sent");
          else
            Console.WriteLine("POLL message not sent");
        }
        else if (splitCommand[0] == "la")
        {
          if (splitCommand.Length > 1)
            Lib.SetLogicalAddress((CecLogicalAddress)byte.Parse(splitCommand[1], System.Globalization.NumberStyles.HexNumber));
        }
        else if (splitCommand[0] == "pa")
        {
          if (splitCommand.Length > 1)
            Lib.SetPhysicalAddress(short.Parse(splitCommand[1], System.Globalization.NumberStyles.HexNumber));
        }
        else if (splitCommand[0] == "osd")
        {
          if (splitCommand.Length > 2)
          {
            StringBuilder osdString = new StringBuilder();
            for (int iPtr = 1; iPtr < splitCommand.Length; iPtr++)
            {
              osdString.Append(splitCommand[iPtr]);
              if (iPtr != splitCommand.Length - 1)
                osdString.Append(" ");
            }
            Lib.SetOSDString((CecLogicalAddress)byte.Parse(splitCommand[1], System.Globalization.NumberStyles.HexNumber), CecDisplayControl.DisplayForDefaultTime, osdString.ToString());
          }
        }
        else if (splitCommand[0] == "ping")
        {
          Lib.PingAdapter();
        }
        else if (splitCommand[0] == "mon")
        {
          bool enable = splitCommand.Length > 1 ? splitCommand[1] == "1" : false;
          Lib.SwitchMonitoring(enable);
        }
        else if (splitCommand[0] == "bl")
        {
          Lib.StartBootloader();
        }
        else if (splitCommand[0] == "lang")
        {
          if (splitCommand.Length > 1)
          {
            string language = Lib.GetDeviceMenuLanguage((CecLogicalAddress)byte.Parse(splitCommand[1], System.Globalization.NumberStyles.HexNumber));
            Console.WriteLine("Menu language: " + language);
          }
        }
        else if (splitCommand[0] == "ven")
        {
          if (splitCommand.Length > 1)
          {
            ulong vendor = Lib.GetDeviceVendorId((CecLogicalAddress)byte.Parse(splitCommand[1], System.Globalization.NumberStyles.HexNumber));
            Console.WriteLine("Vendor ID: " + vendor);
          }
        }
        else if (splitCommand[0] == "ver")
        {
          if (splitCommand.Length > 1)
          {
            CecVersion version = Lib.GetDeviceCecVersion((CecLogicalAddress)byte.Parse(splitCommand[1], System.Globalization.NumberStyles.HexNumber));
            switch (version)
            {
              case CecVersion.V1_2:
                Console.WriteLine("CEC version 1.2");
                break;
              case CecVersion.V1_2A:
                Console.WriteLine("CEC version 1.2a");
                break;
              case CecVersion.V1_3:
                Console.WriteLine("CEC version 1.3");
                break;
              case CecVersion.V1_3A:
                Console.WriteLine("CEC version 1.3a");
                break;
              case CecVersion.V1_4:
                Console.WriteLine("CEC version 1.4");
                break;
              default:
                Console.WriteLine("unknown CEC version");
                break;
            }
          }
        }
        else if (splitCommand[0] == "pow")
        {
          if (splitCommand.Length > 1)
          {
            CecPowerStatus power = Lib.GetDevicePowerStatus((CecLogicalAddress)byte.Parse(splitCommand[1], System.Globalization.NumberStyles.HexNumber));
            switch (power)
            {
              case CecPowerStatus.On:
                Console.WriteLine("powered on");
                break;
              case CecPowerStatus.InTransitionOnToStandby:
                Console.WriteLine("on -> standby");
                break;
              case CecPowerStatus.InTransitionStandbyToOn:
                Console.WriteLine("standby -> on");
                break;
              case CecPowerStatus.Standby:
                Console.WriteLine("standby");
                break;
              default:
                Console.WriteLine("unknown power status");
                break;
            }
          }
        }
        else if (splitCommand[0] == "r")
        {
          Console.WriteLine("closing the connection");
          Lib.Close();
          FlushLog();

          Console.WriteLine("opening a new connection");
          Connect(10000);
          FlushLog();

          Console.WriteLine("setting active source");
          Lib.SetActiveSource(CecDeviceType.PlaybackDevice);
        }
        else if (splitCommand[0] == "h" || splitCommand[0] == "help")
          ShowConsoleHelp();
        else if (splitCommand[0] == "q" || splitCommand[0] == "quit")
          bContinue = false;
        else if (splitCommand[0] == "log" && splitCommand.Length > 1)
          LogLevel = int.Parse(splitCommand[1]);

        FlushLog();
      }
    }

    static void Main(string[] args)
    {
      CecSharpClient p = new CecSharpClient();
      if (p.Connect(10000))
      {
        p.MainLoop();
      }
      else
      {
        Console.WriteLine("Could not open a connection to the CEC adapter");
      }
      p.FlushLog();
    }

    private int         LogLevel;
    private LibCecSharp Lib;
  }
}
