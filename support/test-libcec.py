## demo of the python-libcec API

# import libCEC
import cec
print cec

# don't enable debug logging by default
log_level = cec.CEC_LOG_TRAFFIC

# logging callback
def log_callback(level, time, message):
  if level > log_level:
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

  print levelstr + "[" + str(time) + "]     " + message
  return 0

# key press callback
def key_press_callback(key, duration):
  print "[key]" + message
  return 0

# create a new libcec_configuration
def SetConfiguration():
  cecconfig = cec.libcec_configuration()
  cecconfig.strDeviceName   = "pyLibCec"
  cecconfig.bActivateSource = 0
  cecconfig.deviceTypes.Add(cec.CEC_DEVICE_TYPE_RECORDING_DEVICE)
  cecconfig.clientVersion = cec.LIBCEC_VERSION_CURRENT
  cecconfig.SetLogCallback(log_callback)
  cecconfig.SetKeyPressCallback(key_press_callback)
  return cecconfig

# detect an adapter and return the com port path
def DetectAdapter(lib):
  adapters = cec.cec_adapter_descriptor()
  if lib.DetectAdapters(adapters, 1) == 1:
    print "found a CEC adapter:"
    print "port:     " + adapters.strComName
    print "vendor:   " + hex(adapters.iVendorId)
    print "product:  " + hex(adapters.iProductId)
    return adapters.strComName
  else:
    return None

# initialise libCEC
def InitLibCec(cecconfig):
  lib = cec.CECInitialise(cecconfig)
  # print libCEC version and compilation information
  print "libCEC version " + lib.VersionToString(cecconfig.serverVersion) + " loaded: " + lib.GetLibInfo()

  lib.InitVideoStandalone()

  # search for adapters
  adapter = DetectAdapter(lib)
  if adapter == None:
    print "No adapters found"
  else:
    if lib.Open(adapter):
      print "connection opened"
      return lib
    else:
      print "failed to open a connection to the CEC adapter"
  return None

# display the addresses controlled by libCEC
def ProcessCommandSelf(lib):
  addresses = lib.GetLogicalAddresses()
  strOut = "Addresses controlled by libCEC: "
  x = 0
  notFirst = False
  while x < 15:
    if addresses.IsSet(x):
      if notFirst:
        strOut += ", "
      strOut += lib.LogicalAddressToString(x)
      if lib.IsActiveSource(x):
        strOut += " (*)"
      notFirst = True
    x += 1
  print strOut

# send an active source message
def ProcessCommandActiveSource(lib):
  lib.SetActiveSource()

# send a standby command
def ProcessCommandStandby(lib):
  lib.StandbyDevices(CECDEVICE_BROADCAST)

# scan the bus and display devices that were found
def ProcessCommandScan(lib):
  print "requesting CEC bus information ..."
  strLog = "CEC bus information\n===================\n"
  addresses = lib.GetActiveDevices()
  activeSource = lib.GetActiveSource()
  x = 0
  while x < 15:
    if addresses.IsSet(x):
      vendorId        = lib.GetDeviceVendorId(x)
      physicalAddress = lib.GetDevicePhysicalAddress(x)
      active          = lib.IsActiveSource(x)
      cecVersion      = lib.GetDeviceCecVersion(x)
      power           = lib.GetDevicePowerStatus(x)
      osdName         = lib.GetDeviceOSDName(x)
      strLog += "device #" + str(x) +": " + lib.LogicalAddressToString(x)  + "\n"
      strLog += "address:       " + str(physicalAddress) + "\n"
      strLog += "active source: " + str(active) + "\n"
      strLog += "vendor:        " + lib.VendorIdToString(vendorId) + "\n"
      strLog += "CEC version:   " + lib.CecVersionToString(cecVersion) + "\n"
      strLog += "power status:  " + lib.PowerStatusToString(power) + "\n\n\n"
    x += 1
  print strLog

# main loop, ask forr commands
def MainLoop(lib):
  runLoop = True
  while runLoop:
    command = raw_input("Enter command:")
    if command == "q" or command == "quit":
      runLoop = False
    elif command == "self":
      ProcessCommandSelf(lib)
    elif command == "as" or command == "activesource":
      ProcessCommandActiveSource(lib)
    elif command == "standby":
      ProcessCommandStandby(lib)
    elif command == "scan":
      ProcessCommandScan(lib)
  print "Exiting..."
  # TODO do this in the dtor...
  cecconfig.ClearCallbacks()

# initialise libCEC and enter the main loop
if __name__ == '__main__':
  # initialise libCEC
  cecconfig = SetConfiguration()
  lib = InitLibCec(cecconfig)
  if lib != None:
    MainLoop(lib)

