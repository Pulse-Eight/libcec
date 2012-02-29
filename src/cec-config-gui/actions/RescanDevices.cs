using CecSharp;

namespace CecConfigGui.actions
{
  class RescanDevices : UpdateProcess
  {
    public RescanDevices(ref LibCecSharp lib)
    {
      Lib = lib;
    }

    public override void Process()
    {
      SendEvent(UpdateEventType.ProgressBar, 10);
      SendEvent(UpdateEventType.StatusText, "Polling active devices");
      Lib.RescanActiveDevices();

      SendEvent(UpdateEventType.ProgressBar, 80);
      SendEvent(UpdateEventType.StatusText, "Refreshing device list");
      SendEvent(UpdateEventType.PollDevices);

      SendEvent(UpdateEventType.ProgressBar, 100);
      SendEvent(UpdateEventType.StatusText, "Ready.");
    }

    private LibCecSharp Lib;
  }
}
