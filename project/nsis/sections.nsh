Section "USB-CEC driver" SecDriver
  SetShellVarContext current
  SectionIn RO
  SectionIn 1 2 3
  ; Copy the driver installer
  SetOutPath "$INSTDIR\driver"
  File "..\build\p8-usbcec-driver-installer.exe"
  ;install driver
  ExecWait '"$INSTDIR\driver\p8-usbcec-driver-installer.exe" /S'
SectionEnd

Section "libCEC" SecLibCec
  SetShellVarContext current
  SectionIn 1 2
  SectionIn RO

  ; Renamed to cec.dll
  Delete "$INSTDIR\libcec.dll"
  ${If} ${RunningX64}
    Delete "$INSTDIR\x64\libcec.dll"
  ${EndIf}

  ; Copy to the installation directory
  SetOutPath "$INSTDIR"
  File "..\ChangeLog"
  File "..\README.md"
  File "..\docs\README.developers.md"
  File "..\docs\README.windows.md"
  File "..\build\x86\cec.dll"
  File "..\support\windows\tv_on.cmd"
  File "..\support\windows\tv_off.cmd"
  SetOutPath "$INSTDIR\x64"
  File /nonfatal "..\build\amd64\cec.dll"

  ; Copy the headers
  SetOutPath "$INSTDIR\include"
  File /r /x *.so "..\build\x86\include\libcec\*.h"

  ; Copy to the installation directory
  SetOutPath "$INSTDIR"
  File "..\AUTHORS"
  File "..\COPYING"

  ;Store installation folder
  WriteRegStr HKLM "Software\Pulse-Eight\USB-CEC Adapter software" "" $INSTDIR

  ;Package uninstaller
  !ifndef INNER
    SetOutPath $INSTDIR
    File $%TEMP%\uninstall_libcec.exe
  !endif

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  SetOutPath "$INSTDIR"

  CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall Pulse-Eight USB-CEC Adapter software.lnk" "$INSTDIR\uninstall_libcec.exe" \
    "" "$INSTDIR\Uninstall.exe" 0 SW_SHOWNORMAL \
    "" "Uninstall Pulse-Eight USB-CEC Adapter software."

  WriteINIStr "$SMPROGRAMS\$StartMenuFolder\Visit Pulse-Eight.url" "InternetShortcut" "URL" "http://www.pulse-eight.com/"
  !insertmacro MUI_STARTMENU_WRITE_END

  ;add entry to add/remove programs
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" \
                 "DisplayName" "Pulse-Eight USB-CEC Adapter software"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" \
                 "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" \
                 "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" \
                 "NoRepair" 1
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" \
                 "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" \
                 "DisplayIcon" "$INSTDIR\cec-client.exe,0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" \
                 "Publisher" "Pulse-Eight Limited"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" \
                 "HelpLink" "http://www.pulse-eight.com/"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" \
                 "URLInfoAbout" "http://www.pulse-eight.com"
SectionEnd

Section "libCEC for Python" SecPythonCec
  SetShellVarContext current
  SectionIn 1 2

  ; Copy to the installation directory
  SetOutPath "$INSTDIR\python"
  File "..\build\x86\python\_cec.pyd"
  SetOutPath "$INSTDIR\python\cec"
  File "..\build\x86\python\cec\cec.py"
  File "..\build\x86\python\cec\__init__.py"
SectionEnd

Section "libCEC for .Net Framework" SecDotNet
  SetShellVarContext current
  SectionIn 1 2

  ; Moved to x86\netfx subdir
  RMDIR /R "$INSTDIR\netfx"
  Delete "$INSTDIR\CecSharpTester.exe"
  Delete "$INSTDIR\cec-tray.exe"
  Delete "$INSTDIR\LibCecSharp.dll"
  Delete "$INSTDIR\LibCecSharp.xml"

  ; Copy to the installation directory
  SetOutPath "$INSTDIR\x86\netfx"
  File "..\build\x86\LibCecSharp.dll"
  File "..\build\x86\LibCecSharp.xml"
  File /nonfatal "..\build\x86\CecSharpTester.exe"

  ${If} ${RunningX64}
    ; Moved to netfx subdir
    Delete "$INSTDIR\x64\CecSharpTester.exe"
    Delete "$INSTDIR\x64\cec-tray.exe"
    Delete "$INSTDIR\x64\LibCecSharp.dll"
    Delete "$INSTDIR\x64\LibCecSharp.xml"

    ; Copy to the installation directory
    SetOutPath "$INSTDIR\x64\netfx"
    File /nonfatal "..\build\amd64\CecSharpTester.exe"
    File /nonfatal "..\build\amd64\LibCecSharp.dll"
    File /nonfatal "..\build\amd64\LibCecSharp.xml"
  ${EndIf}
SectionEnd

Section "libCEC for .Net Core" SecDotNetCore
  SetShellVarContext current
  SectionIn 1 2

  ; Moved to x86\netcore subdir
  RMDIR /R "$INSTDIR\netcore"

  ; Copy to the installation directory
  SetOutPath "$INSTDIR\x86\netcore"
  File "..\build\x86\netcore\LibCecSharpCore.deps.json"
  File "..\build\x86\netcore\LibCecSharpCore.dll"
  File "..\build\x86\netcore\LibCecSharpCore.runtimeconfig.json"
  File "..\build\x86\netcore\LibCecSharpCore.xml"
  File /nonfatal "..\build\x86\netcore\CecSharpCoreTester.exe"
  File /nonfatal "..\build\x86\netcore\CecSharpCoreTester.deps.json"
  File /nonfatal "..\build\x86\netcore\CecSharpCoreTester.dll"
  File /nonfatal "..\build\x86\netcore\CecSharpCoreTester.runtimeconfig.json"
  File /nonfatal "..\build\x86\netcore\Ijwhost.dll"

  ${If} ${RunningX64}
    SetOutPath "$INSTDIR\x64\netcore"
    File /nonfatal "..\build\amd64\netcore\LibCecSharpCore.deps.json"
    File /nonfatal "..\build\amd64\netcore\LibCecSharpCore.dll"
    File /nonfatal "..\build\amd64\netcore\LibCecSharpCore.runtimeconfig.json"
    File /nonfatal "..\build\amd64\netcore\LibCecSharpCore.xml"
    File /nonfatal "..\build\amd64\netcore\CecSharpCoreTester.exe"
    File /nonfatal "..\build\amd64\netcore\CecSharpCoreTester.deps.json"
    File /nonfatal "..\build\amd64\netcore\CecSharpCoreTester.dll"
    File /nonfatal "..\build\amd64\netcore\CecSharpCoreTester.runtimeconfig.json"
    File /nonfatal "..\build\amd64\netcore\Ijwhost.dll"
  ${EndIf}
SectionEnd

!ifdef NSISDOTNETAPPS
!include "nsis\cec-tray.nsh"
!endif

Section "libCEC client (cec-client)" SecCecClient
  SetShellVarContext current
  SectionIn 1

  ; Copy to the installation directory
  SetOutPath "$INSTDIR"
  File "..\build\x86\cec-client.exe"
  File "..\build\x86\cecc-client.exe"
  SetOutPath "$INSTDIR\x64"
  File /nonfatal "..\build\amd64\cec-client.exe"
  File /nonfatal "..\build\amd64\cecc-client.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  SetOutPath "$INSTDIR"

  CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
  ${If} ${RunningX64}
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\CEC Test client (x64).lnk" "$INSTDIR\x64\cec-client.exe" \
      "" "$INSTDIR\x64\cec-client.exe" 0 SW_SHOWNORMAL \
      "" "Start the CEC Test client (x64)."
  ${Else}
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\CEC Test client.lnk" "$INSTDIR\cec-client.exe" \
      "" "$INSTDIR\cec-client.exe" 0 SW_SHOWNORMAL \
      "" "Start the CEC Test client."
  ${EndIf}
  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

Section "Python client" SecPythonCecClient
  SetShellVarContext current
  SectionIn 1

  SetOutPath "$INSTDIR\python"
  File "..\build\x86\python\pyCecClient.py"
SectionEnd

Section "Adapter Firmware" SecFwUpgrade
  SetShellVarContext current
  SectionIn 1

  ; Copy the driver installer
  SetOutPath "$INSTDIR\driver"
  File "..\build\p8-usbcec-bootloader-driver-installer.exe"
  ;install driver
  ExecWait '"$INSTDIR\driver\p8-usbcec-bootloader-driver-installer.exe" /S'

  SetOutPath "$INSTDIR"
  NSISdl::download https://p8.opdenkamp.eu/cec/cec-firmware-latest.exe cec-firmware-latest.exe

  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Firmware Upgrade.lnk" "$INSTDIR\cec-firmware-latest.exe" \
    "" "$INSTDIR\cec-firmware-latest.exe" 0 SW_SHOWNORMAL \
    "" "Upgrade the firmware of the CEC adapter to the latest version."
SectionEnd

!define KODI_X86_SECTIONNAME "Kodi integration (x86)"
Section "" SecLibCecKodi86
  SetShellVarContext current
  SectionIn 1

  SetOutPath "$PROGRAMFILES32\Kodi"
  File "..\build\x86\cec.dll"
SectionEnd

!define KODI_X64_SECTIONNAME "Kodi integration (x64)"
Section "" SecLibCecKodi64
  SetShellVarContext current
  SectionIn 1

  SetOutPath "$PROGRAMFILES64\Kodi"
  File "..\build\amd64\cec.dll"
SectionEnd

!define EVENTGHOST_SECTIONNAME "EventGhost plugin"
Section "" SecEvGhostCec
  SetShellVarContext current
  SectionIn 1

  SetOutPath "$INSTDIR\EventGhost"
  File "..\build\EventGhost\pulse_eight.egplugin"

  ${If} $EventGhostLocation != ""
    ExecWait '"$EventGhostLocation\eventghost.exe" "$INSTDIR\EventGhost\pulse_eight.egplugin"'
  ${EndIf}
SectionEnd

!ifdef NSISINCLUDEPDB
!include "nsis\libcec-pdb.nsh"
!endif

!define REDISTRIBUTABLE_X86_SECTIONNAME "Microsoft Visual C++ Redistributable Package (x86)"
Section "" SecVCRedistX86
  SetShellVarContext current
  SectionIn 1 2 3
  SectionIn RO

  SetOutPath "$TEMP\vc_x86"

  ${If} $VSRedistInstalledX86 != "Yes"
    NSISdl::download https://aka.ms/vs/16/release/vc_redist.x86.exe vc_redist.x86.exe
    ExecWait '"$TEMP\vc_x86\vc_redist.x86.exe" /q' $VSRedistSetupError
  ${Endif}

  RMDIR /r "$TEMP\vc_x86"
SectionEnd

!define REDISTRIBUTABLE_X64_SECTIONNAME "Microsoft Visual C++ Redistributable Package (x64)"
Section "" SecVCRedistX64
  SetShellVarContext current
  SectionIn 1 2 3
  SectionIn RO

  SetOutPath "$TEMP\vc_x64"

  ${If} $VSRedistInstalledX64 != "Yes"
    NSISdl::download https://aka.ms/vs/16/release/vc_redist.x64.exe vc_redist.x64.exe
    ExecWait '"$TEMP\vc_x64\vc_redist.x64.exe" /q' $VSRedistSetupError
  ${Endif}

  RMDIR /r "$TEMP\vc_x64"
SectionEnd

; Required options
Function .onSelChange
!ifdef NSISDOTNETAPPS
${If} ${SectionIsSelected} ${SecTray}
    !define /math MYSECTIONFLAGS ${SF_SELECTED} | ${SF_RO}
    !insertmacro SetSectionFlag ${SecDotNet} ${MYSECTIONFLAGS} 
    !undef MYSECTIONFLAGS
${Else}
    !insertmacro ClearSectionFlag ${SecDotNet} ${SF_RO}
${EndIf}
!endif

${If} ${SectionIsSelected} ${SecPythonCecClient}
    !define /math MYSECTIONFLAGS ${SF_SELECTED} | ${SF_RO}
    !insertmacro SetSectionFlag ${SecPythonCec} ${MYSECTIONFLAGS} 
    !undef MYSECTIONFLAGS
${Else}
    !insertmacro ClearSectionFlag ${SecPythonCec} ${SF_RO}
${EndIf}

${If} ${SectionIsSelected} ${SecCecClient}
${OrIf} ${SectionIsSelected} ${SecDotNet}
${OrIf} ${SectionIsSelected} ${SecDotNetCore}
${OrIf} ${SectionIsSelected} ${SecPythonCec}
    !define /math MYSECTIONFLAGS ${SF_SELECTED} | ${SF_RO}
    !insertmacro SetSectionFlag ${SecLibCec} ${MYSECTIONFLAGS} 
    !undef MYSECTIONFLAGS
${Else}
    !insertmacro ClearSectionFlag ${SecLibCec} ${SF_RO}
${EndIf}

FunctionEnd
