// Jenkinsfile for libCEC
//
// Behaviour:
//   PR / branch push          → build on Linux + Windows, no artefacts kept.
//   master push               → same, plus the five Windows release artefacts.
//   Tag push (libcec-<x.y.z>) → version cross-check against CMakeLists.txt, then
//                               the same five artefacts.
//
// Archived per build: a tarball of the Linux install tree. master and tags add
// the amd64 Debian packages, and the same five Windows files a GitHub release
// carries — libcec-{x64,x86}-<ver>.exe, libcec-{x64,x86}-dbg-<ver>.exe and
// libcec-eventghost-plugin-<ver>.egplugin.
//
// The Debian packages are amd64 only: building arm64/armhf needs qemu binfmt
// emulation, which the Linux agent does not have.
//
// The Jenkins controller is on an internal network and GitHub cannot reach it,
// so there is no webhook: the multibranch job polls GitHub on a timer
// ("Scan Repository Triggers" → periodically). Nothing in this file needs to
// know that, but it explains why a push takes a few minutes to show up.
//
// Reference Jenkinsfile patterns:
//   - PulseEight.SMT.Panasonic/Jenkinsfile — tag-driven release vs branch/PR build
//
// SECRETS: none are used here, and none belong here. Code signing (planned as a
// follow-up for tagged builds) will bind the existing Jenkins credentials
// (AZURE_* / CERT_SIGNING_NAME) through a `withCredentials` block at the point
// of use — never as literals and never in the repo.
//
// Agent expectations:
//   linux   — podman only; the build toolchain lives in the container image.
//   windows — Visual Studio Community 2026 (toolchain id '2026c'), CMake, NSIS,
//             swig, the .NET 8 SDK, and Python 3.12+ in *both* 64-bit and 32-bit:
//             the EventGhost plugin embeds the x86 library and the x86 Python
//             module, so an x86 build needs 32-bit Python headers and .lib to
//             produce that module. This is what windows/create-installer.py
//             drives; see CLAUDE.md.
//
// Only the default Pulse-Eight USB adapter backend is built on Linux. The
// SoC-native backends (-DHAVE_LINUX_API=1 etc.) each need their own kernel/vendor
// headers and, per CLAUDE.md, rewrite generated files in the *source* tree — so
// they cannot share a workspace with another flag set. If they get CI coverage
// later it should be as separate stages with their own workspaces.

pipeline {
    agent none

    options {
        timeout(time: 90, unit: 'MINUTES')
        timestamps()
        buildDiscarder(logRotator(numToKeepStr: '30', artifactNumToKeepStr: '10'))
        disableConcurrentBuilds()
    }

    environment {
        DOTNET_CLI_TELEMETRY_OPTOUT = '1'
        DOTNET_NOLOGO = '1'
        DOTNET_SKIP_FIRST_TIME_EXPERIENCE = '1'

        GITHUB_REPO = 'Pulse-Eight/libcec'

        // Windows toolchain id understood by windows/create-installer.py -t.
        // '2026c' is Visual Studio Community 2026, the edition on the build agent.
        WIN_TOOLCHAIN = '2026c'
    }

    stages {
        stage('Context') {
            agent { label 'linux' }
            steps {
                script {
                    env.IS_TAG = (env.TAG_NAME != null && env.TAG_NAME.startsWith('libcec-')) ? 'true' : 'false'
                    env.IS_MASTER = (env.BRANCH_NAME == 'master') ? 'true' : 'false'

                    // The version lives in the top-level CMakeLists.txt and is the
                    // source of truth for include/version.h, the SONAME and the
                    // installer filename, all of which are generated from it.
                    def cml = readFile 'CMakeLists.txt'
                    def part = { name ->
                        def m = (cml =~ /(?m)^\s*set\(LIBCEC_VERSION_${name}\s+(\d+)\s*\)/)
                        if (!m) { error "Could not read LIBCEC_VERSION_${name} from CMakeLists.txt" }
                        return m[0][1]
                    }
                    env.LIBCEC_VERSION = "${part('MAJOR')}.${part('MINOR')}.${part('PATCH')}"

                    if (env.IS_TAG == 'true') {
                        // Tags are 'libcec-<x.y.z>' (e.g. libcec-8.1.1).
                        env.RELEASE_VERSION = env.TAG_NAME.substring('libcec-'.length())
                        currentBuild.displayName = "#${BUILD_NUMBER} (${env.TAG_NAME})"
                    }
                }
                sh '''
                    echo "Branch:        ${BRANCH_NAME:-<none>}"
                    echo "Tag:           ${TAG_NAME:-<none>}"
                    echo "IsTag:         ${IS_TAG}"
                    echo "IsMaster:      ${IS_MASTER}"
                    echo "Git commit:    ${GIT_COMMIT:-<none>}"
                    echo "CMake version: ${LIBCEC_VERSION}"
                    echo "---"
                    podman --version
                '''
            }
            post {
                cleanup { cleanWs() }
            }
        }

        stage('Validate tag version') {
            // A tag whose version does not match CMakeLists.txt would produce an
            // installer and a SONAME that disagree with the tag name. Catch the
            // typo here rather than after publishing.
            agent none
            when { expression { env.IS_TAG == 'true' } }
            steps {
                script {
                    if (env.RELEASE_VERSION != env.LIBCEC_VERSION) {
                        error("Tag ${env.TAG_NAME} implies version ${env.RELEASE_VERSION}, " +
                              "but CMakeLists.txt declares ${env.LIBCEC_VERSION}. " +
                              "Either bump LIBCEC_VERSION_* or retag.")
                    }
                    echo "Version check: OK (${env.LIBCEC_VERSION})"
                }
            }
        }

        stage('Build') {
            parallel {
                stage('Linux') {
                    agent { label 'linux && podman' }
                    steps {
                        // The build runs in a container rather than on the agent so
                        // the agent stays generic: it carries podman, and the C++
                        // toolchain and -dev packages live in the image. The package
                        // list is debian/control's Build-Depends minus the .NET and
                        // Node.js entries, which only the optional ENABLE_DOTNET_LIB
                        // and ENABLE_NODE_LIB targets need.
                        //
                        // bookworm matches the agent's own distribution, so a failure
                        // here is one that would reproduce on the agent.
                        //
                        // Rootless podman maps container root to the agent's own uid,
                        // so the build output is owned by the agent and cleanWs can
                        // remove it.
                        //
                        // No HAVE_*_API flags: this is the Pulse-Eight USB adapter
                        // backend only.
                        //
                        // cec-client needs no CEC hardware to print its usage, so the
                        // smoke test proves the shared library links and loads. It is
                        // inside the container because that is where libcec.so's
                        // dependencies are installed.
                        sh '''
                            set -e
                            rm -rf dist stage
                            mkdir -p dist
                            podman run --rm \\
                                -e LIBCEC_VERSION="$LIBCEC_VERSION" \\
                                -v "$WORKSPACE":/src:z \\
                                -w /src \\
                                docker.io/library/debian:bookworm \\
                                bash -c '
                                    set -e
                                    apt-get update -qq
                                    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \\
                                        build-essential cmake pkg-config swig \\
                                        libudev-dev libxrandr-dev x11proto-core-dev \\
                                        libncurses-dev python3-dev
                                    rm -rf build
                                    mkdir -p build
                                    cd build
                                    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
                                    make -j"$(nproc)"
                                    ./src/cec-client/cec-client --help 2>&1 | head -20
                                    ldd ./src/libcec/libcec.so | head -20
                                    make install DESTDIR=/src/stage
                                    tar -czf "/src/dist/libcec-linux-x86_64-${LIBCEC_VERSION}.tar.gz" -C /src/stage .
                                '
                            tar -tzf "dist/libcec-linux-x86_64-${LIBCEC_VERSION}.tar.gz" | head -20
                        '''
                        script {
                            // The Debian packages are the shippable Linux artefact, so
                            // they are built where the Windows installers are: master
                            // and tags. amd64 only — the agent has no binfmt/qemu, so
                            // arm64 and armhf cannot be emulated here.
                            //
                            // trixie rather than the bookworm used above, because that
                            // is what these packages target. debian/control
                            // build-depends on dotnet-sdk-8.0, which Debian does not
                            // ship in any suite, so Microsoft's archive is added for it.
                            if (env.IS_TAG == 'true' || env.IS_MASTER == 'true') {
                                // A tag is the released version and takes the changelog
                                // version as-is. Every other build gets a snapshot
                                // suffix so two builds of the same changelog entry are
                                // tellable apart and traceable to a commit.
                                env.DEB_GIT_HASH = (env.IS_TAG == 'true') ? '' : (env.GIT_COMMIT ?: '').take(8)
                                sh '''
                                    set -e
                                    podman run --rm \\
                                        -e DEB_GIT_HASH="$DEB_GIT_HASH" \\
                                        -v "$WORKSPACE":/src:z \\
                                        -w /src \\
                                        docker.io/library/debian:trixie \\
                                        bash -c '
                                            set -e
                                            export DEBIAN_FRONTEND=noninteractive
                                            apt-get update -qq
                                            apt-get install -y --no-install-recommends \\
                                                ca-certificates curl devscripts equivs dpkg-dev
                                            curl -fsSL -o /tmp/ms-prod.deb \\
                                                https://packages.microsoft.com/config/debian/13/packages-microsoft-prod.deb
                                            dpkg -i /tmp/ms-prod.deb
                                            apt-get update -qq
                                            # dpkg-buildpackage writes the .debs to the parent
                                            # directory, and the cmake build above leaves output
                                            # in the workspace, so build from a clean copy
                                            rm -rf /work
                                            mkdir -p /work/libcec
                                            cp -a /src/. /work/libcec/
                                            rm -rf /work/libcec/build /work/libcec/stage /work/libcec/dist
                                            cd /work/libcec
                                            # debian/changelog is generated: changelog.in
                                            # carries a #DIST# placeholder per suite.
                                            # The first #DIST# is the one inside the
                                            # version, so a snapshot suffix goes there
                                            # and the suite name is left alone. The
                                            # suffix starts with ~ so a snapshot sorts
                                            # below the release it leads up to.
                                            SUFFIX=""
                                            if [ -n "$DEB_GIT_HASH" ]; then
                                                SUFFIX="~git$(date -u +%Y%m%d).$DEB_GIT_HASH"
                                            fi
                                            sed -e "1s/#DIST#/trixie${SUFFIX}/" \\
                                                -e "s/#DIST#/trixie/g" \\
                                                debian/changelog.in > debian/changelog
                                            head -1 debian/changelog
                                            if [ -n "$SUFFIX" ]; then
                                                BASE=$(head -1 debian/changelog.in | sed -e "s/#DIST#/trixie/g" -e "s/^libcec (//" -e "s/).*//")
                                                SNAP=$(head -1 debian/changelog     | sed -e "s/^libcec (//" -e "s/).*//")
                                                dpkg --compare-versions "$SNAP" lt "$BASE"
                                                echo "version check: $SNAP sorts below $BASE"
                                            fi
                                            mk-build-deps -i -r -t "apt-get -y --no-install-recommends"
                                            dpkg-buildpackage -us -uc -b
                                            cp /work/*.deb /src/dist/
                                        '
                                    ls -l dist/*.deb
                                '''
                            }
                        }
                    }
                    post {
                        success {
                            archiveArtifacts artifacts: 'dist/*',
                                             fingerprint: true
                        }
                        cleanup { cleanWs() }
                    }
                }

                stage('Windows') {
                    // p8-ci-win-2 rather than the 'windows' label: it is the only
                    // Windows agent carrying a C++ toolchain, CMake and NSIS.
                    // Widen this to a label once a second agent is provisioned
                    // the same way.
                    agent { label 'p8-ci-win-2' }
                    steps {
                        // src/dotnet (cec-dotnet: cec-tray + CecSharpTester) is only
                        // needed by the Windows build and the multibranch GitHub source
                        // does not fetch submodules, so init it explicitly. The
                        // submodule URL is https and the repo is public, so this needs
                        // no credentials.
                        bat '''
                            git submodule update --init --recursive
                        '''
                        bat '''
                            py -3-64 --version
                            cmake --version
                            dotnet --version
                        '''
                        script {
                            // 'py -3-64', not 'python': windows/toolchain.py locates
                            // Visual Studio through %ProgramFiles%, which Windows
                            // redirects to the x86 directory for a 32-bit process, and
                            // the 32-bit interpreter the EventGhost plugin needs comes
                            // first on PATH.
                            //
                            // Branch/PR: x64, no installer and no EventGhost plugin.
                            // The plugin always embeds the x86 library, so building it
                            // forces a second full build of the library on top of the
                            // requested architecture.
                            //
                            // master and tags produce the five artefacts a release
                            // ships: a Release and a Debug installer for each of x64
                            // and x86, plus the EventGhost plugin. 'Debug' is what
                            // names an installer '-dbg' and has it carry the PDBs.
                            // This is the set the signing step will consume.
                            if (env.IS_TAG == 'true' || env.IS_MASTER == 'true') {
                                bat "py -3-64 windows\\create-installer.py -t %WIN_TOOLCHAIN% -m Release -a x64 -v"
                                bat "py -3-64 windows\\create-installer.py -t %WIN_TOOLCHAIN% -m Release -a x86 -v -nn"
                                bat "py -3-64 windows\\create-installer.py -t %WIN_TOOLCHAIN% -m Debug   -a x64 -v"
                                bat "py -3-64 windows\\create-installer.py -t %WIN_TOOLCHAIN% -m Debug   -a x86 -v -nn"
                                // The Release builds write the plugin to build/EventGhost
                                // under a fixed name; dist/ is what gets archived and the
                                // release asset carries the version.
                                bat "copy /y build\\EventGhost\\pulse_eight.egplugin dist\\libcec-eventghost-plugin-${env.LIBCEC_VERSION}.egplugin"
                                bat "dir /b dist"
                            } else {
                                bat "py -3-64 windows\\create-installer.py -t %WIN_TOOLCHAIN% -m Release -a x64 -ne -ni -v"
                            }
                        }
                    }
                    post {
                        success {
                            // dist/ only has content when an installer was built
                            // (master + tags); allowEmptyArchive keeps PR builds green.
                            archiveArtifacts artifacts: 'dist/*.exe, dist/*.egplugin',
                                             allowEmptyArchive: true,
                                             fingerprint: true
                        }
                        cleanup { cleanWs() }
                    }
                }
            }
        }
    }

    post {
        always {
            script {
                def mode = (env.IS_TAG == 'true') ? "release ${env.TAG_NAME}"
                         : (env.IS_MASTER == 'true') ? 'master'
                         : 'branch/PR'
                echo "Build summary: mode=${mode} version=${env.LIBCEC_VERSION}"
            }
        }
    }
}
