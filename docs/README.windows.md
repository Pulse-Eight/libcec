## Microsoft Windows

### Developing a .Net application
To develop a .Net application that uses LibCecSharp:
* download the latest binary version from [our website](http://libcec.pulse-eight.com/Downloads)
* add a reference to `LibCecSharp.dll`. It's installed to `C:\Program Files (x86)\Pulse-Eight\USB-CEC Adapter\net8.0` by default
* the minimum .Net version required for LibCecSharp is 8.0. `LibCecSharp` is now a single pure-C# assembly (namespace `CecSharp`) — the old C++/CLI `LibCecSharp` (.NET Framework) and `LibCecSharpCore` (net8.0) wrappers have been replaced by this one binding. It also builds and runs on Linux/macOS/Raspberry Pi.
* WinForms/WPF apps target `net8.0-windows`; console/service apps target `net8.0`
* running a .Net app that uses it needs the [.NET 8 Desktop Runtime](https://dotnet.microsoft.com/download/dotnet/8.0) (the installer offers to install it for you)

An example implementation can be found on [Github](https://github.com/Pulse-Eight/cec-dotnet/tree/master/src/CecSharpTester/netcore/).

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
