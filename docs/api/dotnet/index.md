# libCEC — .NET API (LibCecSharp)

`LibCecSharp` (namespace `CecSharp`) is the managed binding for libCEC: a **pure
C# assembly** that binds the native library through its C API (P/Invoke over
`cecc.h`), targeting **net8.0**. Unlike the old C++/CLI wrappers it runs anywhere
the native library does — Windows, Linux, macOS and Raspberry Pi.

It lets .NET applications control CEC-capable HDMI devices — power a TV on or to
standby, become the active source, send remote keys, read device state — and
receive bus events. Every binding wraps the same engine, so the concepts match
the [C/C++ reference](https://pulse-eight.github.io/libcec/cpp/), which documents them in the most depth.

## Requirements

- A **Pulse-Eight USB-CEC adapter** (or a supported SoC-native CEC backend).
- The native **libCEC** library available at runtime (`cec.dll` on Windows,
  `libcec.so` on Linux, `libcec.dylib` on macOS).
- The **.NET 8** runtime (the Windows installer prompts for the .NET 8 Desktop
  Runtime when the managed component is selected).

## Installing

### Debian / Ubuntu

```sh
sudo apt-get install libcec-dotnet
```

This installs the `LibCecSharp` assembly and its NuGet package alongside the
native library.

### Windows

The **USB-CEC Adapter software** installer ships `LibCecSharp.dll` and the native
`cec.dll` together (select the managed component). To build both from source, run
`python windows\create-installer.py`, which drives cmake with
`-DENABLE_DOTNET_LIB=1`.

### Build from source (any platform with the .NET SDK)

The managed binding is a cmake option, off by default:

```sh
mkdir build && cd build
cmake .. -DENABLE_DOTNET_LIB=1
make -j4
```

Add `-DENABLE_DOTNET_APPS=1` to also build the Windows-only `cec-tray` and the
cross-platform `CecSharpTester` sample. Reference the resulting `LibCecSharp.dll`
(or its NuGet package) from your project, and make sure the native library is on
the loader path at runtime.

## Using the API

Derive from [`CecCallbackMethods`](xref:CecSharp.CecCallbackMethods) to receive
events, describe yourself in a
[`LibCECConfiguration`](xref:CecSharp.LibCECConfiguration), then construct
[`LibCecSharp`](xref:CecSharp.LibCecSharp) and open a connection.

```csharp
using CecSharp;

// 1. handle events by overriding the callback methods
class CecClient : CecCallbackMethods
{
    public override int ReceiveLogMessage(CecLogMessage message)
    {
        Console.WriteLine($"log: {message.Message}");
        return 1;
    }

    public override int ReceiveKeypress(CecKeypress key)
    {
        Console.WriteLine($"key: {key.Keycode}");
        return 1;
    }
}

var client = new CecClient();

// 2. describe this client
var config = new LibCECConfiguration();
config.DeviceName = "CecSharp";
config.DeviceTypes.Types[0] = CecDeviceType.RecordingDevice;
config.ClientVersion = LibCECConfiguration.CurrentVersion;
config.Callbacks = client;

// 3. create the instance and open the first adapter found
using var lib = new LibCecSharp(client, config);
CecAdapter[] adapters = lib.FindAdapters(string.Empty);
if (adapters.Length > 0 && lib.Open(adapters[0].ComPort, 10000))
{
    // 4. control the bus
    lib.PowerOnDevices(CecLogicalAddress.Tv);
    foreach (var address in lib.GetActiveDevices().Addresses)
        if (address != CecLogicalAddress.Unknown)
            Console.WriteLine(lib.GetDeviceOSDName(address));

    // 5. clean up
    lib.Close();
}
```

`FindAdapters(string.Empty)` enumerates connected adapters;
[`Open`](xref:CecSharp.LibCecSharp.Open*) connects to one. The callback methods
are invoked from libCEC's worker thread, so keep them short. Dispose the
`LibCecSharp` instance (the `using` above) to close the connection and release
the native resources.

See the `CecSharpTester` console sample and the `cec-tray` WinForms app in the
[cec-dotnet repository](https://github.com/Pulse-Eight/cec-dotnet) for complete,
runnable programs.

## API reference

Browse the full type reference under **Api** in the navigation, starting with
[`LibCecSharp`](xref:CecSharp.LibCecSharp).
