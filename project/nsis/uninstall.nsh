Function un.DeleteInstalledFiles
	Delete "$INSTDIR\AUTHORS"
	Delete "$INSTDIR\cec.dll"
	Delete "$INSTDIR\cec.pdb"
	Delete "$INSTDIR\cecc-client.exe"
	Delete "$INSTDIR\cec-client.exe"
	Delete "$INSTDIR\cec-firmware-latest.exe"
	Delete "$INSTDIR\ChangeLog"
	Delete "$INSTDIR\COPYING"
	Delete "$INSTDIR\libusb0.dll"
	Delete "$INSTDIR\LICENSE"
	Delete "$INSTDIR\README.*"
	Delete "$INSTDIR\tv_on.cmd"
	Delete "$INSTDIR\tv_off.cmd"
	Delete "$INSTDIR\include\cec*.h"
	Delete "$INSTDIR\net8.0\LibCecSharp.deps.json"
	Delete "$INSTDIR\net8.0\LibCecSharp.dll"
	Delete "$INSTDIR\net8.0\LibCecSharp.pdb"
	Delete "$INSTDIR\net8.0\LibCecSharp.xml"
	Delete "$INSTDIR\net8.0\CecSharpTester.deps.json"
	Delete "$INSTDIR\net8.0\CecSharpTester.dll"
	Delete "$INSTDIR\net8.0\CecSharpTester.runtimeconfig.json"
	Delete "$INSTDIR\net8.0\CecSharpTester.exe"
	Delete "$INSTDIR\net8.0\CecSharpTester.pdb"
	Delete "$INSTDIR\net8.0\cec-tray.exe"
	Delete "$INSTDIR\net8.0\cec-tray.dll"
	Delete "$INSTDIR\net8.0\cec-tray.deps.json"
	Delete "$INSTDIR\net8.0\cec-tray.runtimeconfig.json"
	Delete "$INSTDIR\net8.0\cec-tray.pdb"
	; legacy files from the C++/CLI-based and .NET Framework installs, cleaned on upgrade
	Delete "$INSTDIR\netfx\LibCecSharp.dll"
	Delete "$INSTDIR\netfx\LibCecSharp.pdb"
	Delete "$INSTDIR\netfx\LibCecSharp.xml"
	Delete "$INSTDIR\netfx\cec-tray.exe"
	Delete "$INSTDIR\net8.0\LibCecSharpCore.deps.json"
	Delete "$INSTDIR\net8.0\LibCecSharpCore.dll"
	Delete "$INSTDIR\net8.0\LibCecSharpCore.runtimeconfig.json"
	Delete "$INSTDIR\net8.0\LibCecSharpCore.xml"
	Delete "$INSTDIR\net8.0\LibCecSharpCore.pdb"
	Delete "$INSTDIR\net8.0\CecSharpCoreTester.exe"
	Delete "$INSTDIR\net8.0\CecSharpCoreTester.dll"
	Delete "$INSTDIR\net8.0\CecSharpCoreTester.deps.json"
	Delete "$INSTDIR\net8.0\CecSharpCoreTester.runtimeconfig.json"
	Delete "$INSTDIR\net8.0\CecSharpCoreTester.pdb"
	Delete "$INSTDIR\net8.0\Ijwhost.dll"
	Delete "$INSTDIR\python\_cec.pyd"
	Delete "$INSTDIR\python\cec\cec.py"
	Delete "$INSTDIR\python\cec\__init__.py"
	Delete "$INSTDIR\python\pyCecClient.py"
	RMDir /r "$INSTDIR\nodejs"
FunctionEnd

; Uninstaller Section
Section "Uninstall"
	SetShellVarContext all

	Call un.DeleteInstalledFiles

	; Uninstall EventGhost plugin
	; Eventghost has no uninstall plugin feature so we simply delete the plugin
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
	Delete "$INSTDIR\uninstall_libcec.exe"
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

	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter software"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter driver"
	DeleteRegKey /ifempty HKLM "Software\Pulse-Eight\USB-CEC Adapter software"
	DeleteRegKey /ifempty HKLM "Software\Pulse-Eight"
SectionEnd