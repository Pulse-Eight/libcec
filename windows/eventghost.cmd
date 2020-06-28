@ECHO OFF

SETLOCAL

SET MYDIR=%~dp0
SET EXITCODE=0

ECHO. * creating EventGhost plugin
SET EGSOURCES=%MYDIR%..\build\EventGhost\egplugin_sources
RMDIR /s /q "%MYDIR%..\build\EventGhost" >nul 2>&1
MKDIR "%MYDIR%..\build\EventGhost"

XCOPY /E /I "%MYDIR%..\src\eventghost\egplugin_sources" "%EGSOURCES%" >nul

COPY "%MYDIR%..\build\x86\python\cec\_cec.pyd" "%EGSOURCES%\PulseEight" >nul
COPY "%MYDIR%..\build\x86\cec.dll" "%EGSOURCES%\PulseEight" >nul

COPY "%MYDIR%..\build\x86\python\cec\__init__.py" "%EGSOURCES%\PulseEight\cec" >nul

PowerShell -ExecutionPolicy ByPass -Command "Add-Type -Assembly System.IO.Compression.FileSystem;[System.IO.Compression.ZipFile]::CreateFromDirectory('%EGSOURCES%\', '%EGSOURCES%\..\pulse_eight.egplugin', [System.IO.Compression.CompressionLevel]::Optimal, $false)"

DEL /q /f "%EGSOURCES%" >nul 2>&1

IF NOT EXIST "%EGSOURCES%\..\pulse_eight.egplugin" (
  ECHO. *** failed to create EventGhost plugin ***
  SET EXITCODE=1
  GOTO EXIT
)

exit /b %EXITCODE%
