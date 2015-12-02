@ECHO OFF

SETLOCAL

rem set paths
SET BASEDIR=%CD%\..
SET BUILDBASEDIR=%BASEDIR%\cmake-build
SET SCRIPTSDIR=%BASEDIR%\cmake
SET PROJECT_DIR=%3
SET BUILDDIR=%4
SET BUILDTYPE=%5
SET TOOLCHAIN32=""
SET TOOLCHAIN64=""

IF "%6" == "14" (
  SET TOOLCHAIN32="%VS140COMNTOOLS%..\..\VC\bin\vcvars32.bat"
  SET TOOLCHAIN64="%VS140COMNTOOLS%..\..\VC\bin\amd64\vcvars64.bat"
  SET TOOLCHAIN_NAME=Visual Studio 14 2015
)
IF "%6" == "12" (
  SET TOOLCHAIN32="%VS120COMNTOOLS%..\..\VC\bin\vcvars32.bat"
  SET TOOLCHAIN64="%VS120COMNTOOLS%..\..\VC\bin\amd64\vcvars64.bat"
  SET TOOLCHAIN_NAME=Visual Studio 12 2015
)
IF %TOOLCHAIN32% == "" (
  echo Toolchain not set
  GOTO END
)

rem set Visual C++ build environment
IF "%1" == "amd64" (
  echo Generating for win64
  call %TOOLCHAIN64%
  SET CMWAKE_WIN64=^-DWIN64^=1
  SET INSTALLDIR=%BASEDIR%\build\x64
)
IF NOT "%1" == "amd64" (
  echo Generating for win32
  call %TOOLCHAIN32%
  SET CMWAKE_WIN64=^-DWIN32^=1
  SET INSTALLDIR=%BASEDIR%\build
)

SET GEN_PROJECT_TYPE="NMake Makefiles"
IF "%2" == "vs" (
  SET GEN_PROJECT_TYPE="%TOOLCHAIN_NAME%"
  IF "%1" == "amd64" (
    SET GEN_PROJECT_TYPE="%TOOLCHAIN_NAME% Win64"
  )
)

SET GEN_SHARED_LIBS=^-DBUILD_SHARED_LIBS^=1
if "%7" == "static" (
  SET GEN_SHARED_LIBS=^-DBUILD_SHARED_LIBS^=0
)

rem create the build directories
IF NOT EXIST "%BUILDBASEDIR%" MKDIR "%BUILDBASEDIR%"
IF NOT EXIST "%INSTALLDIR%" MKDIR "%INSTALLDIR%"
IF NOT EXIST "%BUILDDIR%" MKDIR "%BUILDDIR%"

echo Generating project files for %GEN_PROJECT_TYPE% from %PROJECT_DIR% in %BUILDDIR%

rem go into the build directory
CD "%BUILDDIR%"
rem execute cmake to generate makefiles processable by nmake
cmake %PROJECT_DIR% -G %GEN_PROJECT_TYPE% ^
      -DCMAKE_BUILD_TYPE=%BUILDTYPE% ^
      -DCMAKE_USER_MAKE_RULES_OVERRIDE="%SCRIPTSDIR%\windows\c-flag-overrides.cmake" ^
      -DCMAKE_USER_MAKE_RULES_OVERRIDE_CXX="%SCRIPTSDIR%\windows\cxx-flag-overrides.cmake" ^
      -DCMAKE_INSTALL_PREFIX=%INSTALLDIR% ^
      %GEN_SHARED_LIBS% ^
      %CMWAKE_WIN64%"

:END
