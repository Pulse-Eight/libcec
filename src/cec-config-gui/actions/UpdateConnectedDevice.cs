using CecSharp;

namespace CecConfigGui.actions
{
  class UpdateConnectedDevice : UpdateProcess
  {
    public UpdateConnectedDevice(ref LibCecSharp lib, CecLogicalAddress address, int portNumber)
    {
      Lib = lib;
      Address = address;
      PortNumber = portNumber;
    }

    public override void Process()
    {
      SendEvent(UpdateEventType.StatusText, "Requesting physical address...");
      SendEvent(UpdateEventType.ProgressBar, 0);

      ushort iPhysicalAddress = Lib.GetDevicePhysicalAddress(Address);
      SendEvent(UpdateEventType.BaseDevicePhysicalAddress, iPhysicalAddress);

      SendEvent(UpdateEventType.StatusText, "Setting new configuration...");
      SendEvent(UpdateEventType.ProgressBar, 25);

      if (!Lib.SetHDMIPort(Address, (byte)PortNumber))
      {
        SendEvent(UpdateEventType.StatusText, "Could not activate the new source");
      }
      else
      {
        LibCECConfiguration config = new LibCECConfiguration();
        Lib.GetCurrentConfiguration(config);

        SendEvent(UpdateEventType.StatusText, "Activating source...");
        SendEvent(UpdateEventType.ProgressBar, 50);
        Lib.SetActiveSource(config.DeviceTypes.Types[0]);

        SendEvent(UpdateEventType.StatusText, "Reading configuration...");
        SendEvent(UpdateEventType.ProgressBar, 75);
        Lib.GetCurrentConfiguration(config);

        SendEvent(config);
        SendEvent(UpdateEventType.StatusText, "Ready.");
      }
      SendEvent(UpdateEventType.ProgressBar, 100);
    }

    private LibCecSharp       Lib;
    private CecLogicalAddress Address;
    private int               PortNumber;
  }
}
