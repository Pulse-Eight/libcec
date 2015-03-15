project(cec)

cmake_minimum_required(VERSION 2.6)

set(cec_NAME cec)
set(cec_DESCRIPTION "libCEC")

enable_language(CXX)

include(CheckCXXSourceCompiles)
include(CheckLibraryExists)
include(CheckIncludeFiles)
include(../../cmake/UseMultiArch.cmake)

find_package(platform REQUIRED)
find_package(Threads REQUIRED)

include_directories(${platform_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/../../include)

# main libCEC files
set(CEC_SOURCES CECClient.cpp
                CECProcessor.cpp
                LibCEC.cpp
                LibCECC.cpp
                adapter/AdapterFactory.cpp
                adapter/Pulse-Eight/USBCECAdapterMessage.cpp
                adapter/Pulse-Eight/USBCECAdapterCommands.cpp
                adapter/Pulse-Eight/USBCECAdapterCommunication.cpp
                adapter/Pulse-Eight/USBCECAdapterMessageQueue.cpp
                adapter/Pulse-Eight/USBCECAdapterDetection.cpp
                devices/CECAudioSystem.cpp
                devices/CECBusDevice.cpp
                devices/CECDeviceMap.cpp
                devices/CECPlaybackDevice.cpp
                devices/CECRecordingDevice.cpp
                devices/CECTuner.cpp
                devices/CECTV.cpp
                implementations/ANCommandHandler.cpp
                implementations/CECCommandHandler.cpp
                implementations/SLCommandHandler.cpp
                implementations/VLCommandHandler.cpp
                implementations/RLCommandHandler.cpp
                implementations/PHCommandHandler.cpp
                implementations/RHCommandHandler.cpp
                implementations/AQCommandHandler.cpp
                platform/adl/adl-edid.cpp
                platform/nvidia/nv-edid.cpp)

# platform and device specific
include(cmake/CheckPlatformSupport.cmake)

# write env.h
set(LIBCEC_LIBREQUIRES "platform ${PLATFORM_LIBREQUIRES}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/env.h.in ${CMAKE_CURRENT_SOURCE_DIR}/env.h)
if (WIN32)
  # write libcec.rc
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libcec.rc.in ${CMAKE_CURRENT_SOURCE_DIR}/libcec.rc)
else()
  # write pkgconfig
  include(../../cmake/PkgConfigHandler.cmake)
  configure_pc_file(cec ${CMAKE_CURRENT_SOURCE_DIR}/libcec.pc.in
                        ${CMAKE_CURRENT_SOURCE_DIR}/libcec.pc
                        ${CMAKE_INSTALL_PREFIX}
                        ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
                        ${CMAKE_INSTALL_PREFIX}/include)

  install(FILES         ${CMAKE_CURRENT_SOURCE_DIR}/libcec.pc
          DESTINATION   ${CMAKE_INSTALL_LIBDIR_NOARCH}/pkgconfig)
endif()

# install headers
install(FILES ${PROJECT_SOURCE_DIR}/../../include/cec.h
              ${PROJECT_SOURCE_DIR}/../../include/cecc.h
              ${PROJECT_SOURCE_DIR}/../../include/cecloader.h
              ${PROJECT_SOURCE_DIR}/../../include/cectypes.h
        DESTINATION include/libcec)

# libCEC target
add_library(cec ${CEC_SOURCES} ${PLATFORM_SOURCES})
install(TARGETS cec
        DESTINATION ${LIB_DESTINATION})
set_target_properties(cec PROPERTIES VERSION   ${LIBCEC_VERSION_MAJOR}.${LIBCEC_VERSION_MINOR}.${LIBCEC_VERSION_PATCH}
                                     SOVERSION ${LIBCEC_VERSION_MAJOR}.0)
target_link_libraries(cec ${platform_LIBRARIES})
target_link_libraries(cec ${CMAKE_THREAD_LIBS_INIT})

include(cmake/LinkPlatformSupport.cmake)
include(cmake/DisplayPlatformSupport.cmake)

