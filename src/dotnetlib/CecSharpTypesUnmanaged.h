#pragma once
/*
* This file is part of the libCEC(R) library.
*
* libCEC(R) is Copyright (C) 2011-2020 Pulse-Eight Limited.  All rights reserved.
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
*
* Author: Lars Op den Kamp <lars@opdenkamp.eu>
*
*/

#include "p8-platform/threads/mutex.h"
#include <vcclr.h>
#include <msclr/marshal.h>
#include "../../include/cec.h"
#include <vector>

#using <System.dll>

/// <summary>
/// LibCecSharp namespace
/// </summary>
/// <see cref="LibCecSharp" />
namespace CecSharp
{
#pragma unmanaged
  struct UnmanagedCecCallbacks;

  static P8PLATFORM::CMutex                        g_callbackMutex;
  static std::vector<struct UnmanagedCecCallbacks> g_unmanagedCallbacks;
  static CEC::ICECCallbacks                        g_cecCallbacks;

  // unmanaged callback methods
  typedef void (__stdcall *LOGCB)    (const CEC::cec_log_message* message);
  typedef void (__stdcall *KEYCB)    (const CEC::cec_keypress* key);
  typedef void (__stdcall *COMMANDCB)(const CEC::cec_command* command);
  typedef void (__stdcall *CONFIGCB) (const CEC::libcec_configuration* config);
  typedef void (__stdcall *ALERTCB)  (const CEC::libcec_alert, const CEC::libcec_parameter &data);
  typedef int  (__stdcall *MENUCB)   (const CEC::cec_menu_state newVal);
  typedef void (__stdcall *ACTICB)   (const CEC::cec_logical_address logicalAddress, const uint8_t bActivated);

  /// <summary>
  /// libCEC callback methods. Unmanaged code.
  /// </summary>
  struct UnmanagedCecCallbacks
  {
    /// <summary>
    /// Log message callback
    /// </summary>
    LOGCB     logCB;
    /// <summary>
    /// Key press/release callback
    /// </summary>
    KEYCB     keyCB;
    /// <summary>
    /// Raw CEC data callback
    /// </summary>
    COMMANDCB commandCB;
    /// <summary>
    /// Updated configuration callback
    /// </summary>
    CONFIGCB  configCB;
    /// <summary>
    /// Alert message callback
    /// </summary>
    ALERTCB   alertCB;
    /// <summary>
    /// Menu status change callback
    /// </summary>
    MENUCB    menuCB;
    /// <summary>
    /// Source (de)activated callback
    /// </summary>
    ACTICB    sourceActivatedCB;
  };
  
  /// <summary>
  /// Called by libCEC to send back a log message to the application
  /// </summary>
  /// <param name="cbParam">Pointer to the callback struct</param>
  /// <param name="message">The log message</param>
  static void CecLogMessageCB(void* cbParam, const CEC::cec_log_message* message)
  {
    struct UnmanagedCecCallbacks* cb = static_cast<struct UnmanagedCecCallbacks*>(cbParam);
    if (!!cb && !!cb->logCB)
      cb->logCB(message);
  }

  /// <summary>
  /// Called by libCEC to send back a key press or release to the application
  /// </summary>
  /// <param name="cbParam">Pointer to the callback struct</param>
  /// <param name="key">The key press command that libCEC received</param>
  static void CecKeyPressCB(void* cbParam, const CEC::cec_keypress* key)
  {
    struct UnmanagedCecCallbacks* cb = static_cast<struct UnmanagedCecCallbacks*>(cbParam);
    if (!!cb && !!cb->keyCB)
      cb->keyCB(key);
  }

  /// <summary>
  /// Called by libCEC to send back raw CEC data to the application
  /// </summary>
  /// <param name="cbParam">Pointer to the callback struct</param>
  /// <param name="command">The raw CEC data</param>
  static void CecCommandCB(void* cbParam, const CEC::cec_command* command)
  {
    struct UnmanagedCecCallbacks* cb = static_cast<struct UnmanagedCecCallbacks*>(cbParam);
    if (!!cb && !!cb->commandCB)
      cb->commandCB(command);
  }

  /// <summary>
  /// Called by libCEC to send back an updated configuration to the application
  /// </summary>
  /// <param name="cbParam">Pointer to the callback struct</param>
  /// <param name="config">The new configuration</param>
  static void CecConfigCB(void* cbParam, const CEC::libcec_configuration* config)
  {
    struct UnmanagedCecCallbacks* cb = static_cast<struct UnmanagedCecCallbacks*>(cbParam);
    if (!!cb && !!cb->configCB)
      cb->configCB(config);
  }

  /// <summary>
  /// Called by libCEC to send back an alert message to the application
  /// </summary>
  /// <param name="cbParam">Pointer to the callback struct</param>
  /// <param name="data">The alert message</param>
  static void CecAlertCB(void* cbParam, const CEC::libcec_alert alert, const CEC::libcec_parameter data)
  {
    struct UnmanagedCecCallbacks* cb = static_cast<struct UnmanagedCecCallbacks*>(cbParam);
    if (!!cb && !!cb->alertCB)
      cb->alertCB(alert, data);
  }

  /// <summary>
  /// Called by libCEC to send back a menu state change to the application
  /// </summary>
  /// <param name="cbParam">Pointer to the callback struct</param>
  /// <param name="newVal">The new menu state</param>
  /// <return>1 when handled, 0 otherwise</return>
  static int CecMenuCB(void* cbParam, const CEC::cec_menu_state newVal)
  {
    struct UnmanagedCecCallbacks* cb = static_cast<struct UnmanagedCecCallbacks*>(cbParam);
    if (!!cb && !!cb->menuCB)
      return cb->menuCB(newVal);
    return 0;
  }

  /// <summary>
  /// Called by libCEC to notify the application that the source that is handled by libCEC was (de)activated
  /// </summary>
  /// <param name="cbParam">Pointer to the callback struct</param>
  /// <param name="logicalAddress">The logical address that was (de)activated</param>
  /// <param name="activated">True when activated, false when deactivated</param>
  static void CecSourceActivatedCB(void* cbParam, const CEC::cec_logical_address logicalAddress, const uint8_t activated)
  {
    struct UnmanagedCecCallbacks* cb = static_cast<struct UnmanagedCecCallbacks*>(cbParam);
    if (!!cb && !!cb->sourceActivatedCB)
      cb->sourceActivatedCB(logicalAddress, activated);
  }

  /// <summary>
  /// Assign the callback methods in the g_cecCallbacks struct and return a pointer to it
  /// </summary>
  static CEC::ICECCallbacks* GetLibCecCallbacks()
  {
    g_cecCallbacks.logMessage = CecLogMessageCB;
    g_cecCallbacks.keyPress = CecKeyPressCB;
    g_cecCallbacks.commandReceived = CecCommandCB;
    g_cecCallbacks.configurationChanged = CecConfigCB;
    g_cecCallbacks.alert = CecAlertCB;
    g_cecCallbacks.menuStateChanged = CecMenuCB;
    g_cecCallbacks.sourceActivated = CecSourceActivatedCB;
    return &g_cecCallbacks;
  }
#pragma managed
}
