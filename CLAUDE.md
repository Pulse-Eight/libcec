# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

libCEC is a cross-platform C++ library for controlling CEC-capable hardware (TVs, AV receivers, etc.) over HDMI, primarily via Pulse-Eight's USB-CEC adapter and SoC-native CEC on Linux/Raspberry Pi. It exposes C, C++, Python (via SWIG), and .NET CLR interfaces over the same core engine. The shared/static library output is named `cec` (`libcec.so` / `cec.dll`).

The `.NET` client apps (cec-tray, CecSharpTester) live in the `src/dotnet` git submodule (the `cec-dotnet` repo). The managed C++/CLI wrappers (`LibCecSharp`, `LibCecSharpCore`) live in `src/dotnetlib` in this repo.

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

### Windows
Do **not** invoke cmake/msbuild directly — use the Python build orchestrator (`windows/create-installer.py`), which compiles libCEC (C/C++/Python), the C++/CLI wrappers, and packages an NSIS installer. Requires Visual Studio (default toolchain `2022c` = VS2022 Community), CMake, and Python 3.12+ (uses `match`/PEP 604 union syntax).

```
python windows\create-installer.py            # full build + installer -> dist\libcec-<arch>-<ver>.exe
python windows\create-installer.py -ni        # build libCEC + LibCecSharp, no installer
python windows\create-installer.py -vs         # generate Visual Studio project files for development
```

Useful flags: `-a {x64,x86,arm,arm64}` (default x64), `-m {Release,Debug,RelWithDebInfo}` (default Release), `-t <toolchain>` (e.g. `2019c`, `2022`, `2026c`), `-nc` (no clean / incremental), `-ne` (skip EventGhost plugin), `-ni` (no installer). Build artifacts land in `build\<target>\<arch>\`. The orchestrator's structure is in `windows/toolchain.py` (toolchain/arch enums) and `windows/mixins.py` / `windows/pathbuilder.py` (helpers).

The Windows C++/CLI solution is `project/libcec.sln`; the .NET apps solution is `src/dotnet/project/cec-dotnet.sln`.

### Tests
There is no automated test suite. Verification is manual via the example clients run against real CEC hardware:
- `cec-client` (C++, `src/cec-client/cec-client.cpp`) — interactive CLI; the primary smoke test.
- `cecc-client` (C example), `pyCecClient` (Python example).

## Architecture

The core lives in `src/libcec/`. Data flow, outermost to innermost:

1. **Public API** — `include/cec.h` defines `ICECAdapter` (C++ interface). `CLibCEC` (`LibCEC.cpp`) implements it and is the object handed to clients. `LibCECC.cpp` wraps it for the C API (`cecc.h`); `libcec.i` + `SwigHelper.h` generate the Python binding; `src/dotnetlib` wraps it for .NET. `include/cectypes.h` holds all shared enums/structs/opcodes and is the single source of truth for the protocol surface.

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
