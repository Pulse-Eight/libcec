/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2025 Pulse-Eight Limited.  All rights reserved.
 * libCEC(R) is an original work, containing original code.
 *
 * libCEC(R) is a trademark of Pulse-Eight Limited.
 *
 * This program is dual-licensed; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * Alternatively, you can license this library under a commercial license,
 * please contact Pulse-Eight Licensing for more information.
 *
 * For more information contact:
 * Pulse-Eight Licensing       <license@pulse-eight.com>
 *     http://www.pulse-eight.com/
 *     http://www.pulse-eight.net/
 */

using System;
using System.Runtime.InteropServices;

namespace CecSharp
{
  /// <summary>
  /// The callback methods that libCEC uses. Derive from this class and override
  /// the Receive*/Source/Configuration methods to handle events from libCEC.
  ///
  /// This is the pure-C# replacement for the C++/CLI CecCallbackMethods. It keeps
  /// managed delegates alive for the lifetime of the object (they must not be
  /// collected while native code holds their function pointers) and builds an
  /// ICECCallbacks table in unmanaged memory that is handed to libCEC.
  /// </summary>
  public class CecCallbackMethods : IDisposable
  {
    public CecCallbackMethods()
    {
      // Keep strong references so the GC can't reclaim the delegates while the
      // native side still holds their function pointers.
      _logDelegate = OnLogMessage;
      _keyDelegate = OnKeyPress;
      _commandDelegate = OnCommand;
      _configDelegate = OnConfig;
      _alertDelegate = OnAlert;
      _menuDelegate = OnMenu;
      _sourceActivatedDelegate = OnSourceActivated;

      var cb = new ICECCallbacks
      {
        logMessage = Marshal.GetFunctionPointerForDelegate(_logDelegate),
        keyPress = Marshal.GetFunctionPointerForDelegate(_keyDelegate),
        commandReceived = Marshal.GetFunctionPointerForDelegate(_commandDelegate),
        configurationChanged = Marshal.GetFunctionPointerForDelegate(_configDelegate),
        alert = Marshal.GetFunctionPointerForDelegate(_alertDelegate),
        menuStateChanged = Marshal.GetFunctionPointerForDelegate(_menuDelegate),
        sourceActivated = Marshal.GetFunctionPointerForDelegate(_sourceActivatedDelegate),
        commandHandler = IntPtr.Zero,
      };

      _callbacks = Marshal.AllocHGlobal(Marshal.SizeOf<ICECCallbacks>());
      Marshal.StructureToPtr(cb, _callbacks, false);
    }

    ~CecCallbackMethods()
    {
      Destroy();
    }

    /// <summary>Pointer to the native ICECCallbacks table.</summary>
    internal IntPtr Callbacks
    {
      get { return _callbacks; }
    }

    public void Dispose()
    {
      Destroy();
      GC.SuppressFinalize(this);
    }

    internal void Destroy()
    {
      if (_callbacks != IntPtr.Zero)
      {
        Marshal.FreeHGlobal(_callbacks);
        _callbacks = IntPtr.Zero;
      }
    }

    // Overridable handlers -------------------------------------------------

    /// <summary>Called by libCEC to send back a log message to the application.</summary>
    public virtual int ReceiveLogMessage(CecLogMessage message)
    {
      return 0;
    }

    /// <summary>Called by libCEC to send back a key press or release to the application.</summary>
    public virtual int ReceiveKeypress(CecKeypress key)
    {
      return 0;
    }

    /// <summary>Called by libCEC to send back raw CEC data to the application.</summary>
    public virtual int ReceiveCommand(CecCommand command)
    {
      return 0;
    }

    /// <summary>Called by libCEC to send back an updated configuration to the application.</summary>
    public virtual int ConfigurationChanged(LibCECConfiguration config)
    {
      return 0;
    }

    /// <summary>Called by libCEC to send back an alert message to the application.</summary>
    public virtual int ReceiveAlert(CecAlert alert, CecParameter data)
    {
      return 0;
    }

    /// <summary>Called by libCEC to send back a menu state change to the application.</summary>
    public virtual int ReceiveMenuStateChange(CecMenuState newVal)
    {
      return 0;
    }

    /// <summary>Called by libCEC to notify the application that a source was (de)activated.</summary>
    public virtual void SourceActivated(CecLogicalAddress logicalAddress, bool activated)
    {
    }

    // Native -> managed trampolines ---------------------------------------

    private unsafe void OnLogMessage(IntPtr cbParam, IntPtr messagePtr)
    {
      try
      {
        var message = (cec_log_message*)messagePtr;
        ReceiveLogMessage(new CecLogMessage(
          Marshal.PtrToStringAnsi(message->message),
          (CecLogLevel)message->level,
          message->time));
      }
      catch { }
    }

    private unsafe void OnKeyPress(IntPtr cbParam, IntPtr keyPtr)
    {
      try
      {
        var key = (cec_keypress*)keyPtr;
        ReceiveKeypress(new CecKeypress((CecUserControlCode)key->keycode, key->duration));
      }
      catch { }
    }

    private unsafe void OnCommand(IntPtr cbParam, IntPtr commandPtr)
    {
      try
      {
        ReceiveCommand(Interop.ToManaged((cec_command*)commandPtr));
      }
      catch { }
    }

    private unsafe void OnConfig(IntPtr cbParam, IntPtr configPtr)
    {
      try
      {
        var config = new LibCECConfiguration();
        Interop.ToManaged((libcec_configuration*)configPtr, config);
        ConfigurationChanged(config);
      }
      catch { }
    }

    private void OnAlert(IntPtr cbParam, int alert, libcec_parameter data)
    {
      try
      {
        if ((CecParameterType)data.paramType == CecParameterType.ParameterTypeString)
        {
          string text = data.paramData != IntPtr.Zero ? Marshal.PtrToStringAnsi(data.paramData) : string.Empty;
          ReceiveAlert((CecAlert)alert, new CecParameter(CecParameterType.ParameterTypeString, text));
        }
      }
      catch { }
    }

    private int OnMenu(IntPtr cbParam, int state)
    {
      try
      {
        return ReceiveMenuStateChange((CecMenuState)state);
      }
      catch { }
      return 0;
    }

    private void OnSourceActivated(IntPtr cbParam, int logicalAddress, byte activated)
    {
      try
      {
        SourceActivated((CecLogicalAddress)logicalAddress, activated == 1);
      }
      catch { }
    }

    // Pinned delegate instances (kept alive for the object's lifetime).
    private readonly LogMessageDelegate _logDelegate;
    private readonly KeyPressDelegate _keyDelegate;
    private readonly CommandDelegate _commandDelegate;
    private readonly ConfigDelegate _configDelegate;
    private readonly AlertDelegate _alertDelegate;
    private readonly MenuDelegate _menuDelegate;
    private readonly SourceActivatedDelegate _sourceActivatedDelegate;

    private IntPtr _callbacks;
  }
}
