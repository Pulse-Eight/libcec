## Linux & BSD

### Prerequisites
libCEC needs the following dependencies in order to work correctly:
* udev v151 or later
* cdc-acm support compiled into the kernel or available as module

To compile libCEC on Linux, you'll need the following dependencies:
* [cmake 3.12.0 or better](https://www.cmake.org/)
* a supported C++ 11 compiler

The following dependencies are recommended. Without them, the adapter can not
be (fully) auto-detected.
* pkg-config
* udev development headers v151 or later
* X randr development headers

### Compilation
To compile libCEC on a new Debian/Ubuntu installation, follow these instructions:
```
apt-get update
apt-get install git cmake build-essential libudev-dev libxrandr-dev python3-dev swig
git clone https://github.com/Pulse-Eight/libcec.git
mkdir libcec/build
cd libcec/build
cmake ..
make -j4
sudo make install
sudo ldconfig
```

### Managed .NET binding (optional)
The pure-C# `LibCecSharp` binding builds on Linux/macOS too. Install the
[.NET 8 SDK](https://dotnet.microsoft.com/download/dotnet/8.0) and pass
`-DENABLE_DOTNET_LIB=1`:
```
cmake -DENABLE_DOTNET_LIB=1 ..
make -j4
```
This produces `LibCecSharp.dll` (net8.0) under `build/<Configuration>/<Platform>/net8.0/`.
The .NET apps (`ENABLE_DOTNET_APPS`) are Windows-only (cec-tray is WinForms).

### Raspberry Pi
See [docs/README.raspberrypi.md](README.raspberrypi.md).

### Exynos
Pass the argument `-DHAVE_EXYNOS_API=1` to the cmake command in the compilation instructions:
```
cmake -DHAVE_EXYNOS_API=1 ..
```

### AOCEC
Pass the argument `-DHAVE_AOCEC_API=1` to the cmake command in the compilation instructions:
```
cmake -DHAVE_AOCEC_API=1 ..
```

### TDA995x
Pass the argument `-DHAVE_TDA995X_API=1` to the cmake command in the compilation instructions:
```
cmake -DHAVE_TDA995X_API=1 ..
```

### Linux CEC Framework (v4.10+)
Pass the argument `-DHAVE_LINUX_API=1` to the cmake command in the compilation instructions:
```
cmake -DHAVE_LINUX_API=1 ..
```

### Systemd
Example systemd units for common use cases can be found in the [systemd folder](../systemd/).


### Debian / Ubuntu .deb packaging
See [docs/README.debian.md](README.debian.md).
