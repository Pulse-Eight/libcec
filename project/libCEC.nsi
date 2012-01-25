;libCEC installer
;Copyright (C) 2011 Pulse-Eight Ltd.
;http://www.pulse-eight.com/

!include "MUI2.nsh"
!include "nsDialogs.nsh"
!include "LogicLib.nsh"
!include "x64.nsh"

Name "libCEC"
OutFile "..\build\libCEC-installer.exe"

XPStyle on
InstallDir "$PROGRAMFILES\libCEC"
InstallDirRegKey HKCU "Software\libCEC" ""
RequestExecutionLevel admin
Var StartMenuFolder

!define MUI_FINISHPAGE_LINK "Visit http://www.pulse-eight.com/ for more information."
!define MUI_FINISHPAGE_LINK_LOCATION "http://www.pulse-eight.com/"
!define MUI_ABORTWARNING  

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\COPYING"
!insertmacro MUI_PAGE_DIRECTORY

!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\libCEC" 
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder  

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

InstType "Full"

Section "libCEC" SecLibCEC
  SetShellVarContext current
  SectionIn RO
  SectionIn 1 #section is in installtype Full

  ; Copy to the installation directory
  SetOutPath "$INSTDIR"
  File "..\AUTHORS"
  File /x dpinst*.exe "..\build\*.exe"
  File "..\ChangeLog"
  File "..\COPYING"
  File "..\README"
  File "..\build\*.dll"
  SetOutPath "$INSTDIR\x64"
  File /nonfatal "..\build\x64\*"

  ; Copy to XBMC\system
  ReadRegStr $1 HKCU "Software\XBMC" ""
  ${If} $1 != ""
    SetOutPath "$1\system"
	File "..\build\libcec.dll"
  ${EndIf}

  ; Copy the driver installer and .inf file
  SetOutPath "$INSTDIR\driver"
  File "..\build\dpinst-amd64.exe"
  File "..\build\dpinst-x86.exe"
  File "..\OEM001.inf"

  ; Copy the headers
  SetOutPath "$INSTDIR\include"
  File /r /x *.so "..\include\cec*.*"

  ; Copy libcec.dll and libcec.x64.dll to the system directory
  SetOutPath "$SYSDIR"
  File "..\build\libcec.dll"
  ${If} ${RunningX64}
    File /nonfatal "..\build\x64\libcec.x64.dll"
  ${EndIf}

  ;Store installation folder
  WriteRegStr HKCU "Software\libCEC" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  SetOutPath "$INSTDIR"

  CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\CEC Test client.lnk" "$INSTDIR\cec-client.exe" \
    "" "$INSTDIR\cec-client.exe" 0 SW_SHOWNORMAL \
    "" "Start the CEC Test client."
  ${If} ${RunningX64}
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\CEC Test client (x64).lnk" "$INSTDIR\x64\cec-client.x64.exe" \
      "" "$INSTDIR\cec-client.x64.exe" 0 SW_SHOWNORMAL \
      "" "Start the CEC Test client (x64)."
  ${EndIf}
  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall libCEC.lnk" "$INSTDIR\Uninstall.exe" \
    "" "$INSTDIR\Uninstall.exe" 0 SW_SHOWNORMAL \
    "" "Uninstall libCEC."

  WriteINIStr "$SMPROGRAMS\$StartMenuFolder\Visit Pulse-Eight.url" "InternetShortcut" "URL" "http://www.pulse-eight.com/"
  !insertmacro MUI_STARTMENU_WRITE_END  
  
  
  ;add entry to add/remove programs
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\libCEC" \
                 "DisplayName" "libCEC"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\libCEC" \
                 "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\libCEC" \
                 "NoModify" 1
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\libCEC" \
                 "NoRepair" 1
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\libCEC" \
                 "InstallLocation" "$INSTDIR"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\libCEC" \
                 "DisplayIcon" "$INSTDIR\cec-client.exe,0"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\libCEC" \
                 "Publisher" "Pulse-Eight Ltd."
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\libCEC" \
                 "HelpLink" "http://www.pulse-eight.com/"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\libCEC" \
                 "URLInfoAbout" "http://www.pulse-eight.com"

  ;install driver
  ${If} ${RunningX64}
	ExecWait '"$INSTDIR\driver\dpinst-amd64.exe" /lm /sa /sw /PATH "$INSTDIR\driver"'
  ${Else}
	ExecWait '"$INSTDIR\driver\dpinst-x86.exe" /lm /sa /sw /PATH "$INSTDIR\driver"'
  ${EndIf}
SectionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  SetShellVarContext current

  ${If} ${RunningX64}
	ExecWait '"$INSTDIR\driver\dpinst-amd64.exe" /u "$INSTDIR\driver\OEM001.inf"'
  ${Else}
	ExecWait '"$INSTDIR\driver\dpinst-x64.exe" /u "$INSTDIR\driver\OEM001.inf"'
  ${EndIf}
  Delete "$INSTDIR\AUTHORS"
  Delete "$INSTDIR\cec*.exe"
  Delete "$INSTDIR\ChangeLog"
  Delete "$INSTDIR\COPYING"
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\*.lib"
  Delete "$INSTDIR\x64\*.dll"
  Delete "$INSTDIR\x64\*.lib"
  Delete "$INSTDIR\README"
  Delete "$INSTDIR\driver\OEM001.inf"
  Delete "$INSTDIR\driver\dpinst-amd64.exe"
  Delete "$INSTDIR\driver\dpinst-x86.exe"

  RMDir /r "$INSTDIR\include"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir /r "$INSTDIR"
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  Delete "$SMPROGRAMS\$StartMenuFolder\CEC Test client.lnk"
  ${If} ${RunningX64}
    Delete "$SMPROGRAMS\$StartMenuFolder\CEC Test client (x64).lnk"
  ${EndIf}
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall libCEC.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Visit Pulse-Eight.url"
  RMDir "$SMPROGRAMS\$StartMenuFolder"  

  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\libCEC"

  DeleteRegKey /ifempty HKCU "Software\libCEC"

SectionEnd
