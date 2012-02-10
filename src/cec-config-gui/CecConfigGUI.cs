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

      InitializeComponent();
      Lib = new LibCecSharp(Config);

      ActiveProcess = new ConnectToDevice(ref Lib);
      ActiveProcess.EventHandler += new EventHandler<UpdateEvent>(ProcessEventHandler);
      (new Thread(new ThreadStart(ActiveProcess.Run))).Start();
    }

    public int ReceiveCommand(CecCommand command)
    {
      return 1;
    }

    public int ReceiveKeypress(CecKeypress key)
    {
      return 1;
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
          string strLog = string.Format("{0} {1,16} {2}", strLevel, message.Time, message.Message) + System.Environment.NewLine;
          tbLog.Text += strLog;
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
      SetControlEnabled(cbConnectedDevice, val);
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
    protected int LogLevel = (int)CecLogLevel.All;

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
