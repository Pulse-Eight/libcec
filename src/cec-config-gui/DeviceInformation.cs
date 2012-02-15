using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using CecSharp;

namespace CecConfigGui
{
  public partial class DeviceInformation : Form
  {
    public DeviceInformation(CecConfigGUI gui, CecLogicalAddress address, ref LibCecSharp lib,
      bool devicePresent, CecVendorId vendor, bool isActiveSource, ushort physicalAddress,
      CecVersion version, CecPowerStatus power, string osdName, string menuLanguage)
    {
      Gui = gui;
      Lib = lib;
      Address = address;
      InitializeComponent();
      this.lDevice.Text = lib.ToString(address);
      this.lLogicalAddress.Text = String.Format("{0,1:X}", (int)address);
      this.lPhysicalAddress.Text = String.Format("{0,4:X}", physicalAddress);
      this.lDevicePresent.Text = devicePresent ? "yes" : "no";
      this.lActiveSource.Visible = isActiveSource;
      this.lInactiveSource.Visible = !isActiveSource;
      this.lVendor.Text = vendor != CecVendorId.Unknown ? lib.ToString(vendor) : "unknown";
      this.lCecVersion.Text = lib.ToString(version);
      this.lPowerStatus.Text = lib.ToString(power);
      bool isPoweredOn = (power == CecPowerStatus.On || power == CecPowerStatus.InTransitionStandbyToOn);
      this.lOsdName.Text = osdName;
      this.lMenuLanguage.Text = menuLanguage;
      this.Text = "Device: " + osdName;
    }

    delegate void SetControlVisibleCallback(Control control, bool val);
    private void SetControlVisible(Control control, bool val)
    {
      if (control.InvokeRequired)
      {
        SetControlVisibleCallback d = new SetControlVisibleCallback(SetControlVisible);
        try
        {
          this.Invoke(d, new object[] { control, val });
        }
        catch (Exception) { }
      }
      else
      {
        control.Visible = val;
      }
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

    private void lInactiveSource_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
    {
      SetControlVisible(lInactiveSource, false);
      SetControlVisible(lActiveSource, true);
      Gui.ActivateSource(Address);
    }


    private void lStandby_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
    {
      LinkLabel label = sender as LinkLabel;
      bool sendPowerOn = label.Text != Lib.ToString(CecPowerStatus.InTransitionStandbyToOn) &&
        label.Text != Lib.ToString(CecPowerStatus.On);

      SetControlText(lPowerStatus, Lib.ToString(sendPowerOn ? CecPowerStatus.On : CecPowerStatus.Standby));
      if (sendPowerOn)
        Gui.SendImageViewOn(Address);
      else
        Gui.SendStandby(Address);
    }

    private CecLogicalAddress Address;
    private CecConfigGUI Gui;
    private LibCecSharp Lib;
  }
}
