@echo off

set EXITCODE=1
SET MYDIR=%~dp0

rem Check for support folder
IF NOT EXIST "%MYDIR%..\support\windows\p8-usbcec-driver-installer.exe" (
  echo "support submodule was not checked out"
  goto RETURNEXIT
)

rem Check for NSIS
IF EXIST "%ProgramFiles%\NSIS\makensis.exe" (
  set NSIS="%ProgramFiles%\NSIS\makensis.exe"
) ELSE IF EXIST "%ProgramFiles(x86)%\NSIS\makensis.exe" (
  set NSIS="%ProgramFiles(x86)%\NSIS\makensis.exe"
) ELSE GOTO NONSIS

rem Check for VC14
IF "%VS140COMNTOOLS%"=="" (
  set COMPILER14="%ProgramFiles%\Microsoft Visual Studio 14.0\Common7\IDE\devenv.com"
) ELSE IF EXIST "%VS140COMNTOOLS%\..\IDE\VCExpress.exe" (
  set COMPILER14="%VS140COMNTOOLS%\..\IDE\VCExpress.exe"
) ELSE IF EXIST "%VS140COMNTOOLS%\..\IDE\devenv.com" (
  set COMPILER14="%VS140COMNTOOLS%\..\IDE\devenv.com"
) ELSE GOTO NOSDK14

rmdir /s /q %MYDIR%..\build 2> nul
call %MYDIR%build.cmd
IF NOT ERRORLEVEL 0 (
  GOTO ERRORCREATINGINSTALLER
)

copy "%MYDIR%..\support\windows\p8-usbcec-driver-installer.exe" "%MYDIR%..\build\."
cd "%MYDIR%..\project"

rem Skip to libCEC/x86 when we're running on win32
if "%PROCESSOR_ARCHITECTURE%"=="x86" if "%PROCESSOR_ARCHITEW6432%"=="" goto libcecx86

rem Compile libCEC and cec-client x64
echo. Cleaning libCEC (x64)
%COMPILER14% libcec.sln /Clean "Release|x64"
echo. Compiling libCEC (x64)
%COMPILER14% libcec.sln /Build "Release|x64" /Project LibCecSharp
%COMPILER14% libcec.sln /Build "Release|x64"
echo. Compiling .Net applications
cd "%MYDIR%..\src\dotnet\project"
%COMPILER14% cec-dotnet.sln /Build "Release|x64"
copy ..\build\x64\CecSharpTester.exe %MYDIR%..\build\amd64\CecSharpTester.exe
copy ..\build\x64\cec-tray.exe %MYDIR%..\build\amd64\cec-tray.exe

:libcecx86
rem Compile libCEC and cec-client Win32
cd "%MYDIR%..\project"
echo. Cleaning libCEC (x86)
%COMPILER14% libcec.sln /Clean "Release|x86"
echo. Compiling libCEC (x86)
%COMPILER14% libcec.sln /Build "Release|x86" /Project LibCecSharp
%COMPILER14% libcec.sln /Build "Release|x86"
echo. Compiling .Net applications
cd "%MYDIR%..\src\dotnet\project"
%COMPILER14% cec-dotnet.sln /Build "Release|x86"
copy ..\build\x86\CecSharpTester.exe %MYDIR%..\build\x86\CecSharpTester.exe
copy ..\build\x86\cec-tray.exe %MYDIR%..\build\x86\cec-tray.exe
cd "%MYDIR%..\project"

rem Clean things up before creating the installer
del /q /f %MYDIR%..\build\x86\LibCecSharp.pdb 2> nul
del /q /f %MYDIR%..\build\amd64\LibCecSharp.pdb 2> nul

GOTO CREATEEGPLUGIN

:SIGNBINARIES
rem Check for sign-binary.cmd, only present on the Pulse-Eight production build system
rem Calls signtool.exe and signs the DLLs with Pulse-Eight's code signing key
IF NOT EXIST "..\support\private\sign-binary.cmd" GOTO CREATEINSTALLER
echo. Signing all binaries
CALL ..\support\private\sign-binary.cmd %MYDIR%..\build\x86\cec.dll
CALL ..\support\private\sign-binary.cmd %MYDIR%..\build\x86\LibCecSharp.dll
CALL ..\support\private\sign-binary.cmd %MYDIR%..\build\x86\cec-client.exe
CALL ..\support\private\sign-binary.cmd %MYDIR%..\build\x86\cecc-client.exe
CALL ..\support\private\sign-binary.cmd %MYDIR%..\build\x86\cec-tray.exe
CALL ..\support\private\sign-binary.cmd %MYDIR%..\build\x86\CecSharpTester.exe
CALL ..\support\private\sign-binary.cmd %MYDIR%..\build\amd64\cec.dll
CALL ..\support\private\sign-binary.cmd %MYDIR%..\build\amd64\LibCecSharp.dll
CALL ..\support\private\sign-binary.cmd %MYDIR%..\build\amd64\cec-client.exe
CALL ..\support\private\sign-binary.cmd %MYDIR%..\build\amd64\cecc-client.exe
CALL ..\support\private\sign-binary.cmd %MYDIR%..\build\amd64\cec-tray.exe
CALL ..\support\private\sign-binary.cmd %MYDIR%..\build\amd64\CecSharpTester.exe


:CREATEINSTALLER
echo. Creating the installer
cd %MYDIR%..\build\x86
copy cec.dll libcec.dll
cd ..\amd64
copy cec.dll cec.x64.dll
cd %MYDIR%..\project
%NSIS% /V1 /X"SetCompressor /FINAL lzma" "libCEC.nsi"

FOR /F "delims=" %%F IN ('dir /b /s "%MYDIR%..\build\libCEC-*.exe" 2^>nul') DO SET INSTALLER=%%F
IF [%INSTALLER%] == [] (
  GOTO EGPLUGINCLEANUP
  GOTO ERRORCREATINGINSTALLER
)

rem Sign the installer if sign-binary.cmd exists
IF EXIST "..\support\private\sign-binary.cmd" (
  echo. Signing the installer binaries
  CALL ..\support\private\sign-binary.cmd %INSTALLER%
)

echo. The installer can be found here: %INSTALLER%
set EXITCODE=0
del /q /f %EGSOURCES%..\pulse_eight.egplugin 2> nul
GOTO EXIT

:CREATEEGPLUGIN
echo. Creating EventGhost plugin file
SET EGSOURCES=%MYDIR%..\src\eventghost\egplugin_sources\
copy %MYDIR%..\build\x86\python\cec\__init__.py %EGSOURCES%PulseEight\cec
copy %MYDIR%..\build\x86\python\cec\_cec.pyd %EGSOURCES%PulseEight
copy %MYDIR%..\build\x86\cec.dll %EGSOURCES%PulseEight
del /q /f %EGSOURCES%..\pulse_eight.egplugin 2> nul
PowerShell -ExecutionPolicy ByPass -Command "Add-Type -Assembly System.IO.Compression.FileSystem;[System.IO.Compression.ZipFile]::CreateFromDirectory('%EGSOURCES%', '%EGSOURCES%..\pulse_eight.egplugin', [System.IO.Compression.CompressionLevel]::Optimal, $false)"
del /q /f %EGSOURCES%PulseEight\cec\__init__.py 2> nul
del /q /f %EGSOURCES%PulseEight\_cec.pyd 2> nul
del /q /f %EGSOURCES%PulseEight\cec.dll 2> nul
IF NOT EXIST "%EGSOURCES%..\pulse_eight.egplugin" (
  GOTO NOEGPLUGIN
)
GOTO SIGNBINARIES

:NOEGPLUGIN
echo. Failed to create the EventGhost plugin file.
GOTO EXIT

:NOSDK14
echo. Visual Studio 2015 was not found on your system.
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
cd %MYDIR%..

:RETURNEXIT
exit /b %EXITCODE%
pause