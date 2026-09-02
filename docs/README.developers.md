# Developers

We provide a C, C++, Python, .NET, Node.js and Rust interface to the adapter.

The .NET, Node.js and Rust bindings are published to their language's registry
from 8.1.6 onwards — `dotnet add package LibCecSharp`, `npm install libcec`,
`cargo add libcec`. All three bind the native library rather than carrying it,
so libCEC still has to be installed alongside them. The per-language sections
below cover building them from this tree instead.

## C++ developers
* the API can be found in `include/cec.h`
* an example implementation can be found on https://github.com/Pulse-Eight/libcec/blob/master/src/cec-client/cec-client.cpp

## C developers
* the API can be found in `include/cecc.h`
* an example implementation can be found on https://github.com/Pulse-Eight/libcec/blob/master/src/cecc-client/cecc-client.c

## .NET developers
* `dotnet add package LibCecSharp` is the usual way in. Alternatively add a reference to `LibCecSharp.dll`, installed to `C:\Program Files (x86)\Pulse-Eight\USB-CEC Adapter\net8.0` by default on Windows. On Debian the `libcec-dotnet` package installs it to `/usr/lib/libcec/LibCecSharp.dll` and ships the same NuGet package under `/usr/share/libcec-dotnet` that downstream projects can `PackageReference`.
* `LibCecSharp` is a single, pure-C# assembly (namespace `CecSharp`) that binds libCEC through P/Invoke over the C API. It targets **net8.0** and is architecture-neutral (the native `cec.dll`/`libcec.so` it loads is the arch-specific part), so it runs on Windows, Linux, macOS and Raspberry Pi.
* it replaces the previous C++/CLI wrappers (`LibCecSharp` for .NET Framework and `LibCecSharpCore` for net8.0), which were Windows-only. Consumers keep the same `CecSharp` API, so existing code compiles unchanged against net8.0.
* WinForms/WPF apps target `net8.0-windows`; console/service apps target `net8.0`.
* an example implementation can be found on https://github.com/Pulse-Eight/cec-dotnet/blob/master/src/CecSharpTester/CecSharpClient.cs

## Python developers
* the API is exported to Python through Swig
* an example implementation can be found on https://github.com/Pulse-Eight/libcec/blob/master/src/pyCecClient/pyCecClient.py

## Node.js developers
* a native N-API addon in `src/nodejs` binds libCEC over the C API (`include/cecc.h`), the same surface the .NET binding uses. It exposes an `EventEmitter`-based `CecAdapter` and works anywhere libCEC and a C++ toolchain do (Linux, macOS, Raspberry Pi, Windows).
* `npm install libcec` pulls it from npm. The package carries sources, so installing compiles the addon against the libCEC on the machine — one package covers every platform and every Node ≥ 16, because N-API is ABI-stable.
* build it from this tree with `cd src/nodejs && npm install` (runs `node-gyp`, compiling against an installed libCEC found via `pkg-config`; on Debian that is the `libcec8-dev` package).
* an example implementation can be found in [src/nodejs/example/simple.js](../src/nodejs/example/simple.js); see [src/nodejs/README.md](../src/nodejs/README.md) for the full API.

## Rust developers
* the `libcec` crate in `src/rust` binds libCEC over the C API (`include/cecc.h`), the same surface the .NET and Node.js bindings use. It has no dependencies of its own and works anywhere libCEC does.
* `libcec::Connection` is the safe API; `libcec::ffi` is the raw C surface, complete and public, for anything the safe layer does not cover.
* `cargo add libcec` pulls it from crates.io.
* build it from this tree with `cd src/rust && cargo build`. `pkg-config` finds an installed libCEC on Unix (on Debian, the `libcec8-dev` package); set `LIBCEC_LIB_DIR` to build against an uninstalled one, which is also how it finds `cec.lib` on Windows.
* the crate's major version has to match libCEC's: `libcec_configuration` gains fields on every major bump, and `build.rs` refuses a mismatch rather than letting a wrong-sized struct through.
* examples are in [src/rust/examples](../src/rust/examples) — `simple.rs` for the shape of it, `cec_client.rs` for an interactive console; see [src/rust/README.md](../src/rust/README.md).

# Developers Agreement

If you wish to contribute to this project, you must first sign our contributors agreement.
Please see [the contributors agreement](http://www.pulse-eight.com/contributors) for more information.
