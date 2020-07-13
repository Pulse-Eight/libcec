project(libcec)
cmake_minimum_required(VERSION 3.12.0)

set(LIBCEC_VERSION_MAJOR 6)
set(LIBCEC_VERSION_MINOR 0)
set(LIBCEC_VERSION_PATCH 2)

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

# windows specific files
if(WIN32)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/project/nsis/libcec-version.nsh.in
                 ${CMAKE_CURRENT_SOURCE_DIR}/project/nsis/libcec-version.nsh)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/dotnetlib/LibCecSharp/LibCecSharp.rc.in
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/dotnetlib/LibCecSharp/LibCecSharp.rc)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/dotnetlib/LibCecSharp/AssemblyInfo.cpp.in
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/dotnetlib/LibCecSharp/AssemblyInfo.cpp)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/dotnetlib/LibCecSharpCore/AssemblyInfo.cpp.in
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/dotnetlib/LibCecSharpCore/AssemblyInfo.cpp)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/dotnet/src/LibCecTray/LibCECTray.csproj.in
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/dotnet/src/LibCecTray/LibCECTray.csproj)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/dotnet/src/LibCecTray/Properties/AssemblyInfo.cs.in
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/dotnet/src/LibCecTray/Properties/AssemblyInfo.cs)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/dotnet/src/CecSharpTester/netcore/CecSharpCoreTester.csproj.in
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/dotnet/src/CecSharpTester/netcore/CecSharpCoreTester.csproj)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/dotnet/src/CecSharpTester/netfx/CecSharpTester.csproj.in
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/dotnet/src/CecSharpTester/netfx/CecSharpTester.csproj)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/dotnet/src/CecSharpTester/netfx/Properties/AssemblyInfo.cs.in
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/dotnet/src/CecSharpTester/netfx/Properties/AssemblyInfo.cs)
endif()
