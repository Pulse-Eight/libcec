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
    public DeviceInformation(CecLogicalAddress address, ref LibCecSharp lib,
      bool devicePresent, CecVendorId vendor, bool isActiveSource, ushort physicalAddress,
      CecVersion version, CecPowerStatus power, string osdName, string menuLanguage)
    {
      InitializeComponent();
      this.lDevice.Text = lib.ToString(address);
      this.lLogicalAddress.Text = "#" + (int)address;
      this.lPhysicalAddress.Text = physicalAddress.ToString();
      this.lDevicePresent.Text = devicePresent ? "yes" : "no";
      this.lActiveSource.Text = isActiveSource ? "yes" : "no";
      this.lVendor.Text = lib.ToString(vendor) + " (" + (UInt64)vendor +")";
      this.lCecVersion.Text = lib.ToString(version);
      this.lPowerStatus.Text = lib.ToString(power);
      this.lOsdName.Text = osdName;
      this.lMenuLanguage.Text = menuLanguage;
    }
  }
}
