from functools import cached_property, reduce
import os
import re
import shutil
import logging
from mixins import exec_command
import time

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('libcec-builder')

def fix_dir_separator(path:str|None) -> str|None:
	if (path is None):
		return None
	return path.replace('/', '\\')

def replace_path_env(path:str) -> str:
	envs = re.findall('%.*?%', path)
	for v in envs:
		evar = os.getenv(v.replace('%',''))
		if (evar is None):
			raise Exception(f"can't find environment variable: {v}")
		path = path.replace(v, evar)
	return path

class PathBuilder:
	def __init__(self, path:'str|PathBuilder|None'=None) -> None:
		if (path is not None) and isinstance(path, PathBuilder):
			self.path:str|None = path.path
		else:
			self.path:str|None = fix_dir_separator(path)
		if self.path is not None:
			while self.path.endswith('\\'):
				self.path = self.path[0:-1]
			self.path = replace_path_env(self.path)

	@cached_property
	def filename(self) -> str:
		return str(self).split('\\')[-1]

	def add(self, *args) -> 'PathBuilder':
		rv = PathBuilder()
		rv.path = self.path
		return rv.__add(*args)

	def is_filtered(self, filtered:list['str|PathBuilder']) -> bool:
		path = str(self)
		for f in filtered:	
			if (str(f) in path):
				return True
			if (str(f) in self.filename):
				return True
			if isinstance(f, str) and (re.search(f, path) is not None):
				return True
		return False

	@cached_property
	def extension(self) -> str:
		return str(self).split('.')[-1]

	@cached_property
	def exists(self) -> bool:
		return os.path.exists(str(self))

	@cached_property
	def is_dir(self) -> bool:
		return os.path.isdir(str(self))

	@cached_property
	def is_file(self) -> bool:
		return os.path.isfile(str(self))

	def mkdir(self) -> str:
		self.clear_cache()
		if self.exists:
			if not self.is_dir:
				raise Exception(f"{self} exists but is not a directory")
			return str(self)

		parent = self.parent()
		if (parent is None):
			raise Exception(f"FAILED: can't find parent for {self}")
		if not parent.exists:
			parent.mkdir()
		rv = os.mkdir(str(self))
		logger.debug(f"created directory {str(self)} = {rv}")
		self.clear_cache()
		if not self.exists:
			raise Exception(f"FAILED: can't create dir {self}")
		return rv

	def delete(self) -> None:
		logger.debug(f"deleting {self}")
		self.clear_cache()
		if self.exists:
			if self.is_dir:
				shutil.rmtree(str(self))
			elif self.is_file:
				os.remove(str(self))
			else:
				raise Exception(f"Can't delete {str(self)}")
			self.clear_cache()
		else:
			logger.debug(f"not deleting {self} - doesn't exist")

	def copy(self, dest_path:'PathBuilder|str') -> None:
		if isinstance(dest_path, str):
			dest_path = PathBuilder(dest_path)
		dest_parent = dest_path.parent()
		if (dest_parent is None):
			raise Exception(f"can't find parent for {dest_path}")
		if not dest_parent.exists:
			dest_parent.mkdir()
		logger.debug(f"copy '{str(self)}' -> {str(dest_path)}")
		if self.is_dir:
			shutil.copytree(str(self), str(dest_path))
			return
		if dest_path.is_dir:
			dest_path = dest_path.add(self.filename)
		shutil.copyfile(str(self), str(dest_path))

	def parent(self) -> 'PathBuilder|None':
		spl = self.path.split('\\')
		spl.pop()
		if (len(spl) == 0):
			return None
		return PathBuilder(path=reduce(lambda x, y: x + '\\' + y, spl))

	@cached_property
	def filename(self) -> str:
		if (self.path is None):
			return ''
		spl = self.path.split('\\')
		return spl[-1]

	def exec(self, args:str|None=None, capture_output:bool=False, hide_output:bool=False) -> tuple[bytes, bytes]|list[str]:
		if not self.is_file:
			raise Exception(f'{str(self)} is not a file')
		cmd = str(self) if (args is None) else f'{str(self)} {args}'
		return exec_command(args=cmd, capture_output=capture_output, hide_output=hide_output)

	def exec_dir(self, cmd:str, capture_output:bool=False, hide_output:bool=False) -> tuple[bytes, bytes]|list[str]:
		if not self.is_dir:
			raise Exception(f'{str(self)} is not a dir')
		return exec_command(args=cmd, capture_output=capture_output, hide_output=hide_output)

	def __add(self, *args) -> 'PathBuilder':
		for path in args:
			self.__add_single(path)
		return self

	def clear_cache(self) -> None:
		if hasattr(self, 'filename'): del self.filename
		if hasattr(self, 'exists'): del self.exists
		if hasattr(self, 'is_dir'): del self.is_dir
		if hasattr(self, 'is_file'): del self.is_file
		if hasattr(self, 'extension'): del self.extension
		if hasattr(self, 'filename'): del self.filename

	def __add_single(self, path:str|None) -> 'PathBuilder':
		if (path is None) or (path == ''):
			return self
		path = fix_dir_separator(path)
		if (self.path is None):
			self.path = path
		else:
			self.path = os.path.join(self.path, path)
		self.clear_cache()
		return self

	def __str__(self) -> str:
		return self.path

	def __repr__(self) -> str:
		return str(self)
