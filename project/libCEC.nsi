;libCEC installer
;Copyright (C) 2011-2016 Pulse-Eight Ltd.
;http://www.pulse-eight.com/

!include "MUI2.nsh"
!include "nsDialogs.nsh"
!include "LogicLib.nsh"
!include "x64.nsh"
!include "libCEC-version.nsh"

XPStyle on
InstallDir "$PROGRAMFILES\Pulse-Eight\USB-CEC Adapter"
InstallDirRegKey HKLM "Software\Pulse-Eight\USB-CEC Adapter software" ""
RequestExecutionLevel admin
Var StartMenuFolder
Var VSRedistSetupError
Var VSRedistInstalledX64
Var VSRedistInstalledX86
Var EventGhostLocation

!define MUI_FINISHPAGE_LINK "Visit http://libcec.pulse-eight.com/ for more information."
!define MUI_FINISHPAGE_LINK_LOCATION "http://libcec.pulse-eight.com/"
!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\COPYING"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY

!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Pulse-Eight\USB-CEC Adapter sofware"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

InstType "Full installation"
InstType "USB-CEC Driver & libCEC"
InstType "USB-CEC Driver Only"

Section "USB-CEC Driver" SecDriver
  SetShellVarContext current
  SectionIn RO
  SectionIn 1 2 3

  ; Renamed to cec.dll
  Delete "$INSTDIR\libcec.dll"
  ${If} ${RunningX64}
    Delete "$INSTDIR\x64\libcec.dll"
  ${EndIf}

  ; Copy to the installation directory
  SetOutPath "$INSTDIR"
  File "..\AUTHORS"
  File "..\COPYING"

  ; Copy the driver installer
  SetOutPath "$INSTDIR\driver"
  File "..\build\p8-usbcec-driver-installer.exe"

  ;Store installation folder
  WriteRegStr HKLM "Software\Pulse-Eight\USB-CEC Adapter software" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  SetOutPath "$INSTDIR"

  CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall Pulse-Eight USB-CEC Adapter software.lnk" "$INSTDIR\Uninstall.exe" \
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

  ;install driver
  ExecWait '"$INSTDIR\driver\p8-usbcec-driver-installer.exe" /S'
  Delete "$INSTDIR\driver\p8-usbcec-driver-installer.exe"
SectionEnd

Section "libCEC" SecLibCec
  SetShellVarContext current
  SectionIn 1 2

  ; Copy to the installation directory
  SetOutPath "$INSTDIR"
  File "..\ChangeLog"
  File "..\README.md"
  File "..\build\x86\*.dll"
  File "..\build\x86\*.xml"
  SetOutPath "$INSTDIR\x64"
  File /nonfatal "..\build\amd64\*.dll"
  File /nonfatal "..\build\amd64\*.xml"

  ; Copy to Kodi\system
  ReadRegStr $1 HKCU "Software\Kodi" ""
  ${If} $1 != ""
    SetOutPath "$1\system"
	File "..\build\x86\libcec.dll"
  ${EndIf}

  ; Copy the headers
  SetOutPath "$INSTDIR\include"
  File /r /x *.so "..\build\x86\include\libcec\cec*.*"
  File /r /x *.so "..\build\x86\include\libcec\version.h"
SectionEnd

Section "CEC Debug Client" SecCecClient
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

Section "libCEC Tray" SecDotNet
  SetShellVarContext current
  SectionIn 1 2

  ; Copy to the installation directory
  SetOutPath "$INSTDIR"
  File "..\build\x86\CecSharpTester.exe"
  File "..\build\x86\cec-tray.exe"
  SetOutPath "$INSTDIR\x64"
  File /nonfatal "..\build\amd64\CecSharpTester.exe"
  File /nonfatal "..\build\amd64\cec-tray.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  SetOutPath "$INSTDIR"

  CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
  ${If} ${RunningX64}
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\cec-tray.lnk" "$INSTDIR\x64\cec-tray.exe" \
      "" "$INSTDIR\x64\cec-tray.exe" 0 SW_SHOWNORMAL \
      "" "Start libCEC Tray (x64)."
  ${Else}
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\cec-tray.lnk" "$INSTDIR\cec-tray.exe" \
      "" "$INSTDIR\cec-tray.exe" 0 SW_SHOWNORMAL \
      "" "Start libCEC Tray."
  ${EndIf}
  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

Section "Python bindings" SecPythonCec
  SetShellVarContext current
  SectionIn 1 2

  ; Copy to the installation directory
  SetOutPath "$INSTDIR\python"
  File "..\build\x86\python\pyCecClient.py"
  SetOutPath "$INSTDIR\python\cec"
  File "..\build\x86\python\cec\_cec.pyd"
  File "..\build\x86\python\cec\__init__.py"
SectionEnd

; Function used to get the parent directory of the installer
Function GetParentDirectory

  Exch $R0
  Push $R1
  Push $R2
  Push $R3

  StrCpy $R1 0
  StrLen $R2 $R0

  loop:
    IntOp $R1 $R1 + 1
    IntCmp $R1 $R2 get 0 get
    StrCpy $R3 $R0 1 -$R1
    StrCmp $R3 "\" get
  Goto loop

  get:
    StrCpy $R0 $R0 -$R1

    Pop $R3
    Pop $R2
    Pop $R1
    Exch $R0

FunctionEnd

!define EVENTGHOST_SECTIONNAME "EventGhost plugin"
Section "" SecEvGhostCec
  SetShellVarContext current
  SectionIn 1 2

  ${If} $EventGhostLocation != ""
    ; We get the directory of the installer then pass it to GetParentDirectory
    ; which we then append the path to the plugin file to the returned value
    ; This is done because EventGhost needs to see the full path to the plugin
    ; file.
    Push $EXEDIR
    Call GetParentDirectory
    Pop $R0
    ExecWait '"$EventGhostLocation\eventghost.exe" $R0\src\EventGhost\pulse_eight.egplugin'
  ${EndIf}
SectionEnd

!define REDISTRIBUTABLE_X86_SECTIONNAME "Microsoft Visual C++ 2015 Redistributable Package (x86)"
Section "" SecVCRedistX86
  SetShellVarContext current
  SectionIn 1 2 3

  SetOutPath "$TEMP\vc2015_x86"

  ${If} $VSRedistInstalledX86 != "Yes"
    NSISdl::download https://download.microsoft.com/download/6/D/F/6DF3FF94-F7F9-4F0B-838C-A328D1A7D0EE/vc_redist.x86.exe vc_redist.x86.exe
    ExecWait '"$TEMP\vc2015_x86\vc_redist.x86.exe" /q' $VSRedistSetupError
  ${Endif}

  RMDIR /r "$TEMP\vc2015_x86"
SectionEnd

!define REDISTRIBUTABLE_X64_SECTIONNAME "Microsoft Visual C++ 2015 Redistributable Package (x64)"
Section "" SecVCRedistX64
  SetShellVarContext current
  SectionIn 1 2 3

  SetOutPath "$TEMP\vc2015_x64"

  ${If} $VSRedistInstalledX64 != "Yes"
    NSISdl::download https://download.microsoft.com/download/6/D/F/6DF3FF94-F7F9-4F0B-838C-A328D1A7D0EE/vc_redist.x64.exe vc_redist.x64.exe
    ExecWait '"$TEMP\vc2015_x64\vc_redist.x64.exe" /q' $VSRedistSetupError
  ${Endif}

  RMDIR /r "$TEMP\vc2015_x64"
SectionEnd

Function .onInit
  ; check for vc2015 x86 redist
  ReadRegDword $1 HKLM "SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{462f63a8-6347-4894-a1b3-dbfe3a4c981d}" "BundleVersion"
  ${If} $1 != ""
    StrCpy $VSRedistInstalledX86 "Yes"
  ${Endif}

  ${If} $VSRedistInstalledX86 == "Yes"
    !insertMacro UnSelectSection ${SecVCRedistX86}
    SectionSetText ${SecVCRedistX86} ""
  ${Else}
    !insertMacro SelectSection ${SecVCRedistX86}
    SectionSetText ${SecVCRedistX86} "${REDISTRIBUTABLE_X86_SECTIONNAME}"
  ${Endif}

  ${If} ${RunningX64}
    ; check for vc2015 x64 redist
    ReadRegDword $1 HKLM "SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{323dad84-0974-4d90-a1c1-e006c7fdbb7d}" "BundleVersion"
    ${If} $1 != ""
      StrCpy $VSRedistInstalledX64 "Yes"
    ${Endif}

    ${If} $VSRedistInstalledX64 == "Yes"
      !insertMacro UnSelectSection ${SecVCRedistX64}
      SectionSetText ${SecVCRedistX64} ""
    ${Else}
      !insertMacro SelectSection ${SecVCRedistX64}
      SectionSetText ${SecVCRedistX64} "${REDISTRIBUTABLE_X64_SECTIONNAME}"
    ${Endif}
  ${Else}
    !insertMacro UnSelectSection ${SecVCRedistX64}
    SectionSetText ${SecVCRedistX64} ""
  ${Endif}

  ; check for EventGhost
  ReadRegDword $1 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\EventGhost_is1" "InstallLocation"
  ${If} $1 != ""
    StrCpy $EventGhostLocation "$1"
    !insertMacro SelectSection ${SecEvGhostCec}
    SectionSetText ${SecEvGhostCec} "${EVENTGHOST_SECTIONNAME}"
  ${Else}
    !insertMacro UnSelectSection ${SecEvGhostCec}
    SectionSetText ${SecEvGhostCec} ""
    MessageBox MB_OK \
      "EventGhost was not found, so the plugin for EventGhost will not be installed. You can download EventGhost from http://www.eventghost.org/"
  ${Endif}

FunctionEnd

;--------------------------------
;Uninstaller Section
Section "Uninstall"

  SetShellVarContext current

  Delete "$INSTDIR\AUTHORS"
  Delete "$INSTDIR\*.exe"
  Delete "$INSTDIR\ChangeLog"
  Delete "$INSTDIR\COPYING"
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\*.lib"
  Delete "$INSTDIR\*.xml"
  Delete "$INSTDIR\x64\*.dll"
  Delete "$INSTDIR\x64\*.lib"
  Delete "$INSTDIR\x64\*.exe"
  Delete "$INSTDIR\x64\*.xml"
  Delete "$INSTDIR\README.md"
  Delete "$SYSDIR\libcec.dll"
  ${If} ${RunningX64}
    Delete "$SYSDIR\libcec.x64.dll"
  ${EndIf}

  ; Uninstall EventGhost plugin
  ; Eventghost has no uninstall plugin feature so we sinply delete the plugin
  ; from the directory.
  ReadRegDword $1 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\EventGhost_is1" "InstallLocation"
  ${If} $1 != ""
    RMDir /r "$%PROGRAMDATA%\EventGhost\plugins\PulseEight"
  ${Endif}

  ; Uninstall the driver
  ReadRegStr $1 HKLM "Software\Pulse-Eight\USB-CEC Adapter driver" ""
  ${If} $1 != ""
    ExecWait '"$1\Uninstall.exe" /S _?=$1'
  ${EndIf}

  RMDir /r "$INSTDIR\include"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir /r "$INSTDIR"
  RMDir "$PROGRAMFILES\Pulse-Eight"

  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  Delete "$SMPROGRAMS\$StartMenuFolder\libCEC Tray.lnk"
  ${If} ${RunningX64}
    Delete "$SMPROGRAMS\$StartMenuFolder\libCEC Tray (x64).lnk"
  ${EndIf}
  Delete "$SMPROGRAMS\$StartMenuFolder\cec-tray.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\CEC Test client.lnk"
  ${If} ${RunningX64}
    Delete "$SMPROGRAMS\$StartMenuFolder\CEC Test client (x64).lnk"
  ${EndIf}
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall Pulse-Eight USB-CEC Adapter software.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Visit Pulse-Eight.url"
  RMDir "$SMPROGRAMS\$StartMenuFolder"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter driver"
  DeleteRegKey /ifempty HKLM "Software\Pulse-Eight\USB-CEC Adapter software"
  DeleteRegKey /ifempty HKLM "Software\Pulse-Eight"
SectionEnd
