!ifndef INNER
Var DotNetSetupError
Var DotNetInstalled

; The managed assembly and apps (LibCecSharp, CecSharpTester, cec-tray) are now
; net8.0, so they need the .NET 8 Desktop Runtime. Detect it and, when missing,
; download + silently install the matching architecture. The aka.ms links always
; redirect to the latest 8.0 patch.
!ifdef NSIS_X86
!define DOTNET_URL "https://aka.ms/dotnet/8.0/windowsdesktop-runtime-win-x86.exe"
!define DOTNET_DIR "$PROGRAMFILES\dotnet\shared\Microsoft.WindowsDesktop.App"
!else
!define DOTNET_URL "https://aka.ms/dotnet/8.0/windowsdesktop-runtime-win-x64.exe"
!define DOTNET_DIR "$PROGRAMFILES64\dotnet\shared\Microsoft.WindowsDesktop.App"
!endif

!define DOTNET_SECTIONNAME "Microsoft .NET 8 Desktop Runtime"

Section "" SecDotNetRuntime
	SetShellVarContext current
	; same install types as the managed "libCEC for .Net" component
	SectionIn 1 2
	SectionIn RO

	; only fetch the runtime when it's actually missing and .Net is being installed
	${If} $DotNetInstalled != "Yes"
	${AndIf} ${SectionIsSelected} ${SecDotNetCore}
		SetOutPath "$TEMP\dotnet"
		NSISdl::download "${DOTNET_URL}" windowsdesktop-runtime.exe
		ExecWait '"$TEMP\dotnet\windowsdesktop-runtime.exe" /install /quiet /norestart' $DotNetSetupError
		RMDir /r "$TEMP\dotnet"
	${EndIf}
SectionEnd

Function dotnetRuntime
	; look for an installed Microsoft.WindowsDesktop.App 8.x shared framework
	StrCpy $DotNetInstalled ""
	FindFirst $0 $1 "${DOTNET_DIR}\8.*"
	${If} $1 != ""
		StrCpy $DotNetInstalled "Yes"
	${EndIf}
	FindClose $0

	${If} $DotNetInstalled == "Yes"
		SectionSetText ${SecDotNetRuntime} ""
	${Else}
		SectionSetText ${SecDotNetRuntime} "${DOTNET_SECTIONNAME}"
	${EndIf}
FunctionEnd
!endif
