# About
This library provides support for Pulse-Eight's USB-CEC adapter and other CEC capable hardware, like the Raspberry Pi.

A list of frequently asked questions can be found on [libCEC's FAQ page] (http://libcec.pulse-eight.com/faq).

.Net client applications, previously part of this repository, have been moved to [this repository](https://github.com/Pulse-Eight/cec-dotnet).

# Supported platforms

## Linux & BSD
See [docs/README.linux.md](docs/README.linux.md).

### Raspberry Pi
See [docs/README.raspberrypi.md](docs/README.raspberrypi.md).

### Exynos
Follow the instructions in [docs/README.linux.md](docs/README.linux.md) and pass the argument -DHAVE_EXYNOS_API=1 to cmake:
```
cmake -DHAVE_EXYNOS_API=1 ..
```

### AOCEC
To compile in support for AOCEC devices, you have to pass the argument -DHAVE_AOCEC_API=1 to cmake:
```
cmake -DHAVE_AOCEC_API=1 ..
```

### TDA995x
Follow the instructions in [docs/README.linux.md](docs/README.linux.md) and pass the argument -DHAVE_TDA995X_API=1 to cmake:
```
cmake -DHAVE_TDA995X_API=1 ..
```

### Debian
See [docs/README.debian.md](docs/README.debian.md).

## Apple OS X
See [docs/README.osx.md](docs/README.osx.md).

## Microsoft Windows
See [docs/README.windows.md](docs/README.windows.md).

# Developers
See [docs/README.developers.md](docs/README.developers.md).
