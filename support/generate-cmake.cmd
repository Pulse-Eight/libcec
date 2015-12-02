@ECHO OFF

SETLOCAL

rem set paths
SET BASEDIR=%CD%\..
SET BUILDBASEDIR=%BASEDIR%\cmake-build
SET SCRIPTSDIR=%BASEDIR%\cmake
SET BUILDARCH=%1
SET PROJECT_TYPE=%2
SET PROJECT_DIR=%3
SET BUILDDIR=%4
SET BUILDTYPE=%5
SET VSVERSION=%6
SET BUILDSTATIC=%7
SET TOOLCHAIN32=""
SET TOOLCHAIN64=""
SET TOOLCHAINARM=""

echo --------------------------------------
echo Generating cmake project:
echo Architecture = %BUILDARCH%
echo Project type = %PROJECT_TYPE%
echo Project = %PROJECT_DIR%
echo Target = %BUILDDIR%
echo Build type = %BUILDTYPE%
echo Visual Studio version = %VSVERSION%
echo --------------------------------------

IF "%VSVERSION%" == "14" (
  SET TOOLCHAIN32="%VS140COMNTOOLS%..\..\VC\bin\vcvars32.bat"
  SET TOOLCHAIN64="%VS140COMNTOOLS%..\..\VC\bin\amd64\vcvars64.bat"
  SET TOOLCHAINARM="%VS140COMNTOOLS%..\..\VC\bin\x86_arm\vcvarsx86_arm.bat"
  SET TOOLCHAIN_NAME=Visual Studio 14 2015
)
IF "%VSVERSION%" == "12" (
  SET TOOLCHAIN32="%VS120COMNTOOLS%..\..\VC\bin\vcvars32.bat"
  SET TOOLCHAIN64="%VS120COMNTOOLS%..\..\VC\bin\amd64\vcvars64.bat"
  SET TOOLCHAIN_NAME=Visual Studio 12 2013
)
IF %TOOLCHAIN32% == "" (
  echo Toolchain not set: %VSVERSION%
  pause
  GOTO END
)

rem set Visual C++ build environment
IF "%BUILDARCH%" == "amd64" (
  echo Generating for win64 using %TOOLCHAIN_NAME%
  call %TOOLCHAIN64%
  SET CMWAKE_WIN64=^-DWIN64^=1
  SET INSTALLDIR=%BASEDIR%\build\x64
) ELSE (
  IF "%BUILDARCH%" == "arm" (
    echo Generating for ARM using %TOOLCHAIN_NAME%
    call %TOOLCHAINARM%
    SET CMWAKE_WIN64=^-DCMAKE_SYSTEM_NAME^=WindowsStore ^-DCMAKE_SYSTEM_VERSION^=10.0 
    SET INSTALLDIR=%BASEDIR%\build\arm
  ) ELSE (
    echo Generating for win32 using %TOOLCHAIN_NAME%
    call %TOOLCHAIN32%
    SET CMWAKE_WIN64=^-DWIN32^=1
    SET INSTALLDIR=%BASEDIR%\build
  )
)

SET GEN_PROJECT_TYPE="NMake Makefiles"
IF "%PROJECT_TYPE%" == "vs" (
  SET GEN_PROJECT_TYPE="%TOOLCHAIN_NAME%"
  IF "%BUILDARCH%" == "amd64" (
    SET GEN_PROJECT_TYPE="%TOOLCHAIN_NAME% Win64"
  ) ELSE (
    IF "%BUILDARCH%" == "arm" (
      SET GEN_PROJECT_TYPE="%TOOLCHAIN_NAME% ARM"
    )
  )
)

SET GEN_SHARED_LIBS=^-DBUILD_SHARED_LIBS^=1
if "%BUILDSTATIC%" == "static" (
  SET GEN_SHARED_LIBS=^-DBUILD_SHARED_LIBS^=0
)

rem create the build directories
IF NOT EXIST "%BUILDBASEDIR%" MKDIR "%BUILDBASEDIR%"
IF NOT EXIST "%INSTALLDIR%" MKDIR "%INSTALLDIR%"
IF NOT EXIST "%BUILDDIR%" MKDIR "%BUILDDIR%"

echo Generating project files for %GEN_PROJECT_TYPE% from %PROJECT_DIR% in %BUILDDIR%

rem go into the build directory
CD "%BUILDDIR%"

echo "project dir = %PROJECT_DIR%"

rem execute cmake to generate makefiles processable by nmake
cmake %PROJECT_DIR% -G %GEN_PROJECT_TYPE% ^
      -DCMAKE_BUILD_TYPE=%BUILDTYPE% ^
      -DCMAKE_USER_MAKE_RULES_OVERRIDE="%SCRIPTSDIR%\windows\c-flag-overrides.cmake" ^
      -DCMAKE_USER_MAKE_RULES_OVERRIDE_CXX="%SCRIPTSDIR%\windows\cxx-flag-overrides.cmake" ^
      -DCMAKE_INSTALL_PREFIX=%INSTALLDIR% ^
      %GEN_SHARED_LIBS% ^
      %CMWAKE_WIN64%"

:END
