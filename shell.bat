@echo off

IF EXIST w: subst /d w:
subst w: "%cd%"
w:

IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" (
	call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64 
) ELSE IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\Shared\14.0\VC\vcvarsall.bat" (
	call "C:\Program Files (x86)\Microsoft Visual Studio\Shared\14.0\VC\vcvarsall.bat" 
) ELSE IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat" (
	call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"
) ELSE IF EXIST "D:\VBS\VC\Auxiliary\Build\vcvarsall.bat" (
	call "D:\VBS\VC\Auxiliary\Build\vcvarsall.bat" x64 
)

set path=%cd%bin;%cd%build;%path%
