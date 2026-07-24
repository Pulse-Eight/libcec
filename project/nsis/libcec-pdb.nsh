Section "libCEC debug symbols" SecPDB
	SetShellVarContext current
	SectionIn 1

	SetOutPath "$INSTDIR"
	File "${BINARY_SOURCE_DIR}\cec.pdb"

	SetOutPath "$INSTDIR\net8.0"
	File /nonfatal "${BINARY_SOURCE_DIR}\net8.0\cec-tray.pdb"
	File /nonfatal "${BINARY_SOURCE_DIR}\net8.0\CecSharpTester.pdb"
	File "${BINARY_SOURCE_DIR}\net8.0\LibCecSharp.pdb"
	File "${BINARY_SOURCE_DIR}\net8.0\LibCecSharp.xml"
SectionEnd
