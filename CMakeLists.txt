cmake_minimum_required(VERSION 3.12.0)
project(libcec)

set(LIBCEC_VERSION_MAJOR 8)
set(LIBCEC_VERSION_MINOR 1)
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

# The managed .NET binding and apps are optional and OFF by default, so an
# ordinary build never requires the .NET SDK. ENABLE_DOTNET_LIB builds the
# pure-C# LibCecSharp binding (any platform with the .NET SDK); ENABLE_DOTNET_APPS
# additionally builds the Windows-only .NET apps (cec-tray + CecSharpTester) and
# implies ENABLE_DOTNET_LIB. Both are built with `dotnet build`, so the native
# build does not depend on them.
option(ENABLE_DOTNET_LIB  "Build the managed .NET LibCecSharp binding" OFF)
option(ENABLE_DOTNET_APPS "Build the managed .NET apps (cec-tray, CecSharpTester); Windows only" OFF)

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

# managed .NET binding + apps (optional; both OFF by default - see the options
# above). These build the SDK-style C# projects with `dotnet build`, so they work
# on any platform with the .NET SDK and never enter the native build graph.
if(ENABLE_DOTNET_APPS)
  set(ENABLE_DOTNET_LIB ON)
endif()

if(ENABLE_DOTNET_LIB AND NOT DISABLE_CLIENT)
  find_program(DOTNET_EXECUTABLE NAMES dotnet)
  if(NOT DOTNET_EXECUTABLE)
    message(FATAL_ERROR "ENABLE_DOTNET_LIB/ENABLE_DOTNET_APPS require the .NET SDK ('dotnet' on PATH)")
  endif()

  # dotnet build configuration + platform
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(_dotnet_config "Debug")
  else()
    set(_dotnet_config "Release")
  endif()
  # DOTNET_ARCH is passed by the Windows orchestrator (x64/x86). Default to x64 on
  # Windows (the apps' project platforms) and AnyCPU elsewhere.
  if(NOT DOTNET_ARCH)
    if(WIN32)
      set(DOTNET_ARCH "x64")
    else()
      set(DOTNET_ARCH "AnyCPU")
    endif()
  endif()

  # the pure-C# LibCecSharp binding (net8.0), built into the shared build tree.
  # `dotnet pack` builds the assembly into the OutputPath the Windows installer
  # expects and additionally emits a NuGet package into the cmake build tree.
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/dotnetlib/LibCecSharp.csproj.in
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/dotnetlib/LibCecSharp.csproj)
  set(_dotnet_nugetdir ${CMAKE_CURRENT_BINARY_DIR}/nuget)
  add_custom_target(LibCecSharp ALL
    COMMAND ${DOTNET_EXECUTABLE} pack ${CMAKE_CURRENT_SOURCE_DIR}/src/dotnetlib/LibCecSharp.csproj
            -c ${_dotnet_config} -p:Platform=${DOTNET_ARCH} -o ${_dotnet_nugetdir}
    VERBATIM
    COMMENT "Building + packing managed LibCecSharp binding (net8.0, ${DOTNET_ARCH})")

  # install the assembly, its XML doc and the NuGet package (used by the Debian
  # libcec-dotnet package). The assembly is AnyCPU IL, so it lives in a plain,
  # architecture-independent lib/libcec dir rather than the multiarch triplet dir.
  # OutputPath in the csproj is relative to the project, so the build tree path is
  # fixed regardless of the cmake build dir.
  set(_dotnet_outdir ${CMAKE_CURRENT_SOURCE_DIR}/build/${_dotnet_config}/${DOTNET_ARCH}/net8.0)
  install(FILES       ${_dotnet_outdir}/LibCecSharp.dll
                      ${_dotnet_outdir}/LibCecSharp.xml
          DESTINATION lib/libcec)
  install(DIRECTORY   ${_dotnet_nugetdir}/
          DESTINATION share/libcec-dotnet
          FILES_MATCHING PATTERN "*.nupkg")

  if(ENABLE_DOTNET_APPS)
    if(NOT WIN32)
      message(FATAL_ERROR "ENABLE_DOTNET_APPS is Windows-only (cec-tray is a WinForms app)")
    endif()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/dotnet/src/LibCecTray/LibCECTray.csproj.in
                   ${CMAKE_CURRENT_SOURCE_DIR}/src/dotnet/src/LibCecTray/LibCECTray.csproj)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/dotnet/src/LibCecTray/Properties/AssemblyInfo.cs.in
                   ${CMAKE_CURRENT_SOURCE_DIR}/src/dotnet/src/LibCecTray/Properties/AssemblyInfo.cs)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/dotnet/src/CecSharpTester/netcore/CecSharpTester.csproj.in
                   ${CMAKE_CURRENT_SOURCE_DIR}/src/dotnet/src/CecSharpTester/netcore/CecSharpTester.csproj)
    add_custom_target(cec-dotnet-apps ALL
      COMMAND ${DOTNET_EXECUTABLE} build ${CMAKE_CURRENT_SOURCE_DIR}/src/dotnet/src/CecSharpTester/netcore/CecSharpTester.csproj
              -c ${_dotnet_config} -p:Platform=${DOTNET_ARCH}
      COMMAND ${DOTNET_EXECUTABLE} build ${CMAKE_CURRENT_SOURCE_DIR}/src/dotnet/src/LibCecTray/LibCECTray.csproj
              -c ${_dotnet_config} -p:Platform=${DOTNET_ARCH}
      VERBATIM
      COMMENT "Building managed .NET apps (cec-tray, CecSharpTester)")
    add_dependencies(cec-dotnet-apps LibCecSharp)
  endif()
endif()

# windows specific files
if(WIN32)
  if(NOT DISABLE_CLIENT)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/project/nsis/libcec-version.nsh.in
                 ${CMAKE_CURRENT_SOURCE_DIR}/project/nsis/libcec-version.nsh)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/windows/version.py.in
                 ${CMAKE_CURRENT_SOURCE_DIR}/windows/version.py)
  endif()
endif()
