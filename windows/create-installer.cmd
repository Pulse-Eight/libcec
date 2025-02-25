@ECHO OFF

rem Build a libCEC installer for Windows
rem Usage: create-installer.cmd [visual studio version] [build type]

SETLOCAL

SET MYDIR=%~dp0
SET NETCORE_DIR=net8.0

rem optional parameter: visual studio version (2022)
IF "%1" == "" (
  SET VSVERSION=2022
) ELSE (
  SET VSVERSION=%1
)
rem optional parameter: build type (Release)
IF "%2" == "" (
  SET BUILDTYPE=Release
) ELSE (
  SET BUILDTYPE=%2
)

SET NSISDOTNET=/DNSISDOTNETAPPS
SET BUILDPATH=%MYDIR%..\build
SET EXITCODE=1

rem Delete previous build dirs
ECHO. * clearing old build directories
RMDIR /s /q "%MYDIR%..\build" >nul 2>&1
RMDIR /s /q "%MYDIR%..\src\dotnet\build" >nul 2>&1
MKDIR "%MYDIR%..\build" >nul

rem Skip to libCEC/x86 if we're running on win32
IF "%PROCESSOR_ARCHITECTURE%"=="x86" IF "%PROCESSOR_ARCHITEW6432%"=="" GOTO libcecx86

:libcecx64
SET X86ONLY=0
CALL "%MYDIR%build-all.cmd" x64 %BUILDTYPE% %VSVERSION%
IF %errorlevel% neq 0 (
  ECHO. *** failed to build libCEC for x64 ***
  SET EXITCODE=1
  GOTO EXIT
)

:libcecx86
SET X86ONLY=1
CALL "%MYDIR%build-all.cmd" x86 %BUILDTYPE% %VSVERSION%
IF %errorlevel% neq 0 (
  ECHO. *** failed to build libCEC for x86 ***
  SET EXITCODE=1
  GOTO EXIT
)

:SIGNBINARIES
rem Check for sign-binary.cmd, only present on the Pulse-Eight production build system
rem Calls signtool.exe and signs the DLLs with Pulse-Eight's code signing key
IF NOT EXIST "..\support\private\sign-binary.cmd" GOTO CREATEEGPLUGIN
ECHO. * signing all binaries
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x86\cec.dll" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x86\LibCecSharp.dll" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x86\%NETCORE_DIR%\LibCecSharpCore.dll" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x86\cec-client.exe" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x86\cecc-client.exe" >nul
IF EXIST "%MYDIR%..\build\x86\cec-tray.exe" (
  CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x86\cec-tray.exe" >nul
)
IF EXIST "%MYDIR%..\build\x86\CecSharpTester.exe" (
  CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x86\CecSharpTester.exe" >nul
)
IF EXIST "%MYDIR%..\build\x86\%NETCORE_DIR%\CecSharpCoreTester.exe" (
  CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x86\%NETCORE_DIR%\CecSharpCoreTester.dll" >nul
  CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x86\%NETCORE_DIR%\CecSharpCoreTester.exe" >nul
)

IF %X86ONLY% == 0 (
  CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x64\cec.dll" >nul
  CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x64\LibCecSharp.dll" >nul
  CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x64\%NETCORE_DIR%\LibCecSharpCore.dll" >nul
  CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x64\cec-client.exe" >nul
  CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x64\cecc-client.exe" >nul
  IF EXIST "%MYDIR%..\build\x64\cec-tray.exe" (
    CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x64\cec-tray.exe" >nul
  )
  IF EXIST "%MYDIR%..\build\x64\CecSharpTester.exe" (
    CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x64\CecSharpTester.exe" >nul
  )
  IF EXIST "%MYDIR%..\build\x64\%NETCORE_DIR%\CecSharpCoreTester.exe" (
    CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x64\%NETCORE_DIR%\CecSharpCoreTester.dll" >nul
    CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x64\%NETCORE_DIR%\CecSharpCoreTester.exe" >nul
  )
)

:CREATEEGPLUGIN
call "%MYDIR%eventghost.cmd"

IF %errorlevel% neq 0 (
  ECHO. *** failed to create EventGhost plugin ***
  SET EXITCODE=1
  GOTO EXIT
)

:CREATEINSTALLER
rem Copy prebuilt drivers
COPY "%MYDIR%..\support\windows\p8-usbcec-driver-installer.exe" "%MYDIR%..\build\." >nul
COPY "%MYDIR%..\support\windows\p8-usbcec-bootloader-driver-installer.exe" "%MYDIR%..\build\." >nul
COPY "%MYDIR%..\support\windows\libusb0.dll" "%MYDIR%..\build\." >nul
RMDIR /s /q "%MYDIR%..\build\ref" >nul 2>&1

rem Build standard x64 installer
CALL "%MYDIR%nsis-helper.cmd" libcec.nsi "libcec-x64-*.exe" "%NSISDOTNET%"
IF %errorlevel% neq 0 (
  ECHO. *** failed to build x64 installer ***
  SET EXITCODE=%errorlevel%
  GOTO EXIT
)

rem Build debug x64 installer
CALL "%MYDIR%nsis-helper.cmd" libcec.nsi "libcec-x64-dbg-*.exe" "/DNSISINCLUDEPDB %NSISDOTNET%"
IF %errorlevel% neq 0 (
  ECHO. *** failed to build debug x64 installer ***
  SET EXITCODE=%errorlevel%
  GOTO EXIT
)

rem Build standard x86 installer
CALL "%MYDIR%nsis-helper.cmd" libcec.nsi "libcec-x86-*.exe" "/DNSIS_X86 %NSISDOTNET%"
IF %errorlevel% neq 0 (
  ECHO. *** failed to build x86 installer ***
  SET EXITCODE=%errorlevel%
  GOTO EXIT
)

rem Build debug x86 installer
CALL "%MYDIR%nsis-helper.cmd" libcec.nsi "libcec-x86-dbg-*.exe" "/DNSIS_X86 /DNSISINCLUDEPDB %NSISDOTNET%"
IF %errorlevel% neq 0 (
  ECHO. *** failed to build debug x86 installer ***
  SET EXITCODE=%errorlevel%
  GOTO EXIT
)

rem Build standalone EventGhost plugin installer
CALL "%MYDIR%nsis-helper.cmd" eventghost.nsi "libcec-eventghost-plugin*.exe"
IF %errorlevel% neq 0 (
  ECHO. *** failed to build EventGhost plugin installer ***
  SET EXITCODE=%errorlevel%
  GOTO EXIT
)

:EXIT
CD "%MYDIR%"
PAUSE
exit /b %EXITCODE%
