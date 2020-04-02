project(cec)

cmake_minimum_required(VERSION 2.8.9)

set(cec_NAME cec)
set(cec_DESCRIPTION "libCEC")

enable_language(CXX)

include(CheckCXXSourceCompiles)
include(CheckLibraryExists)
include(CheckIncludeFiles)
include(CheckCXXCompilerFlag)
include(../../cmake/UseMultiArch.cmake)

check_cxx_compiler_flag("-std=c++11" SUPPORTS_CXX11)
if (SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

find_package(p8-platform REQUIRED)
if (p8-platform_VERSION VERSION_LESS 2.0)
  message(FATAL_ERROR "p8-platform 2.0+ is required")
endif()

find_package(Threads REQUIRED)

include_directories(${p8-platform_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/../../include)

set(CMAKE_POSITION_INDEPENDENT_CODE on)

# main libCEC files
set(CEC_SOURCES CECClient.cpp
                CECProcessor.cpp
                LibCEC.cpp
                LibCECC.cpp)

# /adapter
set(CEC_SOURCES_ADAPTER adapter/AdapterFactory.cpp)

# /adapter/Pulse-Eight
set(CEC_SOURCES_ADAPTER_P8 adapter/Pulse-Eight/USBCECAdapterMessage.cpp
                           adapter/Pulse-Eight/USBCECAdapterCommands.cpp
                           adapter/Pulse-Eight/USBCECAdapterCommunication.cpp
                           adapter/Pulse-Eight/USBCECAdapterMessageQueue.cpp
                           adapter/Pulse-Eight/USBCECAdapterDetection.cpp)

# /devices
set(CEC_SOURCES_DEVICES devices/CECAudioSystem.cpp
                        devices/CECBusDevice.cpp
                        devices/CECDeviceMap.cpp
                        devices/CECPlaybackDevice.cpp
                        devices/CECRecordingDevice.cpp
                        devices/CECTuner.cpp
                        devices/CECTV.cpp)

# /implementations
set(CEC_SOURCES_IMPLEMENTATIONS implementations/ANCommandHandler.cpp
                                implementations/CECCommandHandler.cpp
                                implementations/SLCommandHandler.cpp
                                implementations/VLCommandHandler.cpp
                                implementations/RLCommandHandler.cpp
                                implementations/PHCommandHandler.cpp
                                implementations/RHCommandHandler.cpp
                                implementations/AQCommandHandler.cpp)

# /platform/*
set(CEC_SOURCES_PLATFORM platform/adl/adl-edid.cpp
                         platform/nvidia/nv-edid.cpp
                         platform/drm/drm-edid.cpp)

# headers
set(CEC_EXT_HEADERS ${PROJECT_SOURCE_DIR}/../../include/cec.h
                    ${PROJECT_SOURCE_DIR}/../../include/cecc.h
                    ${PROJECT_SOURCE_DIR}/../../include/cecloader.h
                    ${PROJECT_SOURCE_DIR}/../../include/ceccloader.h
                    ${PROJECT_SOURCE_DIR}/../../include/cectypes.h
                    ${PROJECT_SOURCE_DIR}/../../include/version.h)
source_group("Header Files (external)" FILES ${CEC_EXT_HEADERS})
set(CEC_HEADERS devices/CECRecordingDevice.h
                devices/CECTuner.h
                devices/CECAudioSystem.h
                devices/CECTV.h
                devices/CECBusDevice.h
                devices/CECDeviceMap.h
                devices/CECPlaybackDevice.h
                adapter/Exynos/ExynosCEC.h
                adapter/Exynos/ExynosCECAdapterDetection.h
                adapter/Exynos/ExynosCECAdapterCommunication.h
                adapter/Linux/LinuxCECAdapterDetection.h
                adapter/Linux/LinuxCECAdapterCommunication.h
                adapter/AOCEC/AOCEC.h
                adapter/AOCEC/AOCECAdapterDetection.h
                adapter/AOCEC/AOCECAdapterCommunication.h
                adapter/Pulse-Eight/USBCECAdapterMessageQueue.h
                adapter/Pulse-Eight/USBCECAdapterCommunication.h
                adapter/Pulse-Eight/USBCECAdapterCommands.h
                adapter/Pulse-Eight/USBCECAdapterDetection.h
                adapter/Pulse-Eight/USBCECAdapterMessage.h
                adapter/TDA995x/TDA995xCECAdapterDetection.h
                adapter/TDA995x/AdapterMessageQueue.h
                adapter/TDA995x/TDA995xCECAdapterCommunication.h
                adapter/AdapterFactory.h
                adapter/AdapterCommunication.h
                adapter/RPi/RPiCECAdapterMessageQueue.h
                adapter/RPi/RPiCECAdapterCommunication.h
                adapter/RPi/RPiCECAdapterDetection.h
                CECInputBuffer.h
                platform/util/baudrate.h
                platform/util/edid.h
                platform/nvidia/nv-edid.h
                platform/adl/adl_structures.h
                platform/adl/adl_defines.h
                platform/adl/adl-edid.h
                platform/adl/adl_sdk.h
                platform/sockets/serialport.h
                platform/X11/randr-edid.h
                CECClient.h
                LibCEC.h
                CECTypeUtils.h
                implementations/SLCommandHandler.h
                implementations/CECCommandHandler.h
                implementations/VLCommandHandler.h
                implementations/RLCommandHandler.h
                implementations/ANCommandHandler.h
                implementations/RHCommandHandler.h
                implementations/PHCommandHandler.h
                implementations/AQCommandHandler.h
                CECProcessor.h)
source_group("Header Files" FILES ${CEC_HEADERS})

# platform and device specific
include(cmake/SetBuildInfo.cmake)
include(cmake/CheckPlatformSupport.cmake)

## create project groups
source_group("Source Files" FILES ${CEC_SOURCES})
source_group("Source Files\\adapter" FILES ${CEC_SOURCES_ADAPTER})
source_group("Source Files\\adapter\\Pulse-Eight" FILES ${CEC_SOURCES_ADAPTER_P8})
source_group("Source Files\\devices" FILES ${CEC_SOURCES_DEVICES})
source_group("Source Files\\implementations" FILES ${CEC_SOURCES_IMPLEMENTATIONS})
source_group("Source Files\\platform" FILES ${CEC_SOURCES_PLATFORM})

list(APPEND CEC_SOURCES ${CEC_HEADERS}
                        ${CEC_EXT_HEADERS}
                        ${CEC_SOURCES_ADAPTER}
                        ${CEC_SOURCES_ADAPTER_P8}
                        ${CEC_SOURCES_DEVICES}
                        ${CEC_SOURCES_IMPLEMENTATIONS}
                        ${CEC_SOURCES_PLATFORM})

# write env.h
set(LIBCEC_LIBREQUIRES "p8-platform ${PLATFORM_LIBREQUIRES}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/env.h.in ${CMAKE_CURRENT_SOURCE_DIR}/env.h)
if (WIN32)
  # write libcec.rc
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libcec.rc.in ${CMAKE_CURRENT_SOURCE_DIR}/libcec.rc)
  add_definitions(-DDLL_EXPORT)
else()
  # write pkgconfig
  include(../../cmake/PkgConfigHandler.cmake)
  configure_pc_file(cec ${CMAKE_CURRENT_SOURCE_DIR}/libcec.pc.in
                        ${CMAKE_CURRENT_SOURCE_DIR}/libcec.pc
                        ${CMAKE_INSTALL_PREFIX}
                        ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
                        ${CMAKE_INSTALL_PREFIX}/include)

  install(FILES         ${CMAKE_CURRENT_SOURCE_DIR}/libcec.pc
          DESTINATION   ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

# install headers
install(FILES ${PROJECT_SOURCE_DIR}/../../include/cec.h
              ${PROJECT_SOURCE_DIR}/../../include/cecc.h
              ${PROJECT_SOURCE_DIR}/../../include/cecloader.h
              ${PROJECT_SOURCE_DIR}/../../include/ceccloader.h
              ${PROJECT_SOURCE_DIR}/../../include/cectypes.h
              ${PROJECT_SOURCE_DIR}/../../include/version.h
        DESTINATION include/libcec)

# libCEC target
add_library(cec SHARED ${CEC_SOURCES})
install(TARGETS cec
        DESTINATION ${LIB_DESTINATION})
set_target_properties(cec PROPERTIES VERSION   ${LIBCEC_VERSION_MAJOR}.${LIBCEC_VERSION_MINOR}.${LIBCEC_VERSION_PATCH}
                                     SOVERSION ${LIBCEC_VERSION_MAJOR})
target_link_libraries(cec ${p8-platform_LIBRARIES})
target_link_libraries(cec ${CMAKE_THREAD_LIBS_INIT})

include(cmake/LinkPlatformSupport.cmake)
include(cmake/DisplayPlatformSupport.cmake)
