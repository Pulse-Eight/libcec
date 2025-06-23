Section "USB-CEC driver" SecDriver
	SectionIn RO
	SectionIn 1 2 3

	; Copy the driver installer
	SetOutPath "$INSTDIR\driver"
	File "..\build\p8-usbcec-driver-installer.exe"

	; Install driver
	ExecWait '"$INSTDIR\driver\p8-usbcec-driver-installer.exe" /S'
SectionEnd

Section "libCEC" SecLibCec
	SetShellVarContext all
	SectionIn 1 2
	SectionIn RO

	; Copy binaries and support files
	SetOutPath "$INSTDIR"
	File "..\ChangeLog"
	File "..\README.md"
	File "..\AUTHORS"
	File "..\LICENSE.md"
	File "..\docs\README.developers.md"
	File "..\docs\README.windows.md"
	File "..\support\windows\tv_on.cmd"
	File "..\support\windows\tv_off.cmd"
	File "${BINARY_SOURCE_DIR}\cec.dll"

	; Copy the headers
	SetOutPath "$INSTDIR\include"
	File /r /x *.so "${BINARY_SOURCE_DIR}\include\libcec\*.h"

	; Store installation folder
	WriteRegStr HKLM "Software\Pulse-Eight\${BASE_REGKEY}" "" $INSTDIR

	; Package uninstaller
	!ifndef INNER
		SetOutPath $INSTDIR
		File $%TEMP%\uninstall_libcec.exe
	!endif

	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
	SetOutPath $INSTDIR

	CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall ${BASE_REGKEY}.lnk" "$INSTDIR\uninstall_libcec.exe" \
		"" "$INSTDIR\uninstall.exe" 0 SW_SHOWNORMAL \
		"" "Uninstall ${BASE_REGKEY}."

	WriteINIStr "$SMPROGRAMS\$StartMenuFolder\Visit Pulse-Eight.url" "InternetShortcut" "URL" "https://www.pulse-eight.com/"
	!insertmacro MUI_STARTMENU_WRITE_END

	;add entry to add/remove programs
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${BASE_REGKEY}" \
		"DisplayName" "${BASE_REGKEY}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${BASE_REGKEY}" \
		"UninstallString" "$INSTDIR\uninstall.exe"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${BASE_REGKEY}" \
		"NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${BASE_REGKEY}" \
		"NoRepair" 1
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${BASE_REGKEY}" \
		"InstallLocation" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${BASE_REGKEY}" \
		"DisplayIcon" "$INSTDIR\cec-client.exe,0"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${BASE_REGKEY}" \
		"Publisher" "Pulse-Eight Limited"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${BASE_REGKEY}" \
		"HelpLink" "https://www.pulse-eight.com/"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${BASE_REGKEY}" \
		"URLInfoAbout" "https://www.pulse-eight.com"
SectionEnd

!ifndef NSISINCLUDEPDB
Section "libCEC for Python" SecPythonCec
	SectionIn 1 2

	; Copy binaries
	SetOutPath "$INSTDIR\python\cec"
	
	File "${BINARY_SOURCE_DIR}\cec.dll"
	File "${BINARY_SOURCE_DIR}\python\cec\_pycec.pyd"
	File "${BINARY_SOURCE_DIR}\python\cec\cec.py"
	!ifdef NSIS_X86
		File  "${BINARY_SOURCE_DIR}\python\cec\__init__.py"
	!endif
	SetOutPath "$INSTDIR\python"
	File "${BINARY_SOURCE_DIR}\python\pyCecClient.py"
SectionEnd
!endif

Section "libCEC for .Net" SecDotNetCore
	SectionIn 1 2

	; Copy binaries
	SetOutPath "$INSTDIR\net8.0"
	File "${BINARY_SOURCE_DIR}\cec.dll"
	File "${BINARY_SOURCE_DIR}\net8.0\LibCecSharpCore.deps.json"
	File "${BINARY_SOURCE_DIR}\net8.0\LibCecSharpCore.dll"
	File "${BINARY_SOURCE_DIR}\net8.0\LibCecSharpCore.runtimeconfig.json"
	File "${BINARY_SOURCE_DIR}\net8.0\LibCecSharpCore.xml"
	File /nonfatal "${BINARY_SOURCE_DIR}\net8.0\CecSharpCoreTester.exe"
	File /nonfatal "${BINARY_SOURCE_DIR}\net8.0\CecSharpCoreTester.deps.json"
	File /nonfatal "${BINARY_SOURCE_DIR}\net8.0\CecSharpCoreTester.dll"
	File /nonfatal "${BINARY_SOURCE_DIR}\net8.0\CecSharpCoreTester.runtimeconfig.json"
	File /nonfatal "${BINARY_SOURCE_DIR}\net8.0\Ijwhost.dll"
SectionEnd

Section "libCEC for .Net Framework" SecDotNet
	SectionIn 1 2

	; Copy binaries
	SetOutPath "$INSTDIR\netfx"
	File "${BINARY_SOURCE_DIR}\cec.dll"
	File "${BINARY_SOURCE_DIR}\LibCecSharp.dll"
	File "${BINARY_SOURCE_DIR}\LibCecSharp.xml"
	File /nonfatal "${BINARY_SOURCE_DIR}\CecSharpTester.exe"
SectionEnd

!ifdef NSISDOTNETAPPS
!include "nsis\cec-tray.nsh"
!endif

Section "libCEC client (cec-client)" SecCecClient
	SectionIn 1

	; Copy binaries
	SetOutPath "$INSTDIR"
	File "${BINARY_SOURCE_DIR}\cec-client.exe"
	File "${BINARY_SOURCE_DIR}\cecc-client.exe"

	; Start menu item
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
	SetShellVarContext all
	CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\CEC Test client.lnk" "$INSTDIR\cec-client.exe" \
		"" "$INSTDIR\cec-client.exe" 0 SW_SHOWNORMAL \
		"" "Start the CEC Test client."
	!insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section "Adapter Firmware" SecFwUpgrade
	SectionIn 1

	; Copy the driver installer
	SetOutPath "$INSTDIR\driver"
	File "..\build\p8-usbcec-bootloader-driver-installer.exe"
	; install driver
	ExecWait '"$INSTDIR\driver\p8-usbcec-bootloader-driver-installer.exe" /S'

	SetOutPath "$INSTDIR"
	File "..\build\libusb0.dll"
	NSISdl::download https://p8.opdenkamp.eu/cec/cec-firmware-latest.exe cec-firmware-latest.exe

	SetShellVarContext all
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Firmware Upgrade.lnk" "$INSTDIR\cec-firmware-latest.exe" \
		"" "$INSTDIR\cec-firmware-latest.exe" 0 SW_SHOWNORMAL \
		"" "Upgrade the firmware of the CEC adapter to the latest version."
SectionEnd

!ifdef NSISINCLUDEPDB
!include "nsis\libcec-pdb.nsh"
!endif

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

	${If} ${SectionIsSelected} ${SecCecClient}
	${OrIf} ${SectionIsSelected} ${SecDotNet}
	${OrIf} ${SectionIsSelected} ${SecDotNetCore}
	!ifndef NSISINCLUDEPDB
	${OrIf} ${SectionIsSelected} ${SecPythonCec}
	!endif
		!define /math MYSECTIONFLAGS ${SF_SELECTED} | ${SF_RO}
		!insertmacro SetSectionFlag ${SecLibCec} ${MYSECTIONFLAGS} 
		!undef MYSECTIONFLAGS
	${Else}
		!insertmacro ClearSectionFlag ${SecLibCec} ${SF_RO}
	${EndIf}

FunctionEnd
