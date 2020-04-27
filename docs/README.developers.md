# Developers

We provide a C, C++, Python and .NET CLR interface to the adapter.

## C++ developers
* the API can be found in `include/cec.h`
* an example implementation can be found on https://github.com/Pulse-Eight/libcec/blob/master/src/cec-client/cec-client.cpp

## C developers
* the API can be found in `include/cecc.h`
* an example implementation can be found on https://github.com/Pulse-Eight/libcec/blob/master/src/cecc-client/cecc-client.c

## .NET Framework developers
* add a reference to LibCecSharp.dll for the target architecture (x86/amd64). It's installed to `C:\Program Files (x86)\Pulse-Eight\USB-CEC Adapter\netfx` by default
* the minimum .Net Framework version required for LibCecSharp is 4.0
* an example implementation can be found on https://github.com/Pulse-Eight/cec-dotnet/blob/master/src/CecSharpTester/CecSharpClient.cs

## .NET Core developers
* add a reference to LibCecSharpCore.dll for the target architecture (x86/amd64). It's installed to `C:\Program Files (x86)\Pulse-Eight\USB-CEC Adapter\netcore` by default
* the minimum .Net Core version required for LibCecSharpCore is 3.1
* an example implementation can be found on https://github.com/Pulse-Eight/cec-dotnet/blob/master/src/CecSharpTester/CecSharpClient.cs

## Python developers
* the API is exported to Python through Swig
* an example implementation can be found on https://github.com/Pulse-Eight/libcec/blob/master/src/pyCecClient/pyCecClient.py

# Developers Agreement

If you wish to contribute to this project, you must first sign our contributors agreement.
Please see [the contributors agreement](http://www.pulse-eight.com/contributors) for more information.
