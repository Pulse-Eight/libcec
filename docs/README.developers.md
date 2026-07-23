# Developers

We provide a C, C++, Python, .NET and Node.js interface to the adapter.

## C++ developers
* the API can be found in `include/cec.h`
* an example implementation can be found on https://github.com/Pulse-Eight/libcec/blob/master/src/cec-client/cec-client.cpp

## C developers
* the API can be found in `include/cecc.h`
* an example implementation can be found on https://github.com/Pulse-Eight/libcec/blob/master/src/cecc-client/cecc-client.c

## .NET developers
* add a reference to `LibCecSharp.dll`, installed to `C:\Program Files (x86)\Pulse-Eight\USB-CEC Adapter\net8.0` by default on Windows. On Debian the `libcec-dotnet` package installs it to `/usr/lib/libcec/LibCecSharp.dll` and ships a NuGet package under `/usr/share/libcec-dotnet` that downstream projects can `PackageReference`.
* `LibCecSharp` is a single, pure-C# assembly (namespace `CecSharp`) that binds libCEC through P/Invoke over the C API. It targets **net8.0** and is architecture-neutral (the native `cec.dll`/`libcec.so` it loads is the arch-specific part), so it runs on Windows, Linux, macOS and Raspberry Pi.
* it replaces the previous C++/CLI wrappers (`LibCecSharp` for .NET Framework and `LibCecSharpCore` for net8.0), which were Windows-only. Consumers keep the same `CecSharp` API, so existing code compiles unchanged against net8.0.
* WinForms/WPF apps target `net8.0-windows`; console/service apps target `net8.0`.
* an example implementation can be found on https://github.com/Pulse-Eight/cec-dotnet/blob/master/src/CecSharpTester/CecSharpClient.cs

## Python developers
* the API is exported to Python through Swig
* an example implementation can be found on https://github.com/Pulse-Eight/libcec/blob/master/src/pyCecClient/pyCecClient.py

## Node.js developers
* a native N-API addon in `src/nodejs` binds libCEC over the C API (`include/cecc.h`), the same surface the .NET binding uses. It exposes an `EventEmitter`-based `CecAdapter` and works anywhere libCEC and a C++ toolchain do (Linux, macOS, Raspberry Pi, Windows).
* build it with `cd src/nodejs && npm install` (runs `node-gyp`, compiling against an installed libCEC found via `pkg-config`; on Debian that is the `libcec8-dev` package).
* an example implementation can be found in [src/nodejs/example/simple.js](../src/nodejs/example/simple.js); see [src/nodejs/README.md](../src/nodejs/README.md) for the full API.

# Developers Agreement

If you wish to contribute to this project, you must first sign our contributors agreement.
Please see [the contributors agreement](http://www.pulse-eight.com/contributors) for more information.
