@ECHO OFF

rem Build a libCEC, LibCecSharp, LibCecSharpCore and client applications
rem Usage: build-all.cmd [architecture] [build type] [visual studio version]

SETLOCAL

SET MYDIR=%~dp0
SET NETCORE_DIR=net8.0

rem optional parameter: architecture (x64)
SET RUNTIMEARCH=x64
IF "%PROCESSOR_ARCHITECTURE%"=="x86" IF "%PROCESSOR_ARCHITEW6432%"=="" (
  SET RUNTIMEARCH=x86
)
IF "%PROCESSOR_ARCHITECTURE%"=="arm64" (
  SET RUNTIMEARCH=arm64
)
IF "%1" == "" (
  SET BUILDARCH=%RUNTIMEARCH%
) ELSE (
  SET BUILDARCH=%1
)

rem optional parameter: build type (Release)
IF "%2" == "" (
  SET BUILDTYPE=Release
) ELSE (
  SET BUILDTYPE=%2
)

rem optional parameter: visual studio version (2022)
IF "%3" == "" (
  SET VSVERSION=2022
) ELSE (
  SET VSVERSION=%3
)

rem optional parameter: build .net applications
IF "%4" == "" (
  SET DOTNETAPPS=1
) ELSE (
  SET DOTNETAPPS=0
)

rem Building .NET applications is not supported on ARM64
if "%BUILDARCH%" == "arm64" (
  SET DOTNETAPPS=0
)  

SET BUILDPATH=%MYDIR%..\build
SET EXITCODE=1

rem Create build dir
IF NOT EXIST "%BUILDPATH%" (
  MKDIR "%BUILDPATH%" >nul
)

rem Compile libCEC
CD "%MYDIR%..\project"
ECHO. * compiling release libCEC libraries for %BUILDARCH%
CALL "%MYDIR%build-lib.cmd" %BUILDARCH% Release %VSVERSION% "%BUILDPATH%\Release" nmake
IF %errorlevel% neq 0 (
  ECHO. *** failed to build libCEC for %BUILDARCH% ***
  PAUSE
  EXIT /b 1
)
ECHO. * compiling debug libCEC libraries for %BUILDARCH%
CALL "%MYDIR%build-lib.cmd" %BUILDARCH% Debug %VSVERSION% "%BUILDPATH%\Debug" nmake
IF %errorlevel% neq 0 (
  ECHO. *** failed to build libCEC for %BUILDARCH% ***
  PAUSE
  EXIT /b 1
)

rem Set up the toolchain 
CALL "%MYDIR%..\support\windows\config\toolchain.cmd" >nul
IF "%TOOLCHAIN_NAME%" == "" (
  ECHO.*** Visual Studio toolchain could not be configured for %BUILDARCH% ***
  ECHO.
  ECHO.See docs\README.windows.md
  EXIT /b 2
)
  
rem Building LibCecSharp isn't supported on ARM64
if not "%BUILDARCH%" == "arm64" (
  rem Compile LibCecSharp and LibCecSharpCore
  ECHO. * cleaning LibCecSharp and LibCecSharpCore for %BUILDARCH% Release
  "%DevEnvDir%devenv.com" libcec.sln /Clean "Release|%BUILDARCH%"
  ECHO. * compiling LibCecSharp and LibCecSharpCore for %BUILDARCH% Release
  "%DevEnvDir%devenv.com" libcec.sln /Build "Release|%BUILDARCH%"

  rem Check LibCecSharp
  IF NOT EXIST "%BUILDPATH%\Release\%BUILDARCH%\LibCecSharp.dll" (
    ECHO. *** failed to build Release LibCecSharp for %BUILDARCH% ***
    PAUSE
    EXIT /b 1
  )

  rem Check LibCecSharpCore
  IF NOT EXIST "%BUILDPATH%\Release\%BUILDARCH%\%NETCORE_DIR%\LibCecSharpCore.dll" (
    ECHO. *** failed to build Release LibCecSharpCore for %BUILDARCH% ***
    PAUSE
    EXIT /b 1
  )

  rem Debug builds
  ECHO. * cleaning LibCecSharp and LibCecSharpCore for %BUILDARCH% Debug
  "%DevEnvDir%devenv.com" libcec.sln /Clean "Debug|%BUILDARCH%"
  ECHO. * compiling LibCecSharp and LibCecSharpCore for %BUILDARCH% Debug
  "%DevEnvDir%devenv.com" libcec.sln /Build "Debug|%BUILDARCH%"

  rem Check LibCecSharp
  IF NOT EXIST "%BUILDPATH%\Debug\%BUILDARCH%\LibCecSharp.dll" (
    ECHO. *** failed to build Debug LibCecSharp for %BUILDARCH% ***
    PAUSE
    EXIT /b 1
  )

  rem Check LibCecSharpCore
  IF NOT EXIST "%BUILDPATH%\Debug\%BUILDARCH%\%NETCORE_DIR%\LibCecSharpCore.dll" (
    ECHO. *** failed to build Debug LibCecSharpCore for %BUILDARCH% ***
    PAUSE
    EXIT /b 1
  )
)  

IF %DOTNETAPPS% == 1 (
  rem Compile cec-tray and CecSharpTester apps
  ECHO. * compiling .Net applications for %BUILDARCH%
  CD "%MYDIR%..\src\dotnet\project"
  rem Restore nuget dependencies
  msbuild -t:restore

  "%DevEnvDir%devenv.com" cec-dotnet.sln /Build "Release|%BUILDARCH%"

  rem Check CecSharpTester
  IF NOT EXIST "%BUILDPATH%\Release\%BUILDARCH%\CecSharpTester.exe" (
    ECHO. *** failed to build CecSharpTester for %BUILDARCH% ***
    PAUSE
    EXIT /b 1
  )

  rem Check cec-tray
  IF NOT EXIST "%BUILDPATH%\Release\%BUILDARCH%\cec-tray.exe" (
    ECHO. *** failed to build cec-tray for %BUILDARCH% ***
    PAUSE
    EXIT /b 1
  )

  rem Check CecSharpCoreTester
  IF NOT EXIST "%BUILDPATH%\Release\%BUILDARCH%\%NETCORE_DIR%\CecSharpCoreTester.exe" (
    ECHO. *** failed to build CecSharpCoreTester for %BUILDARCH% ***
    PAUSE
    EXIT /b 1
  )

  "%DevEnvDir%devenv.com" cec-dotnet.sln /Build "Debug|%BUILDARCH%"

  rem Check CecSharpTester
  IF NOT EXIST "%BUILDPATH%\Debug\%BUILDARCH%\CecSharpTester.exe" (
    ECHO. *** failed to build CecSharpTester for %BUILDARCH% ***
    PAUSE
    EXIT /b 1
  )

  rem Check cec-tray
  IF NOT EXIST "%BUILDPATH%\Debug\%BUILDARCH%\cec-tray.exe" (
    ECHO. *** failed to build cec-tray for %BUILDARCH% ***
    PAUSE
    EXIT /b 1
  )

  rem Check CecSharpCoreTester
  IF NOT EXIST "%BUILDPATH%\Debug\%BUILDARCH%\%NETCORE_DIR%\CecSharpCoreTester.exe" (
    ECHO. *** failed to build CecSharpCoreTester for %BUILDARCH% ***
    PAUSE
    EXIT /b 1
  )
)

RMDIR /s /q "%BUILDPATH%\cmake" >nul 2>&1

ECHO. Finished building libCEC for %BUILDARCH%

EXIT /b 0
