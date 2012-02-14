using System;
using System.Collections.Generic;
using System.Text;
using CecSharp;

namespace CecConfigGui.actions
{
  class SendStandby : UpdateProcess
  {
    public SendStandby(ref LibCecSharp lib, CecLogicalAddress address)
    {
      Lib = lib;
      Address = address;
    }

    public override void Process()
    {
      SendEvent(UpdateEventType.StatusText, "Sending the 'standby' command to " + Lib.ToString(Address) + "...");
      SendEvent(UpdateEventType.ProgressBar, 50);

      bool bResult = Lib.StandbyDevices(Address);
      SendEvent(UpdateEventType.StatusText, bResult ? "Command sent successfully." : "The device could not be powered on.");
      SendEvent(UpdateEventType.ProgressBar, 100);
    }

    private LibCecSharp Lib;
    private CecLogicalAddress Address;
  }
}
