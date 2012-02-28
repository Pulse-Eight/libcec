using CecSharp;

namespace CecConfigGui.actions
{
  class SendActivateSource : UpdateProcess
  {
    public SendActivateSource(ref LibCecSharp lib, CecLogicalAddress address)
    {
      Lib = lib;
      Address = address;
    }

    public override void Process()
    {
      SendEvent(UpdateEventType.StatusText, "Sending the 'activate source' command to " + Lib.ToString(Address) + "...");
      SendEvent(UpdateEventType.ProgressBar, 50);

      bool bResult = Lib.SetStreamPath(Address);
      SendEvent(UpdateEventType.StatusText, bResult ? "Command sent successfully." : "The 'active source' command was not acked.");
      SendEvent(UpdateEventType.ProgressBar, 100);
    }

    private LibCecSharp Lib;
    private CecLogicalAddress Address;
  }
}
