### Raspberry Pi
If you're compiling for a Raspberry Pi, then the path to the required headers and libraries can be set manually, in case it's not in a standard system directory:
```
cmake -DRPI_INCLUDE_DIR=/path/to/vc/include \
      -DRPI_LIB_DIR=/path/to/vc/lib \
      ..
```

The headers and libraries are installed in /opt/vc when using Raspbian, but they may be installed somewhere else when using another distribution. Please ask the distribution's maintainers for help in case compilation fails and you're not using Raspbian.

If you're cross compiling, then you can set the correct toolchain like this (for the Raspberry Pi):
```
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/CrossCompile.cmake \
      -DXCOMPILE_BASE_PATH=/path/to/tools/arm-bcm2708/arm-bcm2708hardfp-linux-gnueabi \
      -DXCOMPILE_LIB_PATH=/path/to/firmware/hardfp/opt/vc/lib \
      -DRPI_INCLUDE_DIR=/path/to/firmware/hardfp/opt/vc/include \
      -DRPI_LIB_DIR=/path/to/firmware/hardfp/opt/vc/lib \
      ..
```

To compile libCEC on a new Raspbian installation, follow these instructions:
```
sudo apt-get update
sudo apt-get install cmake libudev-dev libxrandr-dev python-dev swig
cd
git clone https://github.com/Pulse-Eight/platform.git
mkdir platform/build
cd platform/build
cmake ..
make
sudo make install
cd
git clone https://github.com/Pulse-Eight/libcec.git
mkdir libcec/build
cd libcec/build
cmake -DRPI_INCLUDE_DIR=/opt/vc/include -DRPI_LIB_DIR=/opt/vc/lib ..
make -j4
sudo make install
sudo ldconfig
```

## Examples
Example implementations using libCEC can be found here:
* [github.com/Pulse-Eight/libcec/blob/master/src/cec-client/cec-client.cpp](https://github.com/Pulse-Eight/libcec/blob/master/src/cec-client/cec-client.cpp)
* [github.com/DrGeoff/cec_simplest](https://github.com/DrGeoff/cec_simplest)
