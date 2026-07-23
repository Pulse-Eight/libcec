# libCEC .NET API — LibCecSharp

`LibCecSharp` (namespace `CecSharp`) is the managed binding for libCEC: a pure
C# assembly that binds the native library through its C API (P/Invoke over
`cecc.h`), targeting **net8.0**. Unlike the old C++/CLI wrappers it works on
Windows, Linux, macOS and Raspberry Pi.

The entry point is [`LibCecSharp`](xref:CecSharp.LibCecSharp): construct it with
a [`CecCallbackMethods`](xref:CecSharp.CecCallbackMethods) instance and a
[`LibCECConfiguration`](xref:CecSharp.LibCECConfiguration), then `Open()` a
connection to the adapter.

```csharp
using CecSharp;

var config = new LibCECConfiguration();
config.DeviceTypes.Types[0] = CecDeviceType.RecordingDevice;
config.SetCallbacks(myCallbacks);

using var lib = new LibCecSharp(myCallbacks, config);
foreach (var adapter in lib.FindAdapters(string.Empty))
{
    if (lib.Open(adapter.ComPort, 10000))
    {
        lib.PowerOnDevices(CecLogicalAddress.Tv);
        lib.Close();
    }
}
```

Browse the full type reference under **Api** in the navigation.

See also the [CecSharpTester](https://github.com/Pulse-Eight/cec-dotnet) console
sample and the Windows-only `cec-tray` app, which both reference this assembly.
