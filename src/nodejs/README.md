# libCEC — Node.js binding

A native [N-API](https://nodejs.org/api/n-api.html) addon that binds libCEC
through its C API (`include/cecc.h`), the same surface the .NET binding
(`src/dotnetlib`) uses. It lets Node.js applications control CEC-capable HDMI
devices — power a TV on/standby, become the active source, send remote keys,
read device state — and receive bus events (log, key presses, commands,
source changes, alerts).

It works anywhere libCEC and a C++ toolchain do: Linux, macOS, Raspberry Pi and
Windows, over the Pulse-Eight USB-CEC adapter or a SoC-native CEC backend.

## Requirements

* **libCEC** installed with its development files. On Unix these are discovered
  via `pkg-config` (`pkg-config --exists libcec`) — on Debian/Ubuntu that is the
  `libcec8-dev` package; or build and `make install` this repository first.
* **Node.js ≥ 16** and a C++17 toolchain (`node-gyp` prerequisites: a compiler,
  `make`, and Python 3).

## Install / build

```
cd src/nodejs
npm install        # runs node-gyp rebuild, compiling src/addon.cc against libcec
node example/simple.js
```

The compiled addon lands at `build/Release/cec_native.node`.

### Windows

Windows has no `pkg-config`, so tell `node-gyp` where libCEC's headers and its
`cec.lib` import library are with two environment variables (defaults point at an
installed *USB-CEC Adapter* SDK). From a repo build:

```
set LIBCEC_INCLUDE_DIR=..\..\include
set LIBCEC_LIB_DIR=..\..\build\Release\x64
npm install
```

At runtime the addon needs `cec.dll` on the DLL search path — keep it next to
`cec_native.node` (a `.node` resolves its dependencies from its own directory).
The Windows installer ships a prebuilt addon set up this way under its `nodejs`
folder, so end users don't need a compiler; see
[docs/README.windows.md](../../docs/README.windows.md).

## Test client

`client/cec-client.js` is an interactive REPL modelled on the C++ `cec-client`:
the Node counterpart for exercising the library against real hardware and a
worked example of every API call. Run it with `npm run client` (or
`node client/cec-client.js [port]`, or the installed `cec-client-node` bin):

```
$ npm run client
opening /dev/ttyACM0 ...
connection opened. type 'h' for help, 'q' to quit.
scan
...
tx 1F:82:10:00      # transfer raw bytes
on 0                # power on the TV
pow 0               # query the TV's power status
q
```

`node client/cec-client.js --help` lists the command-line options (`--list-devices`,
`--info`, `--type`, `--monitor`, `--log-level`, …).

## Usage

```js
const cec = require('libcec'); // or require('.') from this folder

const adapter = new cec.CecAdapter({
  deviceName: 'CECNode',
  deviceType: cec.CecDeviceType.RecordingDevice,
});

adapter.on('log', (m) => console.log(m.message));
adapter.on('keyPress', (k) => console.log('key', cec.userControlKeyToString(k.keycode)));
adapter.on('command', (c) => console.log('cmd', cec.opcodeToString(c.opcode)));

const [first] = adapter.detectAdapters();
adapter.open(first.path);

adapter.powerOnDevices(cec.CecLogicalAddress.TV);
for (const addr of adapter.getActiveDevices())
  console.log(cec.logicalAddressToString(addr), adapter.getDeviceOSDName(addr));

adapter.close();
```

### Events

`CecAdapter` is an `EventEmitter`. libCEC fires its callbacks from its own
worker thread; the addon marshals each one onto the Node event loop via a
`ThreadSafeFunction`, so handlers run on the main thread like any other event.

| event | argument(s) | source callback |
|-------|-------------|-----------------|
| `log` | `{ message, level, time }` | `logMessage` |
| `keyPress` | `{ keycode, duration }` | `keyPress` |
| `command` | `{ initiator, destination, opcode, parameters, ack, eom, opcodeSet, transmitTimeout }` | `commandReceived` |
| `sourceActivated` | `(logicalAddress, activated)` | `sourceActivated` |
| `alert` | `alert` (a `CecAlert`) | `alert` |
| `configurationChanged` | `{ deviceName, deviceTypes, physicalAddress, logicalAddresses, cecVersion, adapterType, firmwareVersion, … }` | `configurationChanged` |
| `menuStateChanged` | `state` (a `CecMenuState`) | `menuStateChanged` |
| `commandHandler` | same shape as `command` | `commandHandler` (opt-in) |

`commandHandler` is off by default — pass `{ enableCommandHandler: true }` to the
constructor to receive it. It routes *every* command through libCEC's blocking
command-handler path and carries the same data as the cheaper `command` event, so
it's only worth enabling if you specifically need that hook.

### Methods

Lifecycle: `open(port, timeout=10000)`, `close()`.

Control: `transmit(command)`, `powerOnDevices(address)`, `standbyDevices(address)`,
`setActiveSource(deviceType)`, `setInactiveView()`, `volumeUp()`, `volumeDown()`,
`muteAudio()`, `sendKeypress(destination, key, wait=true)`,
`sendKeyRelease(destination, wait=true)`, `setOSDString(destination, duration, message)`.

Queries: `getActiveSource()`, `isActiveSource(address)`,
`getDevicePowerStatus(address)`, `getDeviceVendorId(address)`,
`getDevicePhysicalAddress(address)`, `getDeviceCecVersion(address)`,
`getDeviceOSDName(address)`, `getActiveDevices()`, `pollDevice(address)`,
`rescanDevices()`, `pingAdapters()`, `detectAdapters()`, `getLibInfo()`.

Enum helpers (module-level): `cecVersionToString`, `powerStatusToString`,
`logicalAddressToString`, `vendorIdToString`, `opcodeToString`,
`userControlKeyToString`, plus the enum tables (`CecLogicalAddress`,
`CecDeviceType`, `CecPowerStatus`, `CecUserControlCode`, `CecOpcode`, …).

## Notes / limitations

* `commandHandler` and `menuStateChanged` cannot **suppress** libCEC's default
  handling from JavaScript. Both are dispatched on libCEC's callback thread and
  expect a synchronous "handled?" return; the addon always answers "not handled"
  so the library keeps its default behaviour. Honouring a JS return would mean
  blocking that thread on the Node event loop and racing libCEC's 1000ms timeout.
  They are therefore exposed as observe-only events.
* Call `close()` when done. It stops libCEC's worker thread before releasing the
  thread-safe callbacks, so no event can fire against a torn-down adapter.
