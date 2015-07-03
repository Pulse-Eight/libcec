# - Check for platform support and set variables and definitions
#
# This module sets the following variables
#       PLATFORM_LIBREQUIRES      dependencies
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

SET(RPI_LIB_DIR     "" CACHE STRING "Path to Rapsberry Pi libraries")
SET(RPI_INCLUDE_DIR "" CACHE STRING "Path to Rapsberry Pi headers")

set(PLATFORM_LIBREQUIRES "")

# Raspberry Pi libs and headers are in a non-standard path on some distributions
set(RPI_INCLUDE_DIR "" CACHE FILEPATH "root path to Raspberry Pi includes")
set(RPI_LIB_DIR     "" CACHE FILEPATH "root path to Raspberry Pi libs")

# Pulse-Eight devices are always supported
add_definitions(-DHAVE_P8_USB)

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

  list(APPEND CEC_SOURCES_PLATFORM platform/windows/os-edid.cpp
                                   platform/windows/serialport.cpp)
  list(APPEND CEC_SOURCES LibCECDll.cpp
                          libcec.rc)
else()
  # not Windows
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-missing-field-initializers")
  list(APPEND CEC_SOURCES_PLATFORM platform/posix/os-edid.cpp
                                   platform/posix/serialport.cpp)
  set(HAVE_P8_USB_DETECT 0)
  set(LIB_DESTINATION "${CMAKE_INSTALL_LIBDIR}")
  set(LIB_INFO "${LIB_INFO}, features: P8_USB")

  # always try DRM on Linux if other methods fail
  add_definitions(-DHAS_DRM_EDID_PARSER)

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
    list(APPEND CEC_SOURCES_PLATFORM platform/X11/randr-edid.cpp)
    set(HAVE_RANDR 1)
  else()
    set(HAVE_RANDR 0)
  endif()

  # raspberry pi
  find_library(RPI_BCM_HOST bcm_host "${RPI_LIB_DIR}")
  check_library_exists(bcm_host bcm_host_init "${RPI_LIB_DIR}" HAVE_RPI_API)
  if (HAVE_RPI_API)
    find_library(RPI_VCOS vcos "${RPI_LIB_DIR}")
    find_library(RPI_VCHIQ_ARM vchiq_arm "${RPI_LIB_DIR}")
    include_directories(${RPI_INCLUDE_DIR} ${RPI_INCLUDE_DIR}/interface/vcos/pthreads ${RPI_INCLUDE_DIR}/interface/vmcs_host/linux)

    set(LIB_INFO "${LIB_INFO}, 'RPi'")
    set(CEC_SOURCES_ADAPTER_RPI adapter/RPi/RPiCECAdapterDetection.cpp
                                adapter/RPi/RPiCECAdapterCommunication.cpp
                                adapter/RPi/RPiCECAdapterMessageQueue.cpp)
    source_group("Source Files\\adapter\\RPi" FILES ${CEC_SOURCES_ADAPTER_RPI})
    list(APPEND CEC_SOURCES ${CEC_SOURCES_ADAPTER_RPI})
  endif()

  # TDA995x
  check_include_files("tda998x_ioctl.h;comps/tmdlHdmiCEC/inc/tmdlHdmiCEC_Types.h" HAVE_TDA995X_API)
  if (HAVE_TDA995X_API)
    set(LIB_INFO "${LIB_INFO}, 'TDA995x'")
    set(CEC_SOURCES_ADAPTER_TDA995x adapter/TDA995x/TDA995xCECAdapterDetection.cpp
                                    adapter/TDA995x/TDA995xCECAdapterCommunication.cpp)
    source_group("Source Files\\adapter\\TDA995x" FILES ${CEC_SOURCES_ADAPTER_TDA995x})
    list(APPEND CEC_SOURCES ${CEC_SOURCES_ADAPTER_TDA995x})
  endif()

  # Exynos
  if (${HAVE_EXYNOS_API})
    set(LIB_INFO "${LIB_INFO}, 'Exynos'")
    set(HAVE_EXYNOS_API 1)
    set(CEC_SOURCES_ADAPTER_EXYNOS adapter/Exynos/ExynosCECAdapterDetection.cpp
                                   adapter/Exynos/ExynosCECAdapterCommunication.cpp)
    source_group("Source Files\\adapter\\Exynos" FILES ${CEC_SOURCES_ADAPTER_EXYNOS})
    list(APPEND CEC_SOURCES ${CEC_SOURCES_ADAPTER_EXYNOS})
  else()
    set(HAVE_EXYNOS_API 0)
  endif()
endif()

# rt
check_library_exists(rt clock_gettime "" HAVE_RT)

# check for dlopen
check_library_exists(dl dlopen "" HAVE_DLOPEN)

SET(SKIP_PYTHON_WRAPPER 0 CACHE STRING "Define to 1 to not generate the Python wrapper")

if (${SKIP_PYTHON_WRAPPER})
  message(STATUS "Not generating Python wrapper")
else()
  # Python
  include(FindPythonLibs)
  find_package(PythonLibs)

  # Swig
  find_package(SWIG)
  if (PYTHONLIBS_FOUND AND SWIG_FOUND)
    set(CMAKE_SWIG_FLAGS "")
    set(HAVE_PYTHON 1)
    if ("${PYTHONLIBS_VERSION_STRING}" STREQUAL "")
      message(STATUS "Python version not found, defaulting to 2.7")
      set(PYTHONLIBS_VERSION_STRING "2.7.x")
      set(PYTHON_VERSION "2.7")
    else()
      string(REGEX REPLACE "\\.[0-9]+\\+?$" "" PYTHON_VERSION ${PYTHONLIBS_VERSION_STRING})
    endif()

    include(${SWIG_USE_FILE})
    include_directories(${PYTHON_INCLUDE_PATH})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})

    SET_SOURCE_FILES_PROPERTIES(libcec.i PROPERTIES CPLUSPLUS ON)
    swig_add_module(cec python libcec.i)
    swig_link_libraries(cec ${PYTHON_LIBRARIES})
    swig_link_libraries(cec cec)

    if(WIN32)
      install(TARGETS     ${SWIG_MODULE_cec_REAL_NAME}
              DESTINATION python/cec)
      install(FILES       ${CMAKE_BINARY_DIR}/src/libcec/cec.py
              DESTINATION python/cec
              RENAME      __init__.py)
    else()
      install(TARGETS     ${SWIG_MODULE_cec_REAL_NAME}
              DESTINATION lib/python${PYTHON_VERSION}/dist-packages/cec)
      install(FILES       ${CMAKE_BINARY_DIR}/src/libcec/cec.py
              DESTINATION lib/python${PYTHON_VERSION}/dist-packages/cec
              RENAME      __init__.py)
    endif()
  endif()
endif()
