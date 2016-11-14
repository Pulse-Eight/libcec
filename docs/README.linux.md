## Linux & BSD

### Prerequisites
libCEC needs the following dependencies in order to work correctly:
* [p8-platform] (https://github.com/Pulse-Eight/platform) 2.0 or later
* udev v151 or later
* cdc-acm support compiled into the kernel or available as module

To compile libCEC on Linux, you'll need the following dependencies:
* [cmake 2.6 or better] (http://www.cmake.org/)
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
apt-get install cmake libudev-dev libxrandr-dev python-dev swig
git clone https://github.com/Pulse-Eight/libcec.git
mkdir libcec/build
cd libcec/build
cmake ..
make -j4
sudo make install
sudo ldconfig
```

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

### Debian / Ubuntu .deb packaging
See [docs/README.debian.md](README.debian.md).