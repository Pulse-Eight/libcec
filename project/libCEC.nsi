;libCEC installer
;Copyright (C) 2011-2020 Pulse-Eight Ltd.
;http://www.pulse-eight.com/

Var StartMenuFolder
Var VSRedistSetupError
Var VSRedistInstalledX64
Var VSRedistInstalledX86
Var EventGhostLocation

!include "MUI2.nsh"
!include "nsDialogs.nsh"
!include "LogicLib.nsh"
!include "x64.nsh"
!include "nsis\libcec-version.nsh"
!include "nsis\functions.nsh"

XPStyle on
RequestExecutionLevel admin

Name "Pulse-Eight libCEC v${LIBCEC_VERSION_STRING}"


!ifdef INNER
  ; only generate a temporary installer so we can sign the uninstaller if INNER is defined
  OutFile "$%TEMP%\libcec_temp_installer.exe"
  SetCompress off
!else
  ; create the uninstaller first
  !makensis '/V1 /DINNER "${__FILE__}"' = 0
  !system 'set __COMPAT_LAYER=RunAsInvoker&"$%TEMP%\libcec_temp_installer.exe"' = 2
  ; sign the uninstaller if the signtool is present
  ${!defineifexist} SIGNTOOL_EXISTS ..\support\private\sign-binary.cmd
  !ifdef SIGNTOOL_EXISTS
    !echo "Signing uninstaller binary"
    !system "..\support\private\sign-binary.cmd $%TEMP%\uninstall_libcec.exe" = 0
    !undef SIGNTOOL_EXISTS
  !endif
 
  ; generate a separate installer if pdb files are included, because it more than twice the size
  !ifdef NSISINCLUDEPDB
  OutFile "..\build\libcec-dbg-${LIBCEC_VERSION_STRING}.exe"
  !else
  OutFile "..\build\libcec-${LIBCEC_VERSION_STRING}.exe"
  !endif
  SetCompressor /SOLID lzma
!endif

InstallDir "$PROGRAMFILES\Pulse-Eight\USB-CEC Adapter"
InstallDirRegKey HKLM "Software\Pulse-Eight\USB-CEC Adapter software" ""

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

; installer sections, separate file to declutter a bit
!include "nsis\sections.nsh"

Function .onInit
!ifdef INNER
  ; just write the uninstaller and exit
  SetSilent silent
  WriteUninstaller "$%TEMP%\uninstall_libcec.exe"
  Quit
!else
  ; the actual onInit function

  ; check for vc x86 redist
  ReadRegDword $1 HKLM "SOFTWARE\Wow6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x86" "Version"
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
    ; check for vc x64 redist
    ReadRegDword $1 HKLM "SOFTWARE\Wow6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x64" "Version"
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
  ${Endif}

  ; check for Kodi
  IfFileExists "$PROGRAMFILES32\Kodi\cec.dll" 0 +2
  SectionSetText ${SecLibCecKodi86} "${KODI_X86_SECTIONNAME}"
  ${If} ${RunningX64}
    IfFileExists "$PROGRAMFILES64\Kodi\cec.dll" 0 +2
    SectionSetText ${SecLibCecKodi64} "${KODI_X64_SECTIONNAME}"
  ${Endif}
  !insertMacro UnSelectSection ${SecLibCecKodi86}
  !insertMacro UnSelectSection ${SecLibCecKodi64}
!endif
FunctionEnd

!ifdef INNER
!include "nsis\uninstall.nsh"
!endif
