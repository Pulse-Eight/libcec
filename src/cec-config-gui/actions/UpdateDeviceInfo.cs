using CecSharp;

namespace CecConfigGui.actions
{
  class UpdateDeviceInfo : ShowDeviceInfo
  {
    public UpdateDeviceInfo(CecConfigGUI gui, ref LibCecSharp lib, DeviceInformation dialog) :
      base(gui, ref lib, dialog.Address)
    {
      Dialog = dialog;
    }

    public override void ShowDialog(CecConfigGUI gui, CecLogicalAddress address, ref LibCecSharp lib,
      bool devicePresent, CecVendorId vendor, bool isActiveSource, ushort physicalAddress,
      CecVersion version, CecPowerStatus power, string osdName, string menuLanguage)
    {
      Dialog.Update(devicePresent, vendor, isActiveSource, physicalAddress, version, power, osdName, menuLanguage);
    }

    private DeviceInformation Dialog;
  }
}
