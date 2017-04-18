@echo off

IF NOT EXIST w: subst w: "%cd%"
w:

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
set path=%cd%bin;%path%

cls