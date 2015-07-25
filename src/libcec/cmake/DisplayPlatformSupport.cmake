# - Display platform support found by CheckPlatformSupport.cmake

message(STATUS "Configured features:")

if (HAVE_P8_USB_DETECT)
  message(STATUS "Pulse-Eight CEC Adapter:                yes")
else()
  message(STATUS "Pulse-Eight CEC Adapter:                no")
endif()

if (HAVE_P8_USB_DETECT)
  message(STATUS "Pulse-Eight CEC Adapter detection:      yes")
else()
  message(STATUS "Pulse-Eight CEC Adapter detection:      no")
endif()

if (HAVE_LOCKDEV)
  message(STATUS "lockdev support:                        yes")
else()
  message(STATUS "lockdev support:                        no")
endif()

if (HAVE_RANDR)
  message(STATUS "xrandr support:                         yes")
else()
  message(STATUS "xrandr support:                         no")
endif()

if (HAVE_RPI_API)
  message(STATUS "Raspberry Pi support:                   yes")
else()
  message(STATUS "Raspberry Pi support:                   no")
endif()

if (HAVE_TDA995X_API)
  message(STATUS "TDA995x support:                        yes")
else()
  message(STATUS "TDA995x support:                        no")
endif()

if (HAVE_EXYNOS_API)
  message(STATUS "Exynos support:                         yes")
else()
  message(STATUS "Exynos support:                         no")
endif()

if (HAVE_NETBSD_API)
  message(STATUS "NetBSD support:                         yes")
else()
  message(STATUS "NetBSD support:                         no")
endif()

if (HAVE_PYTHON)
  message(STATUS "Python support:                         version ${PYTHONLIBS_VERSION_STRING} (${PYTHON_VERSION})")
else()
  message(STATUS "Python support:                         no")
endif()

message(STATUS "lib info: ${LIB_INFO}")

