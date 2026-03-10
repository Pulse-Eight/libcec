import subprocess

cmd = r'"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64 && set'
rv = subprocess.check_output(cmd, shell=True).decode()
kvp = {}
for l in rv.split('\n'):
    if l.find('=') < 0:
        continue
    tmp = l.split('=')
    kvp[tmp[0]] = tmp[1].replace('\r', '')
print(kvp)
