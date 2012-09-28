@echo off

set EXITCODE=1

rem Check for NSIS
IF EXIST "%ProgramFiles%\NSIS\makensis.exe" (
  set NSIS="%ProgramFiles%\NSIS\makensis.exe"
) ELSE IF EXIST "%ProgramFiles(x86)%\NSIS\makensis.exe" (
  set NSIS="%ProgramFiles(x86)%\NSIS\makensis.exe"
) ELSE GOTO NONSIS

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

IF EXIST "..\support\p8-usbcec-driver-installer.exe" (
  copy "..\support\p8-usbcec-driver-installer.exe" "..\build\."
) ELSE (
  rem Check for the Windows DDK
  IF NOT EXIST "C:\WinDDK\7600.16385.1" GOTO NODDK
  set DDK="C:\WinDDK\7600.16385.1"

  call create-driver-installer.cmd
)

mkdir ..\build\x64

cd ..\project

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
%COMPILER9% LibCecSharp.sln /build "Release|x64" /project cec-config-gui

copy ..\build\LibCecSharp.dll ..\build\x64\LibCecSharp.dll
copy ..\build\CecSharpTester.exe ..\build\x64\CecSharpTester.exe
copy ..\build\cec-config-gui.exe ..\build\x64\cec-config-gui.exe

:libcecsharpx86
rem Compile LibCecSharp (x86)
echo. Cleaning LibCecSharp (x86)
%COMPILER9% LibCecSharp.sln /clean "Release|x86"
echo. Compiling LibCecSharp (x86)
%COMPILER9% LibCecSharp.sln /build "Release|x86" /project LibCecSharp
%COMPILER9% LibCecSharp.sln /build "Release|x86" /project CecSharpTester
%COMPILER9% LibCecSharp.sln /build "Release|x86" /project cec-config-gui

:NOSDK9
rem Clean things up before creating the installer
del /q /f ..\build\LibCecSharp.pdb
del /q /f ..\build\CecSharpTester.pdb
del /q /f ..\build\cec-config-gui.pdb
del /q /f ..\build\cec-config-gui.vshost.exe.manifest
del /q /f ..\build\cec-config-gui.vshost.exe
copy ..\build\cec-client.x64.exe ..\build\x64\cec-client.x64.exe
del /q /f ..\build\cec-client.x64.exe
copy ..\build\libcec.x64.dll ..\build\x64\libcec.x64.dll
del /q /f ..\build\libcec.x64.dll
copy ..\build\libcec.x64.lib ..\build\x64\libcec.x64.lib
del /q /f ..\build\libcec.x64.lib

rem Check for sign-binary.cmd, only present on the Pulse-Eight production build system
rem Calls signtool.exe and signs the DLLs with Pulse-Eight's code signing key
IF NOT EXIST "..\support\private\sign-binary.cmd" GOTO CREATEINSTALLER
echo. Signing all binaries
CALL ..\support\private\sign-binary.cmd ..\build\cec-client.exe
CALL ..\support\private\sign-binary.cmd ..\build\CecSharpTester.exe
CALL ..\support\private\sign-binary.cmd ..\build\libcec.dll
CALL ..\support\private\sign-binary.cmd ..\build\LibCecSharp.dll
CALL ..\support\private\sign-binary.cmd ..\build\cec-config-gui.exe
CALL ..\support\private\sign-binary.cmd ..\build\x64\cec-client.x64.exe
CALL ..\support\private\sign-binary.cmd ..\build\x64\CecSharpTester.exe
CALL ..\support\private\sign-binary.cmd ..\build\x64\libcec.x64.dll
CALL ..\support\private\sign-binary.cmd ..\build\x64\LibCecSharp.dll
CALL ..\support\private\sign-binary.cmd ..\build\x64\cec-config-gui.exe

:CREATEINSTALLER
echo. Creating the installer
%NSIS% /V1 /X"SetCompressor /FINAL lzma" "libCEC.nsi"

IF NOT EXIST "..\build\libCEC-installer.exe" GOTO :ERRORCREATINGINSTALLER

rem Sign the installer if sign-binary.cmd exists
IF EXIST "..\support\private\sign-binary.cmd" (
  echo. Signing the installer binaries
  CALL ..\support\private\sign-binary.cmd ..\build\libCEC-installer.exe
)

IF "%1%"=="" (
  echo. The installer can be found here: ..\build\libCEC-installer.exe
) ELSE (
  move ..\build\libCEC-installer.exe ..\build\libCEC-%1%-installer.exe
  echo. The installer can be found here: ..\build\libCEC-%1%-installer.exe
)

set EXITCODE=0
GOTO EXIT

:NOSDK10
echo. Both Visual Studio 2010 and Visual C++ Express 2010 were not found on your system.
GOTO EXIT

:NOSIS
echo. NSIS could not be found on your system.
GOTO EXIT

:NODDK
echo. Windows DDK could not be found on your system
GOTO EXIT

:ERRORCREATINGINSTALLER
echo. The installer could not be created. The most likely cause is that something went wrong while compiling.

:EXIT
del /q /f ..\build\cec-client.exe
del /q /f ..\build\CecSharpTester.exe
del /q /f ..\build\cec-config-gui.exe
del /q /f ..\build\*.dll
del /q /f ..\build\*.lib
del /q /f ..\build\*.exp
del /q /f ..\build\*.xml
del /s /f /q ..\build\x64
rmdir ..\build\x64
cd ..\support

IF "%1%"=="" (
  echo. exitcode = %EXITCODE%
) ELSE (
  exit %EXITCODE%
)

