# - Link platform support dependencies found by CheckPlatformSupport.cmake

list(APPEND cec_depends ${p8-platform_LIBRARIES}
                        ${CMAKE_THREAD_LIBS_INIT})

# lockdev
if (HAVE_LOCKDEV)
  list(APPEND cec_depends lockdev)
endif()

# udev
if (HAVE_LIBUDEV)
  list(APPEND cec_depends udev)
endif()

# xrandr
if (HAVE_RANDR)
  list(APPEND cec_depends Xrandr
                          X11)
endif()

# rt
if (HAVE_RT)
  list(APPEND cec_depends rt)
endif()

# dl
if (HAVE_DLOPEN)
  list(APPEND cec_depends dl)
endif()

# raspberry pi
if (HAVE_RPI_API)
  list(APPEND cec_depends ${RPI_VCOS}
                          ${RPI_VCHIQ_ARM}
                          ${RPI_BCM_HOST})
endif()

# Apple
if (APPLE)
  list(APPEND cec_depends "-framework CoreFoundation"
                          "-framework IOKit"
                          "-framework CoreVideo")
endif()

