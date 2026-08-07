cmake_minimum_required(VERSION 3.12.0)
project(libcec)

set(LIBCEC_VERSION_MAJOR 8)
set(LIBCEC_VERSION_MINOR 1)
set(LIBCEC_VERSION_PATCH 6)

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

# The Node.js binding is a native N-API addon, also OFF by default so an ordinary
# build needs neither Node.js nor a second compile step. Built with npm/node-gyp,
# so it never enters the native build graph either.
option(ENABLE_NODE_LIB "Build the Node.js binding (native N-API addon)" OFF)

# The Rust binding, likewise OFF by default. It has no crate dependencies, so
# unlike the Node option this one needs no network: cargo builds it --offline.
option(ENABLE_RUST_LIB "Build the Rust binding" OFF)

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

# Node.js binding (optional; OFF by default - see the option above). Built with
# node-gyp via npm, so it never enters the native build graph. `npm install`
# compiles src/nodejs/src/addon.cc against the just-built libCEC found through
# pkg-config, and lands the addon at build/Release/cec_native.node.
if(ENABLE_NODE_LIB AND NOT DISABLE_CLIENT)
  find_program(NPM_EXECUTABLE NAMES npm)
  if(NOT NPM_EXECUTABLE)
    message(FATAL_ERROR "ENABLE_NODE_LIB requires npm on PATH")
  endif()

  set(_node_srcdir ${CMAKE_CURRENT_SOURCE_DIR}/src/nodejs)
  # PKG_CONFIG_PATH lets node-gyp's pkg-config find this tree's freshly-installed
  # libcec.pc; the caller sets it (or relies on a system libCEC).
  add_custom_target(libcec-node ALL
    COMMAND ${NPM_EXECUTABLE} install --build-from-source --no-audit --no-fund
    WORKING_DIRECTORY ${_node_srcdir}
    VERBATIM
    COMMENT "Building the Node.js binding (native N-API addon)")

  # Install as a global node module. index.js requires ../build/Release from
  # lib/, so both dirs keep their relative layout under the module root. The
  # .node is arch-specific but node resolves global modules from a flat
  # lib/node_modules regardless of arch, matching npm's own default prefix.
  install(DIRECTORY   ${_node_srcdir}/lib/
          DESTINATION lib/node_modules/libcec/lib)
  install(FILES       ${_node_srcdir}/build/Release/cec_native.node
          DESTINATION lib/node_modules/libcec/build/Release)
  install(FILES       ${_node_srcdir}/package.json
                      ${_node_srcdir}/README.md
          DESTINATION lib/node_modules/libcec)
endif()

# Rust binding (optional; OFF by default - see the option above). Built with
# cargo, so like the .NET and Node targets it never enters the native build
# graph. The crate has no dependencies, so --offline works and no vendored
# registry or network access is needed - which is what lets the Debian package
# use this option directly rather than driving the build itself.
if(ENABLE_RUST_LIB AND NOT DISABLE_CLIENT)
  find_program(CARGO_EXECUTABLE NAMES cargo)
  if(NOT CARGO_EXECUTABLE)
    message(FATAL_ERROR "ENABLE_RUST_LIB requires cargo on PATH")
  endif()

  set(_rust_srcdir ${CMAKE_CURRENT_SOURCE_DIR}/src/rust)
  set(LIBCEC_VERSION ${LIBCEC_VERSION_MAJOR}.${LIBCEC_VERSION_MINOR}.${LIBCEC_VERSION_PATCH})

  # Build the examples, not just the library: an rlib is never linked, so
  # `cargo build` alone would compile the bindings without ever proving they
  # resolve against libCEC. The examples are what turns this into a link test.
  #
  # LIBCEC_LIB_DIR points build.rs at the library just built here rather than an
  # installed one, and CARGO_HOME keeps cargo out of the building user's home
  # directory, which a package build has no business writing to.
  add_custom_target(libcec-rust ALL
    COMMAND ${CMAKE_COMMAND} -E env
            LIBCEC_LIB_DIR=$<TARGET_FILE_DIR:cec-shared>
            CARGO_HOME=${CMAKE_CURRENT_BINARY_DIR}/cargo-home
            ${CARGO_EXECUTABLE} build --offline --release --examples
            --manifest-path ${_rust_srcdir}/Cargo.toml
            --target-dir ${CMAKE_CURRENT_BINARY_DIR}/rust
    VERBATIM
    COMMENT "Building the Rust binding (cargo, offline)")
  add_dependencies(libcec-rust cec-shared)

  # Install the crate *sources*, which is how a Rust library is shipped: there
  # is no stable Rust ABI, so a compiled rlib would only be usable by the exact
  # compiler that built it. This is the layout Debian's dh-cargo produces and
  # what a `[source] directory` replacement in .cargo/config.toml consumes.
  set(_rust_registry share/cargo/registry/libcec-${LIBCEC_VERSION})

  # cargo's directory source wants this file next to the crate. An empty `files`
  # map means "verify nothing", which is right here: the sources are owned by
  # the package manager, not downloaded from crates.io.
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/cargo-checksum.json "{\"package\":null,\"files\":{}}")

  install(DIRECTORY   ${_rust_srcdir}/src/
          DESTINATION ${_rust_registry}/src)
  install(DIRECTORY   ${_rust_srcdir}/examples/
          DESTINATION ${_rust_registry}/examples)
  install(DIRECTORY   ${_rust_srcdir}/tests/
          DESTINATION ${_rust_registry}/tests)
  install(FILES       ${_rust_srcdir}/Cargo.toml
                      ${_rust_srcdir}/build.rs
                      ${_rust_srcdir}/README.md
          DESTINATION ${_rust_registry})
  install(FILES       ${CMAKE_CURRENT_BINARY_DIR}/cargo-checksum.json
          DESTINATION ${_rust_registry}
          RENAME      .cargo-checksum.json)
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
