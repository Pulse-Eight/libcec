# CLAUDE.md

## What this is

libCEC is a cross-platform C++ library for controlling CEC-capable hardware over HDMI, via Pulse-Eight's USB-CEC adapter and SoC-native CEC on Linux/RPi. The library output is named `cec` (`libcec.so` / `cec.dll`). The core is `src/libcec/`; every binding sits on the C API (`include/cecc.h`).

## Read these first

Most of what you need is already documented in the tree. Don't restate it here — fix it there.

| | |
|---|---|
| build on Linux/BSD, backend cmake flags | `docs/README.linux.md` |
| build on Windows, orchestrator flags, prerequisites | `docs/README.windows.md` |
| Raspberry Pi, macOS | `docs/README.raspberrypi.md`, `docs/README.osx.md` |
| the bindings, per-language build + install | `docs/README.developers.md` |
| Debian packages and what each contains | `docs/README.debian.md` |
| licence generation, adding a component | `licenses/README.md` |
| API reference generators, building docs locally | `docs/api/README.md` |

## Architecture

`src/libcec/`, outermost to innermost:

1. **Public API** — `include/cec.h` defines `ICECAdapter`; `CLibCEC` (`LibCEC.cpp`) implements it and is the object handed to clients. `LibCECC.cpp` wraps it for the C API.
2. **`CCECClient`** (`CECClient.cpp`) — one logical configuration/connection: the logical addresses this instance claims, the active `libcec_configuration`, the callback registration. Multiple clients can attach to one processor.
3. **`CCECProcessor`** (`CECProcessor.cpp`) — the engine: bus state, the worker thread pumping frames, logical-address allocation, routing to handlers.
4. **`devices/`** — `CCECBusDevice` and subclasses model the *state* of each device on the bus. `CECDeviceMap` tracks all 15 logical addresses.
5. **`implementations/`** — per-vendor `CCECCommandHandler` subclasses for quirky vendor behavior (`SL`, `VL`, `RL`, `PH`, `RH`, `AN`, `AQ`), instantiated by the processor from the device's reported vendor id.
6. **`adapter/`** — backends behind `IAdapterCommunication`, constructed by `AdapterFactory`. `Pulse-Eight/` is the cross-platform default; the SoC backends compile in only under their `HAVE_*_API` flag.
7. **`platform/`** — libCEC's own OS abstraction: `std::`-backed threading/time/buffers (`threads/`, `util/`), sockets and serial (`sockets/`, `posix/`, `windows/`), and the EDID readers that discover the device's own physical address from the GPU (`adl/`, `nvidia/`, `drm/`, `X11/`).

`include/cectypes.h` is the single source of truth for the protocol surface. **No binding is generated from it** — .NET, Node and Rust each mirror it by hand, so a change to a struct or enum there has to be replayed in each. Rust's `tests/layout.rs` asserts every size and offset; the others have nothing that catches drift.

## Conventions

- Vendor-specific handling goes in a `*CommandHandler` under `implementations/`, keyed by vendor id in `cectypes.h` — not scattered through the processor.
- A new transport means a new `adapter/<name>/` implementing `IAdapterCommunication`, wired into `AdapterFactory` and gated by a `HAVE_*_API` flag in `src/libcec/cmake/CheckPlatformSupport.cmake`.
- `include/version.h`, `src/libcec/env.h`, `src/libcec/libcec.pc` and many Windows project files are generated from `.in` templates — edit the `.in`.
- Those are generated **into the source tree, not the build dir, so build dirs are not independent.** Configuring one with `-DHAVE_TEGRA_API=1` rewrites the shared `env.h`, and a different build dir then compiles with the flag set but without the Tegra sources, failing at link on an undefined `TegraCECAdapterDetection`. Reconfigure after switching flags; don't interleave builds with different `HAVE_*_API` sets.
- C++11. Threading, sync and time are `std::`, wrapped in thin helpers under `platform/` that keep the old names: `CMutex`, `CLockObject`, `CCondition`, `CEvent`, `CThread`, `CTimeout`. `CMutex` is recursive and `CLockObject` is a `unique_lock`, so it can be handed to `CCondition::Wait()`.
- A `CThread` subclass that touches its own members from `Process()` **must** stop the thread in its own destructor; every backend does, via `Close()`. `~CThread` stops and joins too, but not until `~CDerived` has destroyed the derived half.
- `platform/os.h` dispatches on `_WIN32` to `platform/windows/os-types.h`, and **that is what defines `__WINDOWS__`** — nothing else does, and 10 other files branch on it. Its header order is load-bearing: `_WINSOCKAPI_` and `NOMINMAX` must precede `windows.h`.
- **libCEC has no third-party C++ dependency for threading, time or IO** — it is all `std::`, under `platform/`. Don't add one; p8-platform is the one a maintainer is likeliest to reach for.

## Building

`docs/README.linux.md` and `docs/README.windows.md` have the invocations. What they don't say:

- **On Windows, don't invoke cmake or msbuild directly** — `windows/create-installer.py` owns the build, including the managed binding (it passes `-DENABLE_DOTNET_LIB=1 -DENABLE_DOTNET_APPS=1`, so cmake drives `dotnet build`). Its structure is in `windows/toolchain.py`, `windows/mixins.py`, `windows/pathbuilder.py`.
- `support/` is needed **to compile** on Windows, not just to package: `create-installer.py` passes `support\windows\cmake\{c,cxx}-flag-overrides.cmake` to every Windows cmake run. The `*.dll`/`*.exe` under it are force-tracked past `.gitignore`.
- `src/dotnet` is the only submodule and is only used by the Windows build.
- The static library is renamed **`cec-static`** on Windows because the DLL's import library is also `cec.lib` and would otherwise be overwritten, in the build *and* install trees.
- Keep the Windows build tree GNUInstallDirs-conformant (`bin\`, `lib\`, `include\libcec\`): downstream cmake consumers such as Kodi's `FindCEC.cmake` rely on it. What the *installer* lays down under Program Files is flat, and separate.
- The EventGhost plugin needs a **32-bit Python alongside the 64-bit one** — it always embeds the x86 library and x86 `cec` module, and cmake only builds that module for x86 if it finds 32-bit Python headers and `.lib`.
- Code signing is Azure Artifact Signing (`windows/codesigner.py`), enabled by the presence of `AZURE_SIGNING_JSON`; credentials come from `AZURE_TENANT_ID`/`AZURE_CLIENT_ID`/`AZURE_CLIENT_SECRET`. Nothing is stored in the repo, and without it the build says so and produces unsigned output.
- Signing covers the installer payload and the installer itself, **not** the NuGet, npm or crates.io packages. That is deliberate — each registry signs what it accepts, and that is what consumers validate on restore. The full reasoning sits at the `dotnet pack` target in `CMakeLists.txt`; read it before treating the missing signature as a bug.

## Binding invariants

- **Rust: the crate has no dependencies, deliberately.** That is what makes `cargo build --offline` work, which is what lets cmake and the Debian package build it with no network and no vendored registry. Don't add one without a reason that outweighs that.
- Rust protocol enums are generated by `support/generate-rust-enums.py` from `cectypes.h` and checked in — re-run it after adding a value (it runs `rustfmt` itself). Duplicate CEC values (`UNREGISTERED`/`BROADCAST`) become associated constants, and every enum keeps an `Other(i32)` catch-all because the bus carries whatever devices put on it.
- Rust callbacks: libCEC keeps the addresses of the callback table and parameter, so both live in a pinned box whose `Drop` closes and destroys before the handler is dropped.
- `ENABLE_RUST_LIB` builds the **examples**, not just the library: an rlib is never linked, so `cargo build` alone would not prove the bindings resolve against libCEC.
- Node: libCEC fires callbacks from its own worker thread, so each trampoline copies its payload and re-enters JS via a `Napi::ThreadSafeFunction`. `commandHandler`/`menuStateChanged` return "not handled" (0) synchronously on purpose — honouring a JS return would block libCEC's callback thread on the event loop and race its 1000ms timeout.
- The Windows Node addon is **x64 only** (`NodeJsBuilder._GYP_ARCH`), and `project/nsis/sections.nsh` derives `SECNODEJS` from `NSISNODEJS` *and* not-`NSIS_X86`, so the x86 installer cannot offer a component with no payload behind it. Building it is skipped non-fatally when the arch has no addon or Node isn't on `PATH`, but a build that *starts* and fails is a hard error, so a broken addon can't silently drop out of the installer.

## Releasing

`support/release.py <tag> <notes.md>` does the whole release non-interactively (merge master→release, tag the merge commit, push, wait for Jenkins, download the signed artefacts, publish the GitHub release). It needs `JENKINS_URL`/`JENKINS_USER`/`JENKINS_TOKEN`/`JENKINS_JOB` and an authenticated `gh`.

Preparing `master` is manual. `CMakeLists.txt` (`LIBCEC_VERSION_*`) is the source of truth, and every file that repeats the version has to be bumped with it — `release.py`'s `SATELLITE_VERSIONS` lists them and the release stops if any disagrees:

- `src/nodejs/package.json`
- `src/dotnetlib/LibCecSharp.csproj` — generated from its `.in`, but tracked so Visual Studio can open it without a cmake run, so the tracked copy goes stale on its own
- `src/rust/Cargo.toml`
- `debian/changelog.in` — a new stanza; note this is the source, not `debian/changelog`

**A new binding that carries its own version file adds a line to that table.**

Publishing a binding to npm / NuGet / crates.io is **not** automated and is a deliberate step: those versions are permanent and can only be yanked, never replaced.

## Tests

**There is no automated test suite.** Verification is manual, against real CEC hardware: `cec-client` (`src/cec-client/cec-client.cpp`) is the primary smoke test; `cecc-client` and `pyCecClient` are the C and Python examples. The exception is Rust's `tests/layout.rs`, which needs no hardware.
