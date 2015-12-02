@ECHO OFF

SETLOCAL

rem set paths
SET BASEDIR=%CD%\..
SET BUILDDIR=%2
SET TOOLCHAIN32=""
SET TOOLCHAIN64=""
SET TOOLCHAINARM=""

IF "%3" == "14" (
  SET TOOLCHAIN32="%VS140COMNTOOLS%..\..\VC\bin\vcvars32.bat"
  SET TOOLCHAIN64="%VS140COMNTOOLS%..\..\VC\bin\amd64\vcvars64.bat"
  SET TOOLCHAINARM="%VS140COMNTOOLS%..\..\VC\bin\x86_arm\vcvarsx86_arm.bat"
  SET TOOLCHAIN_NAME=Visual Studio 14 2015
)
IF "%3" == "12" (
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
  echo Compiling for win64 using %TOOLCHAIN_NAME%
  call %TOOLCHAIN64%
) ELSE (
  IF "%1" == "arm" (
    echo Compiling for ARM using %TOOLCHAIN_NAME%
    call %TOOLCHAINARM%
  ) ELSE (
    echo Compiling for win32 using %TOOLCHAIN_NAME%
    call %TOOLCHAIN32%
  )
)

rem go into the build directory
CD "%BUILDDIR%"
nmake install

:END