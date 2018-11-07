# - Check for platform support and set variables and definitions
#
# This module sets the following variables
#       PLATFORM_LIBREQUIRES      dependencies
#       LIB_INFO                  supported features and compilation information
#       LIB_DESTINATION           destination for the .so/.dll files
#       HAVE_RANDR                ON if xrandr is supported
#       HAVE_LIBUDEV              ON if udev is supported
#       HAVE_RPI_API              ON if Raspberry Pi is supported
#       HAVE_TDA995X_API          ON if TDA995X is supported
#       HAVE_EXYNOS_API           ON if Exynos is supported
#       HAVE_AOCEC_API            ON if AOCEC is supported
#       HAVE_P8_USB               ON if Pulse-Eight devices are supported
#       HAVE_P8_USB_DETECT        ON if Pulse-Eight devices can be auto-detected
#       HAVE_DRM_EDID_PARSER      ON if DRM EDID parsing is supported
#

set(RPI_LIB_DIR     "" CACHE STRING "Path to Raspberry Pi libraries")
set(RPI_INCLUDE_DIR "" CACHE STRING "Path to Raspberry Pi headers")

set(PLATFORM_LIBREQUIRES "")

include(CheckFunctionExists)
include(CheckSymbolExists)
include(FindPkgConfig)

# defaults
SET(HAVE_RANDR           OFF CACHE BOOL "xrandr not supported")
SET(HAVE_LIBUDEV         OFF CACHE BOOL "udev not supported")
SET(HAVE_RPI_API         OFF CACHE BOOL "raspberry pi not supported")
SET(HAVE_TDA995X_API     OFF CACHE BOOL "tda995x not supported")
SET(HAVE_EXYNOS_API      OFF CACHE BOOL "exynos not supported")
SET(HAVE_AOCEC_API       OFF CACHE BOOL "aocec not supported")
# Pulse-Eight devices are always supported
set(HAVE_P8_USB          ON  CACHE BOOL "p8 usb-cec supported" FORCE)
set(HAVE_P8_USB_DETECT   OFF CACHE BOOL "p8 usb-cec detection not supported")
set(HAVE_DRM_EDID_PARSER OFF CACHE BOOL "drm edid parser not supported")
# Raspberry Pi libs and headers are in a non-standard path on some distributions
set(RPI_INCLUDE_DIR      ""  CACHE FILEPATH "root path to Raspberry Pi includes")
set(RPI_LIB_DIR          ""  CACHE FILEPATH "root path to Raspberry Pi libs")

if(WIN32)
  # Windows
  add_definitions(-DTARGET_WINDOWS -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -D_WINSOCKAPI_)
  set(LIB_DESTINATION ".")
  check_symbol_exists(_X64_ Windows.h WIN64)
  if (${WIN64})
    set(LIB_INFO "${LIB_INFO} (x64)")
  else()
    add_definitions(-D_USE_32BIT_TIME_T)
  endif()
  set(HAVE_P8_USB_DETECT ON CACHE BOOL "p8 usb-cec detection supported" FORCE)
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
  set(LIB_DESTINATION "${CMAKE_INSTALL_LIBDIR}")
  set(LIB_INFO "${LIB_INFO}, features: P8_USB")

  # always try DRM on Linux if other methods fail
  if(NOT CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    set(HAVE_DRM_EDID_PARSER ON CACHE BOOL "drm edid parser not supported" FORCE)
    set(LIB_INFO "${LIB_INFO}, DRM")
  endif()

  # flock
  check_include_files(sys/file.h HAVE_SYS_FILE_HEADER)
  check_function_exists(flock HAVE_FLOCK)

  # udev
  pkg_check_modules(UDEV udev)
  if (UDEV_FOUND)
    set(PLATFORM_LIBREQUIRES "${PLATFORM_LIBREQUIRES} ${UDEV_LIBRARIES}")
  else()
    # fall back to finding libudev.pc
    pkg_check_modules(UDEV libudev)
    if (UDEV_FOUND)
      set(PLATFORM_LIBREQUIRES "${PLATFORM_LIBREQUIRES} libudev")
    endif()
  endif()
  if (UDEV_FOUND)
    SET(HAVE_LIBUDEV ON CACHE BOOL "udev supported" FORCE)
    set(LIB_INFO "${LIB_INFO}, P8_detect")
    list(APPEND CMAKE_REQUIRED_LIBRARIES "${UDEV_LIBRARIES}")
    set(HAVE_P8_USB_DETECT ON CACHE BOOL "p8 usb-cec detection supported" FORCE)
  endif()

  # xrandr
  check_include_files("X11/Xlib.h;X11/Xatom.h;X11/extensions/Xrandr.h" HAVE_RANDR_HEADERS)
  check_library_exists(Xrandr XRRGetScreenResources "" HAVE_RANDR_LIB)
  if (HAVE_RANDR_HEADERS AND HAVE_RANDR_LIB)
    set(LIB_INFO "${LIB_INFO}, randr")
    list(APPEND CEC_SOURCES_PLATFORM platform/X11/randr-edid.cpp)
    SET(HAVE_RANDR ON CACHE BOOL "xrandr supported" FORCE)
  endif()

  # raspberry pi
  find_library(RPI_BCM_HOST bcm_host "${RPI_LIB_DIR}")
  check_library_exists(bcm_host bcm_host_init "${RPI_LIB_DIR}" HAVE_RPI_LIB)
  if (HAVE_RPI_LIB)
    SET(HAVE_RPI_API ON CACHE BOOL "raspberry pi supported" FORCE)
    find_library(RPI_VCOS vcos "${RPI_LIB_DIR}")
    find_library(RPI_VCHIQ_ARM vchiq_arm "${RPI_LIB_DIR}")
    include_directories(${RPI_INCLUDE_DIR} ${RPI_INCLUDE_DIR}/interface/vcos/pthreads ${RPI_INCLUDE_DIR}/interface/vmcs_host/linux)

    set(LIB_INFO "${LIB_INFO}, RPi")
    set(CEC_SOURCES_ADAPTER_RPI adapter/RPi/RPiCECAdapterDetection.cpp
                                adapter/RPi/RPiCECAdapterCommunication.cpp
                                adapter/RPi/RPiCECAdapterMessageQueue.cpp)
    source_group("Source Files\\adapter\\RPi" FILES ${CEC_SOURCES_ADAPTER_RPI})
    list(APPEND CEC_SOURCES ${CEC_SOURCES_ADAPTER_RPI})
  endif()

  # TDA995x
  check_include_files("tda998x_ioctl.h;comps/tmdlHdmiCEC/inc/tmdlHdmiCEC_Types.h" HAVE_TDA995X_API_INC)
  if (HAVE_TDA995X_API_INC)
    SET(HAVE_TDA995X_API ON CACHE BOOL "tda995x supported" FORCE)
    set(LIB_INFO "${LIB_INFO}, TDA995x")
    set(CEC_SOURCES_ADAPTER_TDA995x adapter/TDA995x/TDA995xCECAdapterDetection.cpp
                                    adapter/TDA995x/TDA995xCECAdapterCommunication.cpp)
    source_group("Source Files\\adapter\\TDA995x" FILES ${CEC_SOURCES_ADAPTER_TDA995x})
    list(APPEND CEC_SOURCES ${CEC_SOURCES_ADAPTER_TDA995x})
  endif()

  # Exynos
  if (${HAVE_EXYNOS_API})
    set(LIB_INFO "${LIB_INFO}, Exynos")
    SET(HAVE_EXYNOS_API ON CACHE BOOL "exynos supported" FORCE)
    set(CEC_SOURCES_ADAPTER_EXYNOS adapter/Exynos/ExynosCECAdapterDetection.cpp
                                   adapter/Exynos/ExynosCECAdapterCommunication.cpp)
    source_group("Source Files\\adapter\\Exynos" FILES ${CEC_SOURCES_ADAPTER_EXYNOS})
    list(APPEND CEC_SOURCES ${CEC_SOURCES_ADAPTER_EXYNOS})
  endif()

  # AOCEC
  if (${HAVE_AOCEC_API})
    set(LIB_INFO "${LIB_INFO}, AOCEC")
    SET(HAVE_AOCEC_API ON CACHE BOOL "AOCEC supported" FORCE)
    set(CEC_SOURCES_ADAPTER_AOCEC adapter/AOCEC/AOCECAdapterDetection.cpp
                                   adapter/AOCEC/AOCECAdapterCommunication.cpp)
    source_group("Source Files\\adapter\\AOCEC" FILES ${CEC_SOURCES_ADAPTER_AOCEC})
    list(APPEND CEC_SOURCES ${CEC_SOURCES_ADAPTER_AOCEC})
  else()
    set(HAVE_AOCEC_API 0)
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
    set(CMAKE_SWIG_FLAGS "-threads")
    set(HAVE_PYTHON 1)
    if ("${PYTHONLIBS_VERSION_STRING}" STREQUAL "")
      message(STATUS "Python version not found, defaulting to 2.7")
      set(PYTHONLIBS_VERSION_STRING "2.7.x")
      set(PYTHON_VERSION "2.7")
    else()
      string(REGEX REPLACE "\\.[0-9]+\\+?$" "" PYTHON_VERSION ${PYTHONLIBS_VERSION_STRING})
    endif()
    string(REGEX REPLACE "\\..*$" "" PYTHON_MAJOR_VERSION ${PYTHON_VERSION})
    string(REGEX REPLACE "^.*\\." "" PYTHON_MINOR_VERSION ${PYTHON_VERSION})

    include(${SWIG_USE_FILE})
    include_directories(${PYTHON_INCLUDE_PATH})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})

    SET_SOURCE_FILES_PROPERTIES(libcec.i PROPERTIES CPLUSPLUS ON)
    swig_add_module(cec python libcec.i)
    swig_link_libraries(cec ${PYTHON_LIBRARIES})
    swig_link_libraries(cec cec)

    SET(PYTHON_LIB_INSTALL_PATH "/cec" CACHE STRING "python lib path")
    if (${PYTHON_MAJOR_VERSION} EQUAL 2 AND ${PYTHON_MINOR_VERSION} GREATER 6)
	  SET(PYTHON_LIB_INSTALL_PATH "" CACHE STRING "python lib path" FORCE)
    else()
      if (${PYTHON_MAJOR_VERSION} GREATER 2)
        SET(PYTHON_LIB_INSTALL_PATH "" CACHE STRING "python lib path" FORCE)
      endif()
    endif()

    if(WIN32)
      install(TARGETS     ${SWIG_MODULE_cec_REAL_NAME}
              DESTINATION python/${PYTHON_LIB_INSTALL_PATH})
      install(FILES       ${CMAKE_BINARY_DIR}/src/libcec/cec.py
              DESTINATION python/cec)
      if (${PYTHON_MAJOR_VERSION} EQUAL 2)
        install(FILES ${CMAKE_SOURCE_DIR}/src/libcec/cmake/__init__.py
                DESTINATION python/cec)
      endif()
    else()
      if(EXISTS "/etc/os-release")
        file(READ "/etc/os-release" OS_RELEASE)
        string(REGEX MATCH "ID(_LIKE)?=debian" IS_DEBIAN ${OS_RELEASE})
        if (IS_DEBIAN)
          SET(PYTHON_PKG_DIR "dist-packages")
        endif()
      endif()

      if (NOT PYTHON_PKG_DIR)
        SET(PYTHON_PKG_DIR "site-packages")
      endif()

      if (${PYTHON_MAJOR_VERSION} EQUAL 2)
        install(TARGETS     ${SWIG_MODULE_cec_REAL_NAME}
                DESTINATION lib/python${PYTHON_VERSION}/${PYTHON_PKG_DIR}/${PYTHON_LIB_INSTALL_PATH}/cec)
        install(FILES       ${CMAKE_BINARY_DIR}/src/libcec/cec.py
                DESTINATION lib/python${PYTHON_VERSION}/${PYTHON_PKG_DIR})
        install(FILES ${CMAKE_SOURCE_DIR}/src/libcec/cmake/__init__.py
                DESTINATION lib/python${PYTHON_VERSION}/${PYTHON_PKG_DIR}/cec)
      else()
        install(TARGETS     ${SWIG_MODULE_cec_REAL_NAME}
                DESTINATION lib/python${PYTHON_VERSION}/${PYTHON_PKG_DIR}/${PYTHON_LIB_INSTALL_PATH})
        install(FILES       ${CMAKE_BINARY_DIR}/src/libcec/cec.py
                DESTINATION lib/python${PYTHON_VERSION}/${PYTHON_PKG_DIR})
      endif()
    endif()
  endif()
endif()
