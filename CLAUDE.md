# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

libCEC is a cross-platform C++ library for controlling CEC-capable hardware (TVs, AV receivers, etc.) over HDMI, primarily via Pulse-Eight's USB-CEC adapter and SoC-native CEC on Linux/Raspberry Pi. It exposes C, C++, Python (via SWIG), .NET, and Node.js interfaces over the same core engine. The shared/static library output is named `cec` (`libcec.so` / `cec.dll`).

The Node.js binding **`libcec`** lives in `src/nodejs` in this repo. It is a **native N-API addon** (`node-addon-api`, built with `node-gyp`) that binds libCEC via the C API (`include/cecc.h`) — the same surface the .NET binding uses. `src/nodejs/src/addon.cc` is the C++ addon (an `EventEmitter`-based `CecAdapter`); libCEC fires its `ICECCallbacks` from its own worker thread, so each C trampoline copies its payload and re-enters JS via a `Napi::ThreadSafeFunction`. The `commandHandler`/`menuStateChanged` callbacks return "not handled" (0) synchronously — honouring a JS return would mean blocking libCEC's callback thread on the event loop and racing its 1000ms timeout. `src/nodejs/lib/` is the JS wrapper (enums + the EventEmitter surface).

`binding.gyp` is per-platform: Unix gets its cflags/libs from `pkg-config` and `addon.cc` includes the headers under a `<libcec/…>` prefix (the Debian `/usr/include/libcec` layout); Windows has no pkg-config, so it points at libCEC's *flat* headers and `cec.lib` via the `LIBCEC_INCLUDE_DIR` / `LIBCEC_LIB_DIR` environment variables and `addon.cc` includes them flat (`#if defined(_WIN32)`). Because N-API is ABI-stable, one prebuilt addon works for any Node ≥ 16, so the Windows installer ships a *prebuilt* one: `create-installer.py`'s `NodeJsBuilder` runs `node-gyp` (pointed at the repo headers + this build's `cec.lib`), stages a self-contained `nodejs/` tree — the addon with a co-located `cec.dll` so it loads without the install dir on `PATH` — and the NSIS "libCEC for Node.js" component (`project/nsis/nodejs.nsh`, gated by `NSISNODEJS`) installs it. It's best-effort (skipped, non-fatal, if Node isn't installed; `-nn` disables it). Debian ships the same prebuilt as the `node-libcec` package.

The managed binding **`LibCecSharp`** lives in `src/dotnetlib` in this repo. It is a **pure C# assembly** (namespace `CecSharp`) that binds libCEC via P/Invoke over the C API (`include/cecc.h` → `LibCECC.cpp`), targets **net8.0**, and compiles with `dotnet build` (no MSVC `/clr`) — so unlike the old C++/CLI wrappers it works on Linux/macOS/RPi as well as Windows. It replaced two Windows-only C++/CLI wrappers (`LibCecSharp` for .NET Framework + `LibCecSharpCore` for net8.0), unifying them into one assembly. The `.NET` client apps (cec-tray, CecSharpTester) live in the `src/dotnet` git submodule (the `cec-dotnet` repo) and both reference this one binding; `cec-tray` is a Windows-only WinForms app (net8.0-windows), `CecSharpTester` is a cross-platform net8.0 console sample.

## Submodules

Both submodules are **only used by the Windows build** — init them there first:

```
git submodule update --init --recursive
```

- `src/dotnet` — the cec-dotnet .NET apps (Windows installer only).
- `support` — Windows driver installers / signing helpers (libcec-support repo). Also holds the cmake flag overrides (`support\windows\cmake\{c,cxx}-flag-overrides.cmake`) that `create-installer.py` passes to **every** Windows cmake run, so it's needed to compile at all, not just to package.

A Linux/OS X/BSD build works from a non-recursive clone with no submodules checked out at all.

**libCEC has no third-party C++ dependency for threading, time or IO.** It used to link p8-platform for that; everything it used is in the standard library and is now implemented directly on `std::` under `src/libcec/platform/`. Don't reintroduce it.

## Building

### Linux / BSD / macOS / Raspberry Pi
Standard CMake out-of-source build:

```
mkdir build && cd build
cmake ..
make -j4
sudo make install && sudo ldconfig
```

Platform-native CEC backends are **off by default** and selected with cmake flags (only one applies per target): `-DHAVE_LINUX_API=1` (Linux CEC framework, kernel 4.10+), `-DHAVE_RPI_API=1`, `-DHAVE_EXYNOS_API=1`, `-DHAVE_AOCEC_API=1`, `-DHAVE_TDA995X_API=1`, `-DHAVE_IMX_API=1`. Without one, only the Pulse-Eight USB adapter backend is built. See `src/libcec/cmake/CheckPlatformSupport.cmake` for the full detection logic.

The managed .NET binding is also a cmake option, **off by default** so a normal build never needs the .NET SDK: `-DENABLE_DOTNET_LIB=1` builds the pure-C# `LibCecSharp` binding via `dotnet build` (any platform with the SDK), and `-DENABLE_DOTNET_APPS=1` additionally builds the Windows-only .NET apps (cec-tray, CecSharpTester) and implies `ENABLE_DOTNET_LIB`. These are `dotnet build` custom targets in the top-level `CMakeLists.txt`; they never enter the native build graph. `DOTNET_ARCH` (default x64 on Windows, AnyCPU elsewhere) sets the managed `-p:Platform`.

The Node.js binding is likewise a cmake option, **off by default**: `-DENABLE_NODE_LIB=1` builds the native addon via `npm install` (runs `node-gyp`, compiling `src/nodejs` against a `pkg-config`-discoverable libCEC) and installs it as a global node module under `lib/node_modules/libcec`. Like the .NET targets it's a custom target, never in the native build graph. The Debian `node-libcec` package does **not** use this option (npm needs network); `debian/rules` builds the addon with `node-gyp` against the staged libCEC via `PKG_CONFIG_SYSROOT_DIR` instead.

### Windows
Do **not** invoke cmake/msbuild directly — use the Python build orchestrator (`windows/create-installer.py`), which drives cmake to compile libCEC (C/C++/Python) **and** the managed binding + apps (it passes `-DENABLE_DOTNET_LIB=1 -DENABLE_DOTNET_APPS=1`, so cmake owns the `dotnet build`; the orchestrator no longer shells out to `dotnet`/`devenv` itself), then builds the EventGhost plugin and packages an NSIS installer. Requires Visual Studio (default toolchain `2022c` = VS2022 Community), CMake, the .NET SDK (net8.0), and Python 3.12+ (uses `match`/PEP 604 union syntax). The installer checks for the **.NET 8 Desktop Runtime** and installs it when the managed component is selected and it's missing (`project/nsis/dotnet_runtime.nsh`).

```
python windows\create-installer.py            # full build + installer -> dist\libcec-<arch>-<ver>.exe
python windows\create-installer.py -ni        # build libCEC + LibCecSharp, no installer
python windows\create-installer.py -vs         # generate Visual Studio project files for development
```

Useful flags: `-a {x64,x86,arm,arm64}` (default x64), `-m {Release,Debug,RelWithDebInfo}` (default Release), `-t <toolchain>` (e.g. `2019c`, `2022`, `2026c`), `-nc` (no clean / incremental), `-ne` (skip EventGhost plugin), `-ni` (no installer). Build artifacts land in `build\<target>\<arch>\`. The orchestrator's structure is in `windows/toolchain.py` (toolchain/arch enums) and `windows/mixins.py` / `windows/pathbuilder.py` (helpers).

`project/libcec.sln` opens the C# `LibCecSharp` binding (`src/dotnetlib/LibCecSharp.csproj`, an SDK-style project) for development in Visual Studio; the .NET apps solution is `src/dotnet/project/cec-dotnet.sln` (cec-tray + CecSharpTester). The actual build goes through cmake's `ENABLE_DOTNET_*` targets — all SDK-based, no `/clr`.

### Debian / Ubuntu packaging
`debian/` builds the `.deb` set (`dpkg-buildpackage`; see `docs/README.debian.md`). The runtime package is **`libcec8`** — named after the SONAME (`= LIBCEC_VERSION_MAJOR`), so it is renamed on every major bump and `Breaks`/`Replaces`/`Provides` the older `libcec4`-`libcec7` names; `libcec8-dev` likewise supersedes the old `-dev` names. The other packages are `cec-utils` (cec-client + cecc-client), `python-libcec`, `libcec-dotnet` (the managed binding + its NuGet package, built by passing `-DENABLE_DOTNET_LIB=1`), and a `libcec` meta package. `debian/rules` also enables the Linux/Exynos/AOCEC backends and reproducible-build flags. `debian/changelog.in` (not `changelog`) is the source — `#DIST#` is substituted per distribution at build time.

### API documentation
`docs/api/` holds the per-binding API reference, one best-of-breed generator each: **Doxygen** for C/C++ (`docs/api/doxygen/`, main page in `mainpage.md`), **DocFX** for .NET (`docs/api/dotnet/`, compiles a docs-only `docs.csproj` that globs the same `cs/` sources), **TypeDoc** for Node.js (`docs/api/nodejs/`, renders the hand-authored `src/nodejs/index.d.ts` — which also ships to consumers via `package.json` `types`), and **Sphinx** for Python (`docs/api/python/`, autodoc over a `swig -doxygen`-generated `cec.py` with the native `_cec` extension mocked, so no libCEC compile is needed). A landing page (`docs/api/landing/`) links the four; `docs/api/assets/` holds the shared Pulse-Eight logo. `.github/workflows/docs.yml` builds all four and deploys to GitHub Pages — **published at https://pulse-eight.github.io/libcec/**. Generated output is git-ignored; see `docs/api/README.md` for local build steps. The welcome pages carry the install+usage guide for each binding, so keep them current when an API surface changes.

### Tests
There is no automated test suite. Verification is manual via the example clients run against real CEC hardware:
- `cec-client` (C++, `src/cec-client/cec-client.cpp`) — interactive CLI; the primary smoke test.
- `cecc-client` (C example), `pyCecClient` (Python example).

## Architecture

The core lives in `src/libcec/`. Data flow, outermost to innermost:

1. **Public API** — `include/cec.h` defines `ICECAdapter` (C++ interface). `CLibCEC` (`LibCEC.cpp`) implements it and is the object handed to clients. `LibCECC.cpp` wraps it for the C API (`cecc.h`); `libcec.i` + `SwigHelper.h` generate the Python binding; `src/dotnetlib` binds it for .NET (pure C# P/Invoke over the C API — the interop structs in `src/dotnetlib/cs/CecInterop.cs` mirror `cectypes.h` byte-for-byte). `include/cectypes.h` holds all shared enums/structs/opcodes and is the single source of truth for the protocol surface.

2. **`CCECClient`** (`CECClient.cpp`) — represents one logical configuration/connection: which logical addresses this instance claims, the active `libcec_configuration`, and the callback registration. Multiple clients can attach to one processor.

3. **`CCECProcessor`** (`CECProcessor.cpp`) — the engine. Owns the bus state, the worker thread that pumps incoming/outgoing CEC frames, logical-address allocation, and routing of commands to the right handler. This is where most protocol behavior is coordinated.

4. **`devices/`** — `CCECBusDevice` and subclasses (`CECTV`, `CECAudioSystem`, `CECPlaybackDevice`, `CECRecordingDevice`, `CECTuner`) model the *state* of each device on the CEC bus (power status, vendor id, physical/logical address, etc.). `CECDeviceMap` tracks all 15 logical addresses.

5. **`implementations/`** — per-vendor `CCECCommandHandler` subclasses implement quirky vendor behavior (`SL`=Samsung/older, `VL`=Panasonic, `RL`=Toshiba, `PH`=Philips, `RH`, `AN`=Onkyo/Sharp, `AQ`). `CECCommandHandler` is the generic base. The processor instantiates the matching handler based on the device's reported vendor id.

6. **`adapter/`** — pluggable hardware backends behind `IAdapterCommunication` (`AdapterCommunication.h`), constructed by `AdapterFactory`. `Pulse-Eight/` is the USB serial adapter (the cross-platform default; `USBCECAdapterCommunication` + a message-queue/command protocol). `Linux/`, `RPi/`, `Exynos/`, `AOCEC/`, `TDA995x/`, `IMX/`, `Tegra/` are SoC-native backends compiled in only when their `HAVE_*_API` flag is set.

7. **`platform/`** (inside `src/libcec/`) — libCEC's own OS abstraction, all of it. The `std::`-backed threading/time/buffer helpers (`threads/`, `util/`), the socket and serial-port code (`sockets/`, `posix/`, `windows/`), and the EDID readers used to discover the device's own physical address from the GPU: `adl/` (AMD), `nvidia/`, `drm/`, `X11/`.

### Key conventions
- Vendor-specific handling goes in a `*CommandHandler` under `implementations/`, keyed by vendor id in `cectypes.h` — not scattered through the processor.
- A new hardware transport means a new `adapter/<name>/` backend implementing `IAdapterCommunication`, wired into `AdapterFactory` and gated by a `HAVE_*_API` cmake flag in `CheckPlatformSupport.cmake`.
- `include/version.h`, `src/libcec/env.h`, `src/libcec/libcec.pc` and many Windows project files are **generated** from `.in` templates by cmake — edit the `.in`, never the generated file. The version is defined in the top-level `CMakeLists.txt` (`LIBCEC_VERSION_*`).
- Those are generated **into the source tree, not the build dir**, so build dirs are not independent: configuring one with `-DHAVE_TEGRA_API=1` rewrites the shared `env.h`, and a *different* build dir then compiles with the flag set but without the Tegra sources, failing at link with an undefined `TegraCECAdapterDetection`. Reconfigure after switching flags; don't interleave builds with different `HAVE_*_API` sets.
- C++11. Threading, synchronisation and time are `std::` (`recursive_mutex`, `condition_variable_any`, `thread`, `chrono::steady_clock`), wrapped in thin helpers under `src/libcec/platform/` that keep the old names: `CMutex`, `CLockObject`, `CCondition`, `CEvent`, `CThread`, `CTimeout`. `CMutex` is recursive and `CLockObject` is a `unique_lock`, so it can be handed to `CCondition::Wait()`.
- Sockets are the one thing with no `std::` answer: `platform/sockets/` plus the per-OS `platform/{posix,windows}/os-socket.h`. `platform/os.h` dispatches on `_WIN32` to `platform/windows/os-types.h`, and *that* is what defines `__WINDOWS__` — nothing else does, and 10 other files branch on it. Its header order is load-bearing: `_WINSOCKAPI_` and `NOMINMAX` have to precede `windows.h`.
- `CThread::StopThread(int iWaitMs)` takes milliseconds, where **0 means wait forever** and a negative value means don't wait. It is not a bool. Kodi's `CThread::StopThread(bool bWait)` is the same name with the opposite sense, and since the two codebases share authors, `StopThread(false)` has been written here before — it compiles as `0`, i.e. the exact opposite of what it reads like. `CCondition::Wait()` uses the same 0-means-forever convention.
- A `CThread` subclass **must** stop its own thread in its own destructor. `Process()` is pure virtual by the time `~CThread` runs. Every backend does this via `Close()`.
