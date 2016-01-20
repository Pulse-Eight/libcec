project(libcec)
cmake_minimum_required(VERSION 2.8.9)

set(LIBCEC_VERSION_MAJOR 3)
set(LIBCEC_VERSION_MINOR 1)
set(LIBCEC_VERSION_PATCH 0)

# cec-client
add_subdirectory(src/cec-client)
add_dependencies(cec-client cec)

# cecc-client
add_subdirectory(src/cecc-client)
add_dependencies(cecc-client cec)

# pyCecClient
add_subdirectory(src/pyCecClient)

# libCEC
add_subdirectory(src/libcec)

# version number
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/version.h.in
               ${CMAKE_CURRENT_SOURCE_DIR}/include/version.h)

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
