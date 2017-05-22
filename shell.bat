@echo off

IF EXIST w: subst /d w:
subst w: "%cd%"
w:

IF NOT EXIST "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\" echo "You don't have visual studio!"
IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\" call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64

set path=%cd%bin;%path%

cls