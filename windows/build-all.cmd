@ECHO OFF

rem Build a libCEC, LibCecSharp, LibCecSharpCore and client applications
rem Usage: build-all.cmd [architecture] [build type] [visual studio version]

SETLOCAL

SET MYDIR=%~dp0

rem optional parameter: architecture (amd64)
SET RUNTIMEARCH=amd64
IF "%PROCESSOR_ARCHITECTURE%"=="x86" IF "%PROCESSOR_ARCHITEW6432%"=="" (
  SET RUNTIMEARCH=x86
)
IF "%1" == "" (
  SET BUILDARCH=%RUNTIMEARCH%
) ELSE (
  SET BUILDARCH=%1
)
IF "%BUILDARCH%" == "amd64" (
  SET BUILDARCHPROJECT=x64
) ELSE (
  SET BUILDARCHPROJECT=%BUILDARCH%
)

rem optional parameter: build type (Release)
IF "%2" == "" (
  SET BUILDTYPE=Release
) ELSE (
  SET BUILDTYPE=%2
)

rem optional parameter: visual studio version (2019)
IF "%3" == "" (
  SET VSVERSION=2019
) ELSE (
  SET VSVERSION=%3
)

rem optional parameter: build .net applications
IF "%4" == "" (
  SET DOTNETAPPS=1
) ELSE (
  SET DOTNETAPPS=0
)

SET BUILDPATH=%MYDIR%..\build
SET EXITCODE=1

rem Create build dir
IF NOT EXIST "%MYDIR%..\build" (
  MKDIR "%MYDIR%..\build" >nul
)

rem Compile libCEC
ECHO. * compiling libCEC libraries for %BUILDARCH%
CD "%MYDIR%..\project"
CALL "%MYDIR%build-lib.cmd" %BUILDARCH% %BUILDTYPE% %VSVERSION% "%BUILDPATH%" nmake
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

rem Compile LibCecSharp and LibCecSharpCore
ECHO. * cleaning LibCecSharp and LibCecSharpCore for %BUILDARCH%
"%DevEnvDir%devenv.com" libcec.sln /Clean "%BUILDTYPE%|%BUILDARCHPROJECT%"
ECHO. * compiling LibCecSharp and LibCecSharpCore for %BUILDARCH%
"%DevEnvDir%devenv.com" libcec.sln /Build "%BUILDTYPE%|%BUILDARCHPROJECT%"

rem Create dir for referenced libs and check compilation results
RMDIR /s /q "%MYDIR%..\build\ref" >nul 2>&1
MKDIR "%MYDIR%..\build\ref" >nul
MKDIR "%MYDIR%..\build\ref\netcore" >nul

rem Check and copy LibCecSharp
IF EXIST "%MYDIR%..\build\%BUILDARCH%\LibCecSharp.dll" (
  COPY "%MYDIR%..\build\%BUILDARCH%\LibCecSharp.*" "%MYDIR%..\build\ref" >nul
) ELSE (
  ECHO. *** failed to build LibCecSharp for %BUILDARCH% ***
  PAUSE
  EXIT /b 1
)

rem Check and copy LibCecSharpCore
IF EXIST "%MYDIR%..\build\%BUILDARCH%\netcore\LibCecSharpCore.dll" (
  COPY "%MYDIR%..\build\%BUILDARCH%\netcore\LibCecSharpCore.*" "%MYDIR%..\build\ref\netcore\." >nul
) ELSE (
  ECHO. *** failed to build LibCecSharpCore for %BUILDARCH% ***
  PAUSE
  EXIT /b 1
)

IF %DOTNETAPPS% == 1 (
  rem Compile cec-tray and CecSharpTester apps
  ECHO. * compiling .Net applications for %BUILDARCH%
  CD "%MYDIR%..\src\dotnet\project"
  rem Restore nuget dependencies
  msbuild -t:restore
  "%DevEnvDir%devenv.com" cec-dotnet.sln /Build "%BUILDTYPE%|%BUILDARCHPROJECT%"

  rem Check and copy CecSharpTester
  IF EXIST "..\build\%BUILDARCHPROJECT%\CecSharpTester.exe" (
    COPY "..\build\%BUILDARCHPROJECT%\CecSharpTester.exe" "%MYDIR%..\build\%BUILDARCH%\CecSharpTester.exe" >nul
  ) ELSE (
    ECHO. *** failed to build CecSharpTester for %BUILDARCH% ***
    PAUSE
    EXIT /b 1
  )

  rem Check and copy cec-tray
  IF EXIST "..\build\%BUILDARCHPROJECT%\cec-tray.exe" (
    COPY "..\build\%BUILDARCHPROJECT%\cec-tray.exe" "%MYDIR%..\build\%BUILDARCH%\cec-tray.exe" >nul
  ) ELSE (
    ECHO. *** failed to build cec-tray for %BUILDARCH% ***
    PAUSE
    EXIT /b 1
  )

  rem Check and copy CecSharpCoreTester
  IF EXIST "..\build\%BUILDARCHPROJECT%\netcoreapp3.1\CecSharpCoreTester.exe" (
    COPY "..\build\%BUILDARCHPROJECT%\netcoreapp3.1\CecSharpCoreTester.*" "%MYDIR%..\build\%BUILDARCH%\netcore\." >nul
  ) ELSE (
    ECHO. *** failed to build CecSharpCoreTester for %BUILDARCH% ***
    PAUSE
    EXIT /b 1
  )
)

RMDIR /s /q "%BUILDPATH%\cmake" >nul 2>&1
