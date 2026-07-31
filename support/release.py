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
thing, but checking here means a mistyped tag is caught before it is pushed.

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

POLL_SECONDS = 20
# a tag build compiles four installers plus the Debian packages
BUILD_TIMEOUT_SECONDS = 60 * 60


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
    run(['git', 'fetch', 'origin', '--tags'], cwd=repo)
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


def push(repo:Path, tag:str) -> None:
    log('pushing release and the tag')
    run(['git', 'push', 'origin', 'release'], cwd=repo)
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
        check_tag_free(repo, args.tag)
        check_release_free(repo, args.tag, args.gh, args.github_repo)
        run([args.gh, 'auth', 'status'])

        jenkins = Jenkins(os.environ['JENKINS_URL'], os.environ['JENKINS_USER'],
                          os.environ['JENKINS_TOKEN'], args.jenkins_job, args.insecure)

        merge_and_tag(repo, args.tag, version)
        push(repo, args.tag)
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
