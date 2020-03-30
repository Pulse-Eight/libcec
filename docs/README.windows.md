## Microsoft Windows

### Developing a .Net Framework application
To develop a .Net Framework application that uses LibCecSharp:
* download the latest binary version from [our website](http://libcec.pulse-eight.com/Downloads)
* add a reference to LibCecSharp.dll for the target architecture (x86/amd64). It's installed to `C:\Program Files (x86)\Pulse-Eight\USB-CEC Adapter\netfx` by default
* the minimum .Net Framework version required for LibCecSharp is 4.0

An example implementation can be found on [Github](https://github.com/Pulse-Eight/cec-dotnet/tree/master/src/CecSharpTester/netfx/).

### Developing a .Net Core application
To develop a .Net Core application that uses LibCecSharp:
* download the latest binary version from [our website](http://libcec.pulse-eight.com/Downloads)
* add a reference to LibCecSharpCore.dll for the target architecture (x86/amd64). It's installed to `C:\Program Files (x86)\Pulse-Eight\USB-CEC Adapter\netcore` by default
* the minimum .Net Core version required for LibCecSharpCore is 3.1

An example implementation can be found on [Github](https://github.com/Pulse-Eight/cec-dotnet/tree/master/src/CecSharpTester/netcore/).

### Prerequisites
To compile libCEC on Windows, you'll need the following dependencies:
* [p8-platform](https://github.com/Pulse-Eight/platform) 2.0 or later
* [cmake 3.12 or newer](http://www.cmake.org/)
* [Visual Studio 2019 (v16) or newer](https://www.visualstudio.com/), with the following options selected: Universal Windows Platform development, Desktop development with C++, .NET Core cross platform development
* [.Net Core 3.1](https://dotnet.microsoft.com/download/dotnet-core/3.1)
* To create an installer, you'll need [Nullsoft's NSIS](http://nsis.sourceforge.net/)
* You also need two versions of Python to build an installer: [Python 2.7.13 for x86](https://www.python.org/ftp/python/2.7.13/python-2.7.13.msi), required by the EventGhost plugin and [Python 3.6 or newer for x64](https://www.python.org/)

### Visual Studio
The build scripts have been configured for building with Visual Studio 2019. To use another version Visual Studio, pass the verion number as parameter: `windows\visual-studio.cmd 2019`

### Compilation
To only compile libCEC, follow these instructions:
* `git submodule update --init --recursive`
* run `windows\build-all.cmd` to build libCEC and LibCecSharp

To develop for libCEC in Visual Studio:
* `git submodule update --init --recursive`
* run `windows\visual-studio.cmd`

To build an installer on Windows:
* `git submodule update --init --recursive`
* run `windows\create-installer.cmd`
* the installer is stored as `build\libCEC-VERSION.exe`
