/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2013 Pulse-Eight Limited.  All rights reserved.
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
using System.Windows.Forms;
using CecSharp;
using System.IO;
using LibCECTray.Properties;
using LibCECTray.controller;
using LibCECTray.controller.applications;
using LibCECTray.settings;

namespace LibCECTray.ui
{
  /// <summary>
  /// The tab pages in this application
  /// </summary>
  internal enum ConfigTab
  {
    Configuration,
    KeyConfiguration,
    Tester,
    Log,
    WMC,
    XBMC
  }

  /// <summary>
  /// Main LibCecTray GUI
  /// </summary>
  partial class CECTray : AsyncForm
  {
    public CECTray()
    {
      Text = Resources.app_name;
      InitializeComponent();
      VisibleChanged += delegate
                       {
                         if (!Visible)
                           OnHide();
                         else
                           OnShow();
                       };
    }

    public override sealed string Text
    {
      get { return base.Text; }
      set { base.Text = value; }
    }

    public void Initialise()
    {
      Controller.Initialise();
    }

    protected override void Dispose(bool disposing)
    {
      Hide();
      if (disposing)
      {
        Controller.Close();
      }
      if (disposing && (components != null))
      {
        components.Dispose();
      }
      base.Dispose(disposing);
    }

    #region Configuration tab
    /// <summary>
    /// Replaces the gui controls by the ones that are bound to the settings.
    /// this is a fugly way to do it, but the gui designer doesn't allow us to ref CECSettings, since it uses symbols from LibCecSharp
    /// </summary>
    public void InitialiseSettingsComponent(CECSettings settings)
    {
      settings.WakeDevices.ReplaceControls(this, Configuration.Controls, lWakeDevices, cbWakeDevices);
      settings.PowerOffDevices.ReplaceControls(this, Configuration.Controls, lPowerOff, cbPowerOffDevices);
      settings.OverridePhysicalAddress.ReplaceControls(this, Configuration.Controls, cbOverrideAddress);
      settings.OverrideTVVendor.ReplaceControls(this, Configuration.Controls, cbVendorOverride);
      settings.PhysicalAddress.ReplaceControls(this, Configuration.Controls, tbPhysicalAddress);
      settings.HDMIPort.ReplaceControls(this, Configuration.Controls, lPortNumber, cbPortNumber);
      settings.ConnectedDevice.ReplaceControls(this, Configuration.Controls, lConnectedDevice, cbConnectedDevice);
      settings.ActivateSource.ReplaceControls(this, Configuration.Controls, cbActivateSource);
      settings.DeviceType.ReplaceControls(this, Configuration.Controls, lDeviceType, cbDeviceType);
      settings.TVVendor.ReplaceControls(this, Configuration.Controls, cbVendorId);
      settings.StartHidden.ReplaceControls(this, Configuration.Controls, cbStartMinimised);
    }

    private void BSaveClick(object sender, EventArgs e)
    {
      Controller.PersistSettings();
    }
   
    private void BReloadConfigClick(object sender, EventArgs e)
    {
      Controller.ResetDefaultSettings();
    }
    #endregion

    #region CEC Tester tab
    delegate void SetActiveDevicesCallback(string[] activeDevices);
    public void SetActiveDevices(string[] activeDevices)
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

      return CECSettingLogicalAddresses.GetLogicalAddressFromString(cbCommandDestination.Text);
    }

    private void BSendImageViewOnClick(object sender, EventArgs e)
    {
      Controller.CECActions.SendImageViewOn(GetTargetDevice());
    }

    private void BStandbyClick(object sender, EventArgs e)
    {
      Controller.CECActions.SendStandby(GetTargetDevice());
    }

    private void BScanClick(object sender, EventArgs e)
    {
      Controller.CECActions.ShowDeviceInfo(GetTargetDevice());
    }

    private void BActivateSourceClick(object sender, EventArgs e)
    {
      Controller.CECActions.ActivateSource(GetTargetDevice());
    }

    private void CbCommandDestinationSelectedIndexChanged(object sender, EventArgs e)
    {
      bool enableVolumeButtons = (GetTargetDevice() == CecLogicalAddress.AudioSystem);
      bVolUp.Enabled = enableVolumeButtons;
      bVolDown.Enabled = enableVolumeButtons;
      bMute.Enabled = enableVolumeButtons;
      bActivateSource.Enabled = (GetTargetDevice() != CecLogicalAddress.Broadcast);
      bScan.Enabled = (GetTargetDevice() != CecLogicalAddress.Broadcast);
    }

    private void BVolUpClick(object sender, EventArgs e)
    {
      Controller.Lib.VolumeUp(true);
    }

    private void BVolDownClick(object sender, EventArgs e)
    {
      Controller.Lib.VolumeDown(true);
    }

    private void BMuteClick(object sender, EventArgs e)
    {
      Controller.Lib.MuteAudio(true);
    }

    private void BRescanDevicesClick(object sender, EventArgs e)
    {
      Controller.CECActions.RescanDevices();
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
        tbLog.Text = _log;
        tbLog.Select(tbLog.Text.Length, 0);
        tbLog.ScrollToCaret();
      }
    }

    public void AddLogMessage(CecLogMessage message)
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
      }

      if (display)
      {
        string strLog = string.Format("{0} {1,16} {2}", strLevel, message.Time, message.Message) + Environment.NewLine;
        AddLogMessage(strLog);
      }
    }

    public void AddLogMessage(string message)
    {
      _log += message;

      if (_selectedTab == ConfigTab.Log)
        UpdateLog();
    }

    private void BClearLogClick(object sender, EventArgs e)
    {
      _log = string.Empty;
      UpdateLog();
    }

    private void BSaveLogClick(object sender, EventArgs e)
    {
      SaveFileDialog dialog = new SaveFileDialog
      {
        Title = Resources.where_do_you_want_to_store_the_log,
        InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments),
        FileName = Resources.cec_log_filename,
        Filter = Resources.cec_log_filter,
        FilterIndex = 1
      };

      if (dialog.ShowDialog() == DialogResult.OK)
      {
        FileStream fs = (FileStream)dialog.OpenFile();
        if (!fs.CanWrite)
        {
          MessageBox.Show(string.Format(Resources.cannot_open_for_writing, dialog.FileName), Resources.app_name, MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
        else
        {
          StreamWriter writer = new StreamWriter(fs);
          writer.Write(_log);
          writer.Close();
          fs.Close();
          fs.Dispose();
          MessageBox.Show(string.Format(Resources.log_stored_as, dialog.FileName), Resources.app_name, MessageBoxButtons.OK, MessageBoxIcon.Information);
        }
      }
    }
    #endregion

    #region Tray icon and window controls
    private void HideToolStripMenuItemClick(object sender, EventArgs e)
    {
      ShowHideToggle();
    }

    private void CloseToolStripMenuItemClick(object sender, EventArgs e)
    {
      Dispose();
    }

    private void AboutToolStripMenuItemClick(object sender, EventArgs e)
    {
      (new About(Controller.LibServerVersion, Controller.LibClientVersion, Controller.LibInfo)).ShowDialog();
    }

    private void AdvancedModeToolStripMenuItemClick(object sender, EventArgs e)
    {
      Controller.Settings.AdvancedMode.Value = !advancedModeToolStripMenuItem.Checked;
      ShowHideAdvanced(!advancedModeToolStripMenuItem.Checked);
    }

    private void BCancelClick(object sender, EventArgs e)
    {
      Dispose();
    }

    private void TrayIconClick(object sender, EventArgs e)
    {
      if (e is MouseEventArgs && (e as MouseEventArgs).Button == MouseButtons.Left)
        ShowHideToggle();
    }

    public void OnHide()
    {
      ShowInTaskbar = false;
      Visible = false;
      tsMenuShowHide.Text = Resources.show;
    }

    public void OnShow()
    {
      ShowInTaskbar = true;
      WindowState = FormWindowState.Normal;
      Activate();
      tsMenuShowHide.Text = Resources.hide;
    }

    private void ShowHideToggle()
    {
      if (Visible && WindowState != FormWindowState.Minimized)
      {
        Controller.Settings.StartHidden.Value = true;
        Hide();
      }
      else
      {
        Controller.Settings.StartHidden.Value = false;
        Show();
      }
    }

    private void TsMenuCloseClick(object sender, EventArgs e)
    {
      Dispose();
    }

    private void CECTrayResize(object sender, EventArgs e)
    {
      if (WindowState == FormWindowState.Minimized)
        Hide();
      else
        Show();
    }

    private void TsMenuShowHideClick(object sender, EventArgs e)
    {
      ShowHideToggle();
    }

    public void ShowHideAdvanced(bool setTo)
    {
      if (setTo)
      {
        tsAdvanced.Checked = true;
        advancedModeToolStripMenuItem.Checked = true;
        SuspendLayout();
        if (!tabPanel.Controls.Contains(tbTestCommands))
          TabControls.Add(tbTestCommands);
        if (!tabPanel.Controls.Contains(LogOutput))
          TabControls.Add(LogOutput);
        ResumeLayout();
      }
      else
      {
        tsAdvanced.Checked = false;
        advancedModeToolStripMenuItem.Checked = false;
        SuspendLayout();
        tabPanel.Controls.Remove(tbTestCommands);
        tabPanel.Controls.Remove(LogOutput);
        ResumeLayout();
      }
    }

    private void TsAdvancedClick(object sender, EventArgs e)
    {
      Controller.Settings.AdvancedMode.Value = !tsAdvanced.Checked;
      ShowHideAdvanced(!tsAdvanced.Checked);
    }

    public void SetStatusText(string status)
    {
      SetControlText(lStatus, status);
    }

    public void SetProgressBar(int progress, bool visible)
    {
      SetControlVisible(pProgress, visible);
      SetProgressValue(pProgress, progress);
    }

    public void SetControlsEnabled(bool val)
    {
      //main tab
      SetControlEnabled(bClose, val);
      SetControlEnabled(bSaveConfig, val);
      SetControlEnabled(bReloadConfig, val);

      //tester tab
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

    private void TabControl1SelectedIndexChanged(object sender, EventArgs e)
    {
      switch (tabPanel.TabPages[tabPanel.SelectedIndex].Name)
      {
        case "tbTestCommands":
          _selectedTab = ConfigTab.Tester;
          break;
        case "LogOutput":
          _selectedTab = ConfigTab.Log;
          UpdateLog();
          break;
        default:
          _selectedTab = ConfigTab.Configuration;
          break;
      }
    }
    #endregion

    #region Class members
    private ConfigTab _selectedTab = ConfigTab.Configuration;
    private string _log = string.Empty;
    private CECController _controller;
    public CECController Controller
    {
      get
      {
        return _controller ?? (_controller = new CECController(this));
      }
    }
    public Control.ControlCollection TabControls
    {
      get { return tabPanel.Controls; }
    }
    public string SelectedTabName
    {
      get { return GetSelectedTabName(tabPanel, tabPanel.TabPages); }
    }
    #endregion

    private void AddNewApplicationToolStripMenuItemClick(object sender, EventArgs e)
    {
      ConfigureApplication appConfig = new ConfigureApplication(Controller.Settings, Controller);
      Controller.DisplayDialog(appConfig, false);
    }
  }
}
