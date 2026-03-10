from abc import ABC, abstractmethod
import subprocess
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('libcec-builder')

def exec_command(args:str|list[str], capture_output:bool=False, hide_output:bool=False, cwd:str|None=None) -> tuple[bytes, bytes]|list[str]:
	pcwd = f', cwd={str(cwd)}' if cwd is not None else ''
	logger.debug(f"execute {args}{pcwd}")
	if capture_output:
		result = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, bufsize=1, text=True, cwd=str(cwd))
		outbuf = []
		for line in result.stdout:
			outbuf.append(line.replace('\r', '').replace('\n', ''))
			result.poll()
		return outbuf
	outp = None if not hide_output else subprocess.DEVNULL
	if (cwd is not None) and len(str(cwd)) > 240:
		raise Exception(f"cwd path too long: {cwd}")
	return subprocess.Popen(args, stdout=outp, cwd=cwd).communicate()

class LibVersion(ABC):
    @property
    @abstractmethod
    def major(self) -> int:
        pass

    @property
    @abstractmethod
    def minor(self) -> int:
        pass

    @property
    @abstractmethod
    def patch(self) -> int:
        pass

    def __str__(self):
        return f"{self.major}.{self.minor}.{self.patch}"