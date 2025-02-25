;libCEC installer
;Copyright (C) 2011-2025 Pulse-Eight Ltd.
;http://www.pulse-eight.com/

Var StartMenuFolder

XPStyle on
RequestExecutionLevel admin
SetCompressor /SOLID lzma

!include "LogicLib.nsh"
!include "MUI2.nsh"
!include "nsDialogs.nsh"
!include "x64.nsh"
!include "nsis\libcec-version.nsh"

Name "Pulse-Eight libCEC v${LIBCEC_VERSION_STRING}"

InstType "Full installation"
InstType "USB-CEC Driver & libCEC"
InstType "USB-CEC Driver Only"

!ifdef NSIS_X86
	!ifdef NSISINCLUDEPDB
		!define BASE_FILENAME "libcec-x86-dbg-${LIBCEC_VERSION_STRING}.exe"
		!define BINARY_SOURCE_DIR "..\build\Debug\x86"
	!else
		!define BASE_FILENAME "libcec-x86-${LIBCEC_VERSION_STRING}.exe"
		!define BINARY_SOURCE_DIR "..\build\Release\x86"
	!endif
	!define BASE_REGKEY "USB-CEC Adapter software (x86)"
	InstallDir "$PROGRAMFILES\Pulse-Eight\USB-CEC Adapter"
	!define MUI_STARTMENUPAGE_DEFAULTFOLDER "Pulse-Eight USB-CEC Adapter (x86)"
!else
	!ifdef NSISINCLUDEPDB
		!define BASE_FILENAME "libcec-x64-dbg-${LIBCEC_VERSION_STRING}.exe"
		!define BINARY_SOURCE_DIR "..\build\Debug\x64"
	!else
		!define BASE_FILENAME "libcec-x64-${LIBCEC_VERSION_STRING}.exe"
		!define BINARY_SOURCE_DIR "..\build\Release\x64"
	!endif
	InstallDir "$PROGRAMFILES64\Pulse-Eight\USB-CEC Adapter"
	!define BASE_REGKEY "USB-CEC Adapter software"
	!define MUI_STARTMENUPAGE_DEFAULTFOLDER "Pulse-Eight USB-CEC Adapter"
!endif

InstallDirRegKey HKLM "Software\Pulse-Eight\${BASE_REGKEY}" ""

!ifdef INNER
	!echo "Building uninstaller binary"
	; only generate a temporary installer so we can sign the uninstaller if INNER is defined
	OutFile "$%TEMP%\libcec_temp_installer.exe"

	!include "nsis\uninstall.nsh"
!else
	!include "nsis\functions.nsh"

	!echo "Creating uninstaller binary"
	; create the uninstaller first
	!makensis '/V1 /DINNER "${__FILE__}"' = 0
	!system 'set __COMPAT_LAYER=RunAsInvoker&"$%TEMP%\libcec_temp_installer.exe"' = 0

	; sign the uninstaller if the signtool is present
	${!defineifexist} SIGNTOOL_EXISTS ..\support\private\sign-binary.cmd
	!ifdef SIGNTOOL_EXISTS
		!echo "Signing uninstaller binary"
		!system "..\support\private\sign-binary.cmd $%TEMP%\uninstall_libcec.exe" = 0
		!undef SIGNTOOL_EXISTS
	!endif
 
	OutFile "..\build\${BASE_FILENAME}"
!endif

!define MUI_FINISHPAGE_LINK "Visit https://libcec.pulse-eight.com/ for more information."
!define MUI_FINISHPAGE_LINK_LOCATION "https://libcec.pulse-eight.com/"
!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE.md"
!insertmacro MUI_PAGE_COMPONENTS
!define MUI_DIRECTORYPAGE_VARIABLE $INSTDIR
!insertmacro MUI_PAGE_DIRECTORY

!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Pulse-Eight\${BASE_REGKEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

; installer sections, separate file to declutter a bit
!include "nsis\sections.nsh"
!include "nsis\eventghost.nsh"
!include "nsis\vc_redist.nsh"

Function .onInit
!ifdef INNER
	; just write the uninstaller and exit
	SetSilent silent
	WriteUninstaller "$%TEMP%\uninstall_libcec.exe"
	SetErrorLevel 0
	Quit
!else
	; the actual onInit function

	; delete the old version which contained both x64 and x86. this has been split up now. it contained a typo, very useful in this case
	Call UninstallOldVersion

	!ifdef NSIS_X86
		StrCpy $INSTDIR "$PROGRAMFILES\Pulse-Eight\USB-CEC Adapter"
		; check for vc x86 redist
		Call vsRedistX86
	!else
		StrCpy $INSTDIR "$PROGRAMFILES64\Pulse-Eight\USB-CEC Adapter"
		; check for vc x64 redist
		Call vsRedistX64
	!endif

	!ifndef NSISINCLUDEPDB
		; check for EventGhost
		Call EventGhost
	!endif
!endif
FunctionEnd
