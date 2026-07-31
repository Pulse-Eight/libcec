'''Authenticode signing through Azure Artifact Signing (formerly Trusted Signing).

Signing is on when AZURE_SIGNING_JSON is set. It holds the Endpoint,
CodeSigningAccountName and CertificateProfileName that signtool reads through
/dmdf, and the Azure SDK inside the signing dlib picks the credentials up from
AZURE_TENANT_ID, AZURE_CLIENT_ID and AZURE_CLIENT_SECRET. Nothing is read from
or written to the repository: the metadata is written to support/private, which
.gitignore excludes, and removed again once the build is done with it.

The dlib is a native x64 component, so all signing goes through the x64
signtool.exe whatever architecture is being built - the bitness of the signing
host is unrelated to the bitness of the file it signs.
'''
import glob
import json
import os
import shutil
from pathbuilder import PathBuilder
from mixins import exec_command
import logging

logger = logging.getLogger('libcec-builder')

# Azure Artifact Signing certificates are short-lived, so a signature is only
# durable if it carries a countersigned timestamp
TIMESTAMP_URL = 'http://timestamp.acs.microsoft.com'

# what a Windows build ships and can carry an Authenticode signature
SIGNABLE_SUFFIXES = ('.exe', '.dll', '.pyd', '.node')

def enabled() -> bool:
    return bool(os.environ.get('AZURE_SIGNING_JSON', '').strip())

def _signtool() -> str:
    override = os.environ.get('SIGNTOOL')
    if override:
        if not os.path.isfile(override):
            raise Exception(f'SIGNTOOL is set but {override} does not exist')
        return override
    kits = os.path.expandvars(r'%ProgramFiles(x86)%\Windows Kits\10\bin')
    found = sorted(glob.glob(os.path.join(kits, '*', 'x64', 'signtool.exe')))
    if not found:
        raise Exception(f'no x64 signtool.exe under {kits}; install the Windows SDK signing tools')
    # highest SDK version wins; the directory names sort in version order
    return found[-1]

def _dlib() -> str:
    override = os.environ.get('AZURE_SIGNING_DLIB')
    if override:
        if not os.path.isfile(override):
            raise Exception(f'AZURE_SIGNING_DLIB is set but {override} does not exist')
        return override
    # nuget install unpacks to a version-stamped directory, so match on the
    # package name rather than a version that changes under us
    pattern = r'C:\jenkins-deps\Microsoft.ArtifactSigning.Client*\bin\x64\Azure.CodeSigning.Dlib.dll'
    found = sorted(glob.glob(pattern))
    if not found:
        raise Exception(f'no signing dlib matching {pattern}; '
                        'nuget install Microsoft.ArtifactSigning.Client -OutputDirectory C:\\jenkins-deps')
    return found[-1]

class CodeSigner:
    '''Holds the signtool invocation and the metadata file it reads. The metadata
    outlives a single call because makensis signs the uninstaller through
    sign_shim() while the installer is being built.'''

    def __init__(self, repo_dir:PathBuilder) -> None:
        self.repo_dir = repo_dir
        self.signtool = _signtool()
        self.dlib = _dlib()
        self._private = self.repo_dir.add('support/private')

    @property
    def metadata(self) -> str:
        return os.path.join(str(self._private), 'signing-metadata.json')

    @property
    def shim(self) -> str:
        '''libCEC.nsi signs the uninstaller with this if it exists'''
        return os.path.join(str(self._private), 'sign-binary.cmd')

    def prepare(self) -> None:
        os.makedirs(str(self._private), exist_ok=True)
        # fail early and loudly on a malformed blob rather than let signtool
        # report it as an unhelpful signing error
        json.loads(os.environ['AZURE_SIGNING_JSON'])
        with open(self.metadata, 'w', encoding='utf-8') as f:
            f.write(os.environ['AZURE_SIGNING_JSON'])
        with open(self.shim, 'w', encoding='utf-8') as f:
            f.write('@echo off\r\n')
            f.write(f'"{self.signtool}" sign /v /fd SHA256 /tr {TIMESTAMP_URL} /td SHA256 '
                    f'/dlib "{self.dlib}" /dmdf "{self.metadata}" %1\r\n')

    def cleanup(self) -> None:
        for path in (self.metadata, self.shim):
            if os.path.isfile(path):
                os.remove(path)

    def sign(self, paths:list[str]) -> None:
        if not paths:
            return
        for path in paths:
            logger.info(f'* signing {path}')
        # a long file list can exceed the command line limit, so sign in batches
        for batch in (paths[i:i + 20] for i in range(0, len(paths), 20)):
            files = ' '.join(f'"{p}"' for p in batch)
            cmd = f'"{self.signtool}" sign /v /fd SHA256 /tr {TIMESTAMP_URL} /td SHA256 ' \
                  f'/dlib "{self.dlib}" /dmdf "{self.metadata}" {files}'
            rv = exec_command(cmd, capture_output=True)
            if not self._succeeded(rv):
                for line in rv:
                    print(line)
                raise Exception('signing failed')

    @staticmethod
    def _succeeded(output:list[str]) -> bool:
        # signtool reports per-file results and a summary; treat any reported
        # failure as fatal rather than trusting the exit code alone
        for line in output:
            if 'Number of errors: 0' in line:
                return True
        return False

def signable(root:PathBuilder) -> list[str]:
    '''Everything under root that a Windows build ships and can be signed.'''
    root = str(root)
    out = []
    for dirpath, dirnames, filenames in os.walk(root):
        if os.path.normcase(dirpath) == os.path.normcase(root):
            # cmake generates into a subdirectory of its own install prefix, so
            # its working tree sits inside the tree being signed. It holds the
            # compiler-detection probes and a second copy of every binary, none
            # of which is shipped.
            dirnames[:] = [d for d in dirnames if d.lower() != 'cmake']
        for name in filenames:
            if name.lower().endswith(SIGNABLE_SUFFIXES):
                out.append(os.path.join(dirpath, name))
    return sorted(out)
