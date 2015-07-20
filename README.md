# About

This library provides support for Pulse-Eight's USB-CEC adapter and other CEC capable hardware, like the Raspberry Pi.

You can find a list of frequently asked questions on [libCEC's FAQ page] (http://libcec.pulse-eight.com/faq)

# Supported platforms

## Linux & BSD

### Prerequisites
libCEC needs the following dependencies in order to work correctly:
* [libplatform] (https://github.com/Pulse-Eight/platform) 1.0 or later
* udev v151 or later
* cdc-acm support compiled into the kernel or available as module
* [liblockdev] (https://packages.debian.org/search?keywords=liblockdev) 1.0 or later

To compile libCEC on Linux, you'll need the following dependencies:
* [cmake 2.6 or better] (http://www.cmake.org/)
* a supported C++ 11 compiler
* [liblockdev] (https://packages.debian.org/search?keywords=liblockdev) 1.0 development headers

The following dependencies are recommended. Without them, the adapter can not
be (fully) auto-detected.
* pkg-config
* udev development headers v151 or later
* X randr development headers

### Compilation
To compile, execute the following commands:
```
mkdir build
cd build
cmake ..
make
sudo make install
```

### Raspberry Pi
If you're compiling for a Raspberry Pi, then the path to the required headers and libraries can be set manually, in case it's not in a standard system directory:
```
cmake -DRPI_INCLUDE_DIR=/path/to/vc/include \
      -DRPI_LIB_DIR=/path/to/vc/lib \
      -DRPI_BCM_HOST=1 \
      -DRPI_VCHIQ_ARM=1 \
      -DRPI_VCOS=1
      ..
```

If you're cross compiling, then you can set the correct toolchain like this (for the Raspberry Pi):
```
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/CrossCompile.cmake \
      -DXCOMPILE_BASE_PATH=/path/to/tools/arm-bcm2708/arm-bcm2708hardfp-linux-gnueabi \
      -DXCOMPILE_LIB_PATH=/path/to/firmware/hardfp/opt/vc/lib \
      -DRPI_INCLUDE_DIR=/path/to/firmware/hardfp/opt/vc/include \
      -DRPI_LIB_DIR=/path/to/firmware/hardfp/opt/vc/lib \
      ..
```

### Exynos
To compile in support for Exynos devices, you have to pass the argument -DHAVE_EXYNOS_API=1 to cmake:
```
cmake -DHAVE_EXYNOS_API=1 ..
```

### TDA995x
To compile in support for TDA995x devices, you have to pass the argument -DHAVE_TDA995X_API=1 to cmake:
```
cmake -DHAVE_TDA995X_API=1 ..
```

### Debian
Use the following commands to create a debian package:
```
source /etc/lsb-release
sed "s/#DIST#/${DISTRIB_CODENAME}/g" debian/changelog.in > debian/changelog
dpkg-buildpackage
```

## Apple OS X

### Prerequisites
To compile libCEC on OS X, you'll need the following dependencies:
* [libplatform] (https://github.com/Pulse-Eight/platform) 1.0 or later
* [cmake 2.6 or better] (http://www.cmake.org/)
* a supported C++ 11 compiler
* xcode 3.2.6 or later

### Compilation
To compile, execute the following command:
```
mkdir build
cd build
cmake ..
make
sudo make install
```

_Note:_ You may need to copy pkg.m4 to your m4 sources directory

## Microsoft Windows

### Prerequisites
To compile libCEC on Windows, you'll need the following dependencies:
* [libplatform] (https://github.com/Pulse-Eight/platform) 1.0 or later
* [cmake 2.6 or better] (http://www.cmake.org/)
* [Visual Studio 2013] (https://www.visualstudio.com/)
* [Windows DDK (Driver Development Kit)] (https://msdn.microsoft.com/en-us/windows/hardware/hh852365.aspx)
* To create an installer, you'll need [Nullsoft's NSIS] (http://nsis.sourceforge.net/)

When compiling LibCecSharp, you'll need the following versions too:
* Visual Studio 2012
* Visual Studio 2010
* Visual Studio 2008

### Compilation
To compile libCEC, follow these instructions:
* run `support\build.cmd` to build libCEC and cec-client
* open `project\libcec.sln` with Visual Studio 2013.
* build the project.

To develop for libCEC or cec-client in Visual Studio:
* run `support\visual-studio.cmd`

To build an installer on Windows:
* go to `project` and execute `create-installer.bat` to create the installer.
* the installer is stored as `build\libCEC-installer.exe`

# Developers

We provide a C, C++, Python and .NET CLR interface to the adapter.

## C++ developers
* the API can be found in `include/cec.h`
* an example implementation can be found in `src/cec-client/cec-client.cpp`

## C developers
* the API can be found in `include/cecc.h`
* an example implementation can be found in `src/cecc-client/cecc-client.cpp`

## .NET developers
* add a reference to `LibCecSharp.dll`
* an example can be found in `src\CecSharpTester\CecSharpClient.cs`

## Python developers
* the API is exported to Python through Swig
* an example can be found in `src/pyCecClient`

# Developers Agreement

If you wish to contribute to this project, you must first sign our contributors agreement.
Please see [the contributors agreement] (http://www.pulse-eight.net/contributors) for more information.
