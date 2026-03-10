@echo off

python create-installer.py -m Release -a x64
python create-installer.py -m Debug -a x64 -nc
python create-installer.py -m Release -a x86 -nc
python create-installer.py -m Debug -a x86 -nc
