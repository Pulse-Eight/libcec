# LibCecSharp

The managed binding for [libCEC](https://github.com/Pulse-Eight/libcec): control
CEC-capable HDMI devices — power a TV on or to standby, become the active source,
send remote keys, read device state — and receive bus events.

`LibCecSharp` (namespace `CecSharp`) is a pure C# assembly that binds the native
library through its C API (P/Invoke over `cecc.h`), targeting net8.0. It runs
anywhere the native library does: Windows, Linux, macOS and Raspberry Pi.

## Installing

```
dotnet add package LibCecSharp
```

## Requirements

* A **Pulse-Eight USB-CEC adapter**, or a supported SoC-native CEC backend.
* The native **libCEC** library on the loader path at runtime — `cec.dll` on
  Windows, `libcec.so` on Linux, `libcec.dylib` on macOS. This package is managed
  IL only; it does not carry the native library.

Install the native library with the [Windows USB-CEC Adapter
software](https://github.com/Pulse-Eight/libcec/releases), with `apt-get install
libcec8` on Debian/Ubuntu, or build it from source.

## Usage

Derive from `CecCallbackMethods` to receive events, describe yourself in a
`LibCECConfiguration`, then construct `LibCecSharp`.

```csharp
using CecSharp;

class CecClient : CecCallbackMethods
{
    public override int ReceiveLogMessage(CecLogMessage message)
    {
        Console.WriteLine($"log: {message.Message}");
        return 1;
    }
}

var config = new LibCECConfiguration();
config.DeviceTypes.Types[0] = CecDeviceType.RecordingDevice;
config.DeviceName = "example";
config.ClientVersion = LibCECConfiguration.CurrentVersion;

using var lib = new LibCecSharp(new CecClient(), config);

// a null port opens the first adapter that opens, skipping any that another
// process is using
if (lib.Open(null, 10000))
{
    lib.PowerOnDevices(CecLogicalAddress.Tv);
    lib.Close();
}
```

Full API reference: <https://pulse-eight.github.io/libcec/dotnet/>

## Licence

GPL-2.0-or-later, or a commercial licence from
[Pulse-Eight](http://www.pulse-eight.com/) — the same dual licence as libCEC
itself.
