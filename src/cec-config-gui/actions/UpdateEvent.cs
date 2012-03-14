using System;
using CecSharp;

namespace CecConfigGui
{
  public enum UpdateEventType
  {
    ProcessCompleted,
    StatusText,
    ProgressBar,
    TVVendorId,
    BaseDevicePhysicalAddress,
    BaseDevice,
    HDMIPort,
    PhysicalAddress,
    HasAVRDevice,
    AVRVendorId,
    Configuration,
    MenuLanguage,
    PollDevices,
    ExitApplication
  }

  public class UpdateEvent : EventArgs
  {
    public UpdateEvent(UpdateEventType type)
    {
      Type = type;
    }

    public UpdateEvent(UpdateEventType type, bool value)
    {
      Type = type;
      BoolValue = value;
    }

    public UpdateEvent(UpdateEventType type, int value)
    {
      Type = type;
      IntValue = value;
    }

    public UpdateEvent(UpdateEventType type, string value)
    {
      Type = type;
      StringValue = value;
    }

    public UpdateEvent(LibCECConfiguration config)
    {
      Type = UpdateEventType.Configuration;
      ConfigValue = config;
    }

    public UpdateEventType     Type;
    public bool                BoolValue   = false;
    public int                 IntValue    = -1;
    public string              StringValue = String.Empty;
    public LibCECConfiguration ConfigValue = null;
  }

  public abstract class UpdateProcess
  {
      public void SendEvent(UpdateEventType type)
    {
      EventHandler<UpdateEvent> temp = EventHandler;
      if (temp != null)
        temp(this, new UpdateEvent(type));
    }

    public void SendEvent(UpdateEventType type, bool value)
    {
      EventHandler<UpdateEvent> temp = EventHandler;
      if (temp != null)
        temp(this, new UpdateEvent(type, value));
    }

    public void SendEvent(UpdateEventType type, int value)
    {
      EventHandler<UpdateEvent> temp = EventHandler;
      if (temp != null)
        temp(this, new UpdateEvent(type, value));
    }

    public void SendEvent(UpdateEventType type, string value)
    {
      EventHandler<UpdateEvent> temp = EventHandler;
      if (temp != null)
        temp(this, new UpdateEvent(type, value));
    }

    public void SendEvent(LibCECConfiguration config)
    {
      EventHandler<UpdateEvent> temp = EventHandler;
      if (temp != null)
        temp(this, new UpdateEvent(config));
    }

    public void Run()
    {
      Process();
      SendEvent(UpdateEventType.ProcessCompleted, true);
    }

    public abstract void Process();
    public event EventHandler<UpdateEvent> EventHandler;
  }
}
