# -*- coding: utf-8 -*-
# This file is part of the libCEC(R) library.
#
# libCEC(R) is Copyright (C) 2011-2015 Pulse-Eight Limited.  All rights reserved.
# libCEC(R) is an original work, containing original code.
#
# libCEC(R) is a trademark of Pulse-Eight Limited.
#
# This program is dual-licensed; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301  USA
#
#
# Alternatively, you can license this library under a commercial license,
# please contact Pulse-Eight Licensing for more information.
#
# For more information contact:
# Pulse-Eight Licensing       <license@pulse-eight.com>
#     http://www.pulse-eight.com/
#     http://www.pulse-eight.net/

description = """<rst>
Integration with libCEC, which adds support for Pulse-Eight's `CEC adapters <http://www.pulse-eight.com/>`_.

|

.. image:: cec.png
   :align: center

**Notice:**
Set up the HDMI input to which your adapter is connected correctly, or remote control input won't work.
"""

import cec

eg.RegisterPlugin(
  name = 'Pulse-Eight CEC adapter',
  author = 'Lars Op den Kamp',
  version = '0.3',
  kind = 'remote',
  guid = '{fd322eea-c897-470c-bef7-77bf15c52db4}',
  url = 'http://libcec.pulse-eight.com/',
  description = description,
  createMacrosOnAdd = False,
  hardwareId = "USB\\VID_2548&PID_1002",
)

# logging callback
def log_callback(level, time, message):
  return CEC.instance.LogCallback(level, time, message)

# key press callback
def key_press_callback(key, duration):
  return CEC.instance.KeyPressCallback(key, duration)

class CEC(eg.PluginClass):
  instance = {}
  logicalAddressNames = []
  lastKeyPressed = 255

  # detect an adapter and return the com port path
  def DetectAdapter(self):
    retval = None
    adapters = self.lib.DetectAdapters()
    for adapter in adapters:
      print("found a CEC adapter: " + adapter.strComName)
      retval = adapter.strComName
    return retval

  # initialise libCEC
  def InitLibCec(self, connectedAvr, portNumber, deviceName):
    if connectedAvr:
      self.cecconfig.baseDevice = cec.CECDEVICE_AUDIOSYSTEM
    else:
      self.cecconfig.baseDevice = cec.CECDEVICE_TV
    self.cecconfig.iHDMIPort       = portNumber
    self.cecconfig.strDeviceName   = str(deviceName)
    self.cecconfig.bActivateSource = 0

    self.lib = cec.ICECAdapter.Create(self.cecconfig)

    # search for adapters
    adapter = self.DetectAdapter()
    if adapter == None:
      print("No adapters found")
    else:
      if self.lib.Open(adapter):
        print("connection opened")
        return True
      else:
        print("failed to open a connection to the CEC adapter")
    return False
	
  def __init__(self):
    self.log_level = cec.CEC_LOG_WARNING
    self.cecconfig = cec.libcec_configuration()
    self.cecconfig.clientVersion   = cec.LIBCEC_VERSION_CURRENT
    self.cecconfig.deviceTypes.Add(cec.CEC_DEVICE_TYPE_RECORDING_DEVICE)
    self.cecconfig.SetLogCallback(log_callback)
    self.cecconfig.SetKeyPressCallback(key_press_callback)
    CEC.instance = self
    self.AddGroup("Actions", ACTIONS)
    self.AddActionsFromList(ACTIONS)
    self.AddGroup("Queries", QUERIES)
    self.AddActionsFromList(QUERIES)

  def __start__(self, connectedAvr, portNumber, deviceName):
    self.lastConnectedAvr = connectedAvr
    self.lastPortNumber = portNumber
    self.lastDeviceName = deviceName
    if self.InitLibCec(connectedAvr, portNumber, deviceName):
      # print libCEC version and compilation information
      print("libCEC version " + self.lib.VersionToString(self.cecconfig.serverVersion) + " loaded: " + self.lib.GetLibInfo())
      self.logicalAddressNames = [ self.lib.LogicalAddressToString(cec.CECDEVICE_TV),
                                   self.lib.LogicalAddressToString(cec.CECDEVICE_RECORDINGDEVICE1),
                                   self.lib.LogicalAddressToString(cec.CECDEVICE_RECORDINGDEVICE2),
                                   self.lib.LogicalAddressToString(cec.CECDEVICE_TUNER1),
                                   self.lib.LogicalAddressToString(cec.CECDEVICE_PLAYBACKDEVICE1),
                                   self.lib.LogicalAddressToString(cec.CECDEVICE_AUDIOSYSTEM),
                                   self.lib.LogicalAddressToString(cec.CECDEVICE_TUNER2),
                                   self.lib.LogicalAddressToString(cec.CECDEVICE_TUNER3),
                                   self.lib.LogicalAddressToString(cec.CECDEVICE_PLAYBACKDEVICE2),
                                   self.lib.LogicalAddressToString(cec.CECDEVICE_RECORDINGDEVICE3),
                                   self.lib.LogicalAddressToString(cec.CECDEVICE_TUNER4),
                                   self.lib.LogicalAddressToString(cec.CECDEVICE_PLAYBACKDEVICE3),
                                   self.lib.LogicalAddressToString(cec.CECDEVICE_RESERVED1),
                                   self.lib.LogicalAddressToString(cec.CECDEVICE_RESERVED2),
                                   self.lib.LogicalAddressToString(cec.CECDEVICE_FREEUSE),
                                   self.lib.LogicalAddressToString(cec.CECDEVICE_BROADCAST),
                                   self.lib.LogicalAddressToString(cec.CECDEVICE_UNKNOWN),]
    else:
      print("Couldn't initialise libCEC. Please check your configuration.")

  def ReinitLibCec(self):
      self.lib.Close()
      if self.InitLibCec(self.lastConnectedAvr, self.lastPortNumber, self.lastDeviceName):
          print "libCEC reinited sucessfully"
      else:
          print("Could not reinit libCEC")

  def __stop__(self):
    self.lib.Close()

  def Configure(self, connectedAvr = False, portNumber = 1, deviceName = "libCEC"):
    panel = eg.ConfigPanel()
    spPortNumber = panel.SpinIntCtrl(portNumber, min=1, max=15)
    cbConnectedAvr = panel.CheckBox(connectedAvr, "")
    bxConnection = panel.BoxedGroup(
      "Connection",
      ("Connected to AVR", cbConnectedAvr),
      ("HDMI port number:", spPortNumber),
    )
    panel.sizer.Add(bxConnection, 0, wx.EXPAND)

    txDeviceName = panel.TextCtrl(deviceName)
    bxDevice = panel.BoxedGroup(
      "Device configuration",
      ("Device name", txDeviceName),
    )
    panel.sizer.Add(bxDevice, 0, wx.EXPAND)

    while panel.Affirmed():
      panel.SetResult(cbConnectedAvr.GetValue(), spPortNumber.GetValue(), txDeviceName.GetValue())

  def Transmit(self, txcmd):
    self.lib.Transmit(self.lib.CommandFromString(txcmd))
  
  # logging callback
  def LogCallback(self, level, time, message):
    if level > self.log_level:
      return 0

    if level == cec.CEC_LOG_ERROR:
      levelstr = "ERROR:   "
    elif level == cec.CEC_LOG_WARNING:
      levelstr = "WARNING: "
    elif level == cec.CEC_LOG_NOTICE:
      levelstr = "NOTICE:  "
    elif level == cec.CEC_LOG_TRAFFIC:
      levelstr = "TRAFFIC: "
    elif level == cec.CEC_LOG_DEBUG:
      levelstr = "DEBUG:   "

    print("CEC " + levelstr + "[" + str(time) + "]     " + message)
    return 0

  # key press callback
  def KeyPressCallback(self, key, duration):
    if duration == 0 and self.lastKeyPressed != key:
      self.lastKeyPressed = key
      self.TriggerEnduringEvent(self.lib.UserControlCodeToString(key))
    elif duration > 0 and self.lastKeyPressed == key:
      self.lastKeyPressed = 255
      self.EndLastEvent()
    elif self.lastKeyPressed != key:
      self.lastKeyPressed = 255
      self.TriggerEvent(self.lib.UserControlCodeToString(key))
    return 0

  def NumberToLogicalAddress(self, value):
    if value == 0:
      return "cec.CECDEVICE_TV"
    elif value == 1:
      return "cec.CECDEVICE_RECORDINGDEVICE1"
    elif value == 2:
      return "cec.CECDEVICE_RECORDINGDEVICE2"
    elif value == 3:
      return "cec.CECDEVICE_TUNER1"
    elif value == 4:
      return "cec.CECDEVICE_PLAYBACKDEVICE1"
    elif value == 5:
      return "cec.CECDEVICE_AUDIOSYSTEM"
    elif value == 6:
      return "cec.CECDEVICE_TUNER2"
    elif value == 7:
      return "cec.CECDEVICE_TUNER3"
    elif value == 8:
      return "cec.CECDEVICE_PLAYBACKDEVICE2"
    elif value == 9:
      return "cec.CECDEVICE_RECORDINGDEVICE3"
    elif value == 10:
      return "cec.CECDEVICE_TUNER4"
    elif value == 11:
      return "cec.CECDEVICE_PLAYBACKDEVICE3"
    elif value == 12:
      return "cec.CECDEVICE_RESERVED1"
    elif value == 13:
      return "cec.CECDEVICE_RESERVED2"
    elif value == 14:
      return "cec.CECDEVICE_FREEUSE"
    elif value == 15:
      return "cec.CECDEVICE_BROADCAST"
    return "cec.CECDEVICE_UNKNOWN"

  def command(self, command):
    eval(command)

  def query(self, command):
    return eval(command)

class ActionNoParam(eg.ActionClass):
  def __call__(self):
    self.plugin.command(self.value)

class ActionParamString(eg.ActionClass):
  def __call__(self, value = ""):
    self.plugin.command(self.value.format(str(value)))

  def Configure(self, value = ""):
    panel = eg.ConfigPanel()
    valueCtrl = panel.TextCtrl(value)
    valueBox = panel.BoxedGroup("Enter value", ("Value:", valueCtrl),)
    panel.sizer.Add(valueBox, 0, wx.EXPAND)
    while panel.Affirmed():
      panel.SetResult(valueCtrl.GetValue(),)

class ActionParamLogicalAddress(eg.ActionClass):
  names = []
  selectedValue = 0
 
  def __call__(self, value = "cec.CECDEVICE_UNKNOWN"):
    self.plugin.command(self.value.format(value))

  def Configure(self, value = "cec.CECDEVICE_UNKNOWN"):
    self.names = self.plugin.logicalAddressNames
    panel = eg.ConfigPanel()

    cbAddresses = wx.ComboBox(panel, -1, choices = self.names)
    cbAddresses.SetStringSelection(self.plugin.lib.LogicalAddressToString(cec.CECDEVICE_UNKNOWN))

    def cbAddressesChanged(event = None):
      evtName = cbAddresses.GetValue()
      if evtName in self.names:
        self.selectedValue = self.names.index(evtName)
      if event:
        event.Skip()
    cbAddressesChanged()
    cbAddresses.Bind(wx.EVT_COMBOBOX, cbAddressesChanged)
    panel.sizer.Add(cbAddresses)

    while panel.Affirmed():
      panel.SetResult(self.plugin.NumberToLogicalAddress(self.selectedValue),)

class QueryParamLogicalAddress(eg.ActionClass):
  names = []
  selectedValue = 0
 
  def __call__(self, value = "cec.CECDEVICE_UNKNOWN"):
    return self.plugin.query(self.value.format(value))

  def Configure(self, value = "cec.CECDEVICE_UNKNOWN"):
    self.names = self.plugin.logicalAddressNames
    panel = eg.ConfigPanel()

    cbAddresses = wx.ComboBox(panel, -1, choices = self.names)
    cbAddresses.SetStringSelection(self.plugin.lib.LogicalAddressToString(cec.CECDEVICE_UNKNOWN))

    def cbAddressesChanged(event = None):
      evtName = cbAddresses.GetValue()
      if evtName in self.names:
        self.selectedValue = self.names.index(evtName)
      if event:
        event.Skip()
    cbAddressesChanged()
    cbAddresses.Bind(wx.EVT_COMBOBOX, cbAddressesChanged)
    panel.sizer.Add(cbAddresses)

    while panel.Affirmed():
      panel.SetResult(self.plugin.NumberToLogicalAddress(self.selectedValue),)

ACTIONS = (
  (ActionNoParam,             'ActiveSource', 'Active source',       'Mark this device as active source, which will turn on the TV and switch it to the correct input',           u'self.lib.SetActiveSource()'),
  (ActionNoParam,             'InactiveView', 'Inactive view',       'Mark this source as inactive. The result can be different per TV. Most will switch to the previous source', u'self.lib.SetInactiveView()'),
  (ActionParamLogicalAddress, 'PowerOn',      'Power on device',     'Power on the given device',                                                                                 u'self.lib.PowerOnDevices({0})'),
  (ActionNoParam,             'StandbyAll',   'Standby all devices', 'Send the TV and any other CEC capable device to standby',                                                   u'self.lib.StandbyDevices(cec.CECDEVICE_BROADCAST)'),
  (ActionParamLogicalAddress, 'Standby',      'Standby device',      'Send the given device to standby (if present)',                                                             u'self.lib.StandbyDevices({0})'),

  (ActionNoParam,             'VolumeUp',     'Volume up',           'Send a volume up command to the AVR (if present)',                                                          u'self.lib.VolumeUp()'),
  (ActionNoParam,             'VolumeDown',   'Volume down',         'Send a volume down command to the AVR (if present)',                                                        u'self.lib.VolumeDown()'),
  (ActionNoParam,             'ToggleMute',   'Toggle volume mute',  'Send a mute toggle command to the AVR (if present)',                                                        u'self.lib.MuteAudio()'),

  (ActionParamString,         'RawCommand',   'Send command',        'Send a raw CEC command',                                                                                    u'self.lib.Transmit(self.lib.CommandFromString(\'{0}\'))'),
  (ActionNoParam,             'ReinitLibCec',   'Re-initialize Libcec',   'Useful if the device was powered down',                                                                     u'self.ReinitLibCec()'),
)

QUERIES = (
  (QueryParamLogicalAddress, 'GetCecVersion',   'Get device CEC version',   'Request the CEC version',   u'self.lib.CecVersionToString(self.lib.GetDeviceCecVersion({0}))'),
  (QueryParamLogicalAddress, 'GetMenuLanguage', 'Get device menu language', 'Request the menu language', u'self.lib.GetDeviceMenuLanguage({0})'),
  (QueryParamLogicalAddress, 'GetVendorId',     'Get device vendor id',     'Request the vendor id',     u'self.lib.VendorIdToString(self.lib.GetDeviceVendorId({0}))'),
  (QueryParamLogicalAddress, 'GetPowerStatus',  'Get device power status',  'Request the power status',  u'self.lib.PowerStatusToString(self.lib.GetDevicePowerStatus({0}))'),
)
