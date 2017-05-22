@echo off

IF EXIST w: subst /d w:
subst w: "%cd%"
w:

IF EXIST "D:\VBS\VC\Auxiliary\Build" (
	call "D:\VBS\VC\Auxiliary\Build\vcvarsall.bat" x64 
) ELSE IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\" (
	call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64 
)

set path=%cd%bin;%path%
