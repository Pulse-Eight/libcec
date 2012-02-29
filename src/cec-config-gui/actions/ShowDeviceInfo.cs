using CecSharp;

namespace CecConfigGui.actions
{
  class ShowDeviceInfo : UpdateProcess
  {
    public ShowDeviceInfo(CecConfigGUI gui, ref LibCecSharp lib, CecLogicalAddress address)
    {
      Gui = gui;
      Lib = lib;
      Address = address;
    }

    public virtual void ShowDialog(CecConfigGUI gui, CecLogicalAddress address, ref LibCecSharp lib,
      bool devicePresent, CecVendorId vendor, bool isActiveSource, ushort physicalAddress,
      CecVersion version, CecPowerStatus power, string osdName, string menuLanguage)
    {
      DeviceInformation di = new DeviceInformation(Gui, Address, ref Lib, devicePresent, vendor, isActiveSource, physicalAddress, version, power, osdName, menuLanguage);
      Gui.DisplayDialog(di, false);
    }

    public override void Process()
    {
      CecVendorId vendor = CecVendorId.Unknown;
      bool isActiveSource = false;
      ushort physicalAddress = 0xFFFF;
      CecVersion version = CecVersion.Unknown;
      CecPowerStatus power = CecPowerStatus.Unknown;
      string osdName = "unknown";
      string menuLanguage = "unknown";

      SendEvent(UpdateEventType.StatusText, "Checking device presense...");
      SendEvent(UpdateEventType.ProgressBar, 10);

      bool devicePresent = Lib.IsActiveDevice(Address);
      if (devicePresent)
      {
        SendEvent(UpdateEventType.StatusText, "Requesting the vendor ID...");
        SendEvent(UpdateEventType.ProgressBar, 20);
        vendor = Lib.GetDeviceVendorId(Address);

        SendEvent(UpdateEventType.StatusText, "Requesting active source state...");
        SendEvent(UpdateEventType.ProgressBar, 30);
        isActiveSource = Lib.IsActiveSource(Address);

        SendEvent(UpdateEventType.StatusText, "Requesting physical address...");
        SendEvent(UpdateEventType.ProgressBar, 40);
        physicalAddress = Lib.GetDevicePhysicalAddress(Address);

        SendEvent(UpdateEventType.StatusText, "Requesting CEC version...");
        SendEvent(UpdateEventType.ProgressBar, 50);
        version = Lib.GetDeviceCecVersion(Address);

        SendEvent(UpdateEventType.StatusText, "Requesting power status...");
        SendEvent(UpdateEventType.ProgressBar, 60);
        power = Lib.GetDevicePowerStatus(Address);

        SendEvent(UpdateEventType.StatusText, "Requesting OSD name...");
        SendEvent(UpdateEventType.ProgressBar, 70);
        osdName = Lib.GetDeviceOSDName(Address);

        SendEvent(UpdateEventType.StatusText, "Requesting menu language...");
        SendEvent(UpdateEventType.ProgressBar, 80);
        menuLanguage = Lib.GetDeviceMenuLanguage(Address);
      }

      SendEvent(UpdateEventType.StatusText, "Showing device information");
      SendEvent(UpdateEventType.ProgressBar, 90);
      SendEvent(UpdateEventType.ProcessCompleted, true);

      ShowDialog(Gui, Address, ref Lib, devicePresent, vendor, isActiveSource, physicalAddress, version, power, osdName, menuLanguage);

      SendEvent(UpdateEventType.StatusText, "Ready.");
      SendEvent(UpdateEventType.ProgressBar, 100);
    }

    private CecConfigGUI Gui;
    private LibCecSharp Lib;
    private CecLogicalAddress Address;
  }
}
