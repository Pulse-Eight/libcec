@ECHO OFF

rem Generate Visual Studio projects for libCEC

SETLOCAL

SET MYDIR=%~dp0
SET BUILDTYPE=Debug
SET VSVERSION=14
SET INSTALLPATH=%MYDIR%..\build

IF NOT EXIST "%MYDIR%..\support\windows\cmake\build.cmd" (
  ECHO "support git submodule was not checked out"
  GOTO exit
)

IF NOT EXIST "%MYDIR%..\src\platform\windows\build.cmd" (
  ECHO "platform git submodule was not checked out"
  GOTO exit
)

del /s /f /q %MYDIR%..\build

FOR %%T IN (amd64 x86) DO (
  CALL %MYDIR%build-lib.cmd %%T %BUILDTYPE% %VSVERSION% %INSTALLPATH% vs
)

ECHO Visual Studio solutions can be found in:
ECHO 32 bits: %MYDIR%..\build\cmake\x86\libcec.sln
ECHO 64 bits: %MYDIR%..\build\cmake\amd64\libcec.sln
ECHO.
ECHO These projects only compile in %BUILDTYPE% mode

:exit
