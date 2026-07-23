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

namespace CecSharp
{
  /// <summary>
  /// The configuration that libCEC uses.
  /// </summary>
  public class LibCECConfiguration
  {
    public LibCECConfiguration()
    {
      DeviceName = "";
      DeviceTypes = new CecDeviceTypeList();
      AutodetectAddress = true;
      PhysicalAddress = CecDefaults.DefaultPhysicalAddress;
      BaseDevice = (CecLogicalAddress)CecDefaults.DefaultBaseDevice;
      HDMIPort = CecDefaults.DefaultHdmiPort;
      ClientVersion = Native.LibVersionCurrent;
      ServerVersion = 0;
      TvVendor = CecVendorId.Unknown;

      GetSettingsFromROM = false;
      ActivateSource = CecDefaults.ActivateSource == 1;

      WakeDevices = new CecLogicalAddresses();
      if (CecDefaults.ActivateSource == 1)
        WakeDevices.Set(CecLogicalAddress.Tv);

      PowerOffDevices = new CecLogicalAddresses();
      if (CecDefaults.PowerOffShutdown == 1)
        PowerOffDevices.Set(CecLogicalAddress.Broadcast);

      PowerOffOnStandby = CecDefaults.PowerOffOnStandby == 1;

      LogicalAddresses = new CecLogicalAddresses();
      FirmwareVersion = 1;
      DeviceLanguage = CecDefaults.DeviceLanguage;
      FirmwareBuildDate = CecDefaults.Epoch;
      CECVersion = (CecVersion)CecDefaults.CecVersion;
      AdapterType = CecAdapterType.Unknown;

      ComboKey = CecUserControlCode.Stop;
      ComboKeyTimeoutMs = CecDefaults.ComboTimeoutMs;
      ButtonRepeatRateMs = 0;
      ButtonReleaseDelayMs = CecDefaults.ButtonTimeout;
      DoubleTapTimeoutMs = 0;
      AutoWakeAVR = false;
      AutoPowerOn = BoolSetting.NotSet;
      AutonomousMode = BoolSetting.NotSet;
      ButtonRepeatDelayMs = CecDefaults.ButtonRepeatDelayMs;
      DeviceVendorId = CecVendorId.Unknown;
    }

    public static uint CurrentVersion = Native.LibVersionCurrent;

    public string DeviceName { get; set; }
    public CecDeviceTypeList DeviceTypes { get; set; }
    public bool AutodetectAddress { get; set; }
    public ushort PhysicalAddress { get; set; }
    public CecLogicalAddress BaseDevice { get; set; }
    public byte HDMIPort { get; set; }
    public uint ClientVersion { get; set; }
    public uint ServerVersion { get; set; }
    public CecVendorId TvVendor { get; set; }
    public bool GetSettingsFromROM { get; set; }
    public bool ActivateSource { get; set; }
    public CecLogicalAddresses WakeDevices { get; set; }
    public CecLogicalAddresses PowerOffDevices { get; set; }
    public bool PowerOffOnStandby { get; set; }
    public CecLogicalAddresses LogicalAddresses { get; set; }
    public ushort FirmwareVersion { get; set; }
    public bool MonitorOnlyClient { get; set; }
    public string DeviceLanguage { get; set; }
    public CecCallbackMethods Callbacks { get; set; }
    public DateTime FirmwareBuildDate { get; set; }
    public CecVersion CECVersion { get; set; }
    public CecAdapterType AdapterType { get; set; }
    public CecUserControlCode ComboKey { get; set; }
    public uint ComboKeyTimeoutMs { get; set; }
    public uint ButtonRepeatRateMs { get; set; }
    public uint ButtonReleaseDelayMs { get; set; }
    public uint DoubleTapTimeoutMs { get; set; }
    public bool AutoWakeAVR { get; set; }
    public BoolSetting AutoPowerOn { get; set; }
    public BoolSetting AutonomousMode { get; set; }
    public uint ButtonRepeatDelayMs { get; set; }
    public CecVendorId DeviceVendorId { get; set; }

    /// <summary>
    /// Copy the settings of another managed configuration into this one.
    /// </summary>
    public void Update(LibCECConfiguration config)
    {
      DeviceTypes = config.DeviceTypes;
      WakeDevices = config.WakeDevices;
      PowerOffDevices = config.PowerOffDevices;
      LogicalAddresses = config.LogicalAddresses;
      DeviceName = config.DeviceName;
      AutodetectAddress = config.AutodetectAddress;
      PhysicalAddress = config.PhysicalAddress;
      BaseDevice = config.BaseDevice;
      HDMIPort = config.HDMIPort;
      ClientVersion = config.ClientVersion;
      ServerVersion = config.ServerVersion;
      TvVendor = config.TvVendor;
      GetSettingsFromROM = config.GetSettingsFromROM;
      ActivateSource = config.ActivateSource;
      PowerOffOnStandby = config.PowerOffOnStandby;
      FirmwareVersion = config.FirmwareVersion;
      DeviceLanguage = config.DeviceLanguage;
      FirmwareBuildDate = config.FirmwareBuildDate;
      MonitorOnlyClient = config.MonitorOnlyClient;
      CECVersion = config.CECVersion;
      AdapterType = config.AdapterType;
      ComboKey = config.ComboKey;
      ComboKeyTimeoutMs = config.ComboKeyTimeoutMs;
      ButtonRepeatRateMs = config.ButtonRepeatRateMs;
      ButtonReleaseDelayMs = config.ButtonReleaseDelayMs;
      DoubleTapTimeoutMs = config.DoubleTapTimeoutMs;
      AutoWakeAVR = config.AutoWakeAVR;
      AutoPowerOn = config.AutoPowerOn;
      AutonomousMode = config.AutonomousMode;
      ButtonRepeatDelayMs = config.ButtonRepeatDelayMs;
      DeviceVendorId = config.DeviceVendorId;
    }
  }
}
