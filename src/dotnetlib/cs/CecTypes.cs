/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2025 Pulse-Eight Limited.  All rights reserved.
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
using System.Text;

// Managed data types. These mirror the ref classes in the old
// src/dotnetlib/CecSharpTypes.h and keep the same public shape, so existing
// CecSharp consumers compile unchanged. Conversion to/from the native interop
// structs lives here (ToNative/FromNative) rather than in the facade.

namespace CecSharp
{
  // libCEC defaults, kept in sync with include/cectypes.h.
  internal static class CecDefaults
  {
    public const ushort DefaultPhysicalAddress = 0x1000; // CEC_DEFAULT_PHYSICAL_ADDRESS
    public const int    DefaultBaseDevice      = 0;      // CEC_DEFAULT_BASE_DEVICE
    public const byte   DefaultHdmiPort        = 1;      // CEC_DEFAULT_HDMI_PORT
    public const int    ActivateSource         = 1;      // CEC_DEFAULT_SETTING_ACTIVATE_SOURCE
    public const int    PowerOffShutdown       = 1;      // CEC_DEFAULT_SETTING_POWER_OFF_SHUTDOWN
    public const int    PowerOffOnStandby      = 1;      // CEC_DEFAULT_SETTING_POWER_OFF_ON_STANDBY
    public const string DeviceLanguage         = "eng";  // CEC_DEFAULT_DEVICE_LANGUAGE
    public const int    CecVersion             = 0x05;   // CEC_DEFAULT_SETTING_CEC_VERSION
    public const uint   ComboTimeoutMs         = 1000;   // CEC_DEFAULT_COMBO_TIMEOUT_MS
    public const uint   ButtonTimeout          = 500;    // CEC_BUTTON_TIMEOUT
    public const uint   ButtonRepeatDelayMs    = 200;    // CEC_BUTTON_REPEAT_DELAY_MS

    internal static readonly DateTime Epoch = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc);
  }

  /// <summary>
  /// A parameter for the CecAlert callback
  /// </summary>
  public class CecParameter
  {
    public CecParameter(CecParameterType type, string data)
    {
      Type = type;
      Data = data;
    }

    public CecParameterType Type { get; set; }
    public string Data { get; set; }
  }

  /// <summary>
  /// Descriptor of a CEC adapter, returned when scanning for adapters that are connected to the system
  /// </summary>
  public class CecAdapter
  {
    public CecAdapter(string path, string comPort, ushort vendorId, ushort productId,
      ushort firmwareVersion, uint firmwareBuildDate, ushort physicalAddress)
    {
      Path = path;
      ComPort = comPort;
      VendorID = vendorId;
      ProductID = productId;
      FirmwareVersion = firmwareVersion;
      FirmwareBuildDate = CecDefaults.Epoch.AddSeconds(firmwareBuildDate);
      PhysicalAddress = physicalAddress;
    }

    public string Path { get; set; }
    public string ComPort { get; set; }
    public ushort VendorID { get; set; }
    public ushort ProductID { get; set; }
    public ushort FirmwareVersion { get; set; }
    public DateTime FirmwareBuildDate { get; set; }
    public ushort PhysicalAddress { get; set; }
  }

  /// <summary>
  /// A list of CEC device types
  /// </summary>
  public class CecDeviceTypeList
  {
    public CecDeviceTypeList()
    {
      Types = new CecDeviceType[5];
      Clear();
    }

    public void Clear()
    {
      for (int iPtr = 0; iPtr < 5; iPtr++)
        Types[iPtr] = CecDeviceType.Reserved;
    }

    public CecDeviceType[] Types { get; set; }
  }

  /// <summary>
  /// A list of logical addresses
  /// </summary>
  public class CecLogicalAddresses
  {
    public CecLogicalAddresses()
    {
      Addresses = new CecLogicalAddress[16];
      Clear();
    }

    public void Clear()
    {
      Primary = CecLogicalAddress.Unknown;
      for (int iPtr = 0; iPtr < 16; iPtr++)
        Addresses[iPtr] = CecLogicalAddress.Unknown;
    }

    public bool IsSet(CecLogicalAddress address)
    {
      return Addresses[(int)address] != CecLogicalAddress.Unknown;
    }

    public void Set(CecLogicalAddress address)
    {
      Addresses[(int)address] = address;
      if (Primary == CecLogicalAddress.Unknown)
        Primary = address;
    }

    public void Unset(CecLogicalAddress address)
    {
      Addresses[(int)address] = CecLogicalAddress.Unknown;
      if (Primary == address)
      {
        Primary = CecLogicalAddress.Unknown;
        for (int iPtr = 0; iPtr < 16; iPtr++)
        {
          if (IsSet((CecLogicalAddress)iPtr))
          {
            Primary = (CecLogicalAddress)iPtr;
            break;
          }
        }
      }
    }

    public CecLogicalAddress Primary { get; set; }
    public CecLogicalAddress[] Addresses { get; set; }
  }

  /// <summary>
  /// Byte array used for CEC command parameters
  /// </summary>
  public class CecDatapacket
  {
    public CecDatapacket()
    {
      Data = new byte[100];
      Size = 0;
    }

    public void PushBack(byte data)
    {
      if (Size < 100)
      {
        Data[Size] = data;
        Size++;
      }
    }

    public byte[] Data { get; set; }
    public byte Size { get; set; }
  }

  /// <summary>
  /// A CEC command that is received or transmitted over the CEC bus
  /// </summary>
  public class CecCommand
  {
    public CecCommand(CecLogicalAddress initiator, CecLogicalAddress destination, bool ack, bool eom, CecOpcode opcode, int transmitTimeout)
    {
      Initiator = initiator;
      Destination = destination;
      Ack = ack;
      Eom = eom;
      Opcode = opcode;
      OpcodeSet = true;
      TransmitTimeout = transmitTimeout;
      Parameters = new CecDatapacket();
      Empty = false;
    }

    public CecCommand()
    {
      Initiator = CecLogicalAddress.Unknown;
      Destination = CecLogicalAddress.Unknown;
      Ack = false;
      Eom = false;
      Opcode = CecOpcode.None;
      OpcodeSet = false;
      TransmitTimeout = 0;
      Parameters = new CecDatapacket();
      Empty = true;
    }

    public void PushBack(byte data)
    {
      if (Initiator == CecLogicalAddress.Unknown && Destination == CecLogicalAddress.Unknown)
      {
        Initiator = (CecLogicalAddress)(data >> 4);
        Destination = (CecLogicalAddress)(data & 0xF);
      }
      else if (!OpcodeSet)
      {
        OpcodeSet = true;
        Opcode = (CecOpcode)data;
      }
      else
      {
        Parameters.PushBack(data);
      }
    }

    public bool Empty { get; set; }
    public CecLogicalAddress Initiator { get; set; }
    public CecLogicalAddress Destination { get; set; }
    public bool Ack { get; set; }
    public bool Eom { get; set; }
    public CecOpcode Opcode { get; set; }
    public CecDatapacket Parameters { get; set; }
    public bool OpcodeSet { get; set; }
    public int TransmitTimeout { get; set; }
  }

  /// <summary>
  /// A key press that was received
  /// </summary>
  public class CecKeypress
  {
    public CecKeypress(CecUserControlCode keycode, uint duration)
    {
      Keycode = keycode;
      Duration = duration;
      Empty = false;
    }

    public CecKeypress()
    {
      Keycode = CecUserControlCode.Unknown;
      Duration = 0;
      Empty = true;
    }

    public bool Empty { get; set; }
    public CecUserControlCode Keycode { get; set; }
    public uint Duration { get; set; }
  }

  /// <summary>
  /// A log message that libCEC generated
  /// </summary>
  public class CecLogMessage
  {
    public CecLogMessage(string message, CecLogLevel level, long time)
    {
      Message = message;
      Level = level;
      Time = time;
      Empty = false;
    }

    public CecLogMessage()
    {
      Message = "";
      Level = CecLogLevel.None;
      Time = 0;
      Empty = true;
    }

    public bool Empty { get; set; }
    public string Message { get; set; }
    public CecLogLevel Level { get; set; }
    public long Time { get; set; }
  }

  /// <summary>
  /// Statistics about traffic on the CEC line
  /// </summary>
  public class CecAdapterStats
  {
    internal CecAdapterStats(cec_adapter_stats stats)
    {
      TxAck = stats.tx_ack;
      TxNack = stats.tx_nack;
      TxError = stats.tx_error;
      RxTotal = stats.rx_total;
      RxError = stats.rx_error;
    }

    public uint TxAck { get; set; }
    public uint TxNack { get; set; }
    public uint TxError { get; set; }
    public uint RxTotal { get; set; }
    public uint RxError { get; set; }
  }
}
