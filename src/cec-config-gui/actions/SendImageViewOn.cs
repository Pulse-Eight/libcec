using CecSharp;

namespace CecConfigGui.actions
{
  class SendImageViewOn : UpdateProcess
  {
    public SendImageViewOn(ref LibCecSharp lib, CecLogicalAddress address)
    {
      Lib = lib;
      Address = address;
    }

    public override void Process()
    {
      SendEvent(UpdateEventType.StatusText, "Sending the 'power on' command to " + Lib.ToString(Address) + "...");
      SendEvent(UpdateEventType.ProgressBar, 50);

      bool bResult = Lib.PowerOnDevices(Address);
      SendEvent(UpdateEventType.StatusText, bResult ? "Command sent successfully." : "The 'image view on' command was not acked.");
      SendEvent(UpdateEventType.ProgressBar, 100);
    }

    private LibCecSharp Lib;
    private CecLogicalAddress Address;
  }
}
