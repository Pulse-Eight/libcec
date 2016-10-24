@ECHO OFF

rem Build libCEC for Windows

SETLOCAL

SET MYDIR=%~dp0
SET BUILDARCH=%1
SET BUILDTYPE=%2
SET VSVERSION=%3
SET INSTALLPATH=%4
SET GENTYPE=%5
IF [%5] == [] GOTO missingparams

SET BUILDTARGET=%INSTALLPATH%\cmake\%BUILDARCH%
SET TARGET=%INSTALLPATH%\%BUILDARCH%

IF NOT EXIST "%MYDIR%..\src\platform\windows\build.cmd" (
  ECHO "platform git submodule was not checked out"
  GOTO exit
)

ECHO Build platform library for %BUILDARCH%
CALL %MYDIR%..\src\platform\windows\build-lib.cmd %BUILDARCH% %BUILDTYPE% %VSVERSION% %INSTALLPATH%
del /s /f /q %BUILDTARGET%

ECHO Build libCEC for %BUILDARCH%
CALL %MYDIR%..\support\windows\cmake\generate.cmd %BUILDARCH% %GENTYPE% %MYDIR%..\ %BUILDTARGET% %TARGET% %BUILDTYPE% %VSVERSION%
IF "%GENTYPE%" == "nmake" (
  CALL %MYDIR%..\support\windows\cmake\build.cmd %BUILDARCH% %BUILDTARGET% %VSVERSION%
  IF NOT EXIST "%TARGET%\cec.dll" (
    echo "Failed to build %TARGET%\cec.dll"
    exit /b 1
  )
)

exit /b 0

:missingparams
ECHO "build-lib.cmd requires 4 parameters"

:exit
exit 1