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
      SendEvent(UpdateEventType.StatusText, "Opening connection...");
      SendEvent(UpdateEventType.ProgressBar, 10);

      //TODO read the com port setting from the configuration
      CecAdapter[] adapters = Lib.FindAdapters(string.Empty);
      if (adapters.Length == 0)
      {
        DialogResult result = MessageBox.Show("Could not detect to any CEC adapter. Please check your configuration. Do you want to try again?", "Pulse-Eight USB-CEC Adapter", MessageBoxButtons.YesNo);
        if (result == DialogResult.No)
        {
          SendEvent(UpdateEventType.ExitApplication);
          return;
        }
        else
          adapters = Lib.FindAdapters(string.Empty);
      }

      while (!Lib.Open(adapters[0].ComPort, 10000))
      {
        DialogResult result = MessageBox.Show("Could not connect to any CEC adapter. Please check your configuration. Do you want to try again?", "Pulse-Eight USB-CEC Adapter", MessageBoxButtons.YesNo);
        if (result == DialogResult.No)
        {
          SendEvent(UpdateEventType.ExitApplication);
          return;
        }
      }

      SendEvent(UpdateEventType.ProgressBar, 20);
      SendEvent(UpdateEventType.StatusText, "Sending power on commands...");
      Lib.PowerOnDevices(CecLogicalAddress.Broadcast);

      SendEvent(UpdateEventType.StatusText, "Detecting TV vendor...");
      SendEvent(UpdateEventType.ProgressBar, 30);
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
        SendEvent(UpdateEventType.ProgressBar, 60);
        SendEvent(UpdateEventType.StatusText, "Detecting AVR vendor...");
        SendEvent(UpdateEventType.AVRVendorId, (int)Lib.GetDeviceVendorId(CecLogicalAddress.AudioSystem));
      }

      if (!Lib.GetDevicePowerStatus(CecLogicalAddress.Tv).Equals(CecPowerStatus.On))
      {
        SendEvent(UpdateEventType.ProgressBar, 70);
        SendEvent(UpdateEventType.StatusText, "Activating the source...");
        Lib.SetActiveSource(CecDeviceType.Reserved);
      }

      SendEvent(UpdateEventType.ProgressBar, 80);
      SendEvent(UpdateEventType.StatusText, "Reading device configuration...");

      Lib.GetCurrentConfiguration(Config);
      SendEvent(Config);

      SendEvent(UpdateEventType.ProgressBar, 90);
      SendEvent(UpdateEventType.StatusText, "Polling active devices");
      SendEvent(UpdateEventType.PollDevices);

      SendEvent(UpdateEventType.ProgressBar, 100);
      SendEvent(UpdateEventType.StatusText, "Ready.");
    }

    private LibCecSharp Lib;
    private LibCECConfiguration Config;
  }
}
