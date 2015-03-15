project(libcec)
cmake_minimum_required(VERSION 2.6)

set(LIBCEC_VERSION_MAJOR 2)
set(LIBCEC_VERSION_MINOR 2)
set(LIBCEC_VERSION_PATCH 0)

# cec-client
add_subdirectory(src/cec-client)
add_dependencies(cec-client cec)

# libCEC
add_subdirectory(src/libcec)

# resource files for windows
if(WIN32)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/project/LibCecSharp/LibCecSharp.rc.in
                 ${CMAKE_CURRENT_SOURCE_DIR}/project/LibCecSharp/LibCecSharp.rc)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/CecSharpTester/Properties/AssemblyInfo.cs.in
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/CecSharpTester/Properties/AssemblyInfo.cs)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/LibCecSharp/AssemblyInfo.cpp.in
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/LibCecSharp/AssemblyInfo.cpp)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/LibCecTray/Properties/AssemblyInfo.cs.in
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/LibCecTray/Properties/AssemblyInfo.cs)
endif()
