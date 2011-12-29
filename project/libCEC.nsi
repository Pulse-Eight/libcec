;libCEC installer
;Copyright (C) 2011 Pulse-Eight Ltd.
;http://www.pulse-eight.com/

!include "MUI2.nsh"
!include "nsDialogs.nsh"
!include "LogicLib.nsh"
!include "x64.nsh"

Name "libCEC"
OutFile "libCEC-installer.exe"

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
  SetOutPath "$INSTDIR"
  File "..\AUTHORS"
  File "..\cec-client.exe"
  File "..\ChangeLog"
  File "..\COPYING"
  File "..\libcec.dll"
  File "..\libcec.lib"
  File "Release\LibCecSharp.dll"
  File "..\pthreadVC2.dll"
  File "..\README"

  ; Copy to XBMC\system
  ReadRegStr $1 HKCU "Software\XBMC" ""
  ${If} $1 != ""
    SetOutPath "$1\system"
	File "..\libcec.dll"
  ${EndIf}

  SetOutPath "$INSTDIR\driver"
  File "..\dpinst-amd64.exe"
  File "..\dpinst-x86.exe"
  File "..\OEM001.inf"
  SetOutPath "$INSTDIR\include"
  File /r /x *.so "..\include\cec*.*"

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
  Delete "$INSTDIR\cec-client.exe"
  Delete "$INSTDIR\ChangeLog"
  Delete "$INSTDIR\COPYING"
  Delete "$INSTDIR\libcec.dll"
  Delete "$INSTDIR\libcec.lib"
  Delete "$INSTDIR\libcec.pdb"
  Delete "$INSTDIR\LibCecSharp.dll"
  Delete "$INSTDIR\pthreadVC2.dll"
  Delete "$INSTDIR\README"
  Delete "$INSTDIR\driver\OEM001.inf"
  Delete "$INSTDIR\driver\dpinst-amd64.exe"
  Delete "$INSTDIR\driver\dpinst-x86.exe"

  RMDir /r "$INSTDIR\include"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir /r "$INSTDIR"
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  Delete "$SMPROGRAMS\$StartMenuFolder\CEC Test client.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall libCEC.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Visit Pulse-Eight.url"
  RMDir "$SMPROGRAMS\$StartMenuFolder"  

  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\libCEC"

  DeleteRegKey /ifempty HKCU "Software\libCEC"

SectionEnd
