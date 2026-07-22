cmake_minimum_required(VERSION 3.12.0)
project(libcec)

set(LIBCEC_VERSION_MAJOR 8)
set(LIBCEC_VERSION_MINOR 0)
set(LIBCEC_VERSION_PATCH 0)

if(NOT WIN32)
  # set here rather than in CheckPlatformSupport.cmake, which only gets included
  # from src/libcec: cmake scopes these per directory, so the clients inherit
  # them only when they're set before add_subdirectory().
  # -Wno-deprecated-copy is C++ only and warns when passed to the C compiler
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-missing-field-initializers")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-missing-field-initializers -Wno-deprecated-copy")
endif()

# The client applications (cec-client, cecc-client, pyCecClient and, on Windows,
# the .NET wrappers and installer) are built by default. Set DISABLE_CLIENT to
# build only the library, e.g. for embedded targets that just link libcec.
option(DISABLE_CLIENT "Do not build the CEC client applications" OFF)

# When set, the library version string reports only the version, instead of also
# embedding the git revision, build date, and building user/host. Useful for
# reproducible or embedded builds.
option(DISABLE_BUILDINFO "Do not embed git/date/user/host build info in the version string" OFF)

# The static library is built alongside the shared one by default. Set
# DISABLE_STATIC to build only the shared library.
option(DISABLE_STATIC "Do not build the static library" OFF)

if(NOT DISABLE_CLIENT)
  # cec-client
  add_subdirectory(src/cec-client)
  add_dependencies(cec-client cec-shared)

  # cecc-client
  add_subdirectory(src/cecc-client)
  add_dependencies(cecc-client cec-shared)

  # pyCecClient
  add_subdirectory(src/pyCecClient)
endif()

# libCEC
add_subdirectory(src/libcec)

# version number
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/version.h.in
               ${CMAKE_CURRENT_SOURCE_DIR}/include/version.h)

# windows specific files
if(WIN32)
  if(NOT DISABLE_CLIENT)
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
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/windows/version.py.in
                 ${CMAKE_CURRENT_SOURCE_DIR}/windows/version.py)
  endif()
endif()
