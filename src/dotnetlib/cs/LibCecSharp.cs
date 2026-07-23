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

namespace CecSharp
{
  /// <summary>
  /// Create a LibCecSharp instance and pass the configuration as argument.
  /// Then call Open() to open a connection to the adapter. Close() closes the
  /// connection.
  ///
  /// libCEC can send commands to other devices on the CEC bus via the methods
  /// on this class, and all commands that libCEC receives are sent back to the
  /// application via the callback methods on CecCallbackMethods.
  ///
  /// This is the pure-C# implementation that binds the native C API
  /// (include/cecc.h) over P/Invoke, so it runs on every platform the native
  /// library loads on. It replaces the Windows-only C++/CLI wrapper.
  /// </summary>
  public class LibCecSharp : IDisposable
  {
    /// <summary>Create a new LibCecSharp instance.</summary>
    public unsafe LibCecSharp(CecCallbackMethods callbacks, LibCECConfiguration config)
    {
      _callbacks = callbacks;
      libcec_configuration native = Interop.ToNative(config);
      native.callbacks = callbacks.Callbacks;
      native.callbackParam = IntPtr.Zero;

      _handle = LibCec.libcec_initialise(ref native);
      if (_handle == IntPtr.Zero)
        throw new Exception("Could not initialise LibCecSharp");

      Interop.ToManaged(&native, config);
    }

    ~LibCecSharp()
    {
      Destroy();
    }

    public void Dispose()
    {
      Destroy();
      GC.SuppressFinalize(this);
    }

    private void Destroy()
    {
      if (_handle != IntPtr.Zero)
      {
        LibCec.libcec_close(_handle);
        LibCec.libcec_destroy(_handle);
        _handle = IntPtr.Zero;
      }
      if (_callbacks != null)
        _callbacks.Destroy();
    }

    /// <summary>Try to find all connected CEC adapters.</summary>
    public unsafe CecAdapter[] FindAdapters(string path)
    {
      const int max = 10;
      var devices = new cec_adapter_descriptor[max];
      sbyte found = LibCec.libcec_detect_adapters(_handle, devices, max, string.IsNullOrEmpty(path) ? null : path, 0);
      if (found < 0)
        found = 0;

      var adapters = new CecAdapter[found];
      fixed (cec_adapter_descriptor* p = devices)
      {
        for (int i = 0; i < found; i++)
          adapters[i] = Interop.ToManaged(&p[i]);
      }
      return adapters;
    }

    /// <summary>Open a connection to the CEC adapter.</summary>
    public bool Open(string strPort, int iTimeoutMs)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_open(_handle, strPort, (uint)iTimeoutMs) == 1;
    }

    /// <summary>Close the connection to the CEC adapter.</summary>
    public void Close()
    {
      if (_handle != IntPtr.Zero)
        LibCec.libcec_close(_handle);
    }

    public void EnableCallbacks()
    {
      if (_handle != IntPtr.Zero)
        LibCec.libcec_set_callbacks(_handle, _callbacks.Callbacks, IntPtr.Zero);
    }

    public void DisableCallbacks()
    {
      if (_handle != IntPtr.Zero)
        LibCec.libcec_disable_callbacks(_handle);
    }

    /// <summary>Send a ping command to the adapter, to check if it's responding.</summary>
    public bool PingAdapter()
    {
      return _handle != IntPtr.Zero && LibCec.libcec_ping_adapters(_handle) == 1;
    }

    /// <summary>Start the bootloader of the CEC adapter. Closes the connection when successful.</summary>
    public bool StartBootloader()
    {
      return _handle != IntPtr.Zero && LibCec.libcec_start_bootloader(_handle) == 1;
    }

    /// <summary>Transmit a raw CEC command over the CEC line.</summary>
    public bool Transmit(CecCommand command)
    {
      if (_handle == IntPtr.Zero)
        return false;
      cec_command native = Interop.ToNative(command);
      return LibCec.libcec_transmit(_handle, ref native) == 1;
    }

    /// <summary>Change the logical address of the CEC adapter (debugging only).</summary>
    public bool SetLogicalAddress(CecLogicalAddress logicalAddress)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_set_logical_address(_handle, (int)logicalAddress) == 1;
    }

    /// <summary>Change the physical address (HDMI port) of the CEC adapter.</summary>
    public bool SetPhysicalAddress(ushort physicalAddress)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_set_physical_address(_handle, physicalAddress) == 1;
    }

    /// <summary>Power on the given CEC capable devices.</summary>
    public bool PowerOnDevices(CecLogicalAddress logicalAddress)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_power_on_devices(_handle, (int)logicalAddress) == 1;
    }

    /// <summary>Put the given CEC capable devices in standby mode.</summary>
    public bool StandbyDevices(CecLogicalAddress logicalAddress)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_standby_devices(_handle, (int)logicalAddress) == 1;
    }

    /// <summary>Send a POLL message to a device, to check if it's present and responding.</summary>
    public bool PollDevice(CecLogicalAddress logicalAddress)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_poll_device(_handle, (int)logicalAddress) == 1;
    }

    /// <summary>Change the active source to a device type handled by libCEC.</summary>
    public bool SetActiveSource(CecDeviceType type)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_set_active_source(_handle, (int)type) == 1;
    }

    /// <summary>Change the deck control mode, if this adapter is a playback or recording device.</summary>
    public bool SetDeckControlMode(CecDeckControlMode mode, bool sendUpdate)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_set_deck_control_mode(_handle, (int)mode, sendUpdate ? 1 : 0) == 1;
    }

    /// <summary>Change the deck info, if this adapter is a playback or recording device.</summary>
    public bool SetDeckInfo(CecDeckInfo info, bool sendUpdate)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_set_deck_info(_handle, (int)info, sendUpdate ? 1 : 0) == 1;
    }

    /// <summary>Broadcast a message that this device is no longer the active source.</summary>
    public bool SetInactiveView()
    {
      return _handle != IntPtr.Zero && LibCec.libcec_set_inactive_view(_handle) == 1;
    }

    /// <summary>Change the menu state.</summary>
    public bool SetMenuState(CecMenuState state, bool sendUpdate)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_set_menu_state(_handle, (int)state, sendUpdate ? 1 : 0) == 1;
    }

    /// <summary>Display a message on the device with the given logical address.</summary>
    public bool SetOSDString(CecLogicalAddress logicalAddress, CecDisplayControl duration, string message)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_set_osd_string(_handle, (int)logicalAddress, (int)duration, message) == 1;
    }

    /// <summary>Enable or disable monitoring mode, for debugging purposes.</summary>
    public bool SwitchMonitoring(bool enable)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_switch_monitoring(_handle, enable ? 1 : 0) == 1;
    }

    /// <summary>Get the CEC version of the device with the given logical address.</summary>
    public CecVersion GetDeviceCecVersion(CecLogicalAddress logicalAddress)
    {
      if (_handle == IntPtr.Zero)
        return CecVersion.Unknown;
      return (CecVersion)LibCec.libcec_get_device_cec_version(_handle, (int)logicalAddress);
    }

    /// <summary>Get the menu language of the device with the given logical address.</summary>
    public string GetDeviceMenuLanguage(CecLogicalAddress logicalAddress)
    {
      if (_handle == IntPtr.Zero)
        return "not connected";
      var buf = new byte[4]; // cec_menu_language
      LibCec.libcec_get_device_menu_language(_handle, (int)logicalAddress, buf);
      return Interop.GetAnsi(buf);
    }

    /// <summary>Get the vendor ID of the device with the given logical address.</summary>
    public CecVendorId GetDeviceVendorId(CecLogicalAddress logicalAddress)
    {
      if (_handle == IntPtr.Zero)
        return CecVendorId.Unknown;
      return (CecVendorId)LibCec.libcec_get_device_vendor_id(_handle, (int)logicalAddress);
    }

    /// <summary>Get the power status of the device with the given logical address.</summary>
    public CecPowerStatus GetDevicePowerStatus(CecLogicalAddress logicalAddress)
    {
      if (_handle == IntPtr.Zero)
        return CecPowerStatus.Unknown;
      return (CecPowerStatus)LibCec.libcec_get_device_power_status(_handle, (int)logicalAddress);
    }

    /// <summary>Tell libCEC to poll for active devices on the bus.</summary>
    public void RescanActiveDevices()
    {
      if (_handle != IntPtr.Zero)
        LibCec.libcec_rescan_devices(_handle);
    }

    /// <summary>Get the logical addresses of the devices that are active on the bus.</summary>
    public unsafe CecLogicalAddresses GetActiveDevices()
    {
      if (_handle == IntPtr.Zero)
        return new CecLogicalAddresses();
      cec_logical_addresses native = LibCec.libcec_get_active_devices(_handle);
      return Interop.GetManagedAddresses(&native);
    }

    /// <summary>Check whether a device is active on the bus.</summary>
    public bool IsActiveDevice(CecLogicalAddress logicalAddress)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_is_active_device(_handle, (int)logicalAddress) == 1;
    }

    /// <summary>Check whether a device of the given type is active on the bus.</summary>
    public bool IsActiveDeviceType(CecDeviceType type)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_is_active_device_type(_handle, (int)type) == 1;
    }

    /// <summary>Change the active HDMI port.</summary>
    public bool SetHDMIPort(CecLogicalAddress address, byte port)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_set_hdmi_port(_handle, (int)address, port) == 1;
    }

    /// <summary>Send a volume up keypress to an audiosystem if it's present.</summary>
    public byte VolumeUp(bool sendRelease)
    {
      if (_handle == IntPtr.Zero)
        return 0;
      return (byte)LibCec.libcec_volume_up(_handle, sendRelease ? 1 : 0);
    }

    /// <summary>Send a volume down keypress to an audiosystem if it's present.</summary>
    public byte VolumeDown(bool sendRelease)
    {
      if (_handle == IntPtr.Zero)
        return 0;
      return (byte)LibCec.libcec_volume_down(_handle, sendRelease ? 1 : 0);
    }

    /// <summary>Send a mute keypress to an audiosystem if it's present.</summary>
    public byte MuteAudio()
    {
      if (_handle == IntPtr.Zero)
        return 0;
      return LibCec.libcec_audio_toggle_mute(_handle);
    }

    /// <summary>Send a keypress to a device on the CEC bus.</summary>
    public bool SendKeypress(CecLogicalAddress destination, CecUserControlCode key, bool wait)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_send_keypress(_handle, (int)destination, (int)key, wait ? 1 : 0) == 1;
    }

    /// <summary>Send a key release to a device on the CEC bus.</summary>
    public bool SendKeyRelease(CecLogicalAddress destination, bool wait)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_send_key_release(_handle, (int)destination, wait ? 1 : 0) == 1;
    }

    /// <summary>Get the OSD name of a device on the CEC bus.</summary>
    public string GetDeviceOSDName(CecLogicalAddress logicalAddress)
    {
      if (_handle == IntPtr.Zero)
        return "disconnected";
      var buf = new byte[15]; // cec_osd_name is 14 chars + terminator headroom
      LibCec.libcec_get_device_osd_name(_handle, (int)logicalAddress, buf);
      return Interop.GetAnsi(buf);
    }

    /// <summary>Get the logical address of the device that is currently the active source.</summary>
    public CecLogicalAddress GetActiveSource()
    {
      if (_handle == IntPtr.Zero)
        return CecLogicalAddress.Unknown;
      return (CecLogicalAddress)LibCec.libcec_get_active_source(_handle);
    }

    /// <summary>Check whether a device is currently the active source on the CEC bus.</summary>
    public bool IsActiveSource(CecLogicalAddress logicalAddress)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_is_active_source(_handle, (int)logicalAddress) == 1;
    }

    /// <summary>Get the physical address of the device with the given logical address.</summary>
    public ushort GetDevicePhysicalAddress(CecLogicalAddress address)
    {
      if (_handle == IntPtr.Zero)
        return 0xFFFF;
      return LibCec.libcec_get_device_physical_address(_handle, (int)address);
    }

    /// <summary>Set the stream path to the device on the given logical address.</summary>
    public bool SetStreamPath(CecLogicalAddress address)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_set_stream_path_logical(_handle, (int)address) == 1;
    }

    /// <summary>Set the stream path to the device on the given physical address.</summary>
    public bool SetStreamPath(ushort physicalAddress)
    {
      return _handle != IntPtr.Zero && LibCec.libcec_set_stream_path_physical(_handle, physicalAddress) == 1;
    }

    /// <summary>Get the list of logical addresses that libCEC is controlling.</summary>
    public unsafe CecLogicalAddresses GetLogicalAddresses()
    {
      if (_handle == IntPtr.Zero)
        return new CecLogicalAddresses();
      cec_logical_addresses native = LibCec.libcec_get_logical_addresses(_handle);
      return Interop.GetManagedAddresses(&native);
    }

    /// <summary>Get libCEC's current configuration.</summary>
    public unsafe bool GetCurrentConfiguration(LibCECConfiguration configuration)
    {
      if (_handle == IntPtr.Zero)
        return false;
      libcec_configuration native = new libcec_configuration();
      if (LibCec.libcec_get_current_configuration(_handle, ref native) == 1)
      {
        Interop.ToManaged(&native, configuration);
        return true;
      }
      return false;
    }

    /// <summary>Check whether the CEC adapter can save a configuration.</summary>
    public bool CanSaveConfiguration()
    {
      return _handle != IntPtr.Zero && LibCec.libcec_can_save_configuration(_handle) == 1;
    }

    /// <summary>Change libCEC's configuration.</summary>
    public bool SetConfiguration(LibCECConfiguration configuration)
    {
      if (_handle == IntPtr.Zero)
        return false;
      libcec_configuration native = Interop.ToNative(configuration);
      // don't update callbacks
      native.callbacks = IntPtr.Zero;
      native.callbackParam = IntPtr.Zero;
      return LibCec.libcec_set_configuration(_handle, ref native) == 1;
    }

    /// <summary>Check whether libCEC is the active source on the bus.</summary>
    public bool IsLibCECActiveSource()
    {
      return _handle != IntPtr.Zero && LibCec.libcec_is_libcec_active_source(_handle) == 1;
    }

    /// <summary>Get information about the given CEC adapter.</summary>
    public unsafe bool GetDeviceInformation(string port, LibCECConfiguration configuration, uint timeoutMs)
    {
      if (_handle == IntPtr.Zero)
        return false;
      libcec_configuration native = new libcec_configuration();
      if (LibCec.libcec_get_device_information(_handle, string.IsNullOrEmpty(port) ? null : port, ref native, timeoutMs) == 1)
      {
        Interop.ToManaged(&native, configuration);
        return true;
      }
      return false;
    }

    public string ToString(CecLogicalAddress iAddress)
    {
      var buf = new byte[64];
      LibCec.libcec_logical_address_to_string((int)iAddress, buf, (UIntPtr)buf.Length);
      return Interop.GetAnsi(buf);
    }

    public string ToString(CecVendorId iVendorId)
    {
      var buf = new byte[64];
      LibCec.libcec_vendor_id_to_string((int)iVendorId, buf, (UIntPtr)buf.Length);
      return Interop.GetAnsi(buf);
    }

    public string ToString(CecVersion iVersion)
    {
      var buf = new byte[64];
      LibCec.libcec_cec_version_to_string((int)iVersion, buf, (UIntPtr)buf.Length);
      return Interop.GetAnsi(buf);
    }

    public string ToString(CecPowerStatus iState)
    {
      var buf = new byte[64];
      LibCec.libcec_power_status_to_string((int)iState, buf, (UIntPtr)buf.Length);
      return Interop.GetAnsi(buf);
    }

    public string ToString(CecMenuState iState)
    {
      var buf = new byte[64];
      LibCec.libcec_menu_state_to_string((int)iState, buf, (UIntPtr)buf.Length);
      return Interop.GetAnsi(buf);
    }

    public string ToString(CecDeckControlMode iMode)
    {
      var buf = new byte[64];
      LibCec.libcec_deck_control_mode_to_string((int)iMode, buf, (UIntPtr)buf.Length);
      return Interop.GetAnsi(buf);
    }

    public string ToString(CecDeckInfo status)
    {
      var buf = new byte[64];
      LibCec.libcec_deck_status_to_string((int)status, buf, (UIntPtr)buf.Length);
      return Interop.GetAnsi(buf);
    }

    public string ToString(CecOpcode opcode)
    {
      var buf = new byte[64];
      LibCec.libcec_opcode_to_string((int)opcode, buf, (UIntPtr)buf.Length);
      return Interop.GetAnsi(buf);
    }

    public string ToString(CecSystemAudioStatus mode)
    {
      var buf = new byte[64];
      LibCec.libcec_system_audio_status_to_string((int)mode, buf, (UIntPtr)buf.Length);
      return Interop.GetAnsi(buf);
    }

    public string ToString(CecAudioStatus status)
    {
      var buf = new byte[64];
      LibCec.libcec_audio_status_to_string((int)status, buf, (UIntPtr)buf.Length);
      return Interop.GetAnsi(buf);
    }

    public string VersionToString(uint version)
    {
      var buf = new byte[64];
      LibCec.libcec_version_to_string(version, buf, (UIntPtr)buf.Length);
      return Interop.GetAnsi(buf);
    }

    public string PhysicalAddressToString(ushort physicalAddress)
    {
      return string.Format("{0:X}.{1:X}.{2:X}.{3:X}",
        (physicalAddress >> 12) & 0xF, (physicalAddress >> 8) & 0xF,
        (physicalAddress >> 4) & 0xF, physicalAddress & 0xF);
    }

    /// <summary>Get a string with information about how libCEC was compiled.</summary>
    public string GetLibInfo()
    {
      if (_handle == IntPtr.Zero)
        return "disconnected";
      return Marshal.PtrToStringAnsi(LibCec.libcec_get_lib_info(_handle)) ?? string.Empty;
    }

    /// <summary>
    /// Initialise the host on which libCEC is running. On the RPi this calls
    /// bcm_host_init(). Should be called directly after construction and before Open().
    /// </summary>
    public void InitVideoStandalone()
    {
      if (_handle != IntPtr.Zero)
        LibCec.libcec_init_video_standalone(_handle);
    }

    /// <summary>Get statistics about traffic on the CEC line.</summary>
    public CecAdapterStats GetStats()
    {
      var native = new cec_adapter_stats();
      try
      {
        if (_handle != IntPtr.Zero)
          LibCec.libcec_get_stats(_handle, ref native);
      }
      catch (EntryPointNotFoundException)
      {
        // older native library without libcec_get_stats; report zeroed stats
      }
      return new CecAdapterStats(native);
    }

    /// <summary>Get the (virtual) USB vendor id.</summary>
    public ushort GetAdapterVendorId()
    {
      if (_handle == IntPtr.Zero)
        return 0;
      return LibCec.libcec_get_adapter_vendor_id(_handle);
    }

    /// <summary>Get the (virtual) USB product id.</summary>
    public ushort GetAdapterProductId()
    {
      if (_handle == IntPtr.Zero)
        return 0;
      return LibCec.libcec_get_adapter_product_id(_handle);
    }

    private IntPtr _handle;
    private readonly CecCallbackMethods _callbacks;
  }
}
