using CecSharp;
using System.Windows.Forms;

namespace CecConfigGui.actions
{
  class ConnectToDevice : UpdateProcess
  {
    public ConnectToDevice(ref LibCecSharp lib, LibCECConfiguration config)
    {
      Lib = lib;
      Config = config;
    }

    public override void Process()
    {
      SendEvent(UpdateEventType.StatusText, "Detecting TV vendor...");
      SendEvent(UpdateEventType.ProgressBar, 25);
      SendEvent(UpdateEventType.TVVendorId, (int)Lib.GetDeviceVendorId(CecLogicalAddress.Tv));

      SendEvent(UpdateEventType.StatusText, "Detecting menu language...");
      SendEvent(UpdateEventType.ProgressBar, 40);
      SendEvent(UpdateEventType.MenuLanguage, Lib.GetDeviceMenuLanguage(CecLogicalAddress.Tv));

      SendEvent(UpdateEventType.ProgressBar, 50);
      SendEvent(UpdateEventType.StatusText, "Detecting AVR devices...");

      bool hasAVRDevice = Lib.IsActiveDevice(CecLogicalAddress.AudioSystem);
      SendEvent(UpdateEventType.HasAVRDevice, hasAVRDevice);

      if (hasAVRDevice)
      {
        SendEvent(UpdateEventType.ProgressBar, 75);
        SendEvent(UpdateEventType.StatusText, "Detecting AVR vendor...");
        SendEvent(UpdateEventType.AVRVendorId, (int)Lib.GetDeviceVendorId(CecLogicalAddress.AudioSystem));
      }

      if (!Lib.GetDevicePowerStatus(CecLogicalAddress.Tv).Equals(CecPowerStatus.On))
      {
        SendEvent(UpdateEventType.ProgressBar, 80);
        SendEvent(UpdateEventType.StatusText, "Activating the source...");
        Lib.SetActiveSource(CecDeviceType.Reserved);
      }

      SendEvent(UpdateEventType.ProgressBar, 90);
      SendEvent(UpdateEventType.StatusText, "Reading device configuration...");

      Lib.GetCurrentConfiguration(Config);
      SendEvent(Config);

      SendEvent(UpdateEventType.ProgressBar, 100);
      SendEvent(UpdateEventType.StatusText, "Ready.");
    }

    private LibCecSharp Lib;
    private LibCECConfiguration Config;
  }
}
