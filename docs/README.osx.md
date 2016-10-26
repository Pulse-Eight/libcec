## Apple OS X

### Prerequisites
To compile libCEC on OS X, you'll need the following dependencies:
* [p8-platform] (https://github.com/Pulse-Eight/platform) 2.0 or later
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
