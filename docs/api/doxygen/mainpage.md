libCEC — C / C++ API {#mainpage}
================================

libCEC lets applications control **CEC-capable HDMI hardware** — power a TV on or
to standby, become the active source, send remote-control keys, read device
state — and receive everything happening on the CEC bus. It works over
Pulse-Eight's USB-CEC adapter (the cross-platform default) and SoC-native CEC on
Linux and Raspberry Pi.

This is the reference for the **native interface**. Everything else — the
[.NET](https://pulse-eight.github.io/libcec/dotnet/),
[Node.js](https://pulse-eight.github.io/libcec/nodejs/) and
[Python](https://pulse-eight.github.io/libcec/python/) bindings — is a thin
wrapper over the same engine, so the concepts documented here (logical and
physical addresses, device types, opcodes, power states) carry across all of
them.

There are two native surfaces:

- **C++** — the `CEC::ICECAdapter` interface in [cec.h](@ref cec.h), the object
  you send commands through. Events arrive via the `CEC::ICECCallbacks` function
  pointers in [cectypes.h](@ref cectypes.h).
- **C** — the flat `libcec_*` functions in [cecc.h](@ref cecc.h), the same
  surface the .NET and Node.js bindings bind. Prefer this for FFI.

[TOC]

Requirements
------------

- A **Pulse-Eight USB-CEC adapter** (or a supported SoC-native CEC backend).
- libCEC and its development files (headers + the `cec` shared library).
- A C++11 compiler.

Installing
----------

### Debian / Ubuntu

```sh
# development headers + the shared library, plus the cec-client tools
sudo apt-get install libcec-dev cec-utils
```

On Pulse-Eight's own package feed the development package is named after the
SONAME (`libcec8-dev`); distribution repositories ship it as `libcec-dev`.

### Build from source (Linux / macOS / BSD / Raspberry Pi)

```sh
git clone https://github.com/Pulse-Eight/libcec.git
mkdir libcec/build && cd libcec/build
cmake ..
make -j4
sudo make install && sudo ldconfig
```

Platform-native CEC backends are off by default and enabled with cmake flags
(e.g. `-DHAVE_LINUX_API=1`, `-DHAVE_RPI_API=1`); without one, only the
Pulse-Eight USB adapter backend is built. See the platform notes for
[Linux](https://github.com/Pulse-Eight/libcec/blob/master/docs/README.linux.md),
[macOS](https://github.com/Pulse-Eight/libcec/blob/master/docs/README.osx.md) and
[Raspberry Pi](https://github.com/Pulse-Eight/libcec/blob/master/docs/README.raspberrypi.md).

### Windows

Install the **USB-CEC Adapter software** from Pulse-Eight (it ships `cec.dll`,
the import library `cec.lib` and the headers), or build the library and installer
from source with `python windows\create-installer.py` (see the
[Windows build notes](https://github.com/Pulse-Eight/libcec/blob/master/docs/README.windows.md)).
Compile against the flat headers and link `cec.lib`; at runtime keep `cec.dll` on
the DLL search path.

Using the C++ API
-----------------

The lifecycle is: describe yourself in a `CEC::libcec_configuration`, register
callbacks, create the instance with `LibCecInitialise()` (from
[cecloader.h](@ref cecloader.h), which loads the shared library for you), open a
connection, then send commands. Tear down with `Close()` + `UnloadLibCec()`.

```cpp
#include <iostream>
#include <libcec/cec.h>
#include <libcec/cecloader.h>

using namespace CEC;

// --- event callbacks (fired from libCEC's own worker thread) --------------
void onLog(void*, const cec_log_message* m) {
  std::cout << "log: " << m->message << std::endl;
}
void onKey(void*, const cec_keypress* k) {
  std::cout << "key: " << static_cast<int>(k->keycode) << std::endl;
}
void onCommand(void*, const cec_command* c) {
  std::cout << "command: " << static_cast<int>(c->opcode) << std::endl;
}

int main() {
  // 1. describe this client
  libcec_configuration config;
  config.Clear();
  snprintf(config.strDeviceName, sizeof(config.strDeviceName), "CppApp");
  config.clientVersion   = LIBCEC_VERSION_CURRENT;
  config.bActivateSource = 0;                 // don't steal focus on open
  config.deviceTypes.Add(CEC_DEVICE_TYPE_RECORDING_DEVICE);

  // 2. wire up the callbacks
  ICECCallbacks callbacks;
  callbacks.Clear();
  callbacks.logMessage      = &onLog;
  callbacks.keyPress        = &onKey;
  callbacks.commandReceived = &onCommand;
  config.callbacks          = &callbacks;

  // 3. create the libCEC instance (loads cec.dll / libcec.so)
  ICECAdapter* adapter = LibCecInitialise(&config);
  if (!adapter) {
    std::cerr << "could not load libCEC" << std::endl;
    return 1;
  }

  // 4. find an adapter and open it
  cec_adapter_descriptor devices[10];
  int8_t found = adapter->DetectAdapters(devices, 10, nullptr, true);
  if (found <= 0 || !adapter->Open(devices[0].strComName)) {
    std::cerr << "no CEC adapter, or could not open it" << std::endl;
    UnloadLibCec(adapter);
    return 1;
  }

  // 5. control the bus
  adapter->PowerOnDevices(CECDEVICE_TV);
  cec_logical_addresses actives = adapter->GetActiveDevices();
  for (uint8_t la = CECDEVICE_TV; la <= CECDEVICE_BROADCAST; ++la) {
    if (actives[la])
      std::cout << adapter->GetDeviceOSDName((cec_logical_address)la).c_str()
                << std::endl;
  }

  // 6. clean up (Close() stops the worker thread before UnloadLibCec())
  adapter->Close();
  UnloadLibCec(adapter);
  return 0;
}
```

Build it against the installed library with `pkg-config`:

```sh
g++ -std=c++11 example.cpp -o example $(pkg-config --cflags --libs libcec)
```

### Receiving events

Every `CEC::ICECCallbacks` member is optional — set only the ones you need and
leave the rest null (that is what `Clear()` does). libCEC invokes them from its
own worker thread, so treat the payloads as read-only and hand work off to your
own thread if it is expensive. The `menuStateChanged` and `commandHandler`
callbacks additionally return an `int` telling libCEC whether you handled the
event; the others are notifications.

### The C API

For FFI or C code, use [cecc.h](@ref cecc.h) instead: `libcec_initialise()`,
`libcec_open()`, `libcec_power_on_devices()`, `libcec_close()`,
`libcec_destroy()`, and so on — the same operations as `ICECAdapter`, taking a
`libcec_connection_t` handle as the first argument. `ceccloader.h` provides the
matching dynamic loader.

Where to go next
----------------

- `CEC::ICECAdapter` — the full method reference (open, transmit, power, queries).
- `CEC::ICECCallbacks` — the event surface.
- `CEC::libcec_configuration` — every configurable field.
- [cectypes.h](@ref cectypes.h) — all protocol enums, opcodes and structs.

---

libCEC is Copyright &copy; Pulse-Eight Limited, dual-licensed (GPL-2.0-or-later
or commercial). See the [project on GitHub](https://github.com/Pulse-Eight/libcec).
