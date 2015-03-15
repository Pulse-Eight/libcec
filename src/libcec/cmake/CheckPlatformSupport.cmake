# - Check for platform support and set variables and definitions
#
# This module sets the following variables
#       PLATFORM_LIBREQUIRES      dependencies
#       PLATFORM_SOURCES          extra source files
#       LIB_INFO                  supported features and compilation information
#       LIB_DESTINATION           destination for the .so/.dll files
#	HAVE_LOCKDEV              1 if lockdev is supported
#	HAVE_RANDR                1 if xrandr is supported
#	HAVE_LIBUDEV              1 if udev is supported
#	HAVE_RPI_API              1 if Raspberry Pi is supported
#	HAVE_TDA995X_API          1 if TDA995X is supported
#	HAVE_EXYNOS_API           1 if Exynos is supported
#       HAVE_P8_USB_DETECT        1 if Pulse-Eight devices can be auto-detected
#

set(PLATFORM_LIBREQUIRES "")

# Pulse-Eight devices are always supported
add_definitions(-DHAVE_P8_USB)

set(LIB_INFO "compiled on ${CMAKE_SYSTEM}")

if(WIN32)
  # Windows
  add_definitions(-DTARGET_WINDOWS -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -D_WINSOCKAPI_)
  set(LIB_DESTINATION ".")
  if (${WIN64})
    set(LIB_INFO "${LIB_INFO} (x64)")
  else()
    add_definitions(-D_USE_32BIT_TIME_T)
  endif()
  set(HAVE_P8_USB_DETECT 1)
  set(LIB_INFO "${LIB_INFO}, features: P8_USB, P8_detect")
  list(APPEND CEC_SOURCES platform/windows/os-edid.cpp
                          platform/windows/serialport.cpp
                          LibCECDll.cpp
                          libcec.rc)
else()
  # not Windows
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wextra -Wno-missing-field-initializers")
  set(PLATFORM_SOURCES platform/posix/os-edid.cpp
                       platform/posix/serialport.cpp)
  set(HAVE_P8_USB_DETECT 0)
  set(LIB_DESTINATION "${CMAKE_INSTALL_LIBDIR}")
  set(LIB_INFO "${LIB_INFO}, features: P8_USB")

  # lockdev
  check_include_files(lockdev.h HAVE_LOCKDEV_HEADERS)
  check_library_exists(lockdev dev_unlock "" HAVE_LOCKDEV_LIB)
  if (HAVE_LOCKDEV_HEADERS AND HAVE_LOCKDEV_LIB)
    set(HAVE_LOCKDEV 1)
  else()
    set(HAVE_LOCKDEV 0)
  endif()

  # udev
  check_library_exists(udev udev_new "" HAVE_LIBUDEV)
  if (HAVE_LIBUDEV)
    set(LIB_INFO "${LIB_INFO}, P8_detect")
    set(PLATFORM_LIBREQUIRES "${PLATFORM_LIBREQUIRES} udev")
    list(APPEND CMAKE_REQUIRED_LIBRARIES "udev")
    set(HAVE_P8_USB_DETECT 1)
  endif()

  # xrandr
  check_include_files("X11/Xlib.h;X11/Xatom.h;X11/extensions/Xrandr.h" HAVE_RANDR_HEADERS)
  check_library_exists(Xrandr XRRGetScreenResources "" HAVE_RANDR_LIB)
  if (HAVE_RANDR_HEADERS AND HAVE_RANDR_LIB)
    set(LIB_INFO "${LIB_INFO}, randr")
    list(APPEND PLATFORM_SOURCES platform/X11/randr-edid.cpp)
    set(HAVE_RANDR 1)
  else()
    set(HAVE_RANDR 0)
  endif()

  # raspberry pi
  check_library_exists(bcm_host vchi_initialise "" HAVE_RPI_API)
  if (HAVE_RPI_API)
    set(LIB_INFO "${LIB_INFO}, 'RPi'")
    list(APPEND CMAKE_REQUIRED_LIBRARIES "vcos")
    list(APPEND CMAKE_REQUIRED_LIBRARIES "vchiq_arm")
    list(APPEND PLATFORM_SOURCES adapter/RPi/RPiCECAdapterDetection.cpp
                                 adapter/RPi/RPiCECAdapterCommunication.cpp
                                 adapter/RPi/RPiCECAdapterMessageQueue.cpp)
  endif()

  # TDA995x
  check_include_files("tda998x_ioctl.h;comps/tmdlHdmiCEC/inc/tmdlHdmiCEC_Types.h" HAVE_TDA995X_API)
  if (HAVE_TDA995X_API)
    set(LIB_INFO "${LIB_INFO}, 'TDA995x'")
    list(APPEND PLATFORM_SOURCES adapter/TDA995x/TDA995xCECAdapterDetection.cpp
                                 adapter/TDA995x/TDA995xCECAdapterCommunication.cpp)
  endif()


  # Exynos
  if (${HAVE_EXYNOS_API})
    set(LIB_INFO "${LIB_INFO}, 'Exynos'")
    set(HAVE_EXYNOS_API 1)
    list(APPEND PLATFORM_SOURCES adapter/Exynos/ExynosCECAdapterDetection.cpp
                                 adapter/Exynos/ExynosCECAdapterCommunication.cpp)
  else()
    set(HAVE_EXYNOS_API 0)
  endif()
endif()

