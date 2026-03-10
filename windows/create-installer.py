import argparse
from functools import cached_property
from inspect import getsourcefile
import subprocess
from pathbuilder import PathBuilder, replace_path_env
from toolchain import ToolchainConfigs, ToolchainConfig, Toolchain, ToolchainId, BuildTarget, Architecture
from mixins import exec_command, LibVersion
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('libcec-builder')

class BuilderConfig:
    def __init__(self, toolchain:str|ToolchainId, target:str|BuildTarget, architecture:str|Architecture) -> None:
        self._toolchain = toolchain if isinstance(toolchain, ToolchainId) else ToolchainId(toolchain)
        self._target = target if isinstance(target, BuildTarget) else BuildTarget(target)
        self._architecture = architecture if isinstance(architecture, Architecture) else Architecture(architecture)

    @property
    def toolchain_id(self) -> ToolchainId:
        '''Toolchain ID to'''
        return self._toolchain

    @cached_property
    def toolchain_config(self) -> ToolchainConfig:
        return ToolchainConfigs.TOOLCHAINS[self.toolchain_id]

    def cmake_project_type(self, build_type:str='nmake') -> str:
        if (build_type == 'vs'):
            project_type = self.toolchain_config.name
            if not self.toolchain_config.cmake_use_arch:
                if (self.architecture == Architecture.x64):
                    project_type += ' Win64'
                elif (self.architecture == Architecture.x86):
                    project_type += ' Win32'
                elif (self.architecture == Architecture.arm):
                    project_type += ' ARM'
                elif (self.architecture == Architecture.arm64):
                    project_type += ' ARM64'
                else:
                    raise Exception(f'Invalid architecture: {self.architecture}')
            return project_type
        return 'NMake Makefiles'

    def cmake_a_option(self, build_type:str='nmake') -> str:
        if (build_type == 'vs'):
            if not self.toolchain_config.cmake_use_arch:
                if (self.architecture == Architecture.x64):
                    return '-A x64'
                elif (self.architecture == Architecture.x86):
                    return '-A Win32'
                elif (self.architecture == Architecture.arm):
                    return '-A ARM'
                elif (self.architecture == Architecture.arm64):
                    return ' -A ARM64'
                else:
                    raise Exception(f'Invalid architecture: {self.architecture}')
        return ''

    @cached_property
    def toolchain_env(self) -> dict[str,str]:
        rv = subprocess.check_output(f'"{self.toolchain.vcvars}" {self.toolchain.vcvars_opt} && set', shell=True).decode()
        kvp = {}
        for l in rv.split('\n'):
            if l.find('=') < 0:
                continue
            tmp = l.split('=')
            kvp[tmp[0]] = tmp[1].replace('\r', '')
        return kvp

    @cached_property
    def dev_env_dir(self) -> PathBuilder:
        return PathBuilder(self.toolchain_env['DevEnvDir'])

    @cached_property
    def dev_env(self) -> PathBuilder:
        return self.dev_env_dir.add('devenv.com')

    @property
    def cmake_archtitecture_options(self) -> str:
        if (self.architecture == Architecture.x64):
            return '-DWIN64=1'
        if (self.architecture == Architecture.arm64):
            return '-D_M_ARM64=1 -DCMAKE_SYSTEM_VERSION=10.0'
        if (self.architecture == Architecture.arm):
            return '-DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION=10.0'
        if (self.architecture == Architecture.x86):
            return '-DWIN32=1'
        return ''

    @cached_property
    def toolchain(self) -> Toolchain:
        return self.toolchain_config.toolchains[self.architecture]

    @property
    def architecture(self) -> Architecture:
        '''Build architecture (x86/x64/arm64/arm)'''
        return self._architecture

    @property
    def target(self) -> BuildTarget:
        '''Build target (Release/RelWithDebInfo/Debug)'''
        return self._target

    @property
    def is_release(self) -> bool:
        return (self.target == BuildTarget.Release) or (self.target == BuildTarget.ReleaseWithDebugSymbols)

    @property
    def is_debug(self) -> bool:
        return (self.target == BuildTarget.Debug)

    @cached_property
    def script_path(self) -> PathBuilder:
        '''This script'''
        return PathBuilder(getsourcefile(lambda:0))
    
    @cached_property
    def script_dir(self) -> PathBuilder:
        '''Directory of this script'''
        parent = self.script_path.parent()
        if (parent is None):
            raise Exception("Can't find base path")
        return parent

    @cached_property
    def repo_dir(self) -> PathBuilder:
        '''Repository base directory'''
        parent = self.script_dir.parent()
        if (parent is None):
            raise Exception("Can't find base path")
        return parent

    @cached_property
    def build_dir(self) -> PathBuilder:
        '''C/C++ library build directory'''
        return self.repo_dir.add('build')

    @cached_property
    def dotnet_build_dir(self) -> PathBuilder:
        '''.Net build directory'''
        return self.repo_dir.add('src/dotnet/build')

    def dump(self) -> None:
        logger.info("============================================================")
        logger.info(f'Target:               {self.target.value}')
        try:
            logger.info(f'Toolchain:            {self.toolchain_config.name}')
        except:
            raise Exception(f"Invalid toolchain id: {self.toolchain_id.value}")

        if not self.architecture in self.toolchain_config.toolchains:
            raise Exception(f"Invalid architecture for toolchain: {self.architecture.value}")

        try:
            self.toolchain_env
        except:
            raise Exception(f"Invalid toolchain id: {self.toolchain_id.value}/{self.architecture.value} is not installed")

        logger.info(f'Architecture:         {self.architecture.value}')
        logger.info(f'Build Directory:      {self.build_dir}')
        logger.info(f'.Net Build Directory: {self.dotnet_build_dir}')
        logger.info(f"DevEnvDir:            {self.dev_env_dir}")
        logger.info("============================================================")

class CMakeBuilder:
    def __init__(self, config:BuilderConfig, build_dir:str|PathBuilder, build_type:str='nmake', static_lib:bool=False, check_results:list[str]=[]) -> None:
        self.config = config
        self.build_type = build_type
        self.static_lib = static_lib
        self.build_dir = build_dir
        self._check_results = check_results

    @cached_property
    def target_dir(self) -> PathBuilder:
        return self.config.build_dir.add(f'{self.config.target.value}/{self.config.architecture.value}')

    @cached_property
    def gen_dir(self) -> PathBuilder:
        return self.target_dir.add('cmake').add(self.build_dir.filename)

    def needs_compilation(self) -> bool:
        if (len(self._check_results) == 0):
            return True
        try:
            self.check_results()
            logger.debug(f"all files exist {self._check_results}")
            return False
        except:
            return True

    def check_results(self) -> None:
        for f in self._check_results:
            f = self.target_dir.add(f)
            f.clear_cache()
            if not f.exists:
                raise Exception(f"file {f.path} does not exist")

    def clean(self) -> None:
        self.target_dir.delete()

    def prepare(self) -> None:
        self.gen_dir.mkdir()

    def compile(self) -> list[str]:
        self.prepare()
        cmake = PathBuilder(ToolchainConfigs.CMAKE)
        skip_python = f'-DSKIP_PYTHON_WRAPPER={1 if self.config.is_debug else 0}'
        build_shared = f'-DBUILD_SHARED_LIBS={0 if self.static_lib else 1}'
        overrides = self.config.repo_dir.add(r'support\windows\cmake')
        compile_cmd = ' && nmake install' if (self.build_type == 'nmake') else ''
        cmd_args = f'-G "{self.config.cmake_project_type(build_type=self.build_type)}" {self.config.cmake_a_option(build_type=self.build_type)} ' + \
            f'-DCMAKE_BUILD_TYPE={self.config.target.value} ' + \
            f'-DCMAKE_USER_MAKE_RULES_OVERRIDE="{overrides.add("c-flag-overrides.cmake")}" ' + \
            f'-DCMAKE_USER_MAKE_RULES_OVERRIDE_CXX="{overrides.add("cxx-flag-overrides.cmake")}" ' + \
            f'-DCMAKE_INSTALL_PREFIX="{self.target_dir}" ' + \
            f'-DCMAKE_INCLUDE_PATH="{self.target_dir.add('include')}" ' + \
            f'{skip_python} {build_shared} {self.config.cmake_archtitecture_options} {self.build_dir}'
        cmd = f'"{self.config.toolchain.vcvars}" {self.config.toolchain.vcvars_opt} && "{cmake}" {cmd_args} {compile_cmd}'
        return exec_command(cmd, cwd=str(self.gen_dir), capture_output=True)

class PlatformBuilder:
    def __init__(self, config:BuilderConfig) -> None:
        self.config = config
        self.builder = CMakeBuilder(config=self.config, build_dir=self.config.repo_dir.add(r'src\platform'), static_lib=True, check_results=['lib/p8-platform.lib'])

    @property
    def libfile(self) -> PathBuilder:
        return self.builder.target_dir.add('lib/p8-platform.lib')

    def check_submodule(self) -> None:
        platform_src = self.config.repo_dir.add(r'src\platform\README.md')
        if not platform_src.exists:
            self.config.repo_dir.exec_dir('git submodule update --init -r', hide_output=True)
            platform_src = self.config.repo_dir.add(r'src\platform\README.md')
        if not platform_src.exists:
            raise Exception(f'platform git submodule has not been checked out: {platform_src.parent()} not found')

    def clean(self) -> None:
        self.builder.clean()

    def build(self) -> None:
        self.check_submodule()
        if (not self.builder.needs_compilation()):
            logger.debug(f"* skipping platform library {self.config.target.value} for {self.config.architecture.value}")
            return

        logger.info(f"* compiling platform library {self.config.target.value} for {self.config.architecture.value}")
        rv = []
        try:
            rv = self.builder.compile()
            self.builder.check_results()
        except Exception as e:
            logger.warning(e)
            for line in rv:
                print(line)
            raise Exception('Failed to build platform library')

class LibCecLibBuilder:
    def __init__(self, config:BuilderConfig, buildType:str='nmake', staticlib:bool=False) -> None:
        self.config = config
        self.buildType = buildType
        self.staticlib = staticlib
        self.platform_builder = PlatformBuilder(config=config)
        self.builder = CMakeBuilder(config=self.config, build_dir=self.config.repo_dir, static_lib=self.staticlib, build_type=buildType, check_results=[self.libfile_name])

    def clean(self) -> None:
        self.platform_builder.clean()
        self.builder.clean()

    def build(self) -> None:
        self.platform_builder.build()

        if (self.buildType != 'vs') and (not self.builder.needs_compilation()):
            logger.debug(f"* skipping {'static ' if self.staticlib else ''}libCEC C/C++/Python {self.config.target.value} for {self.config.architecture.value}")
            return

        logger.info(f"* compiling {'static ' if self.staticlib else ''}libCEC C/C++/Python {self.config.target.value} for {self.config.architecture.value}")
        rv = []
        try:
            rv = self.builder.compile()
            if (self.buildType != 'vs'):
                self.builder.check_results()
            else:
                logger.info(f"* project files generated in {self.builder.gen_dir}")
        except Exception as e:
            logger.warning(e)
            for line in rv:
                print(line)
            raise Exception('Failed to build cec library')

    @property
    def platform_libfile(self) -> PathBuilder:
        return self.platform_builder.libfile

    @property
    def libfile_name(self) -> str:
        return 'cec.lib' if self.staticlib else 'cec.dll'

    @property
    def libfile(self) -> PathBuilder:
        return self.builder.target_dir.add(self.libfile_name)

    @property
    def version(self) -> LibVersion|None:
        try:
            import version
            return version.LibcecVersion()
        except:
            return None

class CecSharpBuilder:
    def __init__(self, config:BuilderConfig) -> None:
        self.config = config

    @cached_property
    def lib_file(self) -> PathBuilder:
        return self.config.build_dir.add(f'{self.config.target.value}/{self.config.architecture.value}/LibCecSharp.dll')

    @cached_property
    def core_lib_file(self) -> PathBuilder:
        return self.config.build_dir.add(f'{self.config.target.value}/{self.config.architecture.value}/{ToolchainConfigs.NETCORE}/LibCecSharpCore.dll')

    def needs_compilation(self) -> bool:
        return not self.lib_file.exists or not self.core_lib_file.exists

    def clean(self) -> None:
        self.lib_file.delete()
        self.core_lib_file.delete()

    def build(self) -> None:
        if not self.needs_compilation():
            logger.debug(f"* skipping libCEC .Net {self.config.target.value} for {self.config.architecture.value}")
            return

        logger.info(f"* compiling libCEC .Net {self.config.target.value} for {self.config.architecture.value}")
        cwd = self.config.repo_dir.add('project')
        cmds = replace_path_env(f'"{self.config.toolchain.vcvars}" {self.config.toolchain.vcvars_opt} && "{self.config.dev_env}" libcec.sln /Clean "{self.config.target.value}|{self.config.architecture.value}" && "{self.config.dev_env}" libcec.sln /Build "{self.config.target.value}|{self.config.architecture.value}"')
        outbuf = exec_command(cmds, cwd=str(cwd), capture_output=True)
        if not self.config.build_dir.add(f'{self.config.target.value}/{self.config.architecture.value}/LibCecSharp.dll').exists or \
            not self.config.build_dir.add(f'{self.config.target.value}/{self.config.architecture.value}/{ToolchainConfigs.NETCORE}/LibCecSharpCore.dll').exists:
            for line in outbuf:
                print(line)
            raise Exception("Failed to compile libCEC .Net")

class CecSharpApps:
    def __init__(self, config:BuilderConfig) -> None:
        self.config = config

    @cached_property
    def cec_tray(self) -> PathBuilder:
        return self.config.build_dir.add(f'{self.config.target.value}/{self.config.architecture.value}/cec-tray.exe')

    @cached_property
    def core_tester(self) -> PathBuilder:
        return self.config.build_dir.add(f'{self.config.target.value}/{self.config.architecture.value}/{ToolchainConfigs.NETCORE}/CecSharpCoreTester.exe')

    def clean(self) -> None:
        self.cec_tray.delete()
        self.core_tester.delete()

    def needs_compilation(self) -> bool:
        return not self.core_tester.exists or not self.cec_tray.exists

    def build(self) -> None:
        if not self.needs_compilation():
            logger.debug(f"* skipping libCEC .Net {self.config.target.value} Apps for {self.config.architecture.value}")
            return

        logger.info(f"* compiling libCEC .Net {self.config.target.value} Apps for {self.config.architecture.value}")
        cwd = self.config.repo_dir.add('src/dotnet/project')
        cmds = f'"{self.config.toolchain.vcvars}" {self.config.toolchain.vcvars_opt} && msbuild -t:restore'
        exec_command(cmds, hide_output=True, cwd=str(cwd))

        cmds = replace_path_env(f'"{self.config.toolchain.vcvars}" {self.config.toolchain.vcvars_opt} && "{self.config.dev_env}" cec-dotnet.sln /Build "{self.config.target.value}|{self.config.architecture.value}"')
        outbuf = exec_command(cmds, cwd=str(cwd), capture_output=True)
        if not self.config.build_dir.add(f'{self.config.target.value}/{self.config.architecture.value}/cec-tray.exe').exists or \
            not self.config.build_dir.add(f'{self.config.target.value}/{self.config.architecture.value}/{ToolchainConfigs.NETCORE}/CecSharpCoreTester.exe').exists:
            for line in outbuf:
                print(line)
            raise Exception("Failed to compile libCEC .Net Apps")

class NsisBuilder:
    def __init__(self, config:BuilderConfig, project:PathBuilder, options:str=''):
        self.config = config
        self.project = project
        self.options = options
        self.nsis = PathBuilder(r'%ProgramFiles%\NSIS\makensis.exe')
        if not self.nsis.exists:
            self.nsis = PathBuilder(r'%ProgramFiles(x86)%\NSIS\makensis.exe')
        if not self.nsis.exists:
            raise Exception("nsis not found")

    def build(self) -> tuple[bytes, bytes]|list[str]:
        self.config.repo_dir.add(r'support\windows\p8-usbcec-driver-installer.exe').copy(self.config.build_dir.add('p8-usbcec-driver-installer.exe'))
        self.config.repo_dir.add(r'support\windows\p8-usbcec-bootloader-driver-installer.exe').copy(self.config.build_dir.add('p8-usbcec-bootloader-driver-installer.exe'))
        self.config.repo_dir.add(r'support\windows\libusb0.dll').copy(self.config.build_dir.add('libusb0.dll'))
        self.config.build_dir.add('ref').delete()
        env = self.config.repo_dir.add('project')
        return exec_command(f'"{self.nsis}" /V1 {self.options} {self.project}', cwd=str(env))

class EventGhost:
    def __init__(self, config:BuilderConfig, libcec:LibCecLibBuilder) -> None:
        self.config = config
        self.libcec = libcec

    def _libcec_x86(self) -> None:
        if (self.libcec.config.architecture == Architecture.x86):
            return
        config = BuilderConfig(toolchain=self.config.toolchain_id, target=self.config.target, architecture=Architecture.x86)
        libcec = LibCecLibBuilder(config=config)
        libcec.build()
        self.libcec = libcec

    @property
    def build_dir(self) -> PathBuilder:
        return self.config.build_dir.add('EventGhost')

    @property
    def plugin_build_dir(self) -> PathBuilder:
        return self.build_dir.add('egplugin_sources')

    @property
    def plugin(self) -> PathBuilder:
        return self.build_dir.add('pulse_eight.egplugin')

    def clean(self) -> None:
        self.libcec.clean()
        self.build_dir.delete()

    def prepare(self) -> None:
        self.build_dir.mkdir()
        self._libcec_x86()
        plg_sources = self.config.repo_dir.add('src/eventghost/egplugin_sources')
        plg_sources.copy(self.build_dir.add('egplugin_sources'))
        plg_sources = self.build_dir.add('egplugin_sources')
        cec_sources = self.build_dir.add('PulseEight/cec')
        self.libcec.builder.target_dir.add('python/cec/cec.py').copy(cec_sources)
        self.libcec.builder.target_dir.add('python/cec/_pycec.pyd').copy(cec_sources)
        self.libcec.builder.target_dir.add('python/cec/__init__.py').copy(cec_sources)
        self.libcec.libfile.copy(cec_sources)

    def build(self) -> None:
        logger.info("* creating EventGhost plugin")
        self.prepare()
        cmd = f'PowerShell -ExecutionPolicy ByPass -Command "Add-Type -Assembly System.IO.Compression.FileSystem;[System.IO.Compression.ZipFile]::CreateFromDirectory(\'{str(self.plugin_build_dir)}\', \'{str(self.plugin)}\', [System.IO.Compression.CompressionLevel]::Optimal, $false)"'
        exec_command(cmd, cwd=str(self.build_dir))
        if not self.plugin.exists:
            raise Exception(f"Failed to create EventGhost plugin {self.plugin}")

class LibCecInstallerBuilder:
    def __init__(self, toolchain:str|ToolchainId, target:str|BuildTarget, architecture:str|Architecture, installer:bool, clean:bool, eventghost:bool, visual_studio:bool) -> None:
        self.config = BuilderConfig(toolchain=toolchain, target=target, architecture=architecture)
        self._installer = installer
        self._clean = clean
        self._eventghost = eventghost if self.config.is_release else False
        self._visual_studio = visual_studio
        self.libcec = LibCecLibBuilder(config=self.config, buildType=('vs' if visual_studio else 'nmake'))

    def sign_binaries(self) -> None:
        try:
            import codesigner
            codesigner.sign_libcec()
        except:
            logger.info("* not signing binaries")
            return

    @property
    def _options(self) -> str:
        opts = '/DNSISDOTNETAPPS' if self._eventghost else ''
        if self.config.is_debug:
            opts += ' /DNSISINCLUDEPDB'
        if (self.config.architecture == Architecture.x86):
            opts += ' /DNSIS_X86'
        return opts

    @cached_property
    def installer_file(self) -> PathBuilder:
        version = self.libcec.version
        if (version is None):
            raise Exception("Can't detect libCEC version")
        dbg = '-dbg' if self.config.is_debug else ''
        return self.config.repo_dir.add(f'dist/libcec-{self.config.architecture.value}{dbg}-{str(version)}.exe')

    def create_installer(self) -> None:
        self.sign_binaries()
        logger.info(f"* creating {self.installer_file}")
        self.config.repo_dir.add('dist').mkdir()
        rv = NsisBuilder(config=self.config, project=self.config.repo_dir.add('project/libCEC.nsi'), options=self._options).build()
        if (not self.installer_file.exists):
            for line in rv:
                print(line)
            raise Exception('Failed to create installer')

    def build(self) -> None:
        self.config.dump()

        cecsharp = CecSharpBuilder(config=self.config)
        cecsharpapps = CecSharpApps(config=self.config)
        eventghost = EventGhost(config=self.config, libcec=self.libcec)

        if self._eventghost or self._clean:
            logger.info("* cleaning build files")

        if self._eventghost:
            eventghost.clean()
        if self._clean:
            self.config.repo_dir.add('dist').delete()
            self.config.build_dir.delete()
            self.libcec.clean()
            self.libcec.staticlib = True
            self.libcec.clean()
            self.libcec.staticlib = False
            cecsharp.clean()
            cecsharpapps.clean()

        self.libcec.build()
        if not self._visual_studio:
            self.libcec.staticlib = True
            self.libcec.build()
            cecsharp.build()
            self.libcec.staticlib = False
            cecsharpapps.build()

            if self._eventghost:
                eventghost.build()

            if self._installer:
                self.create_installer()

if __name__ == '__main__':
    argparser = argparse.ArgumentParser(description="libCEC Windows Builder")
    argparser.add_argument('-t', '--toolchain',  dest='toolchain', help='Toolchain ID', choices=ToolchainId.as_list(), default=ToolchainId.default(), required=False)
    argparser.add_argument('-m', '--mode', dest='mode', help='Build Mode', choices=BuildTarget.as_list(), default=BuildTarget.default(), required=False)
    argparser.add_argument('-a', '--arch', dest='arch', help='Build Architecture', choices=Architecture.as_list(), default=Architecture.default(), required=False)
    argparser.add_argument('-nc', '--no-clean', dest='no_clean', help="Don't clean before compiling (skips existing binaries)", action=argparse.BooleanOptionalAction)
    argparser.add_argument('-ne', '--no-eventghost', dest='no_eventghost', help="Don't create the EventGhost plugin", action=argparse.BooleanOptionalAction)
    argparser.add_argument('-ni', '--no-installer', dest='no_installer', help="Don't create an installer", action=argparse.BooleanOptionalAction)
    argparser.add_argument('-vs', dest='visual_studio', help="Create Visual Studio projects", action=argparse.BooleanOptionalAction)
    args = argparser.parse_args()
    installer = LibCecInstallerBuilder(
        toolchain=args.toolchain,
        target=args.mode,
        architecture=args.arch,
        installer=(args.no_installer is None),
        clean=(args.no_clean is None),
        eventghost=(args.no_eventghost is None),
        visual_studio=(args.visual_studio is not None))
    installer.build()
