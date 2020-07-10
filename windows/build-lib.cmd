@ECHO OFF

rem Build libCEC cmake projects for Windows
rem Usage: build-all.cmd [architecture] [type] [vs version] [install path] [project type]

SETLOCAL

SET MYDIR=%~dp0
SET BUILDARCH=%1
SET BUILDTYPE=%2
SET VSVERSION=%3
SET INSTALLPATH=%~4
SET GENTYPE=%5
IF [%5] == [] GOTO missingparams

SET INSTALLPATH=%INSTALLPATH:"=%
SET BUILDTARGET=%INSTALLPATH%\cmake\%BUILDARCH%
SET TARGET=%INSTALLPATH%\%BUILDARCH%

rem Check support submodule
IF NOT EXIST "%MYDIR%..\support\windows\cmake\build.cmd" (
  rem Try to init the git submodules
  cd "%MYDIR%.."
  git submodule update --init -r >nul 2>&1

  IF NOT EXIST "%MYDIR%..\support\windows\cmake\build.cmd" (
    ECHO.*** support git submodule has not been checked out ***
    ECHO.
    ECHO.See docs\README.windows.md
    EXIT /b 2
  )
)

rem Check platform submodule
IF NOT EXIST "%MYDIR%..\src\platform\windows\build.cmd" (
  ECHO.*** platform git submodule has not been checked out ***
  ECHO.
  ECHO.See docs\README.windows.md
  EXIT /b 2
)

rem Compile platform library
ECHO. * compiling platform library for %BUILDARCH%
CALL "%MYDIR%..\src\platform\windows\build-lib.cmd" %BUILDARCH% %BUILDTYPE% %VSVERSION% "%INSTALLPATH%"
IF %errorlevel% neq 0 EXIT /b %errorlevel%
RMDIR /s /q "%BUILDTARGET%" >nul 2>&1

rem Compile libCEC
ECHO. * compiling libCEC for %BUILDARCH%
CALL "%MYDIR%..\support\windows\cmake\generate.cmd" %BUILDARCH% %GENTYPE% "%MYDIR%.." "%BUILDTARGET%" "%TARGET%" %BUILDTYPE% %VSVERSION%
IF %errorlevel% neq 0 EXIT /b %errorlevel%

IF "%GENTYPE%" == "nmake" (
  CALL "%MYDIR%..\support\windows\cmake\build.cmd" %BUILDARCH% "%BUILDTARGET%" %VSVERSION%
  IF NOT EXIST "%TARGET%\cec.dll" (
    ECHO. *** failed to build %TARGET%\cec.dll for %BUILDARCH% ***
    EXIT /b 1
  )
  ECHO. * libCEC for %BUILDARCH% built successfully
)

EXIT /b 0

:missingparams
ECHO.%~dp0 requires 5 parameters
ECHO.  %~dp0 [architecture] [type] [vs version] [install path] [project type]
ECHO.
ECHO. architecture:    amd64 x86
ECHO. build type:      Release Debug
ECHO. vs version:      Visual Studio version (2019)
ECHO. install path:    installation path without quotes
ECHO. project type:    nmake vs
EXIT /b 99
