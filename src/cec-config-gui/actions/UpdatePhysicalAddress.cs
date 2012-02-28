using CecSharp;

namespace CecConfigGui.actions
{
  class UpdatePhysicalAddress : UpdateProcess
  {
    public UpdatePhysicalAddress(ref LibCecSharp lib, ushort physicalAddress)
    {
      Lib = lib;
      PhysicalAddress = physicalAddress;
    }

    public override void Process()
    {
      SendEvent(UpdateEventType.BaseDevicePhysicalAddress, 0);
      SendEvent(UpdateEventType.StatusText, "Setting new configuration...");
      SendEvent(UpdateEventType.ProgressBar, 25);

      if (!Lib.SetPhysicalAddress(PhysicalAddress))
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

    protected LibCecSharp Lib;
    protected ushort PhysicalAddress;
  }
}
