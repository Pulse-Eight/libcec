## Apple OS X

### MacPorts

libCEC is available through [MacPorts] (https://www.macports.org/) and has been tested on OS X 10.9 through 10.12

### Prerequisites
To compile libCEC on OS X, you'll need the following dependencies:
* [p8-platform] (https://github.com/Pulse-Eight/platform) 2.0 or later
* [cmake 2.6 or better] (http://www.cmake.org/)
* a supported C++ 11 compiler. Support for C++11 was added in OS X 10.9
* xcode 3.2.6 or later
* optional: [Python 3.4 or later] (https://www.python.org/) and [Swig] (http://www.swig.org/) to generate Python bindings
* optional: libX11 and xrandr to read the sink's EDID, used to determine the PC's HDMI physical address

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
