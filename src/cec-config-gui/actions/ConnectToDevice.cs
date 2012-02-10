using System;
using System.Collections.Generic;
using System.Text;
using CecSharp;
using System.Windows.Forms;
using System.IO;
using System.Xml;
using System.Globalization;

namespace CecConfigGui.actions
{
  class ConnectToDevice : UpdateProcess
  {
    public ConnectToDevice(ref LibCecSharp lib)
    {
      Lib = lib;
    }

    public override void Process()
    {
      SendEvent(UpdateEventType.StatusText, "Connecting to the CEC adapter...");
      SendEvent(UpdateEventType.ProgressBar, 0);

      CecAdapter[] adapters = Lib.FindAdapters(string.Empty);
      if (adapters.Length == 0 || !Lib.Open(adapters[0].ComPort, 10000))
      {
        MessageBox.Show("Could not connect to any CEC adapter. Please check your configuration.", "Pulse-Eight USB-CEC Adapter", MessageBoxButtons.OK);
        Application.Exit();
      }

      SendEvent(UpdateEventType.StatusText, "Detecting TV vendor...");
      SendEvent(UpdateEventType.ProgressBar, 25);
      SendEvent(UpdateEventType.TVVendorId, (int)Lib.GetDeviceVendorId(CecLogicalAddress.Tv));

      SendEvent(UpdateEventType.StatusText, "Detecting menu language...");
      SendEvent(UpdateEventType.ProgressBar, 40);
      SendEvent(UpdateEventType.MenuLanguage, Lib.GetDeviceMenuLanguage(CecLogicalAddress.Tv));

      SendEvent(UpdateEventType.ProgressBar, 50);
      SendEvent(UpdateEventType.StatusText, "Detecting AVR devices...");

      bool hasAVRDevice = Lib.IsActiveDevice(CecLogicalAddress.AudioSystem);
      SendEvent(UpdateEventType.HasAVRDevice, hasAVRDevice);

      if (hasAVRDevice)
      {
        SendEvent(UpdateEventType.ProgressBar, 75);
        SendEvent(UpdateEventType.StatusText, "Detecting AVR vendor...");
        SendEvent(UpdateEventType.AVRVendorId, (int)Lib.GetDeviceVendorId(CecLogicalAddress.AudioSystem));
      }

      if (!Lib.GetDevicePowerStatus(CecLogicalAddress.Tv).Equals(CecPowerStatus.On))
      {
        SendEvent(UpdateEventType.ProgressBar, 80);
        SendEvent(UpdateEventType.StatusText, "Sending power on command...");
        Lib.PowerOnDevices(CecLogicalAddress.Tv);
      }

      SendEvent(UpdateEventType.ProgressBar, 90);
      SendEvent(UpdateEventType.StatusText, "Reading device configuration...");

      LibCECConfiguration config = new LibCECConfiguration();

      if (!Lib.CanPersistConfiguration())
      {
        bool gotConfig = false;
        string xbmcDir = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData) + @"\XBMC\userdata\peripheral_data";
        string defaultDir = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
        string file = defaultDir + @"\usb_2548_1001.xml";
        if (File.Exists(xbmcDir + @"\usb_2548_1001.xml"))
          file = xbmcDir + @"\usb_2548_1001.xml";

        if (File.Exists(file))
        {
          XmlTextReader reader = new XmlTextReader(file);
          while (reader.Read())
          {
            gotConfig = true;
            switch (reader.NodeType)
            {
              case XmlNodeType.Element:
                if (reader.Name.ToLower() == "setting")
                {
                  string name = string.Empty;
                  string value = string.Empty;

                  while (reader.MoveToNextAttribute())
                  {
                    if (reader.Name.ToLower().Equals("id"))
                      name = reader.Value.ToLower();
                    if (reader.Name.ToLower().Equals("value"))
                      value = reader.Value;
                  }

                  switch (name)
                  {
                    case "cec_hdmi_port":
                      {
                        byte iPort;
                        if (byte.TryParse(value, out iPort))
                          config.HDMIPort = iPort;
                      }
                      break;
                    case "connected_device":
                      {
                        ushort iDevice;
                        if (ushort.TryParse(value, out iDevice))
                          config.BaseDevice = (CecLogicalAddress)iDevice;
                      }
                      break;
                    case "physical_address":
                      {
                        ushort physicalAddress = 0;
                        if (ushort.TryParse(value, NumberStyles.AllowHexSpecifier, null, out physicalAddress))
                          config.PhysicalAddress = physicalAddress;
                      }
                      break;
                    case "device_type":
                      {
                        ushort iType;
                        if (ushort.TryParse(value, out iType))
                          config.DeviceTypes.Types[0] = (CecDeviceType)iType;
                      }
                      break;
                    case "cec_power_on_startup":
                      config.PowerOnStartup = value.Equals("1") || value.ToLower().Equals("true") || value.ToLower().Equals("yes");
                      break;
                    case "cec_power_off_shutdown":
                      config.PowerOffShutdown = value.Equals("1") || value.ToLower().Equals("true") || value.ToLower().Equals("yes");
                      break;
                    case "cec_standby_screensaver":
                      config.PowerOffScreensaver = value.Equals("1") || value.ToLower().Equals("true") || value.ToLower().Equals("yes");
                      break;
                    case "standby_pc_on_tv_standby":
                      config.PowerOffOnStandby = value.Equals("1") || value.ToLower().Equals("true") || value.ToLower().Equals("yes");
                      break;
                    case "use_tv_menu_language":
                      config.UseTVMenuLanguage = value.Equals("1") || value.ToLower().Equals("true") || value.ToLower().Equals("yes");
                      break;
                    case "enabled":
                      break;
                    case "port":
                      break;
                    default:
                      break;
                  }
                }
                break;
              default:
                break;
            }
          }
        }

        if (!gotConfig)
          Lib.GetCurrentConfiguration(config);
      }
      else
      {
        Lib.GetCurrentConfiguration(config);
      }
      SendEvent(config);

      SendEvent(UpdateEventType.ProgressBar, 100);
      SendEvent(UpdateEventType.StatusText, "Ready.");
    }

    private LibCecSharp Lib;
  }
}
