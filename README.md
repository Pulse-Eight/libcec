![Pulse-Eight logo](https://pulseeight.files.wordpress.com/2016/02/pulse-eight-logo-white-on-green.png?w=200)

# About
This library provides support for Pulse-Eight's USB-CEC adapter and other CEC capable hardware, like the Raspberry Pi.

A list of frequently asked questions can be found on [libCEC's FAQ page] (http://libcec.pulse-eight.com/faq).

.Net client applications, previously part of this repository, have been moved to [this repository](https://github.com/Pulse-Eight/cec-dotnet).

# Supported platforms

## Linux & BSD
See [docs/README.linux.md](docs/README.linux.md).

## Apple OS X
See [docs/README.osx.md](docs/README.osx.md).

## Microsoft Windows
See [docs/README.windows.md](docs/README.windows.md).

# Supported hardware
* [Pulse-Eight USB-CEC Adapter](https://www.pulse-eight.com/p/104/usb-hdmi-cec-adapter)
* [Pulse-Eight Intel NUC CEC Adapter](https://www.pulse-eight.com/p/154/intel-nuc-hdmi-cec-adapter)
* [Raspberry Pi](https://www.raspberrypi.org/)
* Some Exynos SoCs
* NXP TDA995x
* Odroid C2 (Amlogic S905)

# Developers
See [docs/README.developers.md](docs/README.developers.md).

# Vendor specific notes

## Panasonic
* On Panasonic to enable media control buttons on bottom of the remote, you may have to change the operation mode. To change it, press bottom Power-button, keep it pressed, and press 7 3 Stop. After releasing Power-button, Play, Pause, etc should work in XBMC.

## Raspberry Pi
* If your TV cannot detect the Raspberry Pi's CEC, or if the the Pi can't detect the TV, try adding the following line in `/boot/config.txt` and reboot the Pi: `hdmi_force_hotplug=1`
