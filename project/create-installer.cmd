@echo off

rem Check for NSIS
IF EXIST "%ProgramFiles%\NSIS\makensis.exe" (
  set NSIS="%ProgramFiles%\NSIS\makensis.exe"
) ELSE IF EXIST "%ProgramFiles(x86)%\NSIS\makensis.exe" (
  set NSIS="%ProgramFiles(x86)%\NSIS\makensis.exe"
) ELSE GOTO NONSIS

rem Check for the Windows DDK
IF NOT EXIST "C:\WinDDK\7600.16385.1" GOTO NODDK
set DDK="C:\WinDDK\7600.16385.1"

rem Check for VC10
IF "%VS100COMNTOOLS%"=="" (
  set COMPILER10="%ProgramFiles%\Microsoft Visual Studio 10.0\Common7\IDE\VCExpress.exe"
) ELSE IF EXIST "%VS100COMNTOOLS%\..\IDE\VCExpress.exe" (
  set COMPILER10="%VS100COMNTOOLS%\..\IDE\VCExpress.exe"
) ELSE IF EXIST "%VS100COMNTOOLS%\..\IDE\devenv.exe" (
  set COMPILER10="%VS100COMNTOOLS%\..\IDE\devenv.exe"
) ELSE GOTO NOSDK10

del /s /f /q ..\build
mkdir ..\build
mkdir ..\build\x64

rem Skip to libCEC/x86 when we're running on win32
if "%PROCESSOR_ARCHITECTURE%"=="x86" if "%PROCESSOR_ARCHITEW6432%"=="" goto libcecx86

rem Compile libCEC and cec-client x64
echo. Cleaning libCEC (x64)
%COMPILER10% libcec.sln /clean "Release|x64"
echo. Compiling libCEC (x64)
%COMPILER10% libcec.sln /build "Release|x64" /project libcec
echo. Compiling cec-client (x64)
%COMPILER10% libcec.sln /build "Release|x64" /project testclient

:libcecx86
rem Compile libCEC and cec-client Win32
echo. Cleaning libCEC (x86)
%COMPILER10% libcec.sln /clean "Release|Win32"
echo. Compiling libCEC (x86)
%COMPILER10% libcec.sln /build "Release|Win32" /project libcec
echo. Compiling cec-client (x86)
%COMPILER10% libcec.sln /build "Release|Win32" /project testclient

rem Check for VC9
IF "%VS90COMNTOOLS%"=="" (
  set COMPILER9="%ProgramFiles%\Microsoft Visual Studio 9.0\Common7\IDE\VCExpress.exe"
) ELSE IF EXIST "%VS90COMNTOOLS%\..\IDE\VCExpress.exe" (
  set COMPILER9="%VS90COMNTOOLS%\..\IDE\VCExpress.exe"
) ELSE IF EXIST "%VS90COMNTOOLS%\..\IDE\devenv.exe" (
  set COMPILER9="%VS90COMNTOOLS%\..\IDE\devenv.exe"
) ELSE GOTO NOSDK9

rem Skip to libCEC/x86 when we're running on win32
if "%PROCESSOR_ARCHITECTURE%"=="x86" if "%PROCESSOR_ARCHITEW6432%"=="" goto libcecsharpx86

rem Compile LibCecSharp (x64)
echo. Cleaning LibCecSharp (x64)
%COMPILER9% LibCecSharp.Net2.sln /clean "Release|x64"
echo. Compiling LibCecSharp (x64)
%COMPILER9% LibCecSharp.sln /build "Release|x64" /project LibCecSharp
%COMPILER9% LibCecSharp.sln /build "Release|x64" /project CecSharpTester

copy ..\build\LibCecSharp.dll ..\build\x64\LibCecSharp.dll
copy ..\build\CecSharpTester.exe ..\build\x64\CecSharpTester.exe

:libcecsharpx86
rem Compile LibCecSharp (x86)
echo. Cleaning LibCecSharp (x86)
%COMPILER9% LibCecSharp.sln /clean "Release|x86"
echo. Compiling LibCecSharp (x86)
%COMPILER9% LibCecSharp.sln /build "Release|x86" /project LibCecSharp
%COMPILER9% LibCecSharp.sln /build "Release|x86" /project CecSharpTester

:NOSDK9
:CREATEINSTALLER
echo. Copying driver installer
copy "%DDK%\redist\DIFx\dpinst\MultiLin\amd64\dpinst.exe" ..\build\dpinst-amd64.exe
copy "%DDK%\redist\DIFx\dpinst\MultiLin\x86\dpinst.exe" ..\build\dpinst-x86.exe

rem Clean things up before creating the installer
del ..\build\LibCecSharp.pdb
del ..\build\CecSharpTester.pdb
copy ..\build\cec-client.x64.exe ..\build\x64\cec-client.x64.exe
del ..\build\cec-client.x64.exe
copy ..\build\libcec.x64.dll ..\build\x64\libcec.x64.dll
del ..\build\libcec.x64.dll
copy ..\build\libcec.x64.lib ..\build\x64\libcec.x64.lib
del ..\build\libcec.x64.lib

echo. Creating the installer
%NSIS% /V1 /X"SetCompressor /FINAL lzma" "libCEC.nsi"

echo. The installer can be found here: ..\build\libCEC-installer.exe

GOTO EXIT

:NOSDK10
echo. Both Visual Studio 2010 and Visual C++ Express 2010 were not found on your system.
GOTO EXIT

:NOSIS
echo. NSIS could not be found on your system.
GOTO EXIT

:NODDK
echo. Windows DDK could not be found on your system

:EXIT
