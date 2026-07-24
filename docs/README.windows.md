## Microsoft Windows

### Developing a .Net application
To develop a .Net application that uses LibCecSharp:
* download the latest binary version from [our website](http://libcec.pulse-eight.com/Downloads)
* add a reference to `LibCecSharp.dll`. It's installed to `C:\Program Files (x86)\Pulse-Eight\USB-CEC Adapter\net8.0` by default
* the minimum .Net version required for LibCecSharp is 8.0. `LibCecSharp` is now a single pure-C# assembly (namespace `CecSharp`) — the old C++/CLI `LibCecSharp` (.NET Framework) and `LibCecSharpCore` (net8.0) wrappers have been replaced by this one binding. It also builds and runs on Linux/macOS/Raspberry Pi.
* WinForms/WPF apps target `net8.0-windows`; console/service apps target `net8.0`
* running a .Net app that uses it needs the [.NET 8 Desktop Runtime](https://dotnet.microsoft.com/download/dotnet/8.0) (the installer offers to install it for you)

An example implementation can be found on [Github](https://github.com/Pulse-Eight/cec-dotnet/tree/master/src/CecSharpTester/netcore/).

### Developing a Node.js application
The installer ships an optional "libCEC for Node.js" component: a prebuilt native
N-API addon plus its JavaScript wrapper, installed to
`C:\Program Files\Pulse-Eight\USB-CEC Adapter\nodejs`. Because the addon is N-API
(ABI-stable) it works with any Node.js >= 16 without recompiling. Point your app
at it (e.g. `require('C:/Program Files/Pulse-Eight/USB-CEC Adapter/nodejs')`); the
matching `cec.dll` sits next to the addon, so no `PATH` changes are needed. See
[src/nodejs/README.md](../src/nodejs/README.md) for the API.

To build the addon yourself from a repo checkout you need Node.js and a C++
toolchain (`node-gyp`). Point it at libCEC's headers and import library with the
`LIBCEC_INCLUDE_DIR` / `LIBCEC_LIB_DIR` environment variables:
```
cd src\nodejs
set LIBCEC_INCLUDE_DIR=..\..\include
set LIBCEC_LIB_DIR=..\..\build\Release\x64
npm install
```
`create-installer.py` does exactly this and stages the result; pass `-nn` to skip
building the Node.js binding.

### Prerequisites
To compile libCEC on Windows, you'll need the following dependencies:
* [cmake 3.12 or newer](https://www.cmake.org/)
* [Visual Studio 2022 (v17) or newer](https://www.visualstudio.com/) (2017 and 2019 are also supported), with the following options selected: Desktop development with C++ and .NET desktop development
* [.Net 8.0 SDK](https://dotnet.microsoft.com/download/dotnet/8.0)
* [Python 3.12 or newer](https://www.python.org/), used by the build orchestrator
* To create an installer, you'll need [Nullsoft's NSIS](http://nsis.sourceforge.net/)

### Compilation
To only compile libCEC, follow these instructions:
* `git submodule update --init --recursive`
* run `python windows\create-installer.py -ni` to build libCEC and LibCecSharp

To develop for libCEC in Visual Studio:
* `git submodule update --init --recursive`
* run `python windows\create-installer.py -vs`

To build an installer on Windows:
* `git submodule update --init --recursive`
* run `python windows\create-installer.py`
* the installer is stored as `dist\libcec-<arch>-<version>.exe`
