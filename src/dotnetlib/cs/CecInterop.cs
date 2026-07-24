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
using System.Runtime.InteropServices;
using System.Text;

// Native ABI surface. Every struct here is deliberately *blittable* (fixed
// buffers instead of marshalled arrays/strings) so that ref-passing and
// by-value returns are plain memory copies with no marshaller transformation.
// That removes the classic P/Invoke ambiguities (struct-with-array returned by
// value, padding surprises) and lets LayoutKind.Sequential reproduce the C
// compiler's layout of include/cectypes.h byte-for-byte on both x86 and x64
// (the pointer fields switch width via IntPtr).
//
// All native<->managed conversion lives in the Interop helper at the bottom of
// this file, so the public data classes stay free of unsafe code.

namespace CecSharp
{
  internal static class Native
  {
    // cec.dll / libcec.so / libcec.dylib
    internal const string Library = "cec";

    // uint32 encoding of the current libCEC version (LIBCEC_VERSION_TO_UINT(8,0,0)),
    // kept in sync with include/version.h.
    internal const uint LibVersionCurrent = (8u << 16) | (0u << 8) | 0u;
  }

  [StructLayout(LayoutKind.Sequential)]
  internal unsafe struct cec_datapacket
  {
    public fixed byte data[64]; // CEC_MAX_DATA_PACKET_SIZE
    public byte size;
  }

  [StructLayout(LayoutKind.Sequential)]
  internal struct cec_command
  {
    public int            initiator;        // cec_logical_address
    public int            destination;      // cec_logical_address
    public sbyte          ack;
    public sbyte          eom;
    public int            opcode;           // cec_opcode
    public cec_datapacket parameters;
    public sbyte          opcode_set;
    public int            transmit_timeout;
  }

  [StructLayout(LayoutKind.Sequential)]
  internal struct cec_keypress
  {
    public int  keycode;   // cec_user_control_code
    public uint duration;
  }

  [StructLayout(LayoutKind.Sequential)]
  internal struct cec_log_message
  {
    public IntPtr message; // const char*
    public int    level;   // cec_log_level
    public long   time;
  }

  [StructLayout(LayoutKind.Sequential)]
  internal unsafe struct cec_adapter_descriptor
  {
    public fixed byte strComPath[1024];
    public fixed byte strComName[1024];
    public ushort iVendorId;
    public ushort iProductId;
    public ushort iFirmwareVersion;
    public ushort iPhysicalAddress;
    public uint   iFirmwareBuildDate;
    public int    adapterType; // cec_adapter_type
    public fixed byte strDeviceName[64];
  }

  [StructLayout(LayoutKind.Sequential)]
  internal unsafe struct cec_device_type_list
  {
    public fixed int types[5]; // cec_device_type[5]
  }

  [StructLayout(LayoutKind.Sequential)]
  internal unsafe struct cec_logical_addresses
  {
    public int primary; // cec_logical_address
    public fixed int addresses[16];
  }

  [StructLayout(LayoutKind.Sequential)]
  internal struct libcec_parameter
  {
    public int    paramType; // libcec_parameter_type
    public IntPtr paramData;
  }

  [StructLayout(LayoutKind.Sequential)]
  internal struct cec_adapter_stats
  {
    public uint tx_ack;
    public uint tx_nack;
    public uint tx_error;
    public uint rx_total;
    public uint rx_error;
  }

  // ICECCallbacks: table of C function pointers, filled from pinned delegates in
  // CecCallbackMethods and handed to libcec_set_callbacks / the config struct.
  [StructLayout(LayoutKind.Sequential)]
  internal struct ICECCallbacks
  {
    public IntPtr logMessage;
    public IntPtr keyPress;
    public IntPtr commandReceived;
    public IntPtr configurationChanged;
    public IntPtr alert;
    public IntPtr menuStateChanged;
    public IntPtr sourceActivated;
    public IntPtr commandHandler;
  }

  // Mirror of struct libcec_configuration for CEC_LIB_VERSION_MAJOR == 8
  // (LIBCEC_OSD_NAME_SIZE == 15). Do not reorder: the field order and types are
  // load-bearing.
  [StructLayout(LayoutKind.Sequential)]
  internal unsafe struct libcec_configuration
  {
    public uint clientVersion;
    public fixed byte strDeviceName[15]; // LIBCEC_OSD_NAME_SIZE
    public cec_device_type_list deviceTypes;
    public byte   bAutodetectAddress;
    public ushort iPhysicalAddress;
    public int    baseDevice;      // cec_logical_address
    public byte   iHDMIPort;
    public uint   tvVendor;
    public cec_logical_addresses wakeDevices;
    public cec_logical_addresses powerOffDevices;
    public uint   serverVersion;
    public byte   bGetSettingsFromROM;
    public byte   bActivateSource;
    public byte   bPowerOffOnStandby;
    public IntPtr callbackParam;
    public IntPtr callbacks;
    public cec_logical_addresses logicalAddresses;
    public ushort iFirmwareVersion;
    public fixed byte strDeviceLanguage[3]; // 3 chars, no NUL terminator
    public uint   iFirmwareBuildDate;
    public byte   bMonitorOnly;
    public int    cecVersion;   // cec_version
    public int    adapterType;  // cec_adapter_type
    public int    comboKey;     // cec_user_control_code
    public uint   iComboKeyTimeoutMs;
    public uint   iButtonRepeatRateMs;
    public uint   iButtonReleaseDelayMs;
    public uint   iDoubleTapTimeoutMs;
    public byte   bAutoWakeAVR;
    public byte   bAutoPowerOn;         // CEC_LIB_VERSION_MAJOR >= 5
    public byte   bAutonomousMode;      // CEC_LIB_VERSION_MAJOR >= 8
    public uint   iButtonRepeatDelayMs; // CEC_LIB_VERSION_MAJOR >= 8
    public uint   iDeviceVendorId;      // CEC_LIB_VERSION_MAJOR >= 8
  }

  // Unmanaged callback delegate signatures. CEC_CDECL is __cdecl on Windows and
  // the platform default (cdecl) elsewhere, so Cdecl is correct everywhere.
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
  internal delegate void LogMessageDelegate(IntPtr cbParam, IntPtr message);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
  internal delegate void KeyPressDelegate(IntPtr cbParam, IntPtr key);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
  internal delegate void CommandDelegate(IntPtr cbParam, IntPtr command);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
  internal delegate void ConfigDelegate(IntPtr cbParam, IntPtr config);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
  internal delegate void AlertDelegate(IntPtr cbParam, int alert, libcec_parameter data);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
  internal delegate int MenuDelegate(IntPtr cbParam, int state);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
  internal delegate void SourceActivatedDelegate(IntPtr cbParam, int logicalAddress, byte activated);

  internal static class LibCec
  {
    private const string L = Native.Library;
    private const CallingConvention C = CallingConvention.Cdecl;

    [DllImport(L, CallingConvention = C)]
    internal static extern IntPtr libcec_initialise(ref libcec_configuration configuration);

    [DllImport(L, CallingConvention = C)]
    internal static extern void libcec_destroy(IntPtr connection);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_open(IntPtr connection, [MarshalAs(UnmanagedType.LPStr)] string strPort, uint iTimeout);

    [DllImport(L, CallingConvention = C)]
    internal static extern void libcec_close(IntPtr connection);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_set_callbacks(IntPtr connection, IntPtr callbacks, IntPtr cbParam);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_disable_callbacks(IntPtr connection);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_ping_adapters(IntPtr connection);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_start_bootloader(IntPtr connection);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_power_on_devices(IntPtr connection, int address);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_standby_devices(IntPtr connection, int address);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_set_active_source(IntPtr connection, int type);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_set_deck_control_mode(IntPtr connection, int mode, int bSendUpdate);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_set_deck_info(IntPtr connection, int info, int bSendUpdate);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_set_inactive_view(IntPtr connection);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_set_menu_state(IntPtr connection, int state, int bSendUpdate);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_transmit(IntPtr connection, ref cec_command data);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_set_logical_address(IntPtr connection, int iLogicalAddress);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_set_physical_address(IntPtr connection, ushort iPhysicalAddress);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_set_osd_string(IntPtr connection, int iLogicalAddress, int duration, [MarshalAs(UnmanagedType.LPStr)] string strMessage);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_switch_monitoring(IntPtr connection, int bEnable);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_get_device_cec_version(IntPtr connection, int iLogicalAddress);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_get_device_menu_language(IntPtr connection, int iLogicalAddress, [Out] byte[] language);

    [DllImport(L, CallingConvention = C)]
    internal static extern uint libcec_get_device_vendor_id(IntPtr connection, int iLogicalAddress);

    [DllImport(L, CallingConvention = C)]
    internal static extern ushort libcec_get_device_physical_address(IntPtr connection, int iLogicalAddress);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_get_active_source(IntPtr connection);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_is_active_source(IntPtr connection, int iAddress);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_get_device_power_status(IntPtr connection, int iLogicalAddress);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_poll_device(IntPtr connection, int iLogicalAddress);

    [DllImport(L, CallingConvention = C)]
    internal static extern cec_logical_addresses libcec_get_active_devices(IntPtr connection);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_is_active_device(IntPtr connection, int address);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_is_active_device_type(IntPtr connection, int type);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_set_hdmi_port(IntPtr connection, int baseDevice, byte iPort);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_volume_up(IntPtr connection, int bSendRelease);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_volume_down(IntPtr connection, int bSendRelease);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_mute_audio(IntPtr connection, int bSendRelease);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_send_keypress(IntPtr connection, int iDestination, int key, int bWait);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_send_key_release(IntPtr connection, int iDestination, int bWait);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_get_device_osd_name(IntPtr connection, int iAddress, [Out] byte[] name);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_set_stream_path_logical(IntPtr connection, int iAddress);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_set_stream_path_physical(IntPtr connection, ushort iPhysicalAddress);

    [DllImport(L, CallingConvention = C)]
    internal static extern cec_logical_addresses libcec_get_logical_addresses(IntPtr connection);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_get_current_configuration(IntPtr connection, ref libcec_configuration configuration);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_can_save_configuration(IntPtr connection);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_set_configuration(IntPtr connection, ref libcec_configuration configuration);

    [DllImport(L, CallingConvention = C)]
    internal static extern void libcec_rescan_devices(IntPtr connection);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_is_libcec_active_source(IntPtr connection);

    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_get_device_information(IntPtr connection, [MarshalAs(UnmanagedType.LPStr)] string strPort, ref libcec_configuration config, uint iTimeoutMs);

    [DllImport(L, CallingConvention = C)]
    internal static extern IntPtr libcec_get_lib_info(IntPtr connection);

    [DllImport(L, CallingConvention = C)]
    internal static extern void libcec_init_video_standalone(IntPtr connection);

    [DllImport(L, CallingConvention = C)]
    internal static extern ushort libcec_get_adapter_vendor_id(IntPtr connection);

    [DllImport(L, CallingConvention = C)]
    internal static extern ushort libcec_get_adapter_product_id(IntPtr connection);

    [DllImport(L, CallingConvention = C)]
    internal static extern byte libcec_audio_toggle_mute(IntPtr connection);

    // Added to the C API for GetStats parity (see src/libcec/LibCECC.cpp).
    [DllImport(L, CallingConvention = C)]
    internal static extern int libcec_get_stats(IntPtr connection, ref cec_adapter_stats stats);

    [DllImport(L, CallingConvention = C)]
    internal static extern sbyte libcec_detect_adapters(IntPtr connection, [In, Out] cec_adapter_descriptor[] deviceList, byte iBufSize, [MarshalAs(UnmanagedType.LPStr)] string strDevicePath, int bQuickScan);

    // string helpers (do not require a live connection)
    [DllImport(L, CallingConvention = C)]
    internal static extern void libcec_logical_address_to_string(int address, [Out] byte[] buf, UIntPtr bufsize);
    [DllImport(L, CallingConvention = C)]
    internal static extern void libcec_vendor_id_to_string(int vendor, [Out] byte[] buf, UIntPtr bufsize);
    [DllImport(L, CallingConvention = C)]
    internal static extern void libcec_cec_version_to_string(int version, [Out] byte[] buf, UIntPtr bufsize);
    [DllImport(L, CallingConvention = C)]
    internal static extern void libcec_power_status_to_string(int status, [Out] byte[] buf, UIntPtr bufsize);
    [DllImport(L, CallingConvention = C)]
    internal static extern void libcec_menu_state_to_string(int state, [Out] byte[] buf, UIntPtr bufsize);
    [DllImport(L, CallingConvention = C)]
    internal static extern void libcec_deck_control_mode_to_string(int mode, [Out] byte[] buf, UIntPtr bufsize);
    [DllImport(L, CallingConvention = C)]
    internal static extern void libcec_deck_status_to_string(int status, [Out] byte[] buf, UIntPtr bufsize);
    [DllImport(L, CallingConvention = C)]
    internal static extern void libcec_opcode_to_string(int opcode, [Out] byte[] buf, UIntPtr bufsize);
    [DllImport(L, CallingConvention = C)]
    internal static extern void libcec_system_audio_status_to_string(int mode, [Out] byte[] buf, UIntPtr bufsize);
    [DllImport(L, CallingConvention = C)]
    internal static extern void libcec_audio_status_to_string(int status, [Out] byte[] buf, UIntPtr bufsize);
    [DllImport(L, CallingConvention = C)]
    internal static extern void libcec_version_to_string(uint version, [Out] byte[] buf, UIntPtr bufsize);
  }

  // Centralized native<->managed conversion. All fixed-buffer pointer access is
  // confined here so the public data classes stay unsafe-free.
  internal static unsafe class Interop
  {
    internal static readonly DateTime Epoch = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc);

    // Decode a fixed ANSI byte buffer, stopping at the first NUL or maxLen.
    internal static string GetAnsi(byte* src, int maxLen)
    {
      int len = 0;
      while (len < maxLen && src[len] != 0)
        len++;
      return Encoding.ASCII.GetString(src, len);
    }

    // Copy an ASCII string into a fixed buffer of the given capacity. When
    // reserveNul is true, at most capacity-1 bytes are written so a NUL always
    // terminates (used for name); remaining bytes are already zero-initialized.
    internal static void SetAnsi(byte* dst, int capacity, string value, bool reserveNul)
    {
      for (int i = 0; i < capacity; i++)
        dst[i] = 0;
      if (string.IsNullOrEmpty(value))
        return;
      byte[] bytes = Encoding.ASCII.GetBytes(value);
      int max = reserveNul ? capacity - 1 : capacity;
      int n = Math.Min(bytes.Length, max);
      for (int i = 0; i < n; i++)
        dst[i] = bytes[i];
    }

    // Decode an ANSI string of at most maxLen bytes from a byte[] (to_string bufs).
    internal static string GetAnsi(byte[] buf)
    {
      int len = Array.IndexOf(buf, (byte)0);
      if (len < 0)
        len = buf.Length;
      return Encoding.ASCII.GetString(buf, 0, len);
    }

    // ---- cec_command ----------------------------------------------------

    internal static cec_command ToNative(CecCommand cmd)
    {
      var c = new cec_command
      {
        initiator = (int)cmd.Initiator,
        destination = (int)cmd.Destination,
        ack = (sbyte)(cmd.Ack ? 1 : 0),
        eom = (sbyte)(cmd.Eom ? 1 : 0),
        opcode = (int)cmd.Opcode,
        opcode_set = (sbyte)(cmd.OpcodeSet ? 1 : 0),
        transmit_timeout = cmd.TransmitTimeout,
      };
      byte count = cmd.Parameters.Size;
      if (count > 64)
        count = 64;
      for (byte i = 0; i < count; i++)
        c.parameters.data[i] = cmd.Parameters.Data[i];
      c.parameters.size = count;
      return c;
    }

    internal static CecCommand ToManaged(cec_command* c)
    {
      var command = new CecCommand((CecLogicalAddress)c->initiator, (CecLogicalAddress)c->destination,
        c->ack == 1, c->eom == 1, (CecOpcode)c->opcode, c->transmit_timeout);
      byte size = c->parameters.size;
      for (byte i = 0; i < size; i++)
        command.Parameters.PushBack(c->parameters.data[i]);
      return command;
    }

    // ---- cec_adapter_descriptor ----------------------------------------

    internal static CecAdapter ToManaged(cec_adapter_descriptor* d)
    {
      return new CecAdapter(
        GetAnsi(d->strComPath, 1024),
        GetAnsi(d->strComName, 1024),
        d->iVendorId, d->iProductId, d->iFirmwareVersion,
        d->iFirmwareBuildDate, d->iPhysicalAddress);
    }

    // ---- libcec_configuration ------------------------------------------

    internal static libcec_configuration ToNative(LibCECConfiguration cfg)
    {
      var c = new libcec_configuration();

      // Seed the read-only defaults that the native libcec_configuration::Clear()
      // constructor sets, so a zero-initialized managed struct behaves like the
      // C++ one (e.g. serverVersion is reported after construction). The managed
      // config never owns these fields; libCEC overwrites them once connected.
      c.serverVersion = Native.LibVersionCurrent;
      c.iFirmwareVersion = 0xFFFF; // CEC_FW_VERSION_UNKNOWN
      SetNativeAddresses(&c.logicalAddresses, null); // primary = UNREGISTERED

      SetAnsi(c.strDeviceName, 15, cfg.DeviceName, true);
      for (int i = 0; i < 5; i++)
        c.deviceTypes.types[i] = (int)cfg.DeviceTypes.Types[i];

      c.bAutodetectAddress = (byte)(cfg.AutodetectAddress ? 1 : 0);
      c.iPhysicalAddress = cfg.PhysicalAddress;
      c.baseDevice = (int)cfg.BaseDevice;
      c.iHDMIPort = cfg.HDMIPort;
      c.clientVersion = cfg.ClientVersion;
      c.bGetSettingsFromROM = (byte)(cfg.GetSettingsFromROM ? 1 : 0);
      c.bActivateSource = (byte)(cfg.ActivateSource ? 1 : 0);
      c.tvVendor = (uint)cfg.TvVendor;
      c.comboKey = (int)cfg.ComboKey;
      c.iComboKeyTimeoutMs = cfg.ComboKeyTimeoutMs;
      c.iButtonRepeatRateMs = cfg.ButtonRepeatRateMs;
      c.iButtonReleaseDelayMs = cfg.ButtonReleaseDelayMs;
      c.iDoubleTapTimeoutMs = cfg.DoubleTapTimeoutMs;
      c.bAutoWakeAVR = (byte)(cfg.AutoWakeAVR ? 1 : 0);
      c.bAutoPowerOn = (byte)cfg.AutoPowerOn;
      c.bAutonomousMode = (byte)cfg.AutonomousMode;
      c.iButtonRepeatDelayMs = cfg.ButtonRepeatDelayMs;
      c.iDeviceVendorId = (uint)cfg.DeviceVendorId;
      c.bPowerOffOnStandby = (byte)(cfg.PowerOffOnStandby ? 1 : 0);
      c.bMonitorOnly = (byte)(cfg.MonitorOnlyClient ? 1 : 0);
      c.cecVersion = (int)cfg.CECVersion;

      SetNativeAddresses(&c.wakeDevices, cfg.WakeDevices);
      SetNativeAddresses(&c.powerOffDevices, cfg.PowerOffDevices);

      SetAnsi(c.strDeviceLanguage, 3, cfg.DeviceLanguage, false);
      return c;
    }

    internal static void ToManaged(libcec_configuration* c, LibCECConfiguration cfg)
    {
      cfg.DeviceTypes.Clear();
      for (int i = 0; i < 5; i++)
        cfg.DeviceTypes.Types[i] = (CecDeviceType)c->deviceTypes.types[i];

      cfg.WakeDevices = GetManagedAddresses(&c->wakeDevices);
      cfg.PowerOffDevices = GetManagedAddresses(&c->powerOffDevices);
      cfg.LogicalAddresses = GetManagedAddresses(&c->logicalAddresses);

      cfg.DeviceName = GetAnsi(c->strDeviceName, 15);
      cfg.AutodetectAddress = c->bAutodetectAddress == 1;
      cfg.PhysicalAddress = c->iPhysicalAddress;
      cfg.BaseDevice = (CecLogicalAddress)c->baseDevice;
      cfg.HDMIPort = c->iHDMIPort;
      cfg.ClientVersion = c->clientVersion;
      cfg.ServerVersion = c->serverVersion;
      cfg.TvVendor = (CecVendorId)c->tvVendor;
      cfg.GetSettingsFromROM = c->bGetSettingsFromROM == 1;
      cfg.ActivateSource = c->bActivateSource == 1;
      cfg.PowerOffOnStandby = c->bPowerOffOnStandby == 1;
      cfg.FirmwareVersion = c->iFirmwareVersion;
      cfg.DeviceLanguage = GetAnsi(c->strDeviceLanguage, 3);
      cfg.FirmwareBuildDate = Epoch.AddSeconds(c->iFirmwareBuildDate);
      cfg.MonitorOnlyClient = c->bMonitorOnly == 1;
      cfg.CECVersion = (CecVersion)c->cecVersion;
      cfg.AdapterType = (CecAdapterType)c->adapterType;
      cfg.ComboKey = (CecUserControlCode)c->comboKey;
      cfg.ComboKeyTimeoutMs = c->iComboKeyTimeoutMs;
      cfg.ButtonRepeatRateMs = c->iButtonRepeatRateMs;
      cfg.ButtonReleaseDelayMs = c->iButtonReleaseDelayMs;
      cfg.DoubleTapTimeoutMs = c->iDoubleTapTimeoutMs;
      cfg.AutoWakeAVR = c->bAutoWakeAVR == 1;
      cfg.AutoPowerOn = c->bAutoPowerOn == 1 ? BoolSetting.Enabled : BoolSetting.Disabled;
      cfg.AutonomousMode = c->bAutonomousMode == 1 ? BoolSetting.Enabled : BoolSetting.Disabled;
      cfg.ButtonRepeatDelayMs = c->iButtonRepeatDelayMs;
      cfg.DeviceVendorId = (CecVendorId)c->iDeviceVendorId;
    }

    // ---- cec_logical_addresses -----------------------------------------

    internal static void SetNativeAddresses(cec_logical_addresses* n, CecLogicalAddresses managed)
    {
      n->primary = (int)CecLogicalAddress.Unregistered;
      for (int i = 0; i < 16; i++)
        n->addresses[i] = 0;
      if (managed == null)
        return;
      for (int i = 0; i < 16; i++)
      {
        if (managed.IsSet((CecLogicalAddress)i))
        {
          if (n->primary == (int)CecLogicalAddress.Unregistered)
            n->primary = i;
          n->addresses[i] = 1;
        }
      }
    }

    internal static CecLogicalAddresses GetManagedAddresses(cec_logical_addresses* n)
    {
      var managed = new CecLogicalAddresses();
      for (int i = 0; i < 16; i++)
        if (n->addresses[i] == 1)
          managed.Set((CecLogicalAddress)i);
      return managed;
    }
  }
}
