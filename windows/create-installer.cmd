@ECHO OFF

rem Build a libCEC installer for Windows
rem Usage: create-installer.cmd [visual studio version] [build type]

SETLOCAL

SET MYDIR=%~dp0

rem optional parameter: visual studio version (2019)
IF "%1" == "" (
  SET VSVERSION=2019
) ELSE (
  SET VSVERSION=%1
)
rem optional parameter: build type (Release)
IF "%2" == "" (
  SET BUILDTYPE=Release
) ELSE (
  SET BUILDTYPE=%2
)

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
CALL "%MYDIR%build-all.cmd" amd64 %BUILDTYPE% %VSVERSION%
IF %errorlevel% neq 0 (
  ECHO. *** failed to build libCEC for x64 ***
  SET EXITCODE=1
  GOTO EXIT
)

:libcecx86
CALL "%MYDIR%build-all.cmd" x86 %BUILDTYPE% %VSVERSION%
IF %errorlevel% neq 0 (
  ECHO. *** failed to build libCEC for x86 ***
  SET EXITCODE=1
  GOTO EXIT
)

:CREATEEGPLUGIN
ECHO. * creating EventGhost plugin
SET EGSOURCES=%MYDIR%..\src\eventghost\egplugin_sources\
COPY "%MYDIR%..\build\x86\python\cec\__init__.py" "%EGSOURCES%PulseEight\cec" >nul
COPY "%MYDIR%..\build\x86\python\cec\_cec.pyd" "%EGSOURCES%PulseEight" >nul
COPY "%MYDIR%..\build\x86\cec.dll" "%EGSOURCES%PulseEight" >nul
DEL /q /f "%EGSOURCES%..\pulse_eight.egplugin" >nul 2>&1
PowerShell -ExecutionPolicy ByPass -Command "Add-Type -Assembly System.IO.Compression.FileSystem;[System.IO.Compression.ZipFile]::CreateFromDirectory('%EGSOURCES%', '%EGSOURCES%..\pulse_eight.egplugin', [System.IO.Compression.CompressionLevel]::Optimal, $false)"
DEL /q /f "%EGSOURCES%PulseEight\cec\__init__.py" >nul 2>&1
DEL /q /f "%EGSOURCES%PulseEight\_cec.pyd" >nul 2>&1
DEL /q /f "%EGSOURCES%PulseEight\cec.dll" >nul 2>&1
IF NOT EXIST "%EGSOURCES%..\pulse_eight.egplugin" (
  ECHO. *** failed to create EventGhost plugin ***
  SET EXITCODE=1
  GOTO EXIT
)

:SIGNBINARIES
rem Check for sign-binary.cmd, only present on the Pulse-Eight production build system
rem Calls signtool.exe and signs the DLLs with Pulse-Eight's code signing key
IF NOT EXIST "..\support\private\sign-binary.cmd" GOTO CREATEINSTALLER
ECHO. * signing all binaries
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x86\cec.dll" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x86\LibCecSharp.dll" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x86\cec-tray.exe" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x86\CecSharpTester.exe" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x86\netcore\LibCecSharpCore.dll" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x86\netcore\CecSharpCoreTester.dll" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x86\netcore\CecSharpCoreTester.exe" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x86\cec-client.exe" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\x86\cecc-client.exe" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\amd64\cec.dll" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\amd64\LibCecSharp.dll" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\amd64\cec-tray.exe" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\amd64\CecSharpTester.exe" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\amd64\netcore\LibCecSharpCore.dll" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\amd64\netcore\CecSharpCoreTester.dll" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\amd64\netcore\CecSharpCoreTester.exe" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\amd64\cec-client.exe" >nul
CALL ..\support\private\sign-binary.cmd "%MYDIR%..\build\amd64\cecc-client.exe" >nul

:CREATEINSTALLER
rem Copy prebuilt driver
COPY "%MYDIR%..\support\windows\p8-usbcec-driver-installer.exe" "%MYDIR%..\build\." >nul
RMDIR /s /q "%MYDIR%..\build\ref" >nul 2>&1

CALL "%MYDIR%nsis-helper.cmd" libcec.nsi "libcec-*.exe"
IF %errorlevel% neq 0 (
  ECHO. *** failed to build installer ***
  SET EXITCODE=%errorlevel%
  GOTO EXIT
)

CALL "%MYDIR%nsis-helper.cmd" libcec.nsi "libcec-dbg-*.exe" "/DNSISINCLUDEPDB"
IF %errorlevel% neq 0 (
  ECHO. *** failed to build installer ***
  SET EXITCODE=%errorlevel%
  GOTO EXIT
)

:EXIT
CD "%MYDIR%"
PAUSE
exit /b %EXITCODE%
