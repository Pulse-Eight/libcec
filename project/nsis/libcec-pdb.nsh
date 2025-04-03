Section "libCEC debug symbols" SecPDB
	SetShellVarContext current
	SectionIn 1

	SetOutPath "$INSTDIR"
	File "${BINARY_SOURCE_DIR}\lib\cec.pdb"
	File "${BINARY_SOURCE_DIR}\cec-tray.pdb"

	SetOutPath "$INSTDIR\netfx"
	File /nonfatal "${BINARY_SOURCE_DIR}\CecSharpTester.pdb"
	File "${BINARY_SOURCE_DIR}\LibCecSharp.pdb"
	File "${BINARY_SOURCE_DIR}\LibCecSharp.xml"

	SetOutPath "$INSTDIR\net8.0"
	File /nonfatal "${BINARY_SOURCE_DIR}\net8.0\CecSharpCoreTester.pdb"
	File "${BINARY_SOURCE_DIR}\net8.0\LibCecSharpCore.pdb"
	File "${BINARY_SOURCE_DIR}\net8.0\CecSharpCoreTester.pdb"
SectionEnd
