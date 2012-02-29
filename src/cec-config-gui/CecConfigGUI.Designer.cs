namespace CecConfigGui
{
  partial class CecConfigGUI
  {
    /// <summary>
    /// Required designer variable.
    /// </summary>
    private System.ComponentModel.IContainer components = null;

    #region Windows Form Designer generated code

    /// <summary>
    /// Required method for Designer support - do not modify
    /// the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent()
    {
      this.components = new System.ComponentModel.Container();
      System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(CecConfigGUI));
      this.tabControl1 = new System.Windows.Forms.TabControl();
      this.Configuration = new System.Windows.Forms.TabPage();
      this.cbOverrideAddress = new System.Windows.Forms.CheckBox();
      this.bReloadConfig = new System.Windows.Forms.Button();
      this.cbVendorOverride = new System.Windows.Forms.CheckBox();
      this.cbVendorId = new System.Windows.Forms.ComboBox();
      this.lPowerOff = new System.Windows.Forms.Label();
      this.cbPowerOffDevices = new System.Windows.Forms.CheckedListBox();
      this.lWakeDevices = new System.Windows.Forms.Label();
      this.cbWakeDevices = new System.Windows.Forms.CheckedListBox();
      this.cbPowerOffOnStandby = new System.Windows.Forms.CheckBox();
      this.cbPowerOffScreensaver = new System.Windows.Forms.CheckBox();
      this.cbActivateSource = new System.Windows.Forms.CheckBox();
      this.cbUseTVMenuLanguage = new System.Windows.Forms.CheckBox();
      this.lPlayerConfig = new System.Windows.Forms.Label();
      this.lAdapterConfig = new System.Windows.Forms.Label();
      this.cbDeviceType = new System.Windows.Forms.ComboBox();
      this.bClose = new System.Windows.Forms.Button();
      this.bSaveConfig = new System.Windows.Forms.Button();
      this.cbPortNumber = new System.Windows.Forms.ComboBox();
      this.lConnectedPhysicalAddress = new System.Windows.Forms.Label();
      this.tbPhysicalAddress = new System.Windows.Forms.TextBox();
      this.cbConnectedDevice = new System.Windows.Forms.ComboBox();
      this.lDeviceType = new System.Windows.Forms.Label();
      this.lConnectedDevice = new System.Windows.Forms.Label();
      this.lPortNumber = new System.Windows.Forms.Label();
      this.tbButtons = new System.Windows.Forms.TabPage();
      this.label1 = new System.Windows.Forms.Label();
      this.dgButtons = new System.Windows.Forms.DataGridView();
      this.CecButtonName = new System.Windows.Forms.DataGridViewTextBoxColumn();
      this.cecButtonConfigBindingSource = new System.Windows.Forms.BindingSource(this.components);
      this.tbTestCommands = new System.Windows.Forms.TabPage();
      this.bRescanDevices = new System.Windows.Forms.Button();
      this.bMute = new System.Windows.Forms.Button();
      this.bVolDown = new System.Windows.Forms.Button();
      this.bVolUp = new System.Windows.Forms.Button();
      this.bActivateSource = new System.Windows.Forms.Button();
      this.bScan = new System.Windows.Forms.Button();
      this.bStandby = new System.Windows.Forms.Button();
      this.bSendImageViewOn = new System.Windows.Forms.Button();
      this.lDestination = new System.Windows.Forms.Label();
      this.cbCommandDestination = new System.Windows.Forms.ComboBox();
      this.LogOutput = new System.Windows.Forms.TabPage();
      this.bSaveLog = new System.Windows.Forms.Button();
      this.bClearLog = new System.Windows.Forms.Button();
      this.cbLogDebug = new System.Windows.Forms.CheckBox();
      this.cbLogTraffic = new System.Windows.Forms.CheckBox();
      this.cbLogNotice = new System.Windows.Forms.CheckBox();
      this.cbLogWarning = new System.Windows.Forms.CheckBox();
      this.cbLogError = new System.Windows.Forms.CheckBox();
      this.tbLog = new System.Windows.Forms.TextBox();
      this.pProgress = new System.Windows.Forms.ProgressBar();
      this.lStatus = new System.Windows.Forms.Label();
      this.helpPortNumber = new System.Windows.Forms.ToolTip(this.components);
      this.helpConnectedHDMIDevice = new System.Windows.Forms.ToolTip(this.components);
      this.helpPhysicalAddress = new System.Windows.Forms.ToolTip(this.components);
      this.helpDeviceType = new System.Windows.Forms.ToolTip(this.components);
      this.cbSendInactiveSource = new System.Windows.Forms.CheckBox();
      this.tabControl1.SuspendLayout();
      this.Configuration.SuspendLayout();
      this.tbButtons.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)(this.dgButtons)).BeginInit();
      ((System.ComponentModel.ISupportInitialize)(this.cecButtonConfigBindingSource)).BeginInit();
      this.tbTestCommands.SuspendLayout();
      this.LogOutput.SuspendLayout();
      this.SuspendLayout();
      // 
      // tabControl1
      // 
      this.tabControl1.Controls.Add(this.Configuration);
      this.tabControl1.Controls.Add(this.tbButtons);
      this.tabControl1.Controls.Add(this.tbTestCommands);
      this.tabControl1.Controls.Add(this.LogOutput);
      this.tabControl1.Location = new System.Drawing.Point(12, 12);
      this.tabControl1.Name = "tabControl1";
      this.tabControl1.SelectedIndex = 0;
      this.tabControl1.Size = new System.Drawing.Size(600, 385);
      this.tabControl1.TabIndex = 0;
      this.tabControl1.SelectedIndexChanged += new System.EventHandler(this.tabControl1_SelectedIndexChanged);
      // 
      // Configuration
      // 
      this.Configuration.Controls.Add(this.cbSendInactiveSource);
      this.Configuration.Controls.Add(this.cbOverrideAddress);
      this.Configuration.Controls.Add(this.bReloadConfig);
      this.Configuration.Controls.Add(this.cbVendorOverride);
      this.Configuration.Controls.Add(this.cbVendorId);
      this.Configuration.Controls.Add(this.lPowerOff);
      this.Configuration.Controls.Add(this.cbPowerOffDevices);
      this.Configuration.Controls.Add(this.lWakeDevices);
      this.Configuration.Controls.Add(this.cbWakeDevices);
      this.Configuration.Controls.Add(this.cbPowerOffOnStandby);
      this.Configuration.Controls.Add(this.cbPowerOffScreensaver);
      this.Configuration.Controls.Add(this.cbActivateSource);
      this.Configuration.Controls.Add(this.cbUseTVMenuLanguage);
      this.Configuration.Controls.Add(this.lPlayerConfig);
      this.Configuration.Controls.Add(this.lAdapterConfig);
      this.Configuration.Controls.Add(this.cbDeviceType);
      this.Configuration.Controls.Add(this.bClose);
      this.Configuration.Controls.Add(this.bSaveConfig);
      this.Configuration.Controls.Add(this.cbPortNumber);
      this.Configuration.Controls.Add(this.lConnectedPhysicalAddress);
      this.Configuration.Controls.Add(this.tbPhysicalAddress);
      this.Configuration.Controls.Add(this.cbConnectedDevice);
      this.Configuration.Controls.Add(this.lDeviceType);
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
      // cbOverrideAddress
      // 
      this.cbOverrideAddress.AutoSize = true;
      this.cbOverrideAddress.Enabled = false;
      this.cbOverrideAddress.Location = new System.Drawing.Point(10, 97);
      this.cbOverrideAddress.Name = "cbOverrideAddress";
      this.cbOverrideAddress.Size = new System.Drawing.Size(147, 17);
      this.cbOverrideAddress.TabIndex = 31;
      this.cbOverrideAddress.Text = "Override physical address";
      this.cbOverrideAddress.UseVisualStyleBackColor = true;
      this.cbOverrideAddress.CheckedChanged += new System.EventHandler(this.cbOverrideAddress_CheckedChanged);
      // 
      // bReloadConfig
      // 
      this.bReloadConfig.Enabled = false;
      this.bReloadConfig.Location = new System.Drawing.Point(358, 330);
      this.bReloadConfig.Name = "bReloadConfig";
      this.bReloadConfig.Size = new System.Drawing.Size(125, 23);
      this.bReloadConfig.TabIndex = 30;
      this.bReloadConfig.Text = "Reload configuration";
      this.bReloadConfig.UseVisualStyleBackColor = true;
      this.bReloadConfig.Click += new System.EventHandler(this.bReloadConfig_Click);
      // 
      // cbVendorOverride
      // 
      this.cbVendorOverride.AutoSize = true;
      this.cbVendorOverride.Enabled = false;
      this.cbVendorOverride.Location = new System.Drawing.Point(10, 156);
      this.cbVendorOverride.Name = "cbVendorOverride";
      this.cbVendorOverride.Size = new System.Drawing.Size(130, 17);
      this.cbVendorOverride.TabIndex = 29;
      this.cbVendorOverride.Text = "Override TV vendor id";
      this.cbVendorOverride.UseVisualStyleBackColor = true;
      this.cbVendorOverride.CheckedChanged += new System.EventHandler(this.cbVendorOverride_CheckedChanged);
      // 
      // cbVendorId
      // 
      this.cbVendorId.Enabled = false;
      this.cbVendorId.FormattingEnabled = true;
      this.cbVendorId.Items.AddRange(new object[] {
            "- autodetect -",
            "LG",
            "Onkyo",
            "Panasonic",
            "Philips",
            "Pioneer",
            "Samsung",
            "Sony",
            "Yamaha"});
      this.cbVendorId.Location = new System.Drawing.Point(174, 153);
      this.cbVendorId.Name = "cbVendorId";
      this.cbVendorId.Size = new System.Drawing.Size(157, 21);
      this.cbVendorId.TabIndex = 28;
      this.cbVendorId.Text = "- autodetect -";
      this.helpDeviceType.SetToolTip(this.cbVendorId, "Only set this value when autodetection isn\'t working");
      // 
      // lPowerOff
      // 
      this.lPowerOff.AutoSize = true;
      this.lPowerOff.Location = new System.Drawing.Point(465, 203);
      this.lPowerOff.Name = "lPowerOff";
      this.lPowerOff.Size = new System.Drawing.Size(124, 13);
      this.lPowerOff.TabIndex = 26;
      this.lPowerOff.Text = "Power off when stopping";
      // 
      // cbPowerOffDevices
      // 
      this.cbPowerOffDevices.Enabled = false;
      this.cbPowerOffDevices.FormattingEnabled = true;
      this.cbPowerOffDevices.Items.AddRange(new object[] {
            "0: TV",
            "1: Recorder 1",
            "2: Recorder 2",
            "3: Tuner 1",
            "4: Playback 1",
            "5: Audio system",
            "6: Tuner 2",
            "7: Tuner 3",
            "8: Playback 2",
            "9: Recorder 3",
            "A: Tuner 4",
            "B: Playback 3",
            "C: Reserved 1",
            "D: Reserved 2",
            "E: Free use",
            "F: Broadcast"});
      this.cbPowerOffDevices.Location = new System.Drawing.Point(467, 220);
      this.cbPowerOffDevices.Name = "cbPowerOffDevices";
      this.cbPowerOffDevices.Size = new System.Drawing.Size(118, 94);
      this.cbPowerOffDevices.TabIndex = 25;
      // 
      // lWakeDevices
      // 
      this.lWakeDevices.AutoSize = true;
      this.lWakeDevices.Location = new System.Drawing.Point(345, 203);
      this.lWakeDevices.Name = "lWakeDevices";
      this.lWakeDevices.Size = new System.Drawing.Size(102, 13);
      this.lWakeDevices.TabIndex = 24;
      this.lWakeDevices.Text = "Wake when starting";
      // 
      // cbWakeDevices
      // 
      this.cbWakeDevices.Enabled = false;
      this.cbWakeDevices.FormattingEnabled = true;
      this.cbWakeDevices.Items.AddRange(new object[] {
            "0: TV",
            "1: Recorder 1",
            "2: Recorder 2",
            "3: Tuner 1",
            "4: Playback 1",
            "5: Audio system",
            "6: Tuner 2",
            "7: Tuner 3",
            "8: Playback 2",
            "9: Recorder 3",
            "A: Tuner 4",
            "B: Playback 3",
            "C: Reserved 1",
            "D: Reserved 2",
            "E: Free use",
            "F: Broadcast"});
      this.cbWakeDevices.Location = new System.Drawing.Point(337, 220);
      this.cbWakeDevices.Name = "cbWakeDevices";
      this.cbWakeDevices.Size = new System.Drawing.Size(118, 94);
      this.cbWakeDevices.TabIndex = 23;
      // 
      // cbPowerOffOnStandby
      // 
      this.cbPowerOffOnStandby.AutoSize = true;
      this.cbPowerOffOnStandby.Enabled = false;
      this.cbPowerOffOnStandby.Location = new System.Drawing.Point(9, 278);
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
      this.cbPowerOffScreensaver.Location = new System.Drawing.Point(9, 255);
      this.cbPowerOffScreensaver.Name = "cbPowerOffScreensaver";
      this.cbPowerOffScreensaver.Size = new System.Drawing.Size(301, 17);
      this.cbPowerOffScreensaver.TabIndex = 21;
      this.cbPowerOffScreensaver.Text = "Put devices in standby mode when activating screensaver";
      this.cbPowerOffScreensaver.UseVisualStyleBackColor = true;
      // 
      // cbActivateSource
      // 
      this.cbActivateSource.AutoSize = true;
      this.cbActivateSource.Enabled = false;
      this.cbActivateSource.Location = new System.Drawing.Point(9, 232);
      this.cbActivateSource.Name = "cbActivateSource";
      this.cbActivateSource.Size = new System.Drawing.Size(284, 17);
      this.cbActivateSource.TabIndex = 19;
      this.cbActivateSource.Text = "Make the media player the active source when starting";
      this.cbActivateSource.UseVisualStyleBackColor = true;
      // 
      // cbUseTVMenuLanguage
      // 
      this.cbUseTVMenuLanguage.AutoSize = true;
      this.cbUseTVMenuLanguage.Enabled = false;
      this.cbUseTVMenuLanguage.Location = new System.Drawing.Point(10, 209);
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
      this.lPlayerConfig.Location = new System.Drawing.Point(6, 182);
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
      this.cbDeviceType.Size = new System.Drawing.Size(157, 21);
      this.cbDeviceType.TabIndex = 14;
      this.cbDeviceType.Text = "Recorder";
      this.helpDeviceType.SetToolTip(this.cbDeviceType, "Set this to \'Player\' when your TV is having problems with \'Recorder\'");
      this.cbDeviceType.SelectedIndexChanged += new System.EventHandler(this.cbDeviceType_SelectedIndexChanged);
      // 
      // bClose
      // 
      this.bClose.Enabled = false;
      this.bClose.Location = new System.Drawing.Point(96, 330);
      this.bClose.Name = "bClose";
      this.bClose.Size = new System.Drawing.Size(125, 23);
      this.bClose.TabIndex = 13;
      this.bClose.Text = "Close";
      this.bClose.UseVisualStyleBackColor = true;
      this.bClose.Click += new System.EventHandler(this.bCancel_Click);
      // 
      // bSaveConfig
      // 
      this.bSaveConfig.Enabled = false;
      this.bSaveConfig.Location = new System.Drawing.Point(227, 330);
      this.bSaveConfig.Name = "bSaveConfig";
      this.bSaveConfig.Size = new System.Drawing.Size(125, 23);
      this.bSaveConfig.TabIndex = 12;
      this.bSaveConfig.Text = "Save configuration";
      this.bSaveConfig.UseVisualStyleBackColor = true;
      this.bSaveConfig.Click += new System.EventHandler(this.bSave_Click);
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
      this.lConnectedPhysicalAddress.Location = new System.Drawing.Point(340, 71);
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
      this.cbConnectedDevice.Size = new System.Drawing.Size(157, 21);
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
      // tbButtons
      // 
      this.tbButtons.Controls.Add(this.label1);
      this.tbButtons.Controls.Add(this.dgButtons);
      this.tbButtons.Location = new System.Drawing.Point(4, 22);
      this.tbButtons.Name = "tbButtons";
      this.tbButtons.Padding = new System.Windows.Forms.Padding(3);
      this.tbButtons.Size = new System.Drawing.Size(592, 359);
      this.tbButtons.TabIndex = 2;
      this.tbButtons.Text = "Button Configuration";
      this.tbButtons.UseVisualStyleBackColor = true;
      // 
      // label1
      // 
      this.label1.AutoSize = true;
      this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 20.25F, ((System.Drawing.FontStyle)((System.Drawing.FontStyle.Bold | System.Drawing.FontStyle.Italic))), System.Drawing.GraphicsUnit.Point, ((byte)(0)));
      this.label1.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(64)))), ((int)(((byte)(0)))));
      this.label1.Location = new System.Drawing.Point(118, 252);
      this.label1.Name = "label1";
      this.label1.Size = new System.Drawing.Size(354, 31);
      this.label1.TabIndex = 1;
      this.label1.Text = "NOT IMPLEMENTED YET";
      // 
      // dgButtons
      // 
      this.dgButtons.AllowUserToAddRows = false;
      this.dgButtons.AllowUserToDeleteRows = false;
      this.dgButtons.AllowUserToResizeColumns = false;
      this.dgButtons.AllowUserToResizeRows = false;
      this.dgButtons.AutoGenerateColumns = false;
      this.dgButtons.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.DisableResizing;
      this.dgButtons.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.CecButtonName});
      this.dgButtons.DataSource = this.cecButtonConfigBindingSource;
      this.dgButtons.Location = new System.Drawing.Point(7, 7);
      this.dgButtons.Name = "dgButtons";
      this.dgButtons.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
      this.dgButtons.Size = new System.Drawing.Size(579, 346);
      this.dgButtons.TabIndex = 0;
      this.dgButtons.CellFormatting += new System.Windows.Forms.DataGridViewCellFormattingEventHandler(this.dataGridView1_CellFormatting);
      // 
      // CecButtonName
      // 
      this.CecButtonName.DataPropertyName = "CecButtonName";
      this.CecButtonName.FillWeight = 260F;
      this.CecButtonName.HeaderText = "Button";
      this.CecButtonName.Name = "CecButtonName";
      this.CecButtonName.ReadOnly = true;
      this.CecButtonName.Width = 260;
      // 
      // cecButtonConfigBindingSource
      // 
      this.cecButtonConfigBindingSource.DataSource = typeof(CecConfigGui.CecButtonConfig);
      // 
      // tbTestCommands
      // 
      this.tbTestCommands.Controls.Add(this.bRescanDevices);
      this.tbTestCommands.Controls.Add(this.bMute);
      this.tbTestCommands.Controls.Add(this.bVolDown);
      this.tbTestCommands.Controls.Add(this.bVolUp);
      this.tbTestCommands.Controls.Add(this.bActivateSource);
      this.tbTestCommands.Controls.Add(this.bScan);
      this.tbTestCommands.Controls.Add(this.bStandby);
      this.tbTestCommands.Controls.Add(this.bSendImageViewOn);
      this.tbTestCommands.Controls.Add(this.lDestination);
      this.tbTestCommands.Controls.Add(this.cbCommandDestination);
      this.tbTestCommands.Location = new System.Drawing.Point(4, 22);
      this.tbTestCommands.Name = "tbTestCommands";
      this.tbTestCommands.Padding = new System.Windows.Forms.Padding(3);
      this.tbTestCommands.Size = new System.Drawing.Size(592, 359);
      this.tbTestCommands.TabIndex = 3;
      this.tbTestCommands.Text = "CEC tester";
      this.tbTestCommands.UseVisualStyleBackColor = true;
      // 
      // bRescanDevices
      // 
      this.bRescanDevices.Enabled = false;
      this.bRescanDevices.Location = new System.Drawing.Point(424, 65);
      this.bRescanDevices.Name = "bRescanDevices";
      this.bRescanDevices.Size = new System.Drawing.Size(150, 23);
      this.bRescanDevices.TabIndex = 9;
      this.bRescanDevices.Text = "Re-scan devices";
      this.bRescanDevices.UseVisualStyleBackColor = true;
      this.bRescanDevices.Click += new System.EventHandler(this.bRescanDevices_Click);
      // 
      // bMute
      // 
      this.bMute.Enabled = false;
      this.bMute.Location = new System.Drawing.Point(164, 65);
      this.bMute.Name = "bMute";
      this.bMute.Size = new System.Drawing.Size(150, 23);
      this.bMute.TabIndex = 8;
      this.bMute.Text = "Mute";
      this.bMute.UseVisualStyleBackColor = true;
      this.bMute.Click += new System.EventHandler(this.bMute_Click);
      // 
      // bVolDown
      // 
      this.bVolDown.Enabled = false;
      this.bVolDown.Location = new System.Drawing.Point(164, 36);
      this.bVolDown.Name = "bVolDown";
      this.bVolDown.Size = new System.Drawing.Size(150, 23);
      this.bVolDown.TabIndex = 7;
      this.bVolDown.Text = "Volume down";
      this.bVolDown.UseVisualStyleBackColor = true;
      this.bVolDown.Click += new System.EventHandler(this.bVolDown_Click);
      // 
      // bVolUp
      // 
      this.bVolUp.Enabled = false;
      this.bVolUp.Location = new System.Drawing.Point(164, 7);
      this.bVolUp.Name = "bVolUp";
      this.bVolUp.Size = new System.Drawing.Size(150, 23);
      this.bVolUp.TabIndex = 6;
      this.bVolUp.Text = "Volume up";
      this.bVolUp.UseVisualStyleBackColor = true;
      this.bVolUp.Click += new System.EventHandler(this.bVolUp_Click);
      // 
      // bActivateSource
      // 
      this.bActivateSource.Enabled = false;
      this.bActivateSource.Location = new System.Drawing.Point(8, 65);
      this.bActivateSource.Name = "bActivateSource";
      this.bActivateSource.Size = new System.Drawing.Size(150, 23);
      this.bActivateSource.TabIndex = 5;
      this.bActivateSource.Text = "Make device active";
      this.bActivateSource.UseVisualStyleBackColor = true;
      this.bActivateSource.Click += new System.EventHandler(this.bActivateSource_Click);
      // 
      // bScan
      // 
      this.bScan.Enabled = false;
      this.bScan.Location = new System.Drawing.Point(8, 94);
      this.bScan.Name = "bScan";
      this.bScan.Size = new System.Drawing.Size(150, 23);
      this.bScan.TabIndex = 4;
      this.bScan.Text = "Device information";
      this.bScan.UseVisualStyleBackColor = true;
      this.bScan.Click += new System.EventHandler(this.bScan_Click);
      // 
      // bStandby
      // 
      this.bStandby.Enabled = false;
      this.bStandby.Location = new System.Drawing.Point(8, 36);
      this.bStandby.Name = "bStandby";
      this.bStandby.Size = new System.Drawing.Size(150, 23);
      this.bStandby.TabIndex = 3;
      this.bStandby.Text = "Put device in standby";
      this.bStandby.UseVisualStyleBackColor = true;
      this.bStandby.Click += new System.EventHandler(this.bStandby_Click);
      // 
      // bSendImageViewOn
      // 
      this.bSendImageViewOn.Enabled = false;
      this.bSendImageViewOn.Location = new System.Drawing.Point(8, 7);
      this.bSendImageViewOn.Name = "bSendImageViewOn";
      this.bSendImageViewOn.Size = new System.Drawing.Size(150, 23);
      this.bSendImageViewOn.TabIndex = 2;
      this.bSendImageViewOn.Text = "Power on device";
      this.bSendImageViewOn.UseVisualStyleBackColor = true;
      this.bSendImageViewOn.Click += new System.EventHandler(this.bSendImageViewOn_Click);
      // 
      // lDestination
      // 
      this.lDestination.AutoSize = true;
      this.lDestination.Font = new System.Drawing.Font("Microsoft Sans Serif", 14.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
      this.lDestination.Location = new System.Drawing.Point(420, 3);
      this.lDestination.Name = "lDestination";
      this.lDestination.Size = new System.Drawing.Size(138, 24);
      this.lDestination.TabIndex = 1;
      this.lDestination.Text = "Target device";
      // 
      // cbCommandDestination
      // 
      this.cbCommandDestination.FormattingEnabled = true;
      this.cbCommandDestination.Items.AddRange(new object[] {
            "0: TV",
            "F: Broadcast"});
      this.cbCommandDestination.Location = new System.Drawing.Point(437, 30);
      this.cbCommandDestination.Name = "cbCommandDestination";
      this.cbCommandDestination.Size = new System.Drawing.Size(121, 21);
      this.cbCommandDestination.TabIndex = 0;
      this.cbCommandDestination.Text = "0: TV";
      this.cbCommandDestination.SelectedIndexChanged += new System.EventHandler(this.cbCommandDestination_SelectedIndexChanged);
      // 
      // LogOutput
      // 
      this.LogOutput.Controls.Add(this.bSaveLog);
      this.LogOutput.Controls.Add(this.bClearLog);
      this.LogOutput.Controls.Add(this.cbLogDebug);
      this.LogOutput.Controls.Add(this.cbLogTraffic);
      this.LogOutput.Controls.Add(this.cbLogNotice);
      this.LogOutput.Controls.Add(this.cbLogWarning);
      this.LogOutput.Controls.Add(this.cbLogError);
      this.LogOutput.Controls.Add(this.tbLog);
      this.LogOutput.Location = new System.Drawing.Point(4, 22);
      this.LogOutput.Name = "LogOutput";
      this.LogOutput.Padding = new System.Windows.Forms.Padding(3);
      this.LogOutput.Size = new System.Drawing.Size(592, 359);
      this.LogOutput.TabIndex = 1;
      this.LogOutput.Text = "Log Output";
      this.LogOutput.UseVisualStyleBackColor = true;
      // 
      // bSaveLog
      // 
      this.bSaveLog.Location = new System.Drawing.Point(430, 330);
      this.bSaveLog.Name = "bSaveLog";
      this.bSaveLog.Size = new System.Drawing.Size(75, 23);
      this.bSaveLog.TabIndex = 7;
      this.bSaveLog.Text = "Save";
      this.bSaveLog.UseVisualStyleBackColor = true;
      this.bSaveLog.Click += new System.EventHandler(this.bSaveLog_Click);
      // 
      // bClearLog
      // 
      this.bClearLog.Location = new System.Drawing.Point(511, 330);
      this.bClearLog.Name = "bClearLog";
      this.bClearLog.Size = new System.Drawing.Size(75, 23);
      this.bClearLog.TabIndex = 6;
      this.bClearLog.Text = "Clear";
      this.bClearLog.UseVisualStyleBackColor = true;
      this.bClearLog.Click += new System.EventHandler(this.bClearLog_Click);
      // 
      // cbLogDebug
      // 
      this.cbLogDebug.AutoSize = true;
      this.cbLogDebug.Checked = true;
      this.cbLogDebug.CheckState = System.Windows.Forms.CheckState.Checked;
      this.cbLogDebug.Location = new System.Drawing.Point(269, 336);
      this.cbLogDebug.Name = "cbLogDebug";
      this.cbLogDebug.Size = new System.Drawing.Size(58, 17);
      this.cbLogDebug.TabIndex = 5;
      this.cbLogDebug.Text = "Debug";
      this.cbLogDebug.UseVisualStyleBackColor = true;
      // 
      // cbLogTraffic
      // 
      this.cbLogTraffic.AutoSize = true;
      this.cbLogTraffic.Checked = true;
      this.cbLogTraffic.CheckState = System.Windows.Forms.CheckState.Checked;
      this.cbLogTraffic.Location = new System.Drawing.Point(207, 336);
      this.cbLogTraffic.Name = "cbLogTraffic";
      this.cbLogTraffic.Size = new System.Drawing.Size(56, 17);
      this.cbLogTraffic.TabIndex = 4;
      this.cbLogTraffic.Text = "Traffic";
      this.cbLogTraffic.UseVisualStyleBackColor = true;
      // 
      // cbLogNotice
      // 
      this.cbLogNotice.AutoSize = true;
      this.cbLogNotice.Checked = true;
      this.cbLogNotice.CheckState = System.Windows.Forms.CheckState.Checked;
      this.cbLogNotice.Location = new System.Drawing.Point(138, 336);
      this.cbLogNotice.Name = "cbLogNotice";
      this.cbLogNotice.Size = new System.Drawing.Size(62, 17);
      this.cbLogNotice.TabIndex = 3;
      this.cbLogNotice.Text = "Notices";
      this.cbLogNotice.UseVisualStyleBackColor = true;
      // 
      // cbLogWarning
      // 
      this.cbLogWarning.AutoSize = true;
      this.cbLogWarning.Checked = true;
      this.cbLogWarning.CheckState = System.Windows.Forms.CheckState.Checked;
      this.cbLogWarning.Location = new System.Drawing.Point(66, 336);
      this.cbLogWarning.Name = "cbLogWarning";
      this.cbLogWarning.Size = new System.Drawing.Size(66, 17);
      this.cbLogWarning.TabIndex = 2;
      this.cbLogWarning.Text = "Warning";
      this.cbLogWarning.UseVisualStyleBackColor = true;
      // 
      // cbLogError
      // 
      this.cbLogError.AutoSize = true;
      this.cbLogError.Checked = true;
      this.cbLogError.CheckState = System.Windows.Forms.CheckState.Checked;
      this.cbLogError.Location = new System.Drawing.Point(7, 336);
      this.cbLogError.Name = "cbLogError";
      this.cbLogError.Size = new System.Drawing.Size(53, 17);
      this.cbLogError.TabIndex = 1;
      this.cbLogError.Text = "Errors";
      this.cbLogError.UseVisualStyleBackColor = true;
      // 
      // tbLog
      // 
      this.tbLog.Location = new System.Drawing.Point(6, 6);
      this.tbLog.Multiline = true;
      this.tbLog.Name = "tbLog";
      this.tbLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
      this.tbLog.Size = new System.Drawing.Size(580, 318);
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
      // cbSendInactiveSource
      // 
      this.cbSendInactiveSource.AutoSize = true;
      this.cbSendInactiveSource.Enabled = false;
      this.cbSendInactiveSource.Location = new System.Drawing.Point(9, 301);
      this.cbSendInactiveSource.Name = "cbSendInactiveSource";
      this.cbSendInactiveSource.Size = new System.Drawing.Size(261, 17);
      this.cbSendInactiveSource.TabIndex = 32;
      this.cbSendInactiveSource.Text = "Send \'inactive source\' when shutting down XBMC";
      this.cbSendInactiveSource.UseVisualStyleBackColor = true;
      // 
      // CecConfigGUI
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(624, 442);
      this.Controls.Add(this.lStatus);
      this.Controls.Add(this.pProgress);
      this.Controls.Add(this.tabControl1);
      this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
      this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
      this.MaximizeBox = false;
      this.Name = "CecConfigGUI";
      this.Text = "Pulse-Eight USB-CEC Adapter";
      this.tabControl1.ResumeLayout(false);
      this.Configuration.ResumeLayout(false);
      this.Configuration.PerformLayout();
      this.tbButtons.ResumeLayout(false);
      this.tbButtons.PerformLayout();
      ((System.ComponentModel.ISupportInitialize)(this.dgButtons)).EndInit();
      ((System.ComponentModel.ISupportInitialize)(this.cecButtonConfigBindingSource)).EndInit();
      this.tbTestCommands.ResumeLayout(false);
      this.tbTestCommands.PerformLayout();
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
    private System.Windows.Forms.Label lConnectedDevice;
    private System.Windows.Forms.ComboBox cbConnectedDevice;
    private System.Windows.Forms.TextBox tbPhysicalAddress;
    private System.Windows.Forms.ProgressBar pProgress;
    private System.Windows.Forms.Label lStatus;
    private System.Windows.Forms.TextBox tbLog;
    private System.Windows.Forms.ComboBox cbPortNumber;
    private System.Windows.Forms.Button bClose;
    private System.Windows.Forms.Button bSaveConfig;
    private System.Windows.Forms.ComboBox cbDeviceType;
    private System.Windows.Forms.CheckBox cbPowerOffOnStandby;
    private System.Windows.Forms.CheckBox cbPowerOffScreensaver;
    private System.Windows.Forms.CheckBox cbActivateSource;
    private System.Windows.Forms.CheckBox cbUseTVMenuLanguage;
    private System.Windows.Forms.Label lPlayerConfig;
    private System.Windows.Forms.ToolTip helpPortNumber;
    private System.Windows.Forms.ToolTip helpConnectedHDMIDevice;
    private System.Windows.Forms.ToolTip helpDeviceType;
    private System.Windows.Forms.ToolTip helpPhysicalAddress;
    private System.Windows.Forms.TabPage tbButtons;
    private System.Windows.Forms.CheckBox cbLogDebug;
    private System.Windows.Forms.CheckBox cbLogTraffic;
    private System.Windows.Forms.CheckBox cbLogNotice;
    private System.Windows.Forms.CheckBox cbLogWarning;
    private System.Windows.Forms.CheckBox cbLogError;
    private System.Windows.Forms.Button bClearLog;
    private System.Windows.Forms.Button bSaveLog;
    private System.Windows.Forms.DataGridView dgButtons;
    private System.Windows.Forms.BindingSource cecButtonConfigBindingSource;
    private System.Windows.Forms.DataGridViewTextBoxColumn CecButtonName;
    private System.Windows.Forms.DataGridViewTextBoxColumn playerButtonDataGridViewTextBoxColumn;
    private System.Windows.Forms.TabPage tbTestCommands;
    private System.Windows.Forms.ComboBox cbCommandDestination;
    private System.Windows.Forms.Button bStandby;
    private System.Windows.Forms.Button bSendImageViewOn;
    private System.Windows.Forms.Label lDestination;
    private System.Windows.Forms.Button bActivateSource;
    private System.Windows.Forms.Button bScan;
    private System.Windows.Forms.Button bMute;
    private System.Windows.Forms.Button bVolDown;
    private System.Windows.Forms.Button bVolUp;
    private System.Windows.Forms.Label label1;
    private System.Windows.Forms.Label lWakeDevices;
    private System.Windows.Forms.CheckedListBox cbWakeDevices;
    private System.Windows.Forms.Label lPowerOff;
    private System.Windows.Forms.CheckedListBox cbPowerOffDevices;
    private System.Windows.Forms.CheckBox cbVendorOverride;
    private System.Windows.Forms.ComboBox cbVendorId;
    private System.Windows.Forms.Button bReloadConfig;
    private System.Windows.Forms.Button bRescanDevices;
    private System.Windows.Forms.Label lConnectedPhysicalAddress;
    private System.Windows.Forms.Label lAdapterConfig;
    private System.Windows.Forms.CheckBox cbOverrideAddress;
    private System.Windows.Forms.CheckBox cbSendInactiveSource;
  }
}