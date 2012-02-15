using CecSharp;
using System.Windows.Forms;

namespace CecConfigGui.actions
{
  class RescanDevices : UpdateProcess
  {
    public override void Process()
    {
      SendEvent(UpdateEventType.ProgressBar, 10);
      SendEvent(UpdateEventType.StatusText, "Polling active devices");
      SendEvent(UpdateEventType.PollDevices);

      SendEvent(UpdateEventType.ProgressBar, 100);
      SendEvent(UpdateEventType.StatusText, "Ready.");
    }
  }
}
