#!/usr/bin/env python3
'''Cut a libCEC release in one go.

Merges master into release, tags the merge, waits for Jenkins to build and sign
the tag, then publishes a GitHub release carrying those signed artefacts. It
asks nothing along the way: everything it needs comes from the command line and
the environment, and anything it cannot verify stops the release instead.

    export JENKINS_URL=https://<controller>
    export JENKINS_USER=<user>
    export JENKINS_TOKEN=<api token>
    python support/release.py --tag libcec-8.1.2 --notes-file notes.md

The tag must match LIBCEC_VERSION_* in CMakeLists.txt; Jenkins checks the same
thing, but checking here means a mistyped tag is caught before it is pushed. The
shipped files that repeat that version (SATELLITE_VERSIONS) have to agree with it
too, so a forgotten bump cannot reach a published artefact.

Nothing here is a secret: the controller address and credentials come from the
environment, and GitHub authentication is whatever `gh auth` already holds.
'''
import argparse
import json
import os
import re
import ssl
import subprocess
import sys
import tempfile
import time
import urllib.error
import urllib.parse
import urllib.request
from base64 import b64encode
from pathlib import Path

# what a GitHub release has always carried. The Debian packages and the Linux
# tarball are built too, but they are not release assets today.
ASSET_SUFFIXES = ('.exe', '.egplugin')

# Files that repeat the version from CMakeLists.txt and are shipped as-is.
# Nothing regenerates them from a clean checkout - package.json is hand-written
# and the .csproj, though generated from its .in, is tracked so Visual Studio can
# open it without a cmake run first - so a stale one is published verbatim.
# Each entry is (path, pattern capturing the declared version, suffix the file
# adds to x.y.z). Add a line here whenever a new binding carries its own version.
SATELLITE_VERSIONS = (
    ('src/nodejs/package.json',          r'^\s*"version"\s*:\s*"([^"]+)"',   ''),
    ('src/dotnetlib/LibCecSharp.csproj', r'<Version>([^<]+)</Version>',      '.0'),
    # ^version, not \bversion: rust-version sits two lines below it
    ('src/rust/Cargo.toml',              r'^version\s*=\s*"([^"]+)"',        ''),
    # the trailing .N is the Debian revision, which moves independently
    ('debian/changelog.in',              r'\Alibcec \((\d+\.\d+\.\d+)\.\d+~#DIST#\)', ''),
)

POLL_SECONDS = 20
# a tag build compiles four installers plus the Debian packages
BUILD_TIMEOUT_SECONDS = 60 * 60
# how long to wait for another branch's build to get out of the way
IDLE_TIMEOUT_SECONDS = 30 * 60


class ReleaseError(Exception):
    pass


def log(msg:str) -> None:
    print(f'* {msg}', flush=True)


def run(args:list[str], cwd:Path|None=None, capture:bool=True) -> str:
    rv = subprocess.run(args, cwd=cwd, text=True,
                        stdout=subprocess.PIPE if capture else None,
                        stderr=subprocess.STDOUT if capture else None)
    if rv.returncode != 0:
        raise ReleaseError(f'{" ".join(args)} failed:\n{rv.stdout or ""}')
    return (rv.stdout or '').strip()


class Jenkins:
    def __init__(self, url:str, user:str, token:str, job:str, insecure:bool) -> None:
        self.url = url.rstrip('/')
        self._auth = b64encode(f'{user}:{token}'.encode()).decode()
        self.job = job
        self._ctx = ssl._create_unverified_context() if insecure else None

    def _open(self, path:str, method:str='GET'):
        req = urllib.request.Request(f'{self.url}{path}', method=method)
        req.add_header('Authorization', f'Basic {self._auth}')
        return urllib.request.urlopen(req, context=self._ctx, timeout=60)

    def json(self, path:str) -> dict|None:
        try:
            with self._open(path) as r:
                return json.loads(r.read().decode())
        except urllib.error.HTTPError as e:
            if e.code == 404:
                return None
            raise

    def scan(self) -> None:
        '''Ask the multibranch job to re-scan, so a freshly pushed tag is picked
        up now rather than at the next poll.'''
        try:
            self._open(f'/job/{self.job}/build?delay=0', method='POST').close()
        except urllib.error.HTTPError as e:
            # a scan trigger answers with a redirect
            if e.code not in (200, 201, 302):
                raise

    def download(self, tag:str, build:int, rel_path:str, dest:Path) -> None:
        quoted = urllib.parse.quote(rel_path)
        with self._open(f'/job/{self.job}/job/{urllib.parse.quote(tag)}/{build}/artifact/{quoted}') as r:
            dest.write_bytes(r.read())


def check_clean_tree(repo:Path) -> None:
    if run(['git', 'status', '--porcelain', '--untracked-files=no'], cwd=repo):
        raise ReleaseError('working tree has uncommitted changes; commit or stash first')


def check_version_matches(repo:Path, tag:str) -> str:
    if not tag.startswith('libcec-'):
        raise ReleaseError(f"tag must look like 'libcec-<x.y.z>'; got {tag}")
    want = tag[len('libcec-'):]
    text = (repo / 'CMakeLists.txt').read_text(encoding='utf-8')
    parts = []
    for name in ('MAJOR', 'MINOR', 'PATCH'):
        m = re.search(rf'^\s*set\(LIBCEC_VERSION_{name}\s+(\d+)\s*\)', text, re.M)
        if not m:
            raise ReleaseError(f'could not read LIBCEC_VERSION_{name} from CMakeLists.txt')
        parts.append(m.group(1))
    have = '.'.join(parts)
    if have != want:
        raise ReleaseError(f'tag {tag} implies {want}, but CMakeLists.txt declares {have}')
    return have


def check_satellite_versions(repo:Path, version:str) -> None:
    '''CMakeLists.txt is the source of truth, but every file in
    SATELLITE_VERSIONS repeats it and ships. Report all mismatches at once, so a
    bump that missed two files does not take two release attempts to find.'''
    problems = []
    for rel, pattern, suffix in SATELLITE_VERSIONS:
        path = repo / rel
        if not path.is_file():
            problems.append(f'{rel}: missing')
            continue
        m = re.search(pattern, path.read_text(encoding='utf-8'), re.M)
        want = version + suffix
        if not m:
            problems.append(f'{rel}: no version found, expected {want}')
        elif m.group(1) != want:
            problems.append(f'{rel}: declares {m.group(1)}, expected {want}')
    if problems:
        raise ReleaseError('these files disagree with CMakeLists.txt; bump them '
                           'before releasing:\n  ' + '\n  '.join(problems))


def check_tag_free(repo:Path, tag:str) -> None:
    if run(['git', 'tag', '--list', tag], cwd=repo):
        raise ReleaseError(f'tag {tag} already exists locally')
    if run(['git', 'ls-remote', '--tags', 'origin', tag], cwd=repo):
        raise ReleaseError(f'tag {tag} already exists on origin')


def check_release_free(repo:Path, tag:str, gh:str, github_repo:str) -> None:
    rv = subprocess.run([gh, 'release', 'view', tag, '--repo', github_repo],
                        cwd=repo, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if rv.returncode == 0:
        raise ReleaseError(f'a GitHub release for {tag} already exists')


def merge_and_tag(repo:Path, tag:str, version:str) -> str:
    # branches only, no --tags: fetching every tag aborts the whole fetch if any
    # local tag disagrees with the remote, which stops a release over a stale tag
    # that has nothing to do with it. Nothing here needs the full local tag set -
    # this fetch exists to compare master with origin/master and to reset release
    # to origin/release, and check_tag_free() asks the remote about the one tag
    # being cut directly with ls-remote
    run(['git', 'fetch', 'origin'], cwd=repo)
    # master must already be pushed: the release branch merges the remote state,
    # so anything held locally would silently not be in the release
    local = run(['git', 'rev-parse', 'master'], cwd=repo)
    remote = run(['git', 'rev-parse', 'origin/master'], cwd=repo)
    if local != remote:
        raise ReleaseError('master differs from origin/master; push master first')

    log('merging master into release')
    run(['git', 'checkout', 'release'], cwd=repo)
    run(['git', 'reset', '--hard', 'origin/release'], cwd=repo)
    run(['git', 'merge', '--no-ff', 'master', '-m', "Merge branch 'master' into release"], cwd=repo)
    merge_commit = run(['git', 'rev-parse', 'HEAD'], cwd=repo)

    # annotated, and on the merge commit rather than on master
    log(f'tagging {merge_commit[:8]} as {tag}')
    run(['git', 'tag', '-a', tag, '-m', f'libCEC {version}', merge_commit], cwd=repo)
    return merge_commit


def await_idle(jenkins:Jenkins, branch:str) -> None:
    '''Wait until the given branch job is not building.

    The Windows agent runs one mspdbsrv.exe for every build on it, and two
    concurrent MSVC builds kill it - the loser dies with "fatal error C1090:
    PDB API call failed, error code '23'". /Z7 (see the cmake flag overrides)
    is what actually removes that failure mode; this keeps the release from
    lining three builds up on one agent in the first place, which is also just
    faster. Waiting is best-effort: a branch that Jenkins does not know about,
    or a controller that cannot be reached, must not stop a release.'''
    deadline = time.time() + IDLE_TIMEOUT_SECONDS
    announced = False
    while time.time() < deadline:
        try:
            job = jenkins.json(f'/job/{jenkins.job}/job/{urllib.parse.quote(branch)}/api/json')
        except Exception as e:
            log(f'could not ask Jenkins about {branch} ({e}); continuing')
            return
        last = (job or {}).get('lastBuild')
        if not job or not last or not last.get('building'):
            return
        if not announced:
            log(f'waiting for {branch} #{last["number"]} to finish first')
            announced = True
        time.sleep(POLL_SECONDS)
    log(f'{branch} is still building; continuing anyway')


def push(repo:Path, tag:str, jenkins:Jenkins) -> None:
    # Each push starts a build: master is likely still building the version
    # bump, 'release' gets its own build, and then the tag gets the one this
    # release depends on. Serialise them so the tag build has the agent to
    # itself.
    await_idle(jenkins, 'master')
    log('pushing release')
    run(['git', 'push', 'origin', 'release'], cwd=repo)
    await_idle(jenkins, 'release')
    log('pushing the tag')
    run(['git', 'push', 'origin', tag], cwd=repo)


def await_build(jenkins:Jenkins, tag:str) -> int:
    log('waiting for Jenkins to discover the tag')
    jenkins.scan()
    deadline = time.time() + BUILD_TIMEOUT_SECONDS
    build = None
    while time.time() < deadline:
        job = jenkins.json(f'/job/{jenkins.job}/job/{urllib.parse.quote(tag)}/api/json')
        if job:
            last = job.get('lastBuild')
            if last:
                build = last['number']
                break
        time.sleep(POLL_SECONDS)
    if build is None:
        raise ReleaseError(f'Jenkins did not start a build for {tag} in time')

    log(f'waiting for build #{build}')
    while time.time() < deadline:
        info = jenkins.json(f'/job/{jenkins.job}/job/{urllib.parse.quote(tag)}/{build}/api/json')
        result = (info or {}).get('result')
        if result == 'SUCCESS':
            return build
        if result:
            raise ReleaseError(f'build #{build} finished {result}; not releasing')
        time.sleep(POLL_SECONDS)
    raise ReleaseError(f'build #{build} did not finish in time')


def fetch_assets(jenkins:Jenkins, tag:str, build:int, into:Path) -> list[Path]:
    info = jenkins.json(f'/job/{jenkins.job}/job/{urllib.parse.quote(tag)}/{build}/api/json')
    paths = [a['relativePath'] for a in (info or {}).get('artifacts', [])
             if a['relativePath'].endswith(ASSET_SUFFIXES)]
    if not paths:
        raise ReleaseError(f'build #{build} archived no release assets')
    out = []
    for rel in sorted(paths):
        dest = into / Path(rel).name
        log(f'downloading {dest.name}')
        jenkins.download(tag, build, rel, dest)
        out.append(dest)
    return out


def verify_signed(paths:list[Path]) -> None:
    '''A release exists to ship signed binaries, so refuse to publish one that
    is not signed. Only Windows can check Authenticode.'''
    if sys.platform != 'win32':
        log('skipping signature check: not on Windows')
        return
    for path in paths:
        if path.suffix != '.exe':
            continue
        rv = run(['powershell', '-NoProfile', '-Command',
                  f'(Get-AuthenticodeSignature -LiteralPath "{path}").Status'])
        if rv.strip() != 'Valid':
            raise ReleaseError(f'{path.name} is not validly signed (status: {rv.strip()})')
        log(f'signature OK: {path.name}')


def publish(repo:Path, tag:str, version:str, notes:Path, assets:list[Path],
            gh:str, github_repo:str, draft:bool) -> None:
    log(f'creating the GitHub release for {tag}')
    cmd = [gh, 'release', 'create', tag, '--repo', github_repo,
           '--title', f'libCEC {version}', '--notes-file', str(notes)]
    if draft:
        cmd.append('--draft')
    cmd += [str(a) for a in assets]
    run(cmd, cwd=repo, capture=False)


def main() -> int:
    ap = argparse.ArgumentParser(description='Cut a libCEC release')
    ap.add_argument('--tag', required=True, help="release tag, e.g. libcec-8.1.2")
    ap.add_argument('--notes-file', required=True, help='markdown release notes to post')
    ap.add_argument('--repo', default=str(Path(__file__).resolve().parent.parent))
    ap.add_argument('--github-repo', default='Pulse-Eight/libcec')
    ap.add_argument('--jenkins-job', default=os.environ.get('JENKINS_JOB', 'libcec'))
    ap.add_argument('--gh', default=os.environ.get('GH', 'gh'), help='path to the gh CLI')
    ap.add_argument('--draft', action='store_true', help='create the release as a draft')
    ap.add_argument('--insecure', action='store_true',
                    default=bool(os.environ.get('JENKINS_INSECURE')),
                    help="skip TLS verification (the controller's certificate is self-signed)")
    args = ap.parse_args()

    repo = Path(args.repo).resolve()
    notes = Path(args.notes_file).resolve()

    try:
        missing = [v for v in ('JENKINS_URL', 'JENKINS_USER', 'JENKINS_TOKEN')
                   if not os.environ.get(v)]
        if missing:
            raise ReleaseError(f'set {", ".join(missing)} in the environment')
        if not notes.is_file() or not notes.read_text(encoding='utf-8').strip():
            raise ReleaseError(f'{notes} is missing or empty')

        log('checking the working tree and the tag')
        check_clean_tree(repo)
        version = check_version_matches(repo, args.tag)
        check_satellite_versions(repo, version)
        check_tag_free(repo, args.tag)
        check_release_free(repo, args.tag, args.gh, args.github_repo)
        run([args.gh, 'auth', 'status'])

        jenkins = Jenkins(os.environ['JENKINS_URL'], os.environ['JENKINS_USER'],
                          os.environ['JENKINS_TOKEN'], args.jenkins_job, args.insecure)

        merge_and_tag(repo, args.tag, version)
        push(repo, args.tag, jenkins)
        build = await_build(jenkins, args.tag)

        with tempfile.TemporaryDirectory(prefix='libcec-release-') as tmp:
            assets = fetch_assets(jenkins, args.tag, build, Path(tmp))
            verify_signed(assets)
            publish(repo, args.tag, version, notes, assets,
                    args.gh, args.github_repo, args.draft)

        log(f'released {args.tag} from build #{build}')
        return 0
    except ReleaseError as e:
        print(f'error: {e}', file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())
