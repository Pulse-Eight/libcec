// Jenkinsfile for libCEC
//
// Behaviour:
//   PR / branch push          → build on Linux + Windows, no artefacts kept.
//   master push               → same, plus a complete Windows installer, archived.
//   Tag push (libcec-<x.y.z>) → version cross-check against CMakeLists.txt, then
//                               the same complete installer for both Windows
//                               architectures, archived.
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
                            podman run --rm \\
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
                                    cmake .. -DCMAKE_BUILD_TYPE=Release
                                    make -j"$(nproc)"
                                    ./src/cec-client/cec-client --help 2>&1 | head -20
                                    ldd ./src/libcec/libcec.so | head -20
                                '
                        '''
                    }
                    post {
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
                            // master and tags produce a complete installer: every
                            // component the NSIS script can package, so what is
                            // archived is what a release ships. Tags additionally
                            // build x86. This is the set the signing step will consume.
                            if (env.IS_TAG == 'true') {
                                bat "py -3-64 windows\\create-installer.py -t %WIN_TOOLCHAIN% -m Release -a x64"
                                bat "py -3-64 windows\\create-installer.py -t %WIN_TOOLCHAIN% -m Release -a x86"
                            } else if (env.IS_MASTER == 'true') {
                                bat "py -3-64 windows\\create-installer.py -t %WIN_TOOLCHAIN% -m Release -a x64"
                            } else {
                                bat "py -3-64 windows\\create-installer.py -t %WIN_TOOLCHAIN% -m Release -a x64 -ne -ni"
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
