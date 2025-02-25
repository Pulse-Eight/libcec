!ifndef INNER
Var VSRedistSetupError
Var VSRedistInstalledX64

!ifdef NSIS_X86
Var VSRedistInstalledX86

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

Function vsRedistX86
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
FunctionEnd
!endif

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

Function vsRedistX64
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
FunctionEnd

!endif