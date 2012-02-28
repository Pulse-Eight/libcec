using System;
using System.Windows.Forms;
using CecSharp;

namespace CecConfigGui
{
  public partial class DeviceInformation : AsyncForm
  {
    public DeviceInformation(CecConfigGUI gui, CecLogicalAddress address, ref LibCecSharp lib,
      bool devicePresent, CecVendorId vendor, bool isActiveSource, ushort physicalAddress,
      CecVersion version, CecPowerStatus power, string osdName, string menuLanguage)
    {
      Gui = gui;
      Lib = lib;
      Address = address;
      InitializeComponent();
      lDevice.Text = lib.ToString(address);
      lLogicalAddress.Text = String.Format("{0,1:X}", (int)address);
      Update(devicePresent, vendor, isActiveSource, physicalAddress, version, power, osdName, menuLanguage);
    }

    public void Update(bool devicePresent, CecVendorId vendor, bool isActiveSource, ushort physicalAddress,
      CecVersion version, CecPowerStatus power, string osdName, string menuLanguage)
    {
      SetControlText(lPhysicalAddress, String.Format("{0,4:X}", physicalAddress));
      SetControlText(lDevicePresent, devicePresent ? "yes" : "no");
      SetControlVisible(lActiveSource, isActiveSource);
      SetControlVisible(lInactiveSource, !isActiveSource);
      SetControlText(lVendor, vendor != CecVendorId.Unknown ? Lib.ToString(vendor) : "unknown");
      SetControlText(lCecVersion, Lib.ToString(version));
      SetControlText(lPowerStatus, Lib.ToString(power));
      SetControlText(lOsdName, osdName);
      SetControlText(lMenuLanguage, menuLanguage);
      SetControlText(this, "Device: " + osdName);
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


    private void button1_Click(object sender, EventArgs e)
    {
      Gui.UpdateInfoPanel(this);
    }

    public CecLogicalAddress Address
    {
      private set;
      get;
    }
    private CecConfigGUI Gui;
    private LibCecSharp Lib;
  }
}
