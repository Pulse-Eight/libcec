using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
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
  public partial class CecConfigGUI : Form
  {
    public CecConfigGUI()
    {
      Config = new LibCECConfiguration();
      Config.DeviceTypes.Types[0] = CecDeviceType.RecordingDevice;
      Config.DeviceName = "CEC Config";
      Config.GetSettingsFromROM = true;
      Config.ClientVersion = CecClientVersion.Version1_5_0;
      Callbacks = new CecCallbackWrapper(this);
      Config.SetCallbacks(Callbacks);
      LoadXMLConfiguration(ref Config);
      Lib = new LibCecSharp(Config);

      InitializeComponent();
      LoadButtonConfiguration();

      //TODO read the com port setting from the configuration
      CecAdapter[] adapters = Lib.FindAdapters(string.Empty);
      if (adapters.Length == 0 || !Lib.Open(adapters[0].ComPort, 10000))
      {
        MessageBox.Show("Could not connect to any CEC adapter. Please check your configuration.", "Pulse-Eight USB-CEC Adapter", MessageBoxButtons.OK);
        Application.Exit();
      }

      ActiveProcess = new ConnectToDevice(ref Lib, Config);
      ActiveProcess.EventHandler += new EventHandler<UpdateEvent>(ProcessEventHandler);
      (new Thread(new ThreadStart(ActiveProcess.Run))).Start();
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
                    //TODO
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
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Select", (new CecSharp.CecKeypress() { Keycode = 0x00 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Up", (new CecSharp.CecKeypress() { Keycode = 0x01 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Down", (new CecSharp.CecKeypress() { Keycode = 0x02 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Left", (new CecSharp.CecKeypress() { Keycode = 0x03 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Right", (new CecSharp.CecKeypress() { Keycode = 0x04 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Right+Up", (new CecSharp.CecKeypress() { Keycode = 0x05 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Right+Down", (new CecSharp.CecKeypress() { Keycode = 0x06 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Left+Up", (new CecSharp.CecKeypress() { Keycode = 0x07 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Left+Down", (new CecSharp.CecKeypress() { Keycode = 0x08 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Root menu", (new CecSharp.CecKeypress() { Keycode = 0x09 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Setup menu", (new CecSharp.CecKeypress() { Keycode = 0x0A }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Contents menu", (new CecSharp.CecKeypress() { Keycode = 0x0B }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Favourite menu", (new CecSharp.CecKeypress() { Keycode = 0x0C }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Exit", (new CecSharp.CecKeypress() { Keycode = 0x0D }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("0", (new CecSharp.CecKeypress() { Keycode = 0x20 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("1", (new CecSharp.CecKeypress() { Keycode = 0x21 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("2", (new CecSharp.CecKeypress() { Keycode = 0x22 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("3", (new CecSharp.CecKeypress() { Keycode = 0x23 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("4", (new CecSharp.CecKeypress() { Keycode = 0x24 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("5", (new CecSharp.CecKeypress() { Keycode = 0x25 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("6", (new CecSharp.CecKeypress() { Keycode = 0x26 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("7", (new CecSharp.CecKeypress() { Keycode = 0x27 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("8", (new CecSharp.CecKeypress() { Keycode = 0x28 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("9", (new CecSharp.CecKeypress() { Keycode = 0x29 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem(".", (new CecSharp.CecKeypress() { Keycode = 0x2A }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Enter", (new CecSharp.CecKeypress() { Keycode = 0x2B }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Clear", (new CecSharp.CecKeypress() { Keycode = 0x2C }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Next favourite", (new CecSharp.CecKeypress() { Keycode = 0x2F }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Channel up", (new CecSharp.CecKeypress() { Keycode = 0x30 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Channel down", (new CecSharp.CecKeypress() { Keycode = 0x31 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Previous channel", (new CecSharp.CecKeypress() { Keycode = 0x32 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Sound select", (new CecSharp.CecKeypress() { Keycode = 0x33 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Input select", (new CecSharp.CecKeypress() { Keycode = 0x34 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Display information", (new CecSharp.CecKeypress() { Keycode = 0x35 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Help", (new CecSharp.CecKeypress() { Keycode = 0x36 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Page up", (new CecSharp.CecKeypress() { Keycode = 0x37 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Page down", (new CecSharp.CecKeypress() { Keycode = 0x38 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Power", (new CecSharp.CecKeypress() { Keycode = 0x40 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Volume up", (new CecSharp.CecKeypress() { Keycode = 0x41 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Volume down", (new CecSharp.CecKeypress() { Keycode = 0x42 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Mute", (new CecSharp.CecKeypress() { Keycode = 0x43 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Play", (new CecSharp.CecKeypress() { Keycode = 0x44 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Stop", (new CecSharp.CecKeypress() { Keycode = 0x45 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Pause", (new CecSharp.CecKeypress() { Keycode = 0x46 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Record", (new CecSharp.CecKeypress() { Keycode = 0x47 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Rewind", (new CecSharp.CecKeypress() { Keycode = 0x48 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Fast forward", (new CecSharp.CecKeypress() { Keycode = 0x49 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Eject", (new CecSharp.CecKeypress() { Keycode = 0x4A }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Forward", (new CecSharp.CecKeypress() { Keycode = 0x4B }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Backward", (new CecSharp.CecKeypress() { Keycode = 0x4C }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Stop record", (new CecSharp.CecKeypress() { Keycode = 0x4D }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Pause record", (new CecSharp.CecKeypress() { Keycode = 0x4E }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Angle", (new CecSharp.CecKeypress() { Keycode = 0x50 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Sub picture", (new CecSharp.CecKeypress() { Keycode = 0x51 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Video on demand", (new CecSharp.CecKeypress() { Keycode = 0x52 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Electronic program guide", (new CecSharp.CecKeypress() { Keycode = 0x53 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Timer programming", (new CecSharp.CecKeypress() { Keycode = 0x54 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Initial configuration", (new CecSharp.CecKeypress() { Keycode = 0x55 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Play (function)", (new CecSharp.CecKeypress() { Keycode = 0x60 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Pause play (function)", (new CecSharp.CecKeypress() { Keycode = 0x61 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Record (function)", (new CecSharp.CecKeypress() { Keycode = 0x62 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Pause record (function)", (new CecSharp.CecKeypress() { Keycode = 0x63 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Stop (function)", (new CecSharp.CecKeypress() { Keycode = 0x64 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Mute (function)", (new CecSharp.CecKeypress() { Keycode = 0x65 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Restore volume", (new CecSharp.CecKeypress() { Keycode = 0x66 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Tune", (new CecSharp.CecKeypress() { Keycode = 0x67 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Select media", (new CecSharp.CecKeypress() { Keycode = 0x68 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Select AV input", (new CecSharp.CecKeypress() { Keycode = 0x69 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Select audio input", (new CecSharp.CecKeypress() { Keycode = 0x6A }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Power toggle", (new CecSharp.CecKeypress() { Keycode = 0x6B }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Power off", (new CecSharp.CecKeypress() { Keycode = 0x6C }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Power on", (new CecSharp.CecKeypress() { Keycode = 0x6D }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("F1 (blue)", (new CecSharp.CecKeypress() { Keycode = 0x71 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("F2 (red)", (new CecSharp.CecKeypress() { Keycode = 0x72 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("F3 (green)", (new CecSharp.CecKeypress() { Keycode = 0x73 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("F4 (yellow)", (new CecSharp.CecKeypress() { Keycode = 0x74 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("F5", (new CecSharp.CecKeypress() { Keycode = 0x75 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("Data", (new CecSharp.CecKeypress() { Keycode = 0x76 }), string.Empty));
      this.cecButtonConfigBindingSource.Add(new CecButtonConfigItem("(Samsung) Return", (new CecSharp.CecKeypress() { Keycode = 0x91 }), string.Empty));
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

    delegate void SelectKeypressRowCallback(CecKeypress key);
    private void SelectKeypressRow(CecKeypress key)
    {
      if (dgButtons.InvokeRequired)
      {
        SelectKeypressRowCallback d = new SelectKeypressRowCallback(SelectKeypressRow);
        try
        {
          this.Invoke(d, new object[] { key });
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

    delegate void AddLogMessageCallback(CecLogMessage message);
    private void AddLogMessage(CecLogMessage message)
    {
      if (tbLog.InvokeRequired)
      {
        AddLogMessageCallback d = new AddLogMessageCallback(AddLogMessage);
        try
        {
          this.Invoke(d, new object[] { message });
        }
        catch (Exception) { }
      }
      else
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
          tbLog.Text += strLog;
          tbLog.Select(tbLog.Text.Length, 0);
          tbLog.ScrollToCaret();
        }
      }
    }

    public int ReceiveLogMessage(CecLogMessage message)
    {
      AddLogMessage(message);
      return 1;
    }

    delegate void SetControlEnabledCallback(Control control, bool val);
    private void SetControlEnabled(Control control, bool val)
    {
      if (control.InvokeRequired)
      {
        SetControlEnabledCallback d = new SetControlEnabledCallback(SetControlEnabled);
        try
        {
          this.Invoke(d, new object[] { control, val });
        }
        catch (Exception) { }
      }
      else
      {
        control.Enabled = val;
      }
    }

    private void SetControlsEnabled(bool val)
    {
      SetControlEnabled(cbPortNumber, val);
      SetControlEnabled(cbConnectedDevice, cbConnectedDevice.Items.Count > 1 ? val : false);
      SetControlEnabled(tbPhysicalAddress, val);
      SetControlEnabled(cbDeviceType, val);
      SetControlEnabled(cbUseTVMenuLanguage, val);
      SetControlEnabled(cbPowerOnStartup, val);
      SetControlEnabled(cbPowerOffShutdown, val);
      SetControlEnabled(cbPowerOffScreensaver, val);
      SetControlEnabled(cbPowerOffOnStandby, val);
      SetControlEnabled(bClose, val);
      SetControlEnabled(bSave, val);
    }

    delegate void SetControlTextCallback(Control control, string val);
    private void SetControlText(Control control, string val)
    {
      if (control.InvokeRequired)
      {
        SetControlTextCallback d = new SetControlTextCallback(SetControlText);
        try
        {
          this.Invoke(d, new object[] { control, val });
        }
        catch (Exception) { }
      }
      else
      {
        control.Text = val;
      }
    }

    delegate void SetCheckboxCheckedCallback(CheckBox control, bool val);
    private void SetCheckboxChecked(CheckBox control, bool val)
    {
      if (control.InvokeRequired)
      {
        SetCheckboxCheckedCallback d = new SetCheckboxCheckedCallback(SetCheckboxChecked);
        try
        {
          this.Invoke(d, new object[] { control, val });
        }
        catch (Exception) { }
      }
      else
      {
        control.Checked = val;
      }
    }

    delegate void SetProgressValueCallback(ProgressBar control, int val);
    private void SetProgressValue(ProgressBar control, int val)
    {
      if (control.InvokeRequired)
      {
        SetProgressValueCallback d = new SetProgressValueCallback(SetProgressValue);
        try
        {
          this.Invoke(d, new object[] { control, val });
        }
        catch (Exception) { }
      }
      else
      {
        control.Value = val;
      }
    }

    delegate void SetComboBoxItemsCallback(ComboBox control, string selectedText, object[] val);
    private void SetComboBoxItems(ComboBox control, string selectedText, object[] val)
    {
      if (control.InvokeRequired)
      {
        SetComboBoxItemsCallback d = new SetComboBoxItemsCallback(SetComboBoxItems);
        try
        {
          this.Invoke(d, new object[] { control, selectedText, val });
        }
        catch (Exception) { }
      }
      else
      {
        control.Items.Clear();
        control.Items.AddRange(val);
        control.Text = selectedText;
      }
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
          Config = updateEvent.ConfigValue;
          SetControlText(tbPhysicalAddress, string.Format("{0,4:X}", Config.PhysicalAddress));
          SetControlText(cbConnectedDevice, Config.BaseDevice == CecLogicalAddress.AudioSystem ? AVRVendorString : TVVendorString);
          SetControlText(cbPortNumber, Config.HDMIPort.ToString());
          SetCheckboxChecked(cbUseTVMenuLanguage, Config.UseTVMenuLanguage);
          SetCheckboxChecked(cbPowerOnStartup, Config.PowerOnStartup);
          SetCheckboxChecked(cbPowerOffShutdown, Config.PowerOffShutdown);
          SetCheckboxChecked(cbPowerOffScreensaver, Config.PowerOffScreensaver);
          SetCheckboxChecked(cbPowerOffOnStandby, Config.PowerOffOnStandby);
          UpdateSelectedDevice();
          break;
        case UpdateEventType.ProcessCompleted:
          ActiveProcess = null;
          SetControlsEnabled(true);
          break;
      }
    }

    private void UpdateSelectedDevice()
    {
      if (HasAVRDevice)
        SetComboBoxItems(this.cbConnectedDevice, Config.BaseDevice == CecLogicalAddress.AudioSystem ? AVRVendorString : TVVendorString, new object[] { TVVendorString, AVRVendorString });
      else
        SetComboBoxItems(this.cbConnectedDevice, TVVendorString, new object[] { TVVendorString });
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

    public string AVRVendorString
    {
      get
      {
        return AVRVendor != CecVendorId.Unknown ?
          "AVR (" + Lib.ToString(AVRVendor) + ")" :
          "AVR";
      }
    }

    protected bool HasAVRDevice;
    protected CecVendorId TVVendor = CecVendorId.Unknown;
    protected CecVendorId AVRVendor = CecVendorId.Unknown;
    protected CecLogicalAddress SelectedConnectedDevice = CecLogicalAddress.Unknown;

    protected LibCECConfiguration Config;
    protected LibCecSharp Lib;
    private CecCallbackWrapper Callbacks;
    private UpdateProcess ActiveProcess = null;

    private void connectedDevice_SelectedIndexChanged(object sender, EventArgs e)
    {
      if (ActiveProcess == null)
      {
        SetControlsEnabled(false);
        SelectedConnectedDevice = (this.cbConnectedDevice.Text.Equals(AVRVendorString)) ? CecLogicalAddress.AudioSystem : CecLogicalAddress.Tv;
        int iPortNumber = 0;
        if (!int.TryParse(cbPortNumber.Text, out iPortNumber))
          iPortNumber = 1;
        ActiveProcess = new UpdateConnectedDevice(ref Lib, cbConnectedDevice.Text.Equals(AVRVendorString) ? CecLogicalAddress.AudioSystem : CecLogicalAddress.Tv, iPortNumber);
        ActiveProcess.EventHandler += new EventHandler<UpdateEvent>(ProcessEventHandler);
        (new Thread(new ThreadStart(ActiveProcess.Run))).Start();
      }
    }

    private void bCancel_Click(object sender, EventArgs e)
    {
      this.Dispose();
    }

    private void bSave_Click(object sender, EventArgs e)
    {
      SetControlsEnabled(false);

      Config.UseTVMenuLanguage = cbUseTVMenuLanguage.Checked;
      Config.PowerOnStartup = cbPowerOnStartup.Checked;
      Config.PowerOffShutdown = cbPowerOffShutdown.Checked;
      Config.PowerOffScreensaver = cbPowerOffScreensaver.Checked;
      Config.PowerOffOnStandby = cbPowerOffOnStandby.Checked;

      if (!Lib.CanPersistConfiguration())
      {
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
            FileStream fs = (FileStream)dialog.OpenFile();
            if (fs == null)
            {
              MessageBox.Show("Cannot open '" + dialog.FileName + "' for writing", "Pulse-Eight USB-CEC Adapter", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            else
            {
              StreamWriter writer = new StreamWriter(fs);
              StringBuilder output = new StringBuilder();
              output.AppendLine("<settings>");
              output.AppendLine("<setting id=\"cec_hdmi_port\" value=\"" + Config.HDMIPort + "\" />");
              output.AppendLine("<setting id=\"connected_device\" value=\"" + (Config.BaseDevice == CecLogicalAddress.AudioSystem ? 5 : 1) + "\" />");
              output.AppendLine("<setting id=\"physical_address\" value=\"" + string.Format("{0,4:X}", Config.PhysicalAddress) + "\" />");
              output.AppendLine("<setting id=\"device_type\" value=\"" + (int)Config.DeviceTypes.Types[0] + "\" />");
              output.AppendLine("<setting id=\"cec_power_on_startup\" value=\"" + (Config.PowerOnStartup ? 1 : 0) + "\" />");
              output.AppendLine("<setting id=\"cec_power_off_shutdown\" value=\"" + (Config.PowerOffShutdown ? 1 : 0) + "\" />");
              output.AppendLine("<setting id=\"cec_standby_screensaver\" value=\"" + (Config.PowerOffScreensaver ? 1 : 0) + "\" />");
              output.AppendLine("<setting id=\"standby_pc_on_tv_standby\" value=\"" + (Config.PowerOffOnStandby ? 1 : 0) + "\" />");
              output.AppendLine("<setting id=\"use_tv_menu_language\" value=\"" + (Config.UseTVMenuLanguage ? 1 : 0) + "\" />");
              output.AppendLine("<setting id=\"enabled\" value=\"1\" />");
              output.AppendLine("<setting id=\"port\" value=\"\" />");
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
      else
      {
        if (!Lib.PersistConfiguration(Config))
          MessageBox.Show("Could not persist the new settings.", "Pulse-Eight USB-CEC Adapter", MessageBoxButtons.OK, MessageBoxIcon.Error);
        else
          MessageBox.Show("Settings are stored.", "Pulse-Eight USB-CEC Adapter", MessageBoxButtons.OK, MessageBoxIcon.Information);
      }
      SetControlsEnabled(true);
    }

    private void tbPhysicalAddress_TextChanged(object sender, EventArgs e)
    {
      if (ActiveProcess == null)
      {
        if (tbPhysicalAddress.Text.Length != 4)
          return;
        ushort physicalAddress = 0;
        if (!ushort.TryParse(tbPhysicalAddress.Text, NumberStyles.AllowHexSpecifier, null, out physicalAddress))
          return;
        SetControlsEnabled(false);
        SetControlText(cbPortNumber, string.Empty);
        SetControlText(cbConnectedDevice, string.Empty);
        ActiveProcess = new UpdatePhysicalAddress(ref Lib, physicalAddress);
        ActiveProcess.EventHandler += new EventHandler<UpdateEvent>(ProcessEventHandler);
        (new Thread(new ThreadStart(ActiveProcess.Run))).Start();
      }
    }

    private void bClearLog_Click(object sender, EventArgs e)
    {
      tbLog.Text = string.Empty;
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
          writer.Write(tbLog.Text);
          writer.Close();
          fs.Close();
          fs.Dispose();
          MessageBox.Show("The log file was stored as '" + dialog.FileName + "'.", "Pulse-Eight USB-CEC Adapter", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }
      }
    }

    private void dataGridView1_CellFormatting(object sender, DataGridViewCellFormattingEventArgs e)
    {
      DataGridView grid = sender as DataGridView;
      CecButtonConfigItem data = grid.Rows[e.RowIndex].DataBoundItem as CecButtonConfigItem;
      if (data == null || !data.Enabled)
        e.CellStyle.ForeColor = Color.Gray;
    }
  }

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

    private CecConfigGUI Gui;
  }
}
