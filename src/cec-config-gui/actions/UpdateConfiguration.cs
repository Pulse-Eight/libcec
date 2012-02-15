using System;
using System.Collections.Generic;
using System.Text;
using CecSharp;

namespace CecConfigGui.actions
{
  class UpdateConfiguration : UpdateProcess
  {
    public UpdateConfiguration(ref LibCecSharp lib, LibCECConfiguration config)
    {
      Lib = lib;
      Config = config;
    }

    public override void Process()
    {
      SendEvent(UpdateEventType.ProgressBar, 10);
      SendEvent(UpdateEventType.StatusText, "Setting the new configuration");

      Lib.SetConfiguration(Config);

      SendEvent(UpdateEventType.ProgressBar, 100);
      SendEvent(UpdateEventType.StatusText, "Ready.");
    }

    private LibCecSharp Lib;
    private LibCECConfiguration Config;
  }
}
