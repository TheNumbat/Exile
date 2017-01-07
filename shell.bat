@echo off

IF NOT EXIST w: subst w: "%cd%"
w:

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
set path=%cd%bin;%path%

cls