#! /usr/bin/python3
## demo of the python-libcec API

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

import cec
print(cec)

class pyCecClient:
  cecconfig = cec.libcec_configuration()
  lib = {}
  # don't enable debug logging by default
  log_level = cec.CEC_LOG_TRAFFIC

  # create a new libcec_configuration
  def SetConfiguration(self):
    self.cecconfig.strDeviceName   = "pyLibCec"
    self.cecconfig.bActivateSource = 0
    self.cecconfig.deviceTypes.Add(cec.CEC_DEVICE_TYPE_RECORDING_DEVICE)
    self.cecconfig.clientVersion = cec.LIBCEC_VERSION_CURRENT

  def SetLogCallback(self, callback):
    self.cecconfig.SetLogCallback(callback)

  def SetKeyPressCallback(self, callback):
    self.cecconfig.SetKeyPressCallback(callback)

  def SetCommandCallback(self, callback):
    self.cecconfig.SetCommandCallback(callback)

  # detect an adapter and return the com port path
  def DetectAdapter(self):
    retval = None
    adapters = self.lib.DetectAdapters()
    for adapter in adapters:
      print("found a CEC adapter:")
      print("port:     " + adapter.strComName)
      print("vendor:   " + hex(adapter.iVendorId))
      print("product:  " + hex(adapter.iProductId))
      retval = adapter.strComName
    return retval

  # initialise libCEC
  def InitLibCec(self):
    self.lib = cec.ICECAdapter.Create(self.cecconfig)
    # print libCEC version and compilation information
    print("libCEC version " + self.lib.VersionToString(self.cecconfig.serverVersion) + " loaded: " + self.lib.GetLibInfo())

    # search for adapters
    adapter = self.DetectAdapter()
    if adapter == None:
      print("No adapters found")
    else:
      if self.lib.Open(adapter):
        print("connection opened")
        self.MainLoop()
      else:
        print("failed to open a connection to the CEC adapter")

  # display the addresses controlled by libCEC
  def ProcessCommandSelf(self):
    addresses = self.lib.GetLogicalAddresses()
    strOut = "Addresses controlled by libCEC: "
    x = 0
    notFirst = False
    while x < 15:
      if addresses.IsSet(x):
        if notFirst:
          strOut += ", "
        strOut += self.lib.LogicalAddressToString(x)
        if self.lib.IsActiveSource(x):
          strOut += " (*)"
        notFirst = True
      x += 1
    print(strOut)

  # send an active source message
  def ProcessCommandActiveSource(self):
    self.lib.SetActiveSource()

  # send a standby command
  def ProcessCommandStandby(self):
    self.lib.StandbyDevices(cec.CECDEVICE_BROADCAST)

  # send a custom command
  def ProcessCommandTx(self, data):
    cmd = self.lib.CommandFromString(data)
    print("transmit " + data)
    if self.lib.Transmit(cmd):
      print("command sent")
    else:
      print("failed to send command")

  # scan the bus and display devices that were found
  def ProcessCommandScan(self):
    print("requesting CEC bus information ...")
    strLog = "CEC bus information\n===================\n"
    addresses = self.lib.GetActiveDevices()
    activeSource = self.lib.GetActiveSource()
    x = 0
    while x < 15:
      if addresses.IsSet(x):
        vendorId        = self.lib.GetDeviceVendorId(x)
        physicalAddress = self.lib.GetDevicePhysicalAddress(x)
        active          = self.lib.IsActiveSource(x)
        cecVersion      = self.lib.GetDeviceCecVersion(x)
        power           = self.lib.GetDevicePowerStatus(x)
        osdName         = self.lib.GetDeviceOSDName(x)
        strLog += "device #" + str(x) +": " + self.lib.LogicalAddressToString(x)  + "\n"
        strLog += "address:       " + str(physicalAddress) + "\n"
        strLog += "active source: " + str(active) + "\n"
        strLog += "vendor:        " + self.lib.VendorIdToString(vendorId) + "\n"
        strLog += "CEC version:   " + self.lib.CecVersionToString(cecVersion) + "\n"
        strLog += "OSD name:      " + osdName + "\n"
        strLog += "power status:  " + self.lib.PowerStatusToString(power) + "\n\n\n"
      x += 1
    print(strLog)

  # main loop, ask for commands
  def MainLoop(self):
    runLoop = True
    while runLoop:
      command = raw_input("Enter command:").lower()
      if command == 'q' or command == 'quit':
        runLoop = False
      elif command == 'self':
        self.ProcessCommandSelf()
      elif command == 'as' or command == 'activesource':
        self.ProcessCommandActiveSource()
      elif command == 'standby':
        self.ProcessCommandStandby()
      elif command == 'scan':
        self.ProcessCommandScan()
      elif command[:2] == 'tx':
        self.ProcessCommandTx(command[3:])
    print('Exiting...')

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

    print(levelstr + "[" + str(time) + "]     " + message)
    return 0

  # key press callback
  def KeyPressCallback(self, key, duration):
    print("[key pressed] " + str(key))
    return 0

  # command received callback
  def CommandCallback(self, cmd):
    print("[command received] " + cmd)
    return 0

  def __init__(self):
    self.SetConfiguration()

# logging callback
def log_callback(level, time, message):
  return lib.LogCallback(level, time, message)

# key press callback
def key_press_callback(key, duration):
  return lib.KeyPressCallback(key, duration)

# command callback
def command_callback(cmd):
  return lib.CommandCallback(cmd)

if __name__ == '__main__':
  # initialise libCEC
  lib = pyCecClient()
  lib.SetLogCallback(log_callback)
  lib.SetKeyPressCallback(key_press_callback)
  lib.SetCommandCallback(command_callback)

  # initialise libCEC and enter the main loop
  lib.InitLibCec()

