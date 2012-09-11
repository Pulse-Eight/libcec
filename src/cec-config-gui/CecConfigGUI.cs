using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using CecSharp;
using CecConfigGui.actions;
using System.Globalization;
using System.IO;
using System.Xml;

namespace CecConfigGui
{
  internal enum ConfigTab
  {
    Configuration,
    KeyConfiguration,
    Tester,
    Log
  }

  public partial class CecConfigGUI : AsyncForm
  {
    public CecConfigGUI()
    {
      Config = new LibCECConfiguration();
      Config.DeviceTypes.Types[0] = CecDeviceType.RecordingDevice;
      Config.DeviceName = "CEC Config";
      Config.GetSettingsFromROM = true;
      Config.ClientVersion = CecClientVersion.Version1_9_0;
      Callbacks = new CecCallbackWrapper(this);
      Config.SetCallbacks(Callbacks);
      LoadXMLConfiguration(ref Config);
      Lib = new LibCecSharp(Config);
      Lib.InitVideoStandalone();

      InitializeComponent();
      LoadButtonConfiguration();

      ActiveProcess = new ConnectToDevice(ref Lib, Config);
      ActiveProcess.EventHandler += ProcessEventHandler;
      (new Thread(ActiveProcess.Run)).Start();
    }

    private bool LoadXMLConfiguration(ref LibCECConfiguration config)
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
                  case "cec_power_on_startup":
                    if (value.Equals("1") || value.ToLower().Equals("true") || value.ToLower().Equals("yes"))
                    {
                      config.ActivateSource = true;
                      config.WakeDevices.Set(CecLogicalAddress.Tv);
                    }
                    break;
                  case "cec_power_off_shutdown":
                    if (value.Equals("1") || value.ToLower().Equals("true") || value.ToLower().Equals("yes"))
                      config.PowerOffDevices.Set(CecLogicalAddress.Broadcast);
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
                  // 1.5.0+ settings
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
                  case "tv_vendor":
                    {
                      UInt64 iVendor;
                      if (UInt64.TryParse(value, out iVendor))
                        config.TvVendor = (CecVendorId)iVendor;
                    }
                    break;
                  case "wake_devices":
                    {
                      config.WakeDevices.Clear();
                      string[] split = value.Split(new[] { ' ' });
                      foreach (string dev in split)
                      {
                        byte iLogicalAddress;
                        if (byte.TryParse(dev, out iLogicalAddress))
                          config.WakeDevices.Set((CecLogicalAddress)iLogicalAddress);
                      }
                    }
                    break;
                  case "standby_devices":
                    {
                      config.PowerOffDevices.Clear();
                      string[] split = value.Split(new char[] { ' ' });
                      foreach (string dev in split)
                      {
                        byte iLogicalAddress;
                        if (byte.TryParse(dev, out iLogicalAddress))
                          config.PowerOffDevices.Set((CecLogicalAddress)iLogicalAddress);
                      }
                    }
                    break;
                  case "enabled":
                    break;
                  case "port":
                    //TODO
                    break;
                  // 1.5.1 settings
                  case "send_inactive_source":
                    config.SendInactiveSource = value.Equals("1") || value.ToLower().Equals("true") || value.ToLower().Equals("yes");
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
      return gotConfig;
    }

    private void LoadButtonConfiguration()
    {
      //TODO load the real configuration
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Select", (new CecKeypress { Keycode = CecUserControlCode.Select }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Up", (new CecKeypress { Keycode = CecUserControlCode.Up }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Down", (new CecKeypress { Keycode = CecUserControlCode.Down }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Left", (new CecKeypress { Keycode = CecUserControlCode.Left }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Right", (new CecKeypress { Keycode = CecUserControlCode.Right }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Right+Up", (new CecKeypress { Keycode = CecUserControlCode.RightUp }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Right+Down", (new CecKeypress { Keycode = CecUserControlCode.RightDown }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Left+Up", (new CecKeypress { Keycode = CecUserControlCode.LeftUp }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Left+Down", (new CecKeypress { Keycode = CecUserControlCode.LeftDown }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Root menu", (new CecKeypress { Keycode = CecUserControlCode.RootMenu }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Setup menu", (new CecKeypress { Keycode = CecUserControlCode.SetupMenu }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Contents menu", (new CecKeypress { Keycode = CecUserControlCode.ContentsMenu }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Favourite menu", (new CecKeypress { Keycode = CecUserControlCode.FavoriteMenu }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Exit", (new CecKeypress { Keycode = CecUserControlCode.Exit }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("0", (new CecKeypress { Keycode = CecUserControlCode.Number0 }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("1", (new CecKeypress { Keycode = CecUserControlCode.Number1 }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("2", (new CecKeypress { Keycode = CecUserControlCode.Number2 }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("3", (new CecKeypress { Keycode = CecUserControlCode.Number3 }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("4", (new CecKeypress { Keycode = CecUserControlCode.Number4 }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("5", (new CecKeypress { Keycode = CecUserControlCode.Number5 }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("6", (new CecKeypress { Keycode = CecUserControlCode.Number6 }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("7", (new CecKeypress { Keycode = CecUserControlCode.Number7 }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("8", (new CecKeypress { Keycode = CecUserControlCode.Number8 }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("9", (new CecKeypress { Keycode = CecUserControlCode.Number9 }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem(".", (new CecKeypress { Keycode = CecUserControlCode.Dot }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Enter", (new CecKeypress { Keycode = CecUserControlCode.Enter }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Clear", (new CecKeypress { Keycode = CecUserControlCode.Clear }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Next favourite", (new CecKeypress { Keycode = CecUserControlCode.NextFavorite }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Channel up", (new CecKeypress { Keycode = CecUserControlCode.ChannelUp }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Channel down", (new CecKeypress { Keycode = CecUserControlCode.ChannelDown }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Previous channel", (new CecKeypress { Keycode = CecUserControlCode.PreviousChannel }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Sound select", (new CecKeypress { Keycode = CecUserControlCode.SoundSelect }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Input select", (new CecKeypress { Keycode = CecUserControlCode.InputSelect }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Display information", (new CecKeypress { Keycode = CecUserControlCode.DisplayInformation }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Help", (new CecKeypress { Keycode = CecUserControlCode.Help }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Page up", (new CecKeypress { Keycode = CecUserControlCode.PageUp }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Page down", (new CecKeypress { Keycode = CecUserControlCode.PageDown }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Power", (new CecKeypress { Keycode = CecUserControlCode.Power }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Volume up", (new CecKeypress { Keycode = CecUserControlCode.VolumeUp }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Volume down", (new CecKeypress { Keycode = CecUserControlCode.VolumeDown }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Mute", (new CecKeypress { Keycode = CecUserControlCode.Mute }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Play", (new CecKeypress { Keycode = CecUserControlCode.Play }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Stop", (new CecKeypress { Keycode = CecUserControlCode.Stop }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Pause", (new CecKeypress { Keycode = CecUserControlCode.Pause }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Record", (new CecKeypress { Keycode = CecUserControlCode.Record }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Rewind", (new CecKeypress { Keycode = CecUserControlCode.Rewind }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Fast forward", (new CecKeypress { Keycode = CecUserControlCode.FastForward }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Eject", (new CecKeypress { Keycode = CecUserControlCode.Eject }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Forward", (new CecKeypress { Keycode = CecUserControlCode.Forward }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Backward", (new CecKeypress { Keycode = CecUserControlCode.Backward }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Stop record", (new CecKeypress { Keycode = CecUserControlCode.StopRecord }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Pause record", (new CecKeypress { Keycode = CecUserControlCode.PauseRecord }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Angle", (new CecKeypress { Keycode = CecUserControlCode.Angle }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Sub picture", (new CecKeypress { Keycode = CecUserControlCode.SubPicture }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Video on demand", (new CecKeypress { Keycode = CecUserControlCode.VideoOnDemand }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Electronic program guide", (new CecKeypress { Keycode = CecUserControlCode.ElectronicProgramGuide }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Timer programming", (new CecKeypress { Keycode = CecUserControlCode.TimerProgramming }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Initial configuration", (new CecKeypress { Keycode = CecUserControlCode.InitialConfiguration }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Play (function)", (new CecKeypress { Keycode = CecUserControlCode.PlayFunction }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Pause play (function)", (new CecKeypress { Keycode = CecUserControlCode.PausePlayFunction }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Record (function)", (new CecKeypress { Keycode = CecUserControlCode.RecordFunction }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Pause record (function)", (new CecKeypress { Keycode = CecUserControlCode .PauseRecordFunction }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Stop (function)", (new CecKeypress { Keycode = CecUserControlCode.StopFunction }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Mute (function)", (new CecKeypress { Keycode = CecUserControlCode.MuteFunction }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Restore volume", (new CecKeypress { Keycode = CecUserControlCode.RestoreVolumeFunction }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Tune", (new CecKeypress { Keycode = CecUserControlCode.TuneFunction }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Select media", (new CecKeypress { Keycode = CecUserControlCode.SelectMediaFunction }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Select AV input", (new CecKeypress { Keycode = CecUserControlCode.SelectAVInputFunction }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Select audio input", (new CecKeypress { Keycode = CecUserControlCode.SelectAudioInputFunction }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Power toggle", (new CecKeypress { Keycode = CecUserControlCode.PowerToggleFunction }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Power off", (new CecKeypress { Keycode = CecUserControlCode.PowerOffFunction }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Power on", (new CecKeypress { Keycode = CecUserControlCode.PowerOnFunction }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("F1 (blue)", (new CecKeypress { Keycode = CecUserControlCode.F1Blue }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("F2 (red)", (new CecKeypress { Keycode = CecUserControlCode.F2Red }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("F3 (green)", (new CecKeypress { Keycode = CecUserControlCode.F3Green }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("F4 (yellow)", (new CecKeypress { Keycode = CecUserControlCode.F4Yellow }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("F5", (new CecKeypress { Keycode = CecUserControlCode.F5 }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Data", (new CecKeypress { Keycode = CecUserControlCode.Data }), string.Empty));
      cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Return (Samsung)", (new CecKeypress { Keycode = CecUserControlCode.SamsungReturn }), string.Empty));
    }

    private void ProcessEventHandler(object src, UpdateEvent updateEvent)
    {
      switch (updateEvent.Type)
      {
        case UpdateEventType.StatusText:
          SetControlText(lStatus, updateEvent.StringValue);
          break;
        case UpdateEventType.PhysicalAddress:
          Config.PhysicalAddress = (ushort)updateEvent.IntValue;
          SetControlText(tbPhysicalAddress, string.Format("{0,4:X}", updateEvent.IntValue));
          break;
        case UpdateEventType.ProgressBar:
          SetControlVisible(pProgress, true);
          SetProgressValue(pProgress, updateEvent.IntValue);
          break;
        case UpdateEventType.TVVendorId:
          TVVendor = (CecVendorId)updateEvent.IntValue;
          UpdateSelectedDevice();
          break;
        case UpdateEventType.BaseDevicePhysicalAddress:
          SetControlText(lConnectedPhysicalAddress, string.Format("Address: {0,4:X}", updateEvent.IntValue));
          break;
        case UpdateEventType.BaseDevice:
          Config.BaseDevice = (CecLogicalAddress)updateEvent.IntValue;
          break;
        case UpdateEventType.HDMIPort:
          Config.HDMIPort = (byte)updateEvent.IntValue;
          break;
        case UpdateEventType.MenuLanguage:
          SetControlText(cbUseTVMenuLanguage, "Use the TV's language setting" + (updateEvent.StringValue.Length > 0 ? " (" + updateEvent.StringValue + ")" : ""));
          break;
        case UpdateEventType.HasAVRDevice:
          if (HasAVRDevice != updateEvent.BoolValue)
          {
            HasAVRDevice = updateEvent.BoolValue;
            UpdateSelectedDevice();
          }
          break;
        case UpdateEventType.AVRVendorId:
          AVRVendor = (CecVendorId)updateEvent.IntValue;
          UpdateSelectedDevice();
          break;
        case UpdateEventType.Configuration:
          SuppressUpdates = true;
          ConfigurationChanged(updateEvent.ConfigValue);
          SuppressUpdates = false;
          break;
        case UpdateEventType.PollDevices:
          CheckActiveDevices();
          break;
        case UpdateEventType.ProcessCompleted:
          ActiveProcess = null;
          SetControlsEnabled(true);
          if (UpdatingInfoPanel != null)
          {
            UpdatingInfoPanel.SetControlEnabled(UpdatingInfoPanel.bUpdate, true);
            UpdatingInfoPanel = null;
          }
          SetControlVisible(pProgress, false);
          break;
        case UpdateEventType.ExitApplication:
          ActiveProcess = null;
          SetControlsEnabled(false);
          SetControlVisible(pProgress, false);
          Application.Exit();
          break;
      }
    }

    private void SetControlsEnabled(bool val)
    {
      SetControlEnabled(cbPortNumber, val && !cbOverrideAddress.Checked);
      SetControlEnabled(cbConnectedDevice, cbConnectedDevice.Items.Count > 1 && !cbOverrideAddress.Checked && val);
      SetControlEnabled(cbOverrideAddress, val);
      SetControlEnabled(tbPhysicalAddress, val && !Config.AutodetectAddress && cbOverrideAddress.Checked);
      SetControlEnabled(cbDeviceType, val);
      SetControlEnabled(cbUseTVMenuLanguage, val);
      SetControlEnabled(cbActivateSource, val);
      SetControlEnabled(cbPowerOffScreensaver, val);
      SetControlEnabled(cbPowerOffOnStandby, val);
      SetControlEnabled(cbWakeDevices, val);
      SetControlEnabled(cbPowerOffDevices, val);
      SetControlEnabled(cbVendorOverride, val);
      SetControlEnabled(cbVendorId, val && cbVendorOverride.Checked);
      SetControlEnabled(cbSendInactiveSource, val);
      SetControlEnabled(bClose, val);
      SetControlEnabled(bSaveConfig, val);
      SetControlEnabled(bReloadConfig, val);
      SetControlEnabled(bRescanDevices, val);

      SetControlEnabled(bSendImageViewOn, val);
      SetControlEnabled(bStandby, val);
      SetControlEnabled(bActivateSource, val);
      SetControlEnabled(bScan, val);

      bool enableVolumeButtons = (GetTargetDevice() == CecLogicalAddress.AudioSystem) && val;
      SetControlEnabled(bVolUp, enableVolumeButtons);
      SetControlEnabled(bVolDown, enableVolumeButtons);
      SetControlEnabled(bMute, enableVolumeButtons);
    }

    private void tabControl1_SelectedIndexChanged(object sender, EventArgs e)
    {
      switch (tabControl1.SelectedIndex)
      {
        case 0:
          SelectedTab = ConfigTab.Configuration;
          break;
        case 1:
          SelectedTab = ConfigTab.KeyConfiguration;
          break;
        case 2:
          SelectedTab = ConfigTab.Tester;
          break;
        case 3:
          SelectedTab = ConfigTab.Log;
          UpdateLog();
          break;
        default:
          SelectedTab = ConfigTab.Configuration;
          break;
      }
    }

    protected override void Dispose(bool disposing)
    {
      if (disposing)
      {
        Lib.DisableCallbacks();
        Lib.StandbyDevices(CecLogicalAddress.Broadcast);
        Lib.Close();
      }
      if (disposing && (components != null))
      {
        components.Dispose();
      }
      base.Dispose(disposing);
    }

    #region Actions
    public void ReloadXmlConfiguration()
    {
      LoadXMLConfiguration(ref Config);
      Lib.SetConfiguration(Config);
      ConfigurationChanged(Config);
    }

    public void UpdateInfoPanel(DeviceInformation panel)
    {
      if (!SuppressUpdates && ActiveProcess == null)
      {
        SetControlsEnabled(false);
        UpdatingInfoPanel = panel;
        panel.SetControlEnabled(panel.bUpdate, false);
        ActiveProcess = new UpdateDeviceInfo(this, ref Lib, panel);
        ActiveProcess.EventHandler += ProcessEventHandler;
        (new Thread(ActiveProcess.Run)).Start();
      }
    }

    public void SetPhysicalAddress(ushort physicalAddress)
    {
      if (!SuppressUpdates && ActiveProcess == null && cbOverrideAddress.Checked)
      {
        SetControlsEnabled(false);
        SetControlText(cbPortNumber, string.Empty);
        SetControlText(cbConnectedDevice, string.Empty);
        ActiveProcess = new UpdatePhysicalAddress(ref Lib, physicalAddress);
        ActiveProcess.EventHandler += ProcessEventHandler;
        (new Thread(ActiveProcess.Run)).Start();
      }
    }

    public void UpdateConfigurationAsync()
    {
      if (!SuppressUpdates && ActiveProcess == null)
      {
        SetControlsEnabled(false);
        ActiveProcess = new UpdateConfiguration(ref Lib, Config);
        ActiveProcess.EventHandler += ProcessEventHandler;
        (new Thread(ActiveProcess.Run)).Start();
      }
    }

    public void SendImageViewOn(CecLogicalAddress address)
    {
      if (!SuppressUpdates && ActiveProcess == null)
      {
        SetControlsEnabled(false);
        ActiveProcess = new SendImageViewOn(ref Lib, address);
        ActiveProcess.EventHandler += ProcessEventHandler;
        (new Thread(ActiveProcess.Run)).Start();
      }
    }

    public void ActivateSource(CecLogicalAddress address)
    {
      if (!SuppressUpdates && ActiveProcess == null)
      {
        SetControlsEnabled(false);
        ActiveProcess = new SendActivateSource(ref Lib, address);
        ActiveProcess.EventHandler += ProcessEventHandler;
        (new Thread(ActiveProcess.Run)).Start();
      }
    }

    public void SendStandby(CecLogicalAddress address)
    {
      if (!SuppressUpdates && ActiveProcess == null)
      {
        SetControlsEnabled(false);
        ActiveProcess = new SendStandby(ref Lib, address);
        ActiveProcess.EventHandler += ProcessEventHandler;
        (new Thread(ActiveProcess.Run)).Start();
      }
    }

    public void ShowDeviceInfo(CecLogicalAddress address)
    {
      if (!SuppressUpdates && ActiveProcess == null)
      {
        SetControlsEnabled(false);
        ActiveProcess = new ShowDeviceInfo(this, ref Lib, address);
        ActiveProcess.EventHandler += ProcessEventHandler;
        (new Thread(ActiveProcess.Run)).Start();
      }
    }
    #endregion

    #region Configuration tab
    private void cbOverrideAddress_CheckedChanged(object sender, EventArgs e)
    {
      SetControlEnabled(tbPhysicalAddress, ((CheckBox)sender).Checked);
      SetControlEnabled(cbPortNumber, !((CheckBox)sender).Checked);
      SetControlEnabled(cbConnectedDevice, !((CheckBox)sender).Checked && cbConnectedDevice.Items.Count > 1);
    }

    private void tbPhysicalAddress_TextChanged(object sender, EventArgs e)
    {
      if (tbPhysicalAddress.Text.Length != 4 ||
          cbOverrideAddress.Checked)
        return;
      ushort physicalAddress = 0;
      if (!ushort.TryParse(tbPhysicalAddress.Text, NumberStyles.AllowHexSpecifier, null, out physicalAddress))
        return;

      SetPhysicalAddress(physicalAddress);
    }

    private void UpdateSelectedDevice()
    {
      if (HasAVRDevice)
        SetComboBoxItems(cbConnectedDevice, Config.BaseDevice == CecLogicalAddress.AudioSystem ? AVRVendorString : TVVendorString, new object[] { TVVendorString, AVRVendorString });
      else
        SetComboBoxItems(cbConnectedDevice, TVVendorString, new object[] { TVVendorString });
    }

    public void SetConnectedDevice(CecLogicalAddress address, int portnumber)
    {
      if (!SuppressUpdates && ActiveProcess == null)
      {
        SetControlsEnabled(false);
        ActiveProcess = new UpdateConnectedDevice(ref Lib, address, portnumber);
        ActiveProcess.EventHandler += ProcessEventHandler;
        (new Thread(ActiveProcess.Run)).Start();
      }
    }

    private void connectedDevice_SelectedIndexChanged(object sender, EventArgs e)
    {
      SetConnectedDevice(SelectedConnectedDevice, SelectedPortNumber);
    }

    private void bCancel_Click(object sender, EventArgs e)
    {
      Dispose();
    }

    private void bSave_Click(object sender, EventArgs e)
    {
      SetControlsEnabled(false);

      Config.UseTVMenuLanguage = cbUseTVMenuLanguage.Checked;
      Config.ActivateSource = cbActivateSource.Checked;
      Config.PowerOffScreensaver = cbPowerOffScreensaver.Checked;
      Config.PowerOffOnStandby = cbPowerOffOnStandby.Checked;
      Config.SendInactiveSource = cbSendInactiveSource.Checked;
      Config.WakeDevices = WakeDevices;
      Config.PowerOffDevices = PowerOffDevices;

      /* save settings in the eeprom */
      Lib.PersistConfiguration(Config);

      /* and in xml */
      if (ActiveProcess == null)
      {
        SetControlsEnabled(false);
        string xbmcDir = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData) + @"\XBMC\userdata\peripheral_data";
        string defaultDir = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);

        SaveFileDialog dialog = new SaveFileDialog()
        {
          Title = "Where do you want to store the settings?",
          InitialDirectory = Directory.Exists(xbmcDir) ? xbmcDir : defaultDir,
          FileName = "usb_2548_1001.xml",
          Filter = "xml files (*.xml)|*.xml|All files (*.*)|*.*",
          FilterIndex = 1
        };

        if (dialog.ShowDialog() == DialogResult.OK)
        {
          FileStream fs = null;
          string error = string.Empty;
          try
          {
            fs = (FileStream)dialog.OpenFile();
          }
          catch (Exception ex)
          {
            error = ex.Message;
          }
          if (fs == null)
          {
            MessageBox.Show("Cannot open '" + dialog.FileName + "' for writing" + (error.Length > 0 ? ": " + error : string.Empty ), "Pulse-Eight USB-CEC Adapter", MessageBoxButtons.OK, MessageBoxIcon.Error);
          }
          else
          {
            StreamWriter writer = new StreamWriter(fs);
            StringBuilder output = new StringBuilder();
            output.AppendLine("<settings>");
            output.AppendLine("<setting id=\"cec_hdmi_port\" value=\"" + Config.HDMIPort + "\" />");
            output.AppendLine("<setting id=\"connected_device\" value=\"" + (Config.BaseDevice == CecLogicalAddress.AudioSystem ? 5 : 0) + "\" />");
            output.AppendLine("<setting id=\"cec_power_on_startup\" value=\"" + (Config.ActivateSource ? 1 : 0) + "\" />");
            output.AppendLine("<setting id=\"cec_power_off_shutdown\" value=\"" + (Config.PowerOffDevices.IsSet(CecLogicalAddress.Broadcast) ? 1 : 0) + "\" />");
            output.AppendLine("<setting id=\"cec_standby_screensaver\" value=\"" + (Config.PowerOffScreensaver ? 1 : 0) + "\" />");
            output.AppendLine("<setting id=\"standby_pc_on_tv_standby\" value=\"" + (Config.PowerOffOnStandby ? 1 : 0) + "\" />");
            output.AppendLine("<setting id=\"use_tv_menu_language\" value=\"" + (Config.UseTVMenuLanguage ? 1 : 0) + "\" />");
            output.AppendLine("<setting id=\"enabled\" value=\"1\" />");
            output.AppendLine("<setting id=\"port\" value=\"\" />");

            // only supported by 1.5.0+ clients
            output.AppendLine("<!-- the following lines are only supported by v1.5.0+ clients -->");
            output.AppendLine("<setting id=\"activate_source\" value=\"" + (Config.ActivateSource ? 1 : 0) + "\" />");
            output.AppendLine("<setting id=\"physical_address\" value=\"" + string.Format("{0,4:X}", cbOverrideAddress.Checked ? Config.PhysicalAddress : 0).Trim() + "\" />");
            output.AppendLine("<setting id=\"device_type\" value=\"" + (int)Config.DeviceTypes.Types[0] + "\" />");
            output.AppendLine("<setting id=\"tv_vendor\" value=\"" + string.Format("{0,6:X}", (int)Config.TvVendor).Trim() + "\" />");

            output.Append("<setting id=\"wake_devices\" value=\"");
            StringBuilder strWakeDevices = new StringBuilder();
            foreach (CecLogicalAddress addr in Config.WakeDevices.Addresses)
              if (addr != CecLogicalAddress.Unknown)
                strWakeDevices.Append(" " + (int)addr);
            output.Append(strWakeDevices.ToString().Trim());
            output.AppendLine("\" />");

            output.Append("<setting id=\"standby_devices\" value=\"");
            StringBuilder strSleepDevices = new StringBuilder();
            foreach (CecLogicalAddress addr in Config.PowerOffDevices.Addresses)
              if (addr != CecLogicalAddress.Unknown)
                strSleepDevices.Append(" " + (int)addr);
            output.Append(strSleepDevices.ToString().Trim()); 
            output.AppendLine("\" />");

            // only supported by 1.5.1+ clients
            output.AppendLine("<!-- the following lines are only supported by v1.5.1+ clients -->");
            output.AppendLine("<setting id=\"send_inactive_source\" value=\"" + (Config.SendInactiveSource ? 1 : 0) + "\" />");

            output.AppendLine("</settings>");
            writer.Write(output.ToString());
            writer.Close();
            fs.Close();
            fs.Dispose();
            MessageBox.Show("Settings are stored.", "Pulse-Eight USB-CEC Adapter", MessageBoxButtons.OK, MessageBoxIcon.Information);
          }
        }
        SetControlsEnabled(true);
      }
    }

    private void bReloadConfig_Click(object sender, EventArgs e)
    {
      if (Lib.CanPersistConfiguration())
      {
        Lib.GetCurrentConfiguration(Config);
        ConfigurationChanged(Config);
      }
      else
      {
        ReloadXmlConfiguration();
      }
    }

    private void cbVendorOverride_CheckedChanged(object sender, EventArgs e)
    {
      if (cbVendorOverride.Checked)
      {
        cbVendorId.Enabled = true;
        switch (cbVendorId.Text)
        {
          case "LG":
            Config.TvVendor = CecVendorId.LG;
            break;
          case "Onkyo":
            Config.TvVendor = CecVendorId.Onkyo;
            break;
          case "Panasonic":
            Config.TvVendor = CecVendorId.Panasonic;
            break;
          case "Philips":
            Config.TvVendor = CecVendorId.Philips;
            break;
          case "Pioneer":
            Config.TvVendor = CecVendorId.Pioneer;
            break;
          case "Samsung":
            Config.TvVendor = CecVendorId.Samsung;
            break;
          case "Sony":
            Config.TvVendor = CecVendorId.Sony;
            break;
          case "Yamaha":
            Config.TvVendor = CecVendorId.Yamaha;
            break;
          default:
            Config.TvVendor = CecVendorId.Unknown;
            break;
        }
      }
      else
      {
        cbVendorId.Enabled = false;
        Config.TvVendor = CecVendorId.Unknown;
      }
    }

    private void cbDeviceType_SelectedIndexChanged(object sender, EventArgs e)
    {
      CecDeviceType type = SelectedDeviceType;
      if (type != Config.DeviceTypes.Types[0])
      {
        Config.DeviceTypes.Types[0] = type;
        if (!DeviceChangeWarningDisplayed)
        {
          DeviceChangeWarningDisplayed = true;
          MessageBox.Show("You have changed the device type. Save the configuration, and restart the application to use the new setting.", "Pulse-Eight USB-CEC Adapter", MessageBoxButtons.OK, MessageBoxIcon.Warning);
        }
      }
    }
    #endregion

    #region Key configuration tab
    delegate void SelectKeypressRowCallback(CecKeypress key);
    private void SelectKeypressRow(CecKeypress key)
    {
      if (dgButtons.InvokeRequired)
      {
        SelectKeypressRowCallback d = SelectKeypressRow;
        try
        {
          Invoke(d, new object[] { key });
        }
        catch (Exception) { }
      }
      else
      {
        int rowIndex = -1;
        foreach (DataGridViewRow row in dgButtons.Rows)
        {
          CecButtonConfigItem item = row.DataBoundItem as CecButtonConfigItem;
          if (item != null && item.Key.Keycode == key.Keycode)
          {
            rowIndex = row.Index;
            row.Selected = true;
            item.Enabled = true;
          }
          else
          {
            row.Selected = false;
          }
        }
        if (rowIndex > -1)
          dgButtons.FirstDisplayedScrollingRowIndex = rowIndex;
      }
    }

    private void dataGridView1_CellFormatting(object sender, DataGridViewCellFormattingEventArgs e)
    {
      DataGridView grid = sender as DataGridView;
      CecButtonConfigItem data = grid.Rows[e.RowIndex].DataBoundItem as CecButtonConfigItem;
      if (data == null || !data.Enabled)
        e.CellStyle.ForeColor = Color.Gray;
    }
    #endregion

    #region CEC Tester tab
    public void CheckActiveDevices()
    {
      CecLogicalAddresses activeDevices = Lib.GetActiveDevices();
      List<string> deviceList = new List<string>();
      foreach (CecLogicalAddress activeDevice in activeDevices.Addresses)
      {
        if (activeDevice != CecLogicalAddress.Unknown)
          deviceList.Add(string.Format("{0,1:X} : {1}", (int)activeDevice, Lib.ToString(activeDevice)));
      }
      deviceList.Add(string.Format("{0,1:X} : {1}", (int)CecLogicalAddress.Broadcast, Lib.ToString(CecLogicalAddress.Broadcast)));

      SetActiveDevices(deviceList.ToArray());
    }

    delegate void SetActiveDevicesCallback(string[] activeDevices);
    private void SetActiveDevices(string[] activeDevices)
    {
      if (cbCommandDestination.InvokeRequired)
      {
        SetActiveDevicesCallback d = SetActiveDevices;
        try
        {
          Invoke(d, new object[] { activeDevices });
        }
        catch (Exception) { }
      }
      else
      {
        cbCommandDestination.Items.Clear();
        foreach (string item in activeDevices)
          cbCommandDestination.Items.Add(item);
      }
    }

    delegate CecLogicalAddress GetTargetDeviceCallback();
    private CecLogicalAddress GetTargetDevice()
    {
      if (cbCommandDestination.InvokeRequired)
      {
        GetTargetDeviceCallback d = GetTargetDevice;
        CecLogicalAddress retval = CecLogicalAddress.Unknown;
        try
        {
          retval = (CecLogicalAddress)Invoke(d, new object[] { });
        }
        catch (Exception) { }
        return retval;
      }

      return GetLogicalAddressFromString(cbCommandDestination.Text);
    }

    private CecLogicalAddress GetLogicalAddressFromString(string name)
    {
      switch (name.Substring(0, 1).ToLower())
      {
        case "0":
          return CecLogicalAddress.Tv;
        case "1":
          return CecLogicalAddress.RecordingDevice1;
        case "2":
          return CecLogicalAddress.RecordingDevice2;
        case "3":
          return CecLogicalAddress.Tuner1;
        case "4":
          return CecLogicalAddress.PlaybackDevice1;
        case "5":
          return CecLogicalAddress.AudioSystem;
        case "6":
          return CecLogicalAddress.Tuner2;
        case "7":
          return CecLogicalAddress.Tuner3;
        case "8":
          return CecLogicalAddress.PlaybackDevice2;
        case "9":
          return CecLogicalAddress.RecordingDevice3;
        case "a":
          return CecLogicalAddress.Tuner4;
        case "b":
          return CecLogicalAddress.PlaybackDevice3;
        case "c":
          return CecLogicalAddress.Reserved1;
        case "d":
          return CecLogicalAddress.Reserved2;
        case "e":
          return CecLogicalAddress.FreeUse;
        case "f":
          return CecLogicalAddress.Broadcast;
        default:
          return CecLogicalAddress.Unknown;
      }
    }

    private void bSendImageViewOn_Click(object sender, EventArgs e)
    {
      SendImageViewOn(GetTargetDevice());
    }

    private void bStandby_Click(object sender, EventArgs e)
    {
      SendStandby(GetTargetDevice());
    }

    private void bScan_Click(object sender, EventArgs e)
    {
      ShowDeviceInfo(GetTargetDevice());
    }

    private void bActivateSource_Click(object sender, EventArgs e)
    {
      ActivateSource(GetTargetDevice());
    }

    private void cbCommandDestination_SelectedIndexChanged(object sender, EventArgs e)
    {
      bool enableVolumeButtons = (GetTargetDevice() == CecLogicalAddress.AudioSystem);
      bVolUp.Enabled = enableVolumeButtons;
      bVolDown.Enabled = enableVolumeButtons;
      bMute.Enabled = enableVolumeButtons;
      bActivateSource.Enabled = (GetTargetDevice() != CecLogicalAddress.Broadcast);
      bScan.Enabled = (GetTargetDevice() != CecLogicalAddress.Broadcast);
    }

    private void bVolUp_Click(object sender, EventArgs e)
    {
      SetControlsEnabled(false);
      Lib.VolumeUp(true);
      SetControlsEnabled(true);
    }

    private void bVolDown_Click(object sender, EventArgs e)
    {
      SetControlsEnabled(false);
      Lib.VolumeDown(true);
      SetControlsEnabled(true);
    }

    private void bMute_Click(object sender, EventArgs e)
    {
      SetControlsEnabled(false);
      Lib.MuteAudio(true);
      SetControlsEnabled(true);
    }

    private void bRescanDevices_Click(object sender, EventArgs e)
    {
      if (!SuppressUpdates && ActiveProcess == null)
      {
        SetControlsEnabled(false);
        ActiveProcess = new RescanDevices(ref Lib);
        ActiveProcess.EventHandler += ProcessEventHandler;
        (new Thread(ActiveProcess.Run)).Start();
      }
    }
    #endregion

    #region Log tab
    delegate void UpdateLogCallback();
    private void UpdateLog()
    {
      if (tbLog.InvokeRequired)
      {
        UpdateLogCallback d = UpdateLog;
        try
        {
          Invoke(d, new object[] { });
        }
        catch (Exception) { }
      }
      else
      {
        tbLog.Text = Log;
        tbLog.Select(tbLog.Text.Length, 0);
        tbLog.ScrollToCaret();
      }
    }

    private void AddLogMessage(CecLogMessage message)
    {
      string strLevel = "";
      bool display = false;
      switch (message.Level)
      {
        case CecLogLevel.Error:
          strLevel = "ERROR:   ";
          display = cbLogError.Checked;
          break;
        case CecLogLevel.Warning:
          strLevel = "WARNING: ";
          display = cbLogWarning.Checked;
          break;
        case CecLogLevel.Notice:
          strLevel = "NOTICE:  ";
          display = cbLogNotice.Checked;
          break;
        case CecLogLevel.Traffic:
          strLevel = "TRAFFIC: ";
          display = cbLogTraffic.Checked;
          break;
        case CecLogLevel.Debug:
          strLevel = "DEBUG:   ";
          display = cbLogDebug.Checked;
          break;
        default:
          break;
      }

      if (display)
      {
        string strLog = string.Format("{0} {1,16} {2}", strLevel, message.Time, message.Message) + System.Environment.NewLine;
        Log += strLog;
      }

      if (SelectedTab == ConfigTab.Log)
        UpdateLog();
    }

    private void bClearLog_Click(object sender, EventArgs e)
    {
      Log = string.Empty;
      UpdateLog();
    }

    private void bSaveLog_Click(object sender, EventArgs e)
    {
      SaveFileDialog dialog = new SaveFileDialog()
      {
        Title = "Where do you want to store the log file?",
        InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments),
        FileName = "cec-log.txt",
        Filter = "txt files (*.txt)|*.txt|All files (*.*)|*.*",
        FilterIndex = 1
      };

      if (dialog.ShowDialog() == DialogResult.OK)
      {
        FileStream fs = (FileStream)dialog.OpenFile();
        if (fs == null)
        {
          MessageBox.Show("Cannot open '" + dialog.FileName + "' for writing", "Pulse-Eight USB-CEC Adapter", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
        else
        {
          StreamWriter writer = new StreamWriter(fs);
          writer.Write(Log);
          writer.Close();
          fs.Close();
          fs.Dispose();
          MessageBox.Show("The log file was stored as '" + dialog.FileName + "'.", "Pulse-Eight USB-CEC Adapter", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }
      }
    }
    #endregion

    #region LibCecSharp callbacks
    public int ConfigurationChanged(LibCECConfiguration config)
    {
      Config = config;
      SetControlText(tbPhysicalAddress, string.Format("{0,4:X}", Config.PhysicalAddress));
      SetControlText(cbConnectedDevice, Config.BaseDevice == CecLogicalAddress.AudioSystem ? AVRVendorString : TVVendorString);
      SetControlText(cbPortNumber, Config.HDMIPort.ToString());
      switch (config.DeviceTypes.Types[0])
      {
        case CecDeviceType.RecordingDevice:
          SetControlText(cbDeviceType, "Recorder");
          break;
        case CecDeviceType.PlaybackDevice:
          SetControlText(cbDeviceType, "Player");
          break;
        case CecDeviceType.Tuner:
          SetControlText(cbDeviceType, "Tuner");
          break;
        default:
          SetControlText(cbDeviceType, "Recorder");
          break;
      }
      if (config.TvVendor != CecVendorId.Unknown)
      {
        SetCheckboxChecked(cbVendorOverride, true);
        SetControlText(cbVendorId, Lib.ToString(config.TvVendor));
      }
      else
      {
        SetCheckboxChecked(cbVendorOverride, false);
        SetControlText(cbVendorId, Lib.ToString(TVVendor));
      }

      SetCheckboxChecked(cbUseTVMenuLanguage, Config.UseTVMenuLanguage);
      SetCheckboxChecked(cbActivateSource, Config.ActivateSource);
      SetCheckboxChecked(cbPowerOffScreensaver, Config.PowerOffScreensaver);
      SetCheckboxChecked(cbPowerOffOnStandby, Config.PowerOffOnStandby);
      SetCheckboxChecked(cbSendInactiveSource, Config.SendInactiveSource);
      UpdateSelectedDevice();

      for (int iPtr = 0; iPtr < 15; iPtr++)
        SetCheckboxItemChecked(cbWakeDevices, iPtr, Config.WakeDevices.IsSet((CecLogicalAddress)iPtr));
      for (int iPtr = 0; iPtr < 15; iPtr++)
        SetCheckboxItemChecked(cbPowerOffDevices, iPtr, Config.PowerOffDevices.IsSet((CecLogicalAddress)iPtr));

      SetControlText(this, "Pulse-Eight USB-CEC Adapter - libCEC " + Lib.ToString(Config.ServerVersion));
      return 1;
    }

    public int ReceiveCommand(CecCommand command)
    {
      return 1;
    }

    public int ReceiveKeypress(CecKeypress key)
    {
      SelectKeypressRow(key);
      return 1;
    }

    public int ReceiveLogMessage(CecLogMessage message)
    {
      try
      {
        AddLogMessage(message);
      }
      catch (Exception) { }
      return 1;
    }
    #endregion

    #region Class members
    public bool HasAVRDevice { get; private set; }
    #region TV Vendor
    private CecVendorId _tvVendor = CecVendorId.Unknown;
    public CecVendorId TVVendor
    {
      get { return _tvVendor;}
      private set { _tvVendor = value; }
    }
    public string TVVendorString
    {
      get
      {
        return TVVendor != CecVendorId.Unknown ?
          "Television (" + Lib.ToString(TVVendor) + ")" :
          "Television";
      }
    }
    #endregion
    #region AVR Vendor
    private CecVendorId _avrVendor = CecVendorId.Unknown;
    public CecVendorId AVRVendor
    {
      get { return _avrVendor; }
      private set { _avrVendor = value; }
    }
    public string AVRVendorString
    {
      get
      {
        return AVRVendor != CecVendorId.Unknown ?
          "AVR (" + Lib.ToString(AVRVendor) + ")" :
          "AVR";
      }
    }
    #endregion
    public CecLogicalAddress SelectedConnectedDevice
    {
      get
      {
        return (cbConnectedDevice.Text.Equals(AVRVendorString)) ? CecLogicalAddress.AudioSystem : CecLogicalAddress.Tv;
      }
    }
    public CecDeviceType SelectedDeviceType
    {
      get
      {
        switch (cbDeviceType.Text.ToLower())
        {
          case "player":
            return CecDeviceType.PlaybackDevice;
          case "tuner":
            return CecDeviceType.Tuner;
          default:
            return CecDeviceType.RecordingDevice;
        }
      }
    }
    public int SelectedPortNumber
    {
      get
      {
        int iPortNumber = 0;
        if (!int.TryParse(cbPortNumber.Text, out iPortNumber))
          iPortNumber = 1;
        return iPortNumber;
      }
    }
    protected LibCECConfiguration Config;
    protected LibCecSharp Lib;
    private CecCallbackWrapper Callbacks;
    private UpdateProcess ActiveProcess = null;
    private bool SuppressUpdates = true;
    private ConfigTab SelectedTab = ConfigTab.Configuration;
    private string Log = string.Empty;
    private DeviceInformation UpdatingInfoPanel = null;
    private bool DeviceChangeWarningDisplayed = false;
    public CecLogicalAddresses WakeDevices
    {
      get
      {
        CecLogicalAddresses addr = new CecLogicalAddresses();
        foreach (object item in cbWakeDevices.CheckedItems)
        {
          string c = item as string;
          addr.Set(GetLogicalAddressFromString(c));
        }
        return addr;
      }
    }
    public CecLogicalAddresses PowerOffDevices
    {
      get
      {
        CecLogicalAddresses addr = new CecLogicalAddresses();
        foreach (object item in cbPowerOffDevices.CheckedItems)
        {
          string c = item as string;
          addr.Set(GetLogicalAddressFromString(c));
        }
        return addr;
      }
    }
    #endregion
  }

  /// <summary>
  /// A little wrapper that is needed because we already inherit form
  /// </summary>
  internal class CecCallbackWrapper : CecCallbackMethods
  {
    public CecCallbackWrapper(CecConfigGUI gui)
    {
      Gui = gui;
    }

    public override int ReceiveCommand(CecCommand command)
    {
      return Gui.ReceiveCommand(command);
    }

    public override int ReceiveKeypress(CecKeypress key)
    {
      return Gui.ReceiveKeypress(key);
    }

    public override int ReceiveLogMessage(CecLogMessage message)
    {
      return Gui.ReceiveLogMessage(message);
    }

    public override int ConfigurationChanged(LibCECConfiguration config)
    {
      return Gui.ConfigurationChanged(config);
    }

    private CecConfigGUI Gui;
  }
}
