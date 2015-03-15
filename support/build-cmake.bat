@ECHO OFF

SETLOCAL

SET EXITCODE=0

rem set paths
SET BASEDIR=%CD%\..
SET BUILDBASEDIR=%BASEDIR%\cmake-build
SET SCRIPTSDIR=%BASEDIR%\cmake

rem set Visual C++ build environment
IF "%1" == "amd64" (
  echo Compiling for win64
  call "%VS120COMNTOOLS%..\..\VC\bin\amd64\vcvars64.bat"
  SET CMWAKE_WIN64=^-DWIN64^=1
  SET INSTALLDIR=%BASEDIR%\build\x64
  SET BUILDDIR=%BUILDBASEDIR%\build64
  SET BUILDPLATFORMDIR=%BUILDBASEDIR%\buildplatform64
)
IF NOT "%1" == "amd64" (
  echo Compiling for win32
  call "%VS120COMNTOOLS%..\..\VC\bin\vcvars32.bat"
  SET CMWAKE_WIN64=^-DWIN32^=1
  SET INSTALLDIR=%BASEDIR%\build
  SET BUILDDIR=%BUILDBASEDIR%\build
  SET BUILDPLATFORMDIR=%BUILDBASEDIR%\buildplatform
)

rem create the build directories
IF NOT EXIST "%BUILDBASEDIR%" MKDIR "%BUILDBASEDIR%"
IF NOT EXIST "%INSTALLDIR%" MKDIR "%INSTALLDIR%"
IF NOT EXIST "%BUILDDIR%" MKDIR "%BUILDDIR%"
IF NOT EXIST "%BUILDPLATFORMDIR%" MKDIR "%BUILDPLATFORMDIR%"

rem go into the build directory
CD "%BUILDPLATFORMDIR%"
rem execute cmake to generate makefiles processable by nmake
cmake "%BASEDIR%\src\platform" -G "NMake Makefiles" ^
      -DCMAKE_BUILD_TYPE=Release ^
      -DCMAKE_USER_MAKE_RULES_OVERRIDE="%SCRIPTSDIR%\windows\c-flag-overrides.cmake" ^
      -DCMAKE_USER_MAKE_RULES_OVERRIDE_CXX="%SCRIPTSDIR%\windows\cxx-flag-overrides.cmake" ^
      -DCMAKE_INSTALL_PREFIX=%INSTALLDIR% ^
      %CMWAKE_WIN64%"
rem execute nmake to build platform
nmake install
IF ERRORLEVEL 1 (
  ECHO nmake error level: %ERRORLEVEL%
  GOTO ERROR
)

rem go into the build directory
CD "%BUILDDIR%"
rem execute cmake to generate makefiles processable by nmake
cmake "%BASEDIR%" -G "NMake Makefiles" ^
      -DCMAKE_BUILD_TYPE=Release ^
      -DBUILD_SHARED_LIBS=1 ^
      -DCMAKE_USER_MAKE_RULES_OVERRIDE="%SCRIPTSDIR%\windows\c-flag-overrides.cmake" ^
      -DCMAKE_USER_MAKE_RULES_OVERRIDE_CXX="%SCRIPTSDIR%\windows\cxx-flag-overrides.cmake" ^
      -DCMAKE_INSTALL_PREFIX=%INSTALLDIR% ^
      %CMWAKE_WIN64%"
rem execute nmake to build libCEC
nmake install
IF ERRORLEVEL 1 (
  ECHO cmake error level: %ERRORLEVEL%
  GOTO ERROR
)

rem everything was fine
GOTO END

:ERROR
rem something went wrong
ECHO Failed to build libCEC
SET EXITCODE=1

:END
rem go back to the original directory
cd %BASEDIR%

rem exit the script with the defined exitcode
EXIT /B %EXITCODE%