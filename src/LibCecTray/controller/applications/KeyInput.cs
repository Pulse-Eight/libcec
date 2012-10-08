/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2012 Pulse-Eight Limited.  All rights reserved.
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

using System.Collections.Generic;
using System;
using System.Globalization;
using System.Text;

namespace LibCECTray.controller.applications
{
  class KeyInput : ApplicationAction
  {
    public KeyInput(ApplicationController controller) :
      base(controller, ActionType.SendKey)
    {
    }

    public KeyInput(ApplicationController controller, WindowsAPI.VirtualKeyCode keyCode) :
      base(controller, ActionType.SendKey)
    {
      AddKey(keyCode);
    }

    public static ApplicationAction FromString(ApplicationController controller, string value)
    {
      KeyInput retVal = new KeyInput(controller);
      var split = value.Trim().Split(' ');
      foreach (var item in split)
      {
        var param = retVal.GetParameterFromString(item);
        ushort iValue;
        if (ushort.TryParse(param, NumberStyles.AllowHexSpecifier, null, out iValue))
          retVal.AddKey((WindowsAPI.VirtualKeyCode)iValue);
      }
      return retVal;
    }

    public int KeyCount
    {
      get
      {
        int count = 0;
        foreach (var input in _input)
          if (input.Data.Keyboard.Flags == 0)
            count++;
        return count;
      }
    }

    public override string AsString()
    {
      StringBuilder sb = new StringBuilder();
      foreach (var input in _input)
      {
        if (input.Data.Keyboard.Flags == 0)
          sb.AppendFormat("{0}({1:X}) ", TypePrefix, input.Data.Keyboard.KeyCode);
      }
      return sb.ToString().Trim();
    }

    public override string AsFriendlyString()
    {
      StringBuilder sb = new StringBuilder();
      bool bMultipleKeys = KeyCount > 1;
      foreach (var input in _input)
      {
        if (input.Data.Keyboard.Flags == 0)
        {
          sb.AppendFormat(bMultipleKeys ? "[{0}] " : "{0} ",
                          WindowsAPI.GetVirtualKeyName((WindowsAPI.VirtualKeyCode) input.Data.Keyboard.KeyCode));
        }
      }
      return sb.ToString().Trim();
    }

    public int Size()
    {
      return _input.Count;
    }

    public override bool Empty()
    {
      return _input.Count == 0;
    }

    public override bool CanAppend(ApplicationAction value)
    {
      return (value.ActionType == ActionType.SendKey);
    }

    public override ApplicationAction Append(ApplicationAction value)
    {
      if (value.ActionType == ActionType.SendKey)
        AddKey(value as KeyInput);

      return this;
    }

    public WindowsAPI.Input[] ToArray()
    {
      return _input.ToArray();
    }

    public void AddKey(WindowsAPI.VirtualKeyCode keyCode)
    {
      AddKeyDown(keyCode);
      AddKeyUp(keyCode);
    }

    private void AddKeyDown(WindowsAPI.VirtualKeyCode keyCode)
    {
      var key = new WindowsAPI.Input {
        Type = WindowsAPI.InputType.Keyboard,
        Data = {
          Keyboard = new WindowsAPI.KeyboardInput {
            KeyCode = (UInt16) keyCode,
            Scan = 0,
            Flags = 0,
            Time = 0,
            ExtraInfo = IntPtr.Zero
          }
        }
      };

      _input.Add(key);
    }

    private void AddKeyUp(WindowsAPI.VirtualKeyCode keyCode)
    {
      var key = new WindowsAPI.Input {
        Type = WindowsAPI.InputType.Keyboard,
        Data = {
          Keyboard = new WindowsAPI.KeyboardInput {
            KeyCode = (UInt16) keyCode,
            Scan = 0,
            Flags = (uint)WindowsAPI.KeyEvent.KeyUp,
            Time = 0,
            ExtraInfo = IntPtr.Zero
          }
        }
      };

      _input.Add(key);
    }

    public KeyInput AddKey(KeyInput input)
    {
      if (input != null)
      {
        foreach (var item in input._input)
          _input.Add(item);
      }
      return this;
    }

    public override bool Transmit(IntPtr windowHandle)
    {
      var inputAr = ToArray();
      if (inputAr.Length == 0)
        return false;

      try
      {
        return WindowsAPI.SendInputTo(windowHandle, (uint) inputAr.Length, inputAr,
                                      System.Runtime.InteropServices.Marshal.SizeOf(typeof (WindowsAPI.Input)));
      }
      catch (Exception e)
      {
        Console.Error.WriteLine(e.Message);
        return false;
      }
    }

    public ApplicationAction RemoveItem(int index)
    {
      //both keyup and keydown
      if (index * 2 + 1 < _input.Count)
      {
        _input.RemoveAt(index * 2);
        _input.RemoveAt(index * 2);
      }
      return this;
    }

    public override ApplicationAction RemoveKey(int index)
    {
      var current = 0;
      var item = 0;
      foreach (var input in _input)
      {
        if (input.Data.Keyboard.Flags == 0)
        {
          var tmp = string.Format(KeyCount > 1 ? "[{0}] " : "{0} ",
                                  WindowsAPI.GetVirtualKeyName((WindowsAPI.VirtualKeyCode) input.Data.Keyboard.KeyCode));
          current += tmp.Length + 1;
          if (index <= current)
            return RemoveItem(item);
          item++;
        }
      }

      return this;
    }

    private readonly List<WindowsAPI.Input> _input = new List<WindowsAPI.Input>();
  }
}