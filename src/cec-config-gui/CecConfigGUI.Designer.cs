namespace CecConfigGui
{
  partial class CecConfigGUI
  {
    /// <summary>
    /// Required designer variable.
    /// </summary>
    private System.ComponentModel.IContainer components = null;

    /// <summary>
    /// Clean up any resources being used.
    /// </summary>
    /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
    protected override void Dispose(bool disposing)
    {
      if (disposing && (components != null))
      {
        components.Dispose();
      }
      base.Dispose(disposing);
    }

    #region Windows Form Designer generated code

    /// <summary>
    /// Required method for Designer support - do not modify
    /// the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent()
    {
      this.components = new System.ComponentModel.Container();
      this.tabControl1 = new System.Windows.Forms.TabControl();
      this.Configuration = new System.Windows.Forms.TabPage();
      this.cbPowerOffOnStandby = new System.Windows.Forms.CheckBox();
      this.cbPowerOffScreensaver = new System.Windows.Forms.CheckBox();
      this.cbPowerOffShutdown = new System.Windows.Forms.CheckBox();
      this.cbPowerOnStartup = new System.Windows.Forms.CheckBox();
      this.cbUseTVMenuLanguage = new System.Windows.Forms.CheckBox();
      this.lPlayerConfig = new System.Windows.Forms.Label();
      this.lAdapterConfig = new System.Windows.Forms.Label();
      this.cbDeviceType = new System.Windows.Forms.ComboBox();
      this.bClose = new System.Windows.Forms.Button();
      this.bSave = new System.Windows.Forms.Button();
      this.cbPortNumber = new System.Windows.Forms.ComboBox();
      this.lConnectedPhysicalAddress = new System.Windows.Forms.Label();
      this.tbPhysicalAddress = new System.Windows.Forms.TextBox();
      this.cbConnectedDevice = new System.Windows.Forms.ComboBox();
      this.lDeviceType = new System.Windows.Forms.Label();
      this.lPhysicalAddress = new System.Windows.Forms.Label();
      this.lConnectedDevice = new System.Windows.Forms.Label();
      this.lPortNumber = new System.Windows.Forms.Label();
      this.LogOutput = new System.Windows.Forms.TabPage();
      this.tbLog = new System.Windows.Forms.TextBox();
      this.pProgress = new System.Windows.Forms.ProgressBar();
      this.lStatus = new System.Windows.Forms.Label();
      this.helpPortNumber = new System.Windows.Forms.ToolTip(this.components);
      this.helpConnectedHDMIDevice = new System.Windows.Forms.ToolTip(this.components);
      this.helpPhysicalAddress = new System.Windows.Forms.ToolTip(this.components);
      this.helpDeviceType = new System.Windows.Forms.ToolTip(this.components);
      this.tabControl1.SuspendLayout();
      this.Configuration.SuspendLayout();
      this.LogOutput.SuspendLayout();
      this.SuspendLayout();
      // 
      // tabControl1
      // 
      this.tabControl1.Controls.Add(this.Configuration);
      this.tabControl1.Controls.Add(this.LogOutput);
      this.tabControl1.Location = new System.Drawing.Point(12, 12);
      this.tabControl1.Name = "tabControl1";
      this.tabControl1.SelectedIndex = 0;
      this.tabControl1.Size = new System.Drawing.Size(600, 385);
      this.tabControl1.TabIndex = 0;
      // 
      // Configuration
      // 
      this.Configuration.Controls.Add(this.cbPowerOffOnStandby);
      this.Configuration.Controls.Add(this.cbPowerOffScreensaver);
      this.Configuration.Controls.Add(this.cbPowerOffShutdown);
      this.Configuration.Controls.Add(this.cbPowerOnStartup);
      this.Configuration.Controls.Add(this.cbUseTVMenuLanguage);
      this.Configuration.Controls.Add(this.lPlayerConfig);
      this.Configuration.Controls.Add(this.lAdapterConfig);
      this.Configuration.Controls.Add(this.cbDeviceType);
      this.Configuration.Controls.Add(this.bClose);
      this.Configuration.Controls.Add(this.bSave);
      this.Configuration.Controls.Add(this.cbPortNumber);
      this.Configuration.Controls.Add(this.lConnectedPhysicalAddress);
      this.Configuration.Controls.Add(this.tbPhysicalAddress);
      this.Configuration.Controls.Add(this.cbConnectedDevice);
      this.Configuration.Controls.Add(this.lDeviceType);
      this.Configuration.Controls.Add(this.lPhysicalAddress);
      this.Configuration.Controls.Add(this.lConnectedDevice);
      this.Configuration.Controls.Add(this.lPortNumber);
      this.Configuration.Location = new System.Drawing.Point(4, 22);
      this.Configuration.Name = "Configuration";
      this.Configuration.Padding = new System.Windows.Forms.Padding(3);
      this.Configuration.Size = new System.Drawing.Size(592, 359);
      this.Configuration.TabIndex = 0;
      this.Configuration.Text = "Configuration";
      this.Configuration.UseVisualStyleBackColor = true;
      // 
      // cbPowerOffOnStandby
      // 
      this.cbPowerOffOnStandby.AutoSize = true;
      this.cbPowerOffOnStandby.Enabled = false;
      this.cbPowerOffOnStandby.Location = new System.Drawing.Point(10, 297);
      this.cbPowerOffOnStandby.Name = "cbPowerOffOnStandby";
      this.cbPowerOffOnStandby.Size = new System.Drawing.Size(292, 17);
      this.cbPowerOffOnStandby.TabIndex = 22;
      this.cbPowerOffOnStandby.Text = "Put this PC in standby mode when the TV is switched off";
      this.cbPowerOffOnStandby.UseVisualStyleBackColor = true;
      // 
      // cbPowerOffScreensaver
      // 
      this.cbPowerOffScreensaver.AutoSize = true;
      this.cbPowerOffScreensaver.Enabled = false;
      this.cbPowerOffScreensaver.Location = new System.Drawing.Point(10, 273);
      this.cbPowerOffScreensaver.Name = "cbPowerOffScreensaver";
      this.cbPowerOffScreensaver.Size = new System.Drawing.Size(301, 17);
      this.cbPowerOffScreensaver.TabIndex = 21;
      this.cbPowerOffScreensaver.Text = "Put devices in standby mode when activating screensaver";
      this.cbPowerOffScreensaver.UseVisualStyleBackColor = true;
      // 
      // cbPowerOffShutdown
      // 
      this.cbPowerOffShutdown.AutoSize = true;
      this.cbPowerOffShutdown.Enabled = false;
      this.cbPowerOffShutdown.Location = new System.Drawing.Point(10, 249);
      this.cbPowerOffShutdown.Name = "cbPowerOffShutdown";
      this.cbPowerOffShutdown.Size = new System.Drawing.Size(317, 17);
      this.cbPowerOffShutdown.TabIndex = 20;
      this.cbPowerOffShutdown.Text = "Power off devices when stopping the media player application";
      this.cbPowerOffShutdown.UseVisualStyleBackColor = true;
      // 
      // cbPowerOnStartup
      // 
      this.cbPowerOnStartup.AutoSize = true;
      this.cbPowerOnStartup.Enabled = false;
      this.cbPowerOnStartup.Location = new System.Drawing.Point(10, 225);
      this.cbPowerOnStartup.Name = "cbPowerOnStartup";
      this.cbPowerOnStartup.Size = new System.Drawing.Size(306, 17);
      this.cbPowerOnStartup.TabIndex = 19;
      this.cbPowerOnStartup.Text = "Power on the TV when starting the media player application";
      this.cbPowerOnStartup.UseVisualStyleBackColor = true;
      // 
      // cbUseTVMenuLanguage
      // 
      this.cbUseTVMenuLanguage.AutoSize = true;
      this.cbUseTVMenuLanguage.Enabled = false;
      this.cbUseTVMenuLanguage.Location = new System.Drawing.Point(10, 201);
      this.cbUseTVMenuLanguage.Name = "cbUseTVMenuLanguage";
      this.cbUseTVMenuLanguage.Size = new System.Drawing.Size(168, 17);
      this.cbUseTVMenuLanguage.TabIndex = 18;
      this.cbUseTVMenuLanguage.Text = "Use the TV\'s language setting";
      this.cbUseTVMenuLanguage.UseVisualStyleBackColor = true;
      // 
      // lPlayerConfig
      // 
      this.lPlayerConfig.AutoSize = true;
      this.lPlayerConfig.Font = new System.Drawing.Font("Microsoft Sans Serif", 14.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
      this.lPlayerConfig.Location = new System.Drawing.Point(6, 165);
      this.lPlayerConfig.Name = "lPlayerConfig";
      this.lPlayerConfig.Size = new System.Drawing.Size(198, 24);
      this.lPlayerConfig.TabIndex = 16;
      this.lPlayerConfig.Text = "Player Configuration";
      // 
      // lAdapterConfig
      // 
      this.lAdapterConfig.AutoSize = true;
      this.lAdapterConfig.Font = new System.Drawing.Font("Microsoft Sans Serif", 14.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
      this.lAdapterConfig.Location = new System.Drawing.Point(6, 3);
      this.lAdapterConfig.Name = "lAdapterConfig";
      this.lAdapterConfig.Size = new System.Drawing.Size(213, 24);
      this.lAdapterConfig.TabIndex = 15;
      this.lAdapterConfig.Text = "Adapter Configuration";
      // 
      // cbDeviceType
      // 
      this.cbDeviceType.Enabled = false;
      this.cbDeviceType.FormattingEnabled = true;
      this.cbDeviceType.Items.AddRange(new object[] {
            "Recorder",
            "Player",
            "Tuner"});
      this.cbDeviceType.Location = new System.Drawing.Point(174, 123);
      this.cbDeviceType.Name = "cbDeviceType";
      this.cbDeviceType.Size = new System.Drawing.Size(121, 21);
      this.cbDeviceType.TabIndex = 14;
      this.cbDeviceType.Text = "Recorder";
      this.helpDeviceType.SetToolTip(this.cbDeviceType, "Set this to \'Player\' when your TV is having problems with \'Recorder\'");
      // 
      // bClose
      // 
      this.bClose.Enabled = false;
      this.bClose.Location = new System.Drawing.Point(189, 330);
      this.bClose.Name = "bClose";
      this.bClose.Size = new System.Drawing.Size(75, 23);
      this.bClose.TabIndex = 13;
      this.bClose.Text = "Close";
      this.bClose.UseVisualStyleBackColor = true;
      this.bClose.Click += new System.EventHandler(this.bCancel_Click);
      // 
      // bSave
      // 
      this.bSave.Enabled = false;
      this.bSave.Location = new System.Drawing.Point(298, 330);
      this.bSave.Name = "bSave";
      this.bSave.Size = new System.Drawing.Size(125, 23);
      this.bSave.TabIndex = 12;
      this.bSave.Text = "Save configuration";
      this.bSave.UseVisualStyleBackColor = true;
      this.bSave.Click += new System.EventHandler(this.bSave_Click);
      // 
      // cbPortNumber
      // 
      this.cbPortNumber.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.Append;
      this.cbPortNumber.Enabled = false;
      this.cbPortNumber.FormattingEnabled = true;
      this.cbPortNumber.Items.AddRange(new object[] {
            "1",
            "2",
            "3",
            "4"});
      this.cbPortNumber.Location = new System.Drawing.Point(174, 40);
      this.cbPortNumber.Name = "cbPortNumber";
      this.cbPortNumber.Size = new System.Drawing.Size(38, 21);
      this.cbPortNumber.TabIndex = 11;
      this.cbPortNumber.Text = "1";
      this.helpPortNumber.SetToolTip(this.cbPortNumber, "The HDMI port number, to which you connected your USB-CEC adapter.");
      this.cbPortNumber.SelectedIndexChanged += new System.EventHandler(this.connectedDevice_SelectedIndexChanged);
      // 
      // lConnectedPhysicalAddress
      // 
      this.lConnectedPhysicalAddress.AutoSize = true;
      this.lConnectedPhysicalAddress.Location = new System.Drawing.Point(310, 70);
      this.lConnectedPhysicalAddress.Name = "lConnectedPhysicalAddress";
      this.lConnectedPhysicalAddress.Size = new System.Drawing.Size(75, 13);
      this.lConnectedPhysicalAddress.TabIndex = 10;
      this.lConnectedPhysicalAddress.Text = "Address: 0000";
      // 
      // tbPhysicalAddress
      // 
      this.tbPhysicalAddress.Enabled = false;
      this.tbPhysicalAddress.Location = new System.Drawing.Point(174, 95);
      this.tbPhysicalAddress.MaxLength = 4;
      this.tbPhysicalAddress.Name = "tbPhysicalAddress";
      this.tbPhysicalAddress.Size = new System.Drawing.Size(38, 20);
      this.tbPhysicalAddress.TabIndex = 6;
      this.tbPhysicalAddress.Text = "1000";
      this.helpPhysicalAddress.SetToolTip(this.tbPhysicalAddress, "The physical address of the adapter. Leave this untouched if you want to autodete" +
              "ct this value.");
      this.tbPhysicalAddress.TextChanged += new System.EventHandler(this.tbPhysicalAddress_TextChanged);
      // 
      // cbConnectedDevice
      // 
      this.cbConnectedDevice.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.Append;
      this.cbConnectedDevice.Enabled = false;
      this.cbConnectedDevice.FormattingEnabled = true;
      this.cbConnectedDevice.Location = new System.Drawing.Point(174, 67);
      this.cbConnectedDevice.Name = "cbConnectedDevice";
      this.cbConnectedDevice.Size = new System.Drawing.Size(121, 21);
      this.cbConnectedDevice.TabIndex = 5;
      this.helpConnectedHDMIDevice.SetToolTip(this.cbConnectedDevice, "The HDMI device to which the USB-CEC adapter is connected");
      this.cbConnectedDevice.SelectedIndexChanged += new System.EventHandler(this.connectedDevice_SelectedIndexChanged);
      // 
      // lDeviceType
      // 
      this.lDeviceType.AutoSize = true;
      this.lDeviceType.Location = new System.Drawing.Point(6, 126);
      this.lDeviceType.Name = "lDeviceType";
      this.lDeviceType.Size = new System.Drawing.Size(64, 13);
      this.lDeviceType.TabIndex = 3;
      this.lDeviceType.Text = "Device type";
      // 
      // lPhysicalAddress
      // 
      this.lPhysicalAddress.AutoSize = true;
      this.lPhysicalAddress.Location = new System.Drawing.Point(6, 98);
      this.lPhysicalAddress.Name = "lPhysicalAddress";
      this.lPhysicalAddress.Size = new System.Drawing.Size(86, 13);
      this.lPhysicalAddress.TabIndex = 2;
      this.lPhysicalAddress.Text = "Physical address";
      // 
      // lConnectedDevice
      // 
      this.lConnectedDevice.AutoSize = true;
      this.lConnectedDevice.Location = new System.Drawing.Point(6, 70);
      this.lConnectedDevice.Name = "lConnectedDevice";
      this.lConnectedDevice.Size = new System.Drawing.Size(137, 13);
      this.lConnectedDevice.TabIndex = 1;
      this.lConnectedDevice.Text = "Connected to HDMI device";
      // 
      // lPortNumber
      // 
      this.lPortNumber.AutoSize = true;
      this.lPortNumber.Location = new System.Drawing.Point(6, 43);
      this.lPortNumber.Name = "lPortNumber";
      this.lPortNumber.Size = new System.Drawing.Size(95, 13);
      this.lPortNumber.TabIndex = 0;
      this.lPortNumber.Text = "HDMI Port number";
      // 
      // LogOutput
      // 
      this.LogOutput.Controls.Add(this.tbLog);
      this.LogOutput.Location = new System.Drawing.Point(4, 22);
      this.LogOutput.Name = "LogOutput";
      this.LogOutput.Padding = new System.Windows.Forms.Padding(3);
      this.LogOutput.Size = new System.Drawing.Size(592, 359);
      this.LogOutput.TabIndex = 1;
      this.LogOutput.Text = "Log Output";
      this.LogOutput.UseVisualStyleBackColor = true;
      // 
      // tbLog
      // 
      this.tbLog.Location = new System.Drawing.Point(6, 6);
      this.tbLog.Multiline = true;
      this.tbLog.Name = "tbLog";
      this.tbLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
      this.tbLog.Size = new System.Drawing.Size(580, 347);
      this.tbLog.TabIndex = 0;
      // 
      // pProgress
      // 
      this.pProgress.Location = new System.Drawing.Point(314, 407);
      this.pProgress.Name = "pProgress";
      this.pProgress.Size = new System.Drawing.Size(298, 23);
      this.pProgress.TabIndex = 1;
      // 
      // lStatus
      // 
      this.lStatus.AutoSize = true;
      this.lStatus.Location = new System.Drawing.Point(12, 416);
      this.lStatus.Name = "lStatus";
      this.lStatus.Size = new System.Drawing.Size(61, 13);
      this.lStatus.TabIndex = 2;
      this.lStatus.Text = "Initialising...";
      // 
      // CecConfigGUI
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(624, 442);
      this.Controls.Add(this.lStatus);
      this.Controls.Add(this.pProgress);
      this.Controls.Add(this.tabControl1);
      this.Name = "CecConfigGUI";
      this.Text = "Pulse-Eight USB-CEC Adapter";
      this.tabControl1.ResumeLayout(false);
      this.Configuration.ResumeLayout(false);
      this.Configuration.PerformLayout();
      this.LogOutput.ResumeLayout(false);
      this.LogOutput.PerformLayout();
      this.ResumeLayout(false);
      this.PerformLayout();

    }

    #endregion

    private System.Windows.Forms.TabControl tabControl1;
    private System.Windows.Forms.TabPage Configuration;
    private System.Windows.Forms.TabPage LogOutput;
    private System.Windows.Forms.Label lPortNumber;
    private System.Windows.Forms.Label lDeviceType;
    private System.Windows.Forms.Label lPhysicalAddress;
    private System.Windows.Forms.Label lConnectedDevice;
    private System.Windows.Forms.ComboBox cbConnectedDevice;
    private System.Windows.Forms.TextBox tbPhysicalAddress;
    private System.Windows.Forms.ProgressBar pProgress;
    private System.Windows.Forms.Label lStatus;
    private System.Windows.Forms.Label lConnectedPhysicalAddress;
    private System.Windows.Forms.TextBox tbLog;
    private System.Windows.Forms.ComboBox cbPortNumber;
    private System.Windows.Forms.Button bClose;
    private System.Windows.Forms.Button bSave;
    private System.Windows.Forms.ComboBox cbDeviceType;
    private System.Windows.Forms.Label lAdapterConfig;
    private System.Windows.Forms.CheckBox cbPowerOffOnStandby;
    private System.Windows.Forms.CheckBox cbPowerOffScreensaver;
    private System.Windows.Forms.CheckBox cbPowerOffShutdown;
    private System.Windows.Forms.CheckBox cbPowerOnStartup;
    private System.Windows.Forms.CheckBox cbUseTVMenuLanguage;
    private System.Windows.Forms.Label lPlayerConfig;
    private System.Windows.Forms.ToolTip helpPortNumber;
    private System.Windows.Forms.ToolTip helpConnectedHDMIDevice;
    private System.Windows.Forms.ToolTip helpDeviceType;
    private System.Windows.Forms.ToolTip helpPhysicalAddress;
  }
}