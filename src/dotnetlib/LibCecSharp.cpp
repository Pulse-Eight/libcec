/*
* This file is part of the libCEC(R) library.
*
* libCEC(R) is Copyright (C) 2011-2020 Pulse-Eight Limited.  All rights reserved.
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
*
* Author: Lars Op den Kamp <lars@opdenkamp.eu>
*
*/

#include "CecSharpTypes.h"
#using <System.dll>

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace CEC;
using namespace msclr::interop;

namespace CecSharp
{
  /// <summary>
  /// Create a LibCecSharp instance and pass the configuration as argument.
  /// Then call Open() to open a connection to the adapter. Close() closes the
  /// connection.
  ///
  /// libCEC can send commands to other devices on the CEC bus via the methods
  /// on this interface, and all commands that libCEC received are sent back
  /// to the application via callback methods. The callback methods can be
  /// found in CecSharpTypes.h, CecCallbackMethods.
  /// </summary>
  public ref class LibCecSharp
  {
  public:
    /// <summary>
    /// Create a new LibCecSharp instance.
    /// </summary>
    /// <param name="config">The configuration to pass to libCEC.</param>
    LibCecSharp(CecCallbackMethods^ callbacks, LibCECConfiguration^ config)
    {
      marshal_context^ context = gcnew marshal_context();
      libcec_configuration libCecConfig;
      m_callbacks = callbacks;
      ConvertConfiguration(context, config, libCecConfig);
      m_libCec = (ICECAdapter*)CECInitialise(&libCecConfig);
      if (!m_libCec)
        throw gcnew Exception("Could not initialise LibCecSharp");

      config->Update(libCecConfig);
      delete context;
    }

    ~LibCecSharp(void)
    {
      Destroy();
    }

    /// <summary>
    /// Try to find all connected CEC adapters.
    /// </summary>
    /// <param name="path">The path filter for adapters. Leave empty to return all adapters.</param>
    /// <returns>The adapters that were found.</returns>
    array<CecAdapter ^> ^ FindAdapters(String ^ path)
    {
      cec_adapter_descriptor *devices = new cec_adapter_descriptor[10];

      marshal_context ^ context = gcnew marshal_context();
      const char* strPathC = path->Length > 0 ? context->marshal_as<const char*>(path) : NULL;

      uint8_t iDevicesFound = m_libCec->DetectAdapters(devices, 10, NULL, false);

      array<CecAdapter ^> ^ adapters = gcnew array<CecAdapter ^>(iDevicesFound);
      for (unsigned int iPtr = 0; iPtr < iDevicesFound; iPtr++)
        adapters[iPtr] = gcnew CecAdapter(gcnew String(devices[iPtr].strComPath), gcnew String(devices[iPtr].strComName), devices[iPtr].iVendorId, devices[iPtr].iProductId, devices[iPtr].iFirmwareVersion, devices[iPtr].iFirmwareBuildDate, devices[iPtr].iPhysicalAddress);

      delete devices;
      delete context;
      return adapters;
    }

    /// <summary>
    /// Open a connection to the CEC adapter.
    /// </summary>
    /// <param name="strPort">The COM port of the adapter</param>
    /// <param name="iTimeoutMs">Connection timeout in milliseconds</param>
    /// <returns>True when a connection was opened, false otherwise.</returns>
    bool Open(String ^ strPort, int iTimeoutMs)
    {
      if (!m_libCec)
        return false;
      marshal_context ^ context = gcnew marshal_context();
      const char* strPortC = context->marshal_as<const char*>(strPort);
      bool bReturn = m_libCec->Open(strPortC, iTimeoutMs);
      delete context;
      return bReturn;
    }

    /// <summary>
    /// Close the connection to the CEC adapter
    /// </summary>
    void Close(void)
    {
      if (!!m_libCec)
        m_libCec->Close();
    }

    void EnableCallbacks(void)
    {
      if (!!m_libCec)
      {
#if CEC_LIB_VERSION_MAJOR >= 5
        m_libCec->SetCallbacks(GetLibCecCallbacks(), m_callbacks->Get());
#else
        m_libCec->EnableCallbacks(m_callbacks->Get(), GetLibCecCallbacks());
#endif
      }
    }

    void DisableCallbacks(void)
    {
      if (!!m_libCec)
      {
#if CEC_LIB_VERSION_MAJOR >= 5
        m_libCec->DisableCallbacks();
#else
        m_libCec->EnableCallbacks(nullptr, nullptr);
#endif
      }
    }

    /// <summary>
    /// Sends a ping command to the adapter, to check if it's responding.
    /// </summary>
    /// <returns>True when the ping was successful, false otherwise</returns>
    bool PingAdapter(void)
    {
      return !!m_libCec && m_libCec->PingAdapter();
    }

    /// <summary>
    /// Start the bootloader of the CEC adapter. Closes the connection when successful.
    /// </summary>
    /// <returns>True when the command was sent successfully, false otherwise.</returns>
    bool StartBootloader(void)
    {
      return !!m_libCec && m_libCec->StartBootloader();
    }

    /// <summary>
    /// Transmit a raw CEC command over the CEC line.
    /// </summary>
    /// <param name="command">The command to transmit</param>
    /// <returns>True when the data was sent and acked, false otherwise.</returns>
    bool Transmit(CecCommand ^ command)
    {
      if (!m_libCec) {
        return false;
      }
      cec_command ccommand;
      cec_command::Format(ccommand, (cec_logical_address)command->Initiator, (cec_logical_address)command->Destination, (cec_opcode)command->Opcode);
      ccommand.transmit_timeout = command->TransmitTimeout;
      ccommand.eom              = command->Eom;
      ccommand.ack              = command->Ack;
      for (unsigned int iPtr = 0; iPtr < command->Parameters->Size; iPtr++)
        ccommand.parameters.PushBack(command->Parameters->Data[iPtr]);

      return m_libCec->Transmit(ccommand);
    }

    /// <summary>
    /// Change the logical address on the CEC bus of the CEC adapter. libCEC automatically assigns a logical address, and this method is only available for debugging purposes.
    /// </summary>
    /// <param name="logicalAddress">The CEC adapter's new logical address.</param>
    /// <returns>True when the logical address was set successfully, false otherwise.</returns>
    bool SetLogicalAddress(CecLogicalAddress logicalAddress)
    {
      return !!m_libCec && m_libCec->SetLogicalAddress((cec_logical_address) logicalAddress);
    }

    /// <summary>
    /// Change the physical address (HDMI port) of the CEC adapter. libCEC will try to autodetect the physical address when connecting. If it did, it's set in libcec_configuration.
    /// </summary>
    /// <param name="physicalAddress">The CEC adapter's new physical address.</param>
    /// <returns>True when the physical address was set successfully, false otherwise.</returns>
    bool SetPhysicalAddress(uint16_t physicalAddress)
    {
      return !!m_libCec && m_libCec->SetPhysicalAddress(physicalAddress);
    }

    /// <summary>
    /// Power on the given CEC capable devices. If CECDEVICE_BROADCAST is used, then wakeDevice in libcec_configuration will be used.
    /// </summary>
    /// <param name="logicalAddress">The logical address to power on.</param>
    /// <returns>True when the command was sent successfully, false otherwise.</returns>
    bool PowerOnDevices(CecLogicalAddress logicalAddress)
    {
      return !!m_libCec && m_libCec->PowerOnDevices((cec_logical_address) logicalAddress);
    }

    /// <summary>
    /// Put the given CEC capable devices in standby mode. If CECDEVICE_BROADCAST is used, then standbyDevices in libcec_configuration will be used.
    /// </summary>
    /// <param name="logicalAddress">The logical address of the device to put in standby.</param>
    /// <returns>True when the command was sent successfully, false otherwise.</returns>
    bool StandbyDevices(CecLogicalAddress logicalAddress)
    {
      return !!m_libCec && m_libCec->StandbyDevices((cec_logical_address) logicalAddress);
    }

    /// <summary>
    /// Sends a POLL message to a device, to check if it's present and responding.
    /// </summary>
    /// <param name="logicalAddress">The device to send the message to.</param>
    /// <returns>True if the POLL was acked, false otherwise.</returns>
    bool PollDevice(CecLogicalAddress logicalAddress)
    {
      return !!m_libCec && m_libCec->PollDevice((cec_logical_address) logicalAddress);
    }

    /// <summary>
    /// Change the active source to a device type handled by libCEC. Use CEC_DEVICE_TYPE_RESERVED to make the default type used by libCEC active.
    /// </summary>
    /// <param name="type">The new active source. Use CEC_DEVICE_TYPE_RESERVED to use the primary type</param>
    /// <returns>True when the command was sent successfully, false otherwise.</returns>
    bool SetActiveSource(CecDeviceType type)
    {
      return !!m_libCec && m_libCec->SetActiveSource((cec_device_type) type);
    }

    /// <summary>
    /// Change the deck control mode, if this adapter is registered as playback or recording device.
    /// </summary>
    /// <param name="mode">The new control mode.</param>
    /// <param name="sendUpdate">True to send the new status over the CEC line.</param>
    /// <returns>True if set, false otherwise.</returns>
    bool SetDeckControlMode(CecDeckControlMode mode, bool sendUpdate)
    {
      return !!m_libCec && m_libCec->SetDeckControlMode((cec_deck_control_mode) mode, sendUpdate);
    }

    /// <summary>
    /// Change the deck info, if this adapter is a playback or recording device.
    /// </summary>
    /// <param name="info">The new deck info.</param>
    /// <param name="sendUpdate">True to send the new status over the CEC line.</param>
    /// <returns>True if set, false otherwise.</returns>
    bool SetDeckInfo(CecDeckInfo info, bool sendUpdate)
    {
      return !!m_libCec && m_libCec->SetDeckInfo((cec_deck_info) info, sendUpdate);
    }

    /// <summary>
    /// Broadcast a message that notifies connected CEC capable devices that this device is no longer the active source.
    /// </summary>
    /// <returns>True when the command was sent successfully, false otherwise.</returns>
    bool SetInactiveView(void)
    {
      return !!m_libCec && m_libCec->SetInactiveView();
    }

    /// <summary>
    /// Change the menu state. This value is already changed by libCEC automatically if a device is (de)activated.
    /// </summary>
    /// <param name="state">The new state.</param>
    /// <param name="sendUpdate">True to send the new status over the CEC line.</param>
    /// <returns>True if set, false otherwise.</returns>
    bool SetMenuState(CecMenuState state, bool sendUpdate)
    {
      return !!m_libCec && m_libCec->SetMenuState((cec_menu_state) state, sendUpdate);
    }

    /// <summary>
    /// Display a message on the device with the given logical address. Not supported by most TVs.
    /// </summary>
    /// <param name="logicalAddress">The logical address of the device to display the message on.</param>
    /// <param name="duration">The duration of the message</param>
    /// <param name="message">The message to display.</param>
    /// <returns>True when the command was sent, false otherwise.</returns>
    bool SetOSDString(CecLogicalAddress logicalAddress, CecDisplayControl duration, String ^ message)
    {
      if (!m_libCec) {
        return false;
      }
      marshal_context ^ context = gcnew marshal_context();
      const char* strMessageC = context->marshal_as<const char*>(message);

      bool bReturn = m_libCec->SetOSDString((cec_logical_address) logicalAddress, (cec_display_control) duration, strMessageC);

      delete context;
      return bReturn;
    }

    /// <summary>
    /// Enable or disable monitoring mode, for debugging purposes. If monitoring mode is enabled, libCEC won't respond to any command, but only log incoming data.
    /// </summary>
    /// <param name="enable">True to enable, false to disable.</param>
    /// <returns>True when switched successfully, false otherwise.</returns>
    bool SwitchMonitoring(bool enable)
    {
      return !!m_libCec && m_libCec->SwitchMonitoring(enable);
    }

    /// <summary>
    /// Enable or disable taw traffic mode. Best used with monitoring mode. If raw traffic is enabled, libCEC will report poll traffic.
    /// </summary>
    /// <param name="enable">True to enable, false to disable.</param>
    /// <returns>True when switched successfully, false otherwise.</returns>
    bool SwitchRawTraffic(bool enable)
    {
      return !!m_libCec && m_libCec->SwitchRawTraffic(enable);
    }

    /// <summary>
    /// Get the CEC version of the device with the given logical address
    /// </summary>
    /// <param name="logicalAddress">The logical address of the device to get the CEC version for.</param>
    /// <returns>The version or CEC_VERSION_UNKNOWN when the version couldn't be fetched.</returns>
    CecVersion GetDeviceCecVersion(CecLogicalAddress logicalAddress)
    {
      if (!m_libCec) {
        return CecVersion::Unknown;
      }
      return (CecVersion) m_libCec->GetDeviceCecVersion((cec_logical_address) logicalAddress);
    }

    /// <summary>
    /// Get the menu language of the device with the given logical address
    /// </summary>
    /// <param name="logicalAddress">The logical address of the device to get the menu language for.</param>
    /// <returns>The requested menu language.</returns>
    String ^ GetDeviceMenuLanguage(CecLogicalAddress logicalAddress)
    {
      if (!m_libCec) {
        return gcnew String("not connected");
      }
	    std::string strLang = m_libCec->GetDeviceMenuLanguage((cec_logical_address)logicalAddress);
      return gcnew String(strLang.c_str());
    }

    /// <summary>
    /// Get the vendor ID of the device with the given logical address.
    /// </summary>
    /// <param name="logicalAddress">The logical address of the device to get the vendor ID for.</param>
    /// <returns>The vendor ID or 0 if it wasn't found.</returns>
    CecVendorId GetDeviceVendorId(CecLogicalAddress logicalAddress)
    {
      if (!m_libCec) {
        return CecVendorId::Unknown;
      }
      return (CecVendorId)m_libCec->GetDeviceVendorId((cec_logical_address) logicalAddress);
    }

    /// <summary>
    /// Get the power status of the device with the given logical address.
    /// </summary>
    /// <param name="logicalAddress">The logical address of the device to get the power status for.</param>
    /// <returns>The power status or CEC_POWER_STATUS_UNKNOWN if it wasn't found.</returns>
    CecPowerStatus GetDevicePowerStatus(CecLogicalAddress logicalAddress)
    {
      if (!m_libCec) {
        return CecPowerStatus::Unknown;
      }
      return (CecPowerStatus) m_libCec->GetDevicePowerStatus((cec_logical_address) logicalAddress);
    }

    /// <summary>
    /// Tell libCEC to poll for active devices on the bus.
    /// </summary>
    void RescanActiveDevices(void)
    {
      if (!!m_libCec) {
        m_libCec->RescanActiveDevices();
      }
    }

    /// <summary>
    /// Get the logical addresses of the devices that are active on the bus, including those handled by libCEC.
    /// </summary>
    /// <returns>The logical addresses of the active devices</returns>
    CecLogicalAddresses ^ GetActiveDevices(void)
    {
      CecLogicalAddresses ^ retVal = gcnew CecLogicalAddresses();
      if (!m_libCec) {
        return retVal;
      }
      unsigned int iDevices = 0;

      cec_logical_addresses activeDevices = m_libCec->GetActiveDevices();

      for (uint8_t iPtr = 0; iPtr < 16; iPtr++)
        if (activeDevices[iPtr])
          retVal->Set((CecLogicalAddress)iPtr);

      return retVal;
    }

    /// <summary>
    /// Check whether a device is active on the bus.
    /// </summary>
    /// <param name="logicalAddress">The address to check.</param>
    /// <returns>True when active, false otherwise.</returns>
    bool IsActiveDevice(CecLogicalAddress logicalAddress)
    {
      return !!m_libCec && m_libCec->IsActiveDevice((cec_logical_address)logicalAddress);
    }

    /// <summary>
    /// Check whether a device of the given type is active on the bus.
    /// </summary>
    /// <param name="type">The type to check.</param>
    /// <returns>True when active, false otherwise.</returns>
    bool IsActiveDeviceType(CecDeviceType type)
    {
      return !!m_libCec && m_libCec->IsActiveDeviceType((cec_device_type)type);
    }

    /// <summary>
    /// Changes the active HDMI port.
    /// </summary>
    /// <param name="address">The device to which this libCEC is connected.</param>
    /// <param name="port">The new port number.</param>
    /// <returns>True when changed, false otherwise.</returns>
    bool SetHDMIPort(CecLogicalAddress address, uint8_t port)
    {
      return !!m_libCec && m_libCec->SetHDMIPort((cec_logical_address)address, port);
    }

    /// <summary>
    /// Sends a volume up keypress to an audiosystem if it's present.
    /// </summary>
    /// <param name="sendRelease">Send a key release after the keypress.</param>
    /// <returns>The new audio status.</returns>
    uint8_t VolumeUp(bool sendRelease)
    {
      if (!m_libCec) {
        return 0;
      }
      return m_libCec->VolumeUp(sendRelease);
    }

    /// <summary>
    /// Sends a volume down keypress to an audiosystem if it's present.
    /// </summary>
    /// <param name="sendRelease">Send a key release after the keypress.</param>
    /// <returns>The new audio status.</returns>
    uint8_t VolumeDown(bool sendRelease)
    {
      if (!m_libCec) {
        return 0;
      }
      return m_libCec->VolumeDown(sendRelease);
    }

    /// <summary>
    /// Sends a mute keypress to an audiosystem if it's present.
    /// </summary>
    /// <returns>The new audio status.</returns>
    uint8_t MuteAudio()
    {
      if (!m_libCec) {
        return 0;
      }
      return m_libCec->AudioToggleMute();
    }

    /// <summary>
    /// Send a keypress to a device on the CEC bus.
    /// </summary>
    /// <param name="destination">The logical address of the device to send the message to.</param>
    /// <param name="key">The key to send.</param>
    /// <param name="wait">True to wait for a response, false otherwise.</param>
    /// <returns>True when the keypress was acked, false otherwise.</returns>
    bool SendKeypress(CecLogicalAddress destination, CecUserControlCode key, bool wait)
    {
      return !!m_libCec && m_libCec->SendKeypress((cec_logical_address)destination, (cec_user_control_code)key, wait);
    }

    /// <summary>
    /// Send a key release to a device on the CEC bus.
    /// </summary>
    /// <param name="destination">The logical address of the device to send the message to.</param>
    /// <param name="wait">True to wait for a response, false otherwise.</param>
    /// <returns>True when the key release was acked, false otherwise.</returns>
    bool SendKeyRelease(CecLogicalAddress destination, bool wait)
    {
      return !!m_libCec && m_libCec->SendKeyRelease((cec_logical_address)destination, wait);
    }

    /// <summary>
    /// Get the OSD name of a device on the CEC bus.
    /// </summary>
    /// <param name="logicalAddress">The logical address of the device to get the OSD name for.</param>
    /// <returns>The OSD name.</returns>
    String ^ GetDeviceOSDName(CecLogicalAddress logicalAddress)
    {
      if (!m_libCec) {
        return gcnew String("disconnected");
      }
      std::string osdName = m_libCec->GetDeviceOSDName((cec_logical_address) logicalAddress);
      // we need to terminate with \0, and we only got 14 chars in osd.name
      char strOsdName[15];
      strncpy_s(strOsdName, osdName.c_str(), 15);
      return gcnew String(strOsdName);
    }

    /// <summary>
    /// Get the logical address of the device that is currently the active source on the CEC bus.
    /// </summary>
    /// <returns>The active source or CECDEVICE_UNKNOWN when unknown.</returns>
    CecLogicalAddress GetActiveSource()
    {
      if (!m_libCec) {
        return CecLogicalAddress::Unknown;
      }
      return (CecLogicalAddress)m_libCec->GetActiveSource();
    }

    /// <summary>
    /// Check whether a device is currently the active source on the CEC bus.
    /// </summary>
    /// <param name="logicalAddress">The logical address of the device to check.</param>
    /// <returns>True when it is the active source, false otherwise.</returns>
    bool IsActiveSource(CecLogicalAddress logicalAddress)
    {
      return !!m_libCec && m_libCec->IsActiveSource((cec_logical_address)logicalAddress);
    }

    /// <summary>
    /// Get the physical address of the device with the given logical address.
    /// </summary>
    /// <param name="address">The logical address of the device to get the physical address for.</param>
    /// <returns>The physical address or 0 if it wasn't found.</returns>
    uint16_t GetDevicePhysicalAddress(CecLogicalAddress address)
    {
      if (!m_libCec) {
        return 0xFFFF;
      }
      return m_libCec->GetDevicePhysicalAddress((cec_logical_address)address);
    }

    /// <summary>
    /// Sets the stream path to the device on the given logical address.
    /// </summary>
    /// <param name="address">The address to activate.</param>
    /// <returns>True when the command was sent, false otherwise.</returns>
    bool SetStreamPath(CecLogicalAddress address)
    {
      return !!m_libCec && m_libCec->SetStreamPath((cec_logical_address)address);
    }

    /// <summary>
    /// Sets the stream path to the device on the given physical address.
    /// </summary>
    /// <param name="physicalAddress">The address to activate.</param>
    /// <returns>True when the command was sent, false otherwise.</returns>
    bool SetStreamPath(uint16_t physicalAddress)
    {
      return !!m_libCec && m_libCec->SetStreamPath(physicalAddress);
    }

    /// <summary>
    /// Get the list of logical addresses that libCEC is controlling
    /// </summary>
    /// <returns>The list of logical addresses that libCEC is controlling</returns>
    CecLogicalAddresses ^GetLogicalAddresses(void)
    {
      CecLogicalAddresses ^addr = gcnew CecLogicalAddresses();
      if (!m_libCec) {
        return addr;
      }
      cec_logical_addresses libAddr = m_libCec->GetLogicalAddresses();
      for (unsigned int iPtr = 0; iPtr < 16; iPtr++)
        addr->Addresses[iPtr] = (CecLogicalAddress)libAddr.addresses[iPtr];
      addr->Primary = (CecLogicalAddress)libAddr.primary;
      return addr;
    }

    /// <summary>
    /// Get libCEC's current configuration.
    /// </summary>
    /// <param name="configuration">The configuration.</param>
    /// <returns>True when the configuration was updated, false otherwise.</returns>
    bool GetCurrentConfiguration(LibCECConfiguration ^configuration)
    {
      if (!m_libCec) {
        return false;
      }
      libcec_configuration config;
      config.Clear();

      if (m_libCec->GetCurrentConfiguration(&config))
      {
        configuration->Update(config);
        return true;
      }
      return false;
    }

    /// <summary>
    /// Check whether the CEC adapter can save a configuration.
    /// </summary>
    /// <returns>True when this CEC adapter can save the user configuration, false otherwise.</returns>
#if CEC_LIB_VERSION_MAJOR >= 5
    bool CanSaveConfiguration(void)
    {
      return !!m_libCec &&
        m_libCec->CanSaveConfiguration();
    }
#else
    bool CanPersistConfiguration(void)
    {
      return !!m_libCec &&
        m_libCec->CanPersistConfiguration();
    }

    bool PersistConfiguration(LibCECConfiguration ^configuration)
    {
      return SetConfiguration(configuration);
    }
#endif

    /// <summary>
    /// Change libCEC's configuration. Store it updated settings in the eeprom of the device (if supported)
    /// </summary>
    /// <param name="configuration">The new configuration.</param>
    /// <returns>True when the configuration was changed successfully, false otherwise.</returns>
    bool SetConfiguration(LibCECConfiguration ^configuration)
    {
      if (!m_libCec) {
        return false;
      }
      marshal_context ^ context = gcnew marshal_context();
      libcec_configuration config;
      ConvertConfiguration(context, configuration, config);
      // don't update callbacks
      config.callbacks = NULL;

      bool bReturn = m_libCec->SetConfiguration(&config);

      delete context;
      return bReturn;
    }

    /// <summary>
    /// Check whether libCEC is the active source on the bus.
    /// </summary>
    /// <returns>True when libCEC is the active source on the bus, false otherwise.</returns>
    bool IsLibCECActiveSource()
    {
      return !!m_libCec && m_libCec->IsLibCECActiveSource();
    }

    /// <summary>
    /// Get information about the given CEC adapter.
    /// </summary>
    /// <param name="port">The COM port to which the device is connected</param>
    /// <param name="configuration">The device configuration</param>
    /// <param name="timeoutMs">The timeout in milliseconds</param>
    /// <returns>True when the device was found, false otherwise</returns>
    bool GetDeviceInformation(String ^ port, LibCECConfiguration ^configuration, uint32_t timeoutMs)
    {
      if (!m_libCec) {
        return false;
      }
      bool bReturn(false);
      marshal_context ^ context = gcnew marshal_context();

      libcec_configuration config;
      config.Clear();

      const char* strPortC = port->Length > 0 ? context->marshal_as<const char*>(port) : NULL;

      if (m_libCec->GetDeviceInformation(strPortC, &config, timeoutMs))
      {
        configuration->Update(config);
        bReturn = true;
      }

      delete context;
      return bReturn;
    }

    String ^ ToString(CecLogicalAddress iAddress)
    {
      if (!m_libCec) {
        return gcnew String("disconnected");
      }
      const char *retVal = m_libCec->ToString((cec_logical_address)iAddress);
      return gcnew String(retVal);
    }

    String ^ ToString(CecVendorId iVendorId)
    {
      if (!m_libCec) {
        return gcnew String("disconnected");
      }
      const char *retVal = m_libCec->ToString((cec_vendor_id)iVendorId);
      return gcnew String(retVal);
    }

    String ^ ToString(CecVersion iVersion)
    {
      if (!m_libCec) {
        return gcnew String("disconnected");
      }
      const char *retVal = m_libCec->ToString((cec_version)iVersion);
      return gcnew String(retVal);
    }

    String ^ ToString(CecPowerStatus iState)
    {
      if (!m_libCec) {
        return gcnew String("disconnected");
      }
      const char *retVal = m_libCec->ToString((cec_power_status)iState);
      return gcnew String(retVal);
    }

    String ^ ToString(CecMenuState iState)
    {
      if (!m_libCec) {
        return gcnew String("disconnected");
      }
      const char *retVal = m_libCec->ToString((cec_menu_state)iState);
      return gcnew String(retVal);
    }

    String ^ ToString(CecDeckControlMode iMode)
    {
      if (!m_libCec) {
        return gcnew String("disconnected");
      }
      const char *retVal = m_libCec->ToString((cec_deck_control_mode)iMode);
      return gcnew String(retVal);
    }

    String ^ ToString(CecDeckInfo status)
    {
      if (!m_libCec) {
        return gcnew String("disconnected");
      }
      const char *retVal = m_libCec->ToString((cec_deck_info)status);
      return gcnew String(retVal);
    }

    String ^ ToString(CecOpcode opcode)
    {
      if (!m_libCec) {
        return gcnew String("disconnected");
      }
      const char *retVal = m_libCec->ToString((cec_opcode)opcode);
      return gcnew String(retVal);
    }

    String ^ ToString(CecSystemAudioStatus mode)
    {
      if (!m_libCec) {
        return gcnew String("disconnected");
      }
      const char *retVal = m_libCec->ToString((cec_system_audio_status)mode);
      return gcnew String(retVal);
    }

    String ^ ToString(CecAudioStatus status)
    {
      if (!m_libCec) {
        return gcnew String("disconnected");
      }
      const char *retVal = m_libCec->ToString((cec_audio_status)status);
      return gcnew String(retVal);
    }

    String ^ VersionToString(uint32_t version)
    {
      if (!m_libCec) {
        return gcnew String("disconnected");
      }
      char buf[20];
      m_libCec->PrintVersion(version, buf, 20);
      return gcnew String(buf);
    }

    String^ PhysicalAddressToString(uint16_t physicalAddress)
    {
      if (!m_libCec) {
        return gcnew String("disconnected");
      }
      char buf[8];
      _snprintf_s(buf, 8, "%X.%X.%X.%X", (physicalAddress >> 12) & 0xF, (physicalAddress >> 8) & 0xF, (physicalAddress >> 4) & 0xF, physicalAddress & 0xF);
      return gcnew String(buf);
    }

    /// <summary>
    /// Get a string with information about how libCEC was compiled.
    /// </summary>
    /// <returns>A string with information about how libCEC was compiled.</returns>
    String ^ GetLibInfo()
    {
      if (!m_libCec) {
        return gcnew String("disconnected");
      }
      const char *retVal = m_libCec->GetLibInfo();
      return gcnew String(retVal);
    }

    /// <summary>
    /// Calling this method will initialise the host on which libCEC is running.
    /// On the RPi, it calls bcm_host_init(), which may only be called once per process, and is called by any process using
    /// the video api on that system. So only call this method if libCEC is used in an application that
    /// does not already initialise the video api.
    /// </summary>
    /// <remarks>Should be called as first call to libCEC, directly after CECInitialise() and before using Open()</remarks>
    void InitVideoStandalone()
    {
      if (!!m_libCec) {
        m_libCec->InitVideoStandalone();
      }
    }

#if CEC_LIB_VERSION_MAJOR >= 5
    /// <summary>
    /// Get statistics about traffic on the CEC line
    /// </summary>
    /// <returns>The requested statistics</returns>
    CecAdapterStats ^GetStats(void)
    {
      struct cec_adapter_stats cstats;
      return gcnew CecAdapterStats((!!m_libCec && m_libCec->GetStats(&cstats)) ? &cstats : NULL);
    }
#endif

    /// <summary>
    /// Get the (virtual) USB vendor id
    /// </summary>
    /// <returns>The (virtual) USB vendor id</returns>
    uint16_t GetAdapterVendorId()
    {
      if (!m_libCec) {
        return 0;
      }
      return m_libCec->GetAdapterVendorId();
    }

    /// <summary>
    /// Get the (virtual) USB product id
    /// </summary>
    /// <returns>The (virtual) USB product id</returns>
    uint16_t GetAdapterProductId()
    {
      if (!m_libCec) {
        return 0;
      }
      return m_libCec->GetAdapterProductId();
    }

  protected:
    !LibCecSharp(void)
    {
      Destroy();
    }

  private:
    void Destroy(void)
    {
      Close();
      m_callbacks->Destroy();
      m_libCec = NULL;
    }

    void ConvertConfiguration(marshal_context ^context, LibCECConfiguration ^netConfig, CEC::libcec_configuration &config)
    {
      config.Clear();

      const char *strDeviceName = context->marshal_as<const char*>(netConfig->DeviceName);
      memcpy_s(config.strDeviceName, 13, strDeviceName, 13);
      for (unsigned int iPtr = 0; iPtr < 5; iPtr++)
        config.deviceTypes.types[iPtr] = (cec_device_type)netConfig->DeviceTypes->Types[iPtr];

      config.bAutodetectAddress    = netConfig->AutodetectAddress ? 1 : 0;
      config.iPhysicalAddress      = netConfig->PhysicalAddress;
      config.baseDevice            = (cec_logical_address)netConfig->BaseDevice;
      config.iHDMIPort             = netConfig->HDMIPort;
      config.clientVersion         = netConfig->ClientVersion;
      config.bGetSettingsFromROM   = netConfig->GetSettingsFromROM ? 1 : 0;
      config.bActivateSource       = netConfig->ActivateSource ? 1 : 0;
      config.tvVendor              = (cec_vendor_id)netConfig->TvVendor;
      config.comboKey              = (cec_user_control_code)netConfig->ComboKey;
      config.iComboKeyTimeoutMs    = netConfig->ComboKeyTimeoutMs;
      config.iButtonRepeatRateMs   = netConfig->ButtonRepeatRateMs;
      config.iButtonReleaseDelayMs = netConfig->ButtonReleaseDelayMs;
      config.iDoubleTapTimeoutMs   = netConfig->DoubleTapTimeoutMs;
      config.bAutoWakeAVR          = netConfig->AutoWakeAVR ? 1 : 0;
#if CEC_LIB_VERSION_MAJOR >= 5
      config.bAutoPowerOn          = (uint8_t)netConfig->AutoPowerOn;
#endif

      config.wakeDevices.Clear();
      for (int iPtr = 0; iPtr < 16; iPtr++)
      {
        if (netConfig->WakeDevices->IsSet((CecLogicalAddress)iPtr))
          config.wakeDevices.Set((cec_logical_address)iPtr);
      }
      config.powerOffDevices.Clear();
      for (int iPtr = 0; iPtr < 16; iPtr++)
      {
        if (netConfig->PowerOffDevices->IsSet((CecLogicalAddress)iPtr))
          config.powerOffDevices.Set((cec_logical_address)iPtr);
      }
      config.bPowerOffOnStandby   = netConfig->PowerOffOnStandby ? 1 : 0;
      const char *strDeviceLanguage = context->marshal_as<const char*>(netConfig->DeviceLanguage);
      memcpy_s(config.strDeviceLanguage, 3, strDeviceLanguage, 3);
      config.bMonitorOnly = netConfig->MonitorOnlyClient ? 1 : 0;
      config.bRawTraffic = netConfig->RawTraffic ? 1 : 0;
      config.cecVersion = (cec_version)netConfig->CECVersion;

      config.callbacks = GetLibCecCallbacks();
      config.callbackParam = m_callbacks->Get();
    }

    ICECAdapter*         m_libCec;
    CecCallbackMethods ^ m_callbacks;
  };
}
