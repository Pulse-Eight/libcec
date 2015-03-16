@ECHO OFF

SETLOCAL

rem set paths
SET BASEDIR=%CD%\..
SET BUILDBASEDIR=%BASEDIR%\cmake-build
SET BUILDTYPE=%2

IF "%1" == "amd64" (
  call generate-cmake.cmd amd64 nmake "%BASEDIR%\src\platform" "%BUILDBASEDIR%\buildplatform64" %BUILDTYPE% static
  call build-cmake.cmd amd64 "%BUILDBASEDIR%\buildplatform64"
)
IF NOT "%1" == "amd64" (
  call generate-cmake.cmd x86 nmake "%BASEDIR%\src\platform" "%BUILDBASEDIR%\buildplatform" %BUILDTYPE% static
  call build-cmake.cmd x86 "%BUILDBASEDIR%\buildplatform"
)
