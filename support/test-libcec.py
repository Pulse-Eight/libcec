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
cecconfig = cec.libcec_configuration()
cecconfig.strDeviceName   = "pyLibCec"
cecconfig.bActivateSource = 0;
cecconfig.deviceTypes.Add(cec.CEC_DEVICE_TYPE_RECORDING_DEVICE)
cecconfig.clientVersion = cec.LIBCEC_VERSION_CURRENT;
cecconfig.set_log_callback(log_callback)
cecconfig.set_key_press_callback(key_press_callback)

# initialise libCEC
lib = cec.CECInitialise(cecconfig)

# print libCEC version and compilation information
version = lib.VersionToString(cecconfig.serverVersion)
print "libCEC version " + version + " loaded: " + lib.GetLibInfo()

lib.InitVideoStandalone()

# search for adapters
#TODO
adapters = cec.cec_adapter_descriptor()
if lib.DetectAdapters(adapters, 1) == 1:
  print "found a CEC adapter:"
  print "port:     " + adapters.strComName
  print "vendor:   " + hex(adapters.iVendorId)
  print "product:  " + hex(adapters.iProductId)
  if lib.Open(adapters.strComName):
    print "connection opened"
  else:
    print "failed to open a connection to the CEC adapter"
else:
  print "No adapters found"
