## demo of the python-libcec API

# import libCEC
import cec
print cec

# logging callback
def log_callback(level, time, message):
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
cecconfig.clientVersion = cec.LIBCEC_VERSION_CURRENT;
cecconfig.set_log_callback(log_callback)
cecconfig.set_key_press_callback(key_press_callback)

# initialise libCEC
conn = cec.libcec_initialise(cecconfig)

# print libCEC version and compilation information
version = cec.libcec_version_to_string(cecconfig.serverVersion, 10)
print "libCEC version " + version + " loaded: " + cec.libcec_get_lib_info(conn)

cec.libcec_init_video_standalone(conn)

# search for adapters
adapters = cec.cec_adapter_descriptor()
if cec.libcec_detect_adapters(conn, adapters, 1, None, 0) == 1:
    print "found a CEC adapter:"
    print "port:     " + adapters.strComName
    print "vendor:   " + hex(adapters.iVendorId)
    print "product:  " + hex(adapters.iProductId)
    if cec.libcec_open(conn, adapters.strComName, 5000) == 0:
      print "connection opened"
      print cec.libcec_set_active_source(conn, cec.CEC_DEVICE_TYPE_PLAYBACK_DEVICE)
    else:
      print "failed to open a connection to the CEC adapter"
else:
    print "no CEC adapters found"
