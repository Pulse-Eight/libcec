## Microsoft Windows

### Developing a .Net application
Building this library is only required if you want to change libCEC or LibCecSharp internally.
To develop a .Net application that uses LibCecSharp:
* download the latest binary version from [our website](http://libcec.pulse-eight.com/Downloads)
* add a reference to LibCecSharp.dll for the target architecture (x86/amd64). It's installed to `C:\Program Files (x86)\Pulse-Eight\USB-CEC Adapter` by default
* add cec.dll to your project from the same directory
* right click on cec.dll in the project explorer
* change the copy mode to "copy if newer"

Example implementations can be found on [Github](https://github.com/Pulse-Eight/cec-dotnet).

### Prerequisites
To compile libCEC on Windows, you'll need the following dependencies:
* [p8-platform] (https://github.com/Pulse-Eight/platform) 2.0 or later
* [cmake 2.6 or better] (http://www.cmake.org/)
* [Visual Studio 2008 (v90) Professional] (https://www.visualstudio.com/)
* [Visual Studio 2010 (v110) (for x64 builds: Professional)] (https://www.visualstudio.com/)
* [Visual Studio 2013 (v120) or 2015 (v140)] (https://www.visualstudio.com/)
* To create an installer, you'll need [Nullsoft's NSIS] (http://nsis.sourceforge.net/)

Visual Studio version must be installed in ascending order, and each version of Visual Studio must be started at least once before the next version is installed.

The reason for needing all of these versions is that LibCecSharp targets .Net 2.0, to maximise compatibility, but libCEC itself requires C++11.
This means that LibCecSharp requires the 9.0 toolchain and libCEC the 12.0 toolchain.
Because of changes in Visual Studio's build system, Visual Studio 2010 Professional is required to build LibCecSharp in Visual Studio 2013.

### Installation check

You can check whether all versions of Visual Studio got installed correctly by checking if the following directories exist:
* Visual Studio 2008: `X:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\include\msclr`
* Visual Studio 2010: `X:\Program Files (x86)\MSBuild\Microsoft.Cpp\v4.0\Platforms\Win32\PlatformToolsets\v90`
* Visual Studio 2010 x64: `X:\Program Files (x86)\MSBuild\Microsoft.Cpp\v4.0\Platforms\x64\PlatformToolsets\v90`

### Visual Studio 2015
The build scripts have been configured for building with Visual Studio 2013. To use Visual Studio 2015, change `windows\build.cmd`: `SET VSVERSION=12` to `SET VSVERSION=14`.

### Compilation
To compile libCEC, follow these instructions:
* `git submodule update --init --recursive`
* run `windows\build.cmd` to build libCEC and LibCecSharp

To develop for libCEC in Visual Studio:
* `git submodule update --init --recursive`
* run `windows\visual-studio.cmd`

To build an installer on Windows:
* `git submodule update --init --recursive`
* run `windows\create-installer.cmd`
* the installer is stored as `build\libCEC-VERSION.exe`
