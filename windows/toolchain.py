from enum import Enum
from functools import cached_property
from pathbuilder import PathBuilder

class BuildTarget(Enum):
    Release = 'Release'
    Debug = 'Debug'
    ReleaseWithDebugSymbols = 'RelWithDebInfo'

    @staticmethod
    def default() -> 'BuildTarget':
        return BuildTarget.Release

    @staticmethod
    def as_list() -> list[str]:
        return [m.value for m in BuildTarget]

class Architecture(Enum):
    x64 = 'x64'
    x86 = 'x86'
    arm = 'arm'
    arm64 = 'arm64'

    @staticmethod
    def default() -> 'Architecture':
        return Architecture.x64

    @staticmethod
    def as_list() -> list[str]:
        return [m.value for m in Architecture]

    @property
    def is_arm(self):
        return (self.value == Architecture.arm) or (self.value == Architecture.arm64)

class ToolchainId(Enum):
    VS2015 = '2015'
    VS2017 = '2017'
    VS2017c = '2017c'
    VS2019 = '2019'
    VS2019c = '2019c'
    VS2022 = '2022'
    VS2022c = '2022c'
    VS2026 = '2026'
    VS2026c = '2026c'

    @staticmethod
    def default() -> 'ToolchainId':
        return ToolchainId.VS2022c

    @staticmethod
    def as_list() -> list[str]:
        return [m.value for m in ToolchainId]

class Toolchain:
    def __init__(self, architecture:Architecture, vcvars:str, vcvars_opt:str='') -> None:
        self.architecture = architecture
        self._vcvars = vcvars
        self.vcvars_opt = vcvars_opt

    @cached_property
    def vcvars(self) -> PathBuilder:
        return PathBuilder(self._vcvars)

class ToolchainConfig:
    def __init__(self, id:ToolchainId, name:str, toolchains:dict[str,Toolchain], cmake_use_arch:bool=False) -> None:
        self.id = id
        self.name = name
        self.toolchains = toolchains
        self.cmake_use_arch = cmake_use_arch

class ToolchainConfigs:
    CMAKE = PathBuilder(r"C:\Program Files\CMake\bin\cmake.exe")
    NETCORE = 'net8.0'

    TOOLCHAINS = {
        ToolchainId.VS2015: ToolchainConfig(
            id=ToolchainId.VS2015,
            name='Visual Studio 14 2015',
            toolchains={
                Architecture.x86: Toolchain(
                    architecture=Architecture.x86,
                    vcvars=r'%VS140COMNTOOLS%..\..\VC\bin\vcvars32.bat',
                ),
                Architecture.x64: Toolchain(
                    architecture=Architecture.x64,
                    vcvars=r'%VS140COMNTOOLS%..\..\VC\bin\amd64\vcvars64.bat',
                ),
                Architecture.arm: Toolchain(
                    architecture=Architecture.arm,
                    vcvars=r'%VS140COMNTOOLS%..\..\VC\bin\x86_arm\vcvarsx86_arm.bat',
                ),
            }
        ),
        ToolchainId.VS2017: ToolchainConfig(
            id=ToolchainId.VS2017,
            name='Visual Studio 15 2017',
            toolchains={
                Architecture.x86: Toolchain(
                    architecture=Architecture.x86,
                    vcvars=r'%VS150COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='x86',
                ),
                Architecture.x64: Toolchain(
                    architecture=Architecture.x64,
                    vcvars=r'%VS150COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64',
                ),
                Architecture.arm: Toolchain(
                    architecture=Architecture.arm,
                    vcvars=r'%VS150COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64_arm',
                ),
                Architecture.arm64: Toolchain(
                    architecture=Architecture.arm,
                    vcvars=r'%VS150COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64_arm64',
                ),
            }
        ),
        ToolchainId.VS2017c: ToolchainConfig(
            id=ToolchainId.VS2017c,
            name='Visual Studio 15 2017',
            toolchains={
                Architecture.x86: Toolchain(
                    architecture=Architecture.x86,
                    vcvars=r'%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='x86',
                ),
                Architecture.x64: Toolchain(
                    architecture=Architecture.x64,
                    vcvars=r'%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64',
                ),
                Architecture.arm: Toolchain(
                    architecture=Architecture.arm,
                    vcvars=r'%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64_arm',
                ),
                Architecture.arm: Toolchain(
                    architecture=Architecture.arm64,
                    vcvars=r'%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64_arm64',
                ),
            }
        ),
        ToolchainId.VS2019: ToolchainConfig(
            id=ToolchainId.VS2019,
            name='Visual Studio 16 2019',
            cmake_use_arch=True,
            toolchains={
                Architecture.x86: Toolchain(
                    architecture=Architecture.x86,
                    vcvars=r'%VS160COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='x86',
                ),
                Architecture.x64: Toolchain(
                    architecture=Architecture.x64,
                    vcvars=r'%VS160COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64',
                ),
                Architecture.arm: Toolchain(
                    architecture=Architecture.arm,
                    vcvars=r'%VS160COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64_arm',
                ),
                Architecture.arm64: Toolchain(
                    architecture=Architecture.arm64,
                    vcvars=r'%VS160COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64_arm64',
                ),
            }
        ),
        ToolchainId.VS2019c: ToolchainConfig(
            id=ToolchainId.VS2019c,
            name='Visual Studio 16 2019',
            cmake_use_arch=True,
            toolchains={
                Architecture.x86: Toolchain(
                    architecture=Architecture.x86,
                    vcvars=r'%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='x86',
                ),
                Architecture.x64: Toolchain(
                    architecture=Architecture.x64,
                    vcvars=r'%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64',
                ),
                Architecture.arm: Toolchain(
                    architecture=Architecture.arm,
                    vcvars=r'%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64_arm',
                ),
                Architecture.arm64: Toolchain(
                    architecture=Architecture.arm64,
                    vcvars=r'%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64_arm64',
                ),
            }
        ),
        ToolchainId.VS2022: ToolchainConfig(
            id=ToolchainId.VS2022,
            name='Visual Studio 17 2022',
            cmake_use_arch=True,
            toolchains={
                Architecture.x86: Toolchain(
                    architecture=Architecture.x86,
                    vcvars=r'%VS170COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='x86',
                ),
                Architecture.x64: Toolchain(
                    architecture=Architecture.x64,
                    vcvars=r'%VS170COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64',
                ),
                Architecture.arm: Toolchain(
                    architecture=Architecture.arm,
                    vcvars=r'%VS170COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64_arm',
                ),
                Architecture.arm64: Toolchain(
                    architecture=Architecture.arm64,
                    vcvars=r'%VS170COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64_arm64',
                ),
            }
        ),
        ToolchainId.VS2022c: ToolchainConfig(
            id=ToolchainId.VS2022c,
            name='Visual Studio 17 2022',
            cmake_use_arch=True,
            toolchains={
                Architecture.x86: Toolchain(
                    architecture=Architecture.x86,
                    vcvars=r'%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='x86',
                ),
                Architecture.x64: Toolchain(
                    architecture=Architecture.x64,
                    vcvars=r'%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64',
                ),
                Architecture.arm: Toolchain(
                    architecture=Architecture.arm,
                    vcvars=r'%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64_arm',
                ),
                Architecture.arm64: Toolchain(
                    architecture=Architecture.arm64,
                    vcvars=r'%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64_arm64',
                ),
            }
        ),
        ToolchainId.VS2026: ToolchainConfig(
            id=ToolchainId.VS2026,
            name='Visual Studio 18 2026',
            cmake_use_arch=True,
            toolchains={
                Architecture.x86: Toolchain(
                    architecture=Architecture.x86,
                    vcvars=r'%VS170COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='x86',
                ),
                Architecture.x64: Toolchain(
                    architecture=Architecture.x64,
                    vcvars=r'%VS170COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64',
                ),
                Architecture.arm: Toolchain(
                    architecture=Architecture.arm,
                    vcvars=r'%VS170COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64_arm',
                ),
                Architecture.arm64: Toolchain(
                    architecture=Architecture.arm64,
                    vcvars=r'%VS170COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64_arm64',
                ),
            }
        ),
        ToolchainId.VS2026c: ToolchainConfig(
            id=ToolchainId.VS2026c,
            name='Visual Studio 18 2026',
            cmake_use_arch=True,
            toolchains={
                Architecture.x86: Toolchain(
                    architecture=Architecture.x86,
                    vcvars=r'%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='x86',
                ),
                Architecture.x64: Toolchain(
                    architecture=Architecture.x64,
                    vcvars=r'%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64',
                ),
                Architecture.arm: Toolchain(
                    architecture=Architecture.arm,
                    vcvars=r'%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64_arm',
                ),
                Architecture.arm64: Toolchain(
                    architecture=Architecture.arm64,
                    vcvars=r'%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat',
                    vcvars_opt='amd64_arm64',
                ),
            }
        ),
    }
