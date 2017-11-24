@echo off

if not exist w:\build mkdir w:\build
pushd w:\build

del *.pdb > NUL 2> NUL

set Game_CompilerFlags=-O2 -MTd -nologo -fp:fast -GR- -EHa- -Oi -W4 -Z7 -FC -Fegame.dll -LD -wd4100 -wd4201 -Iw:\build\ -D_HAS_EXCEPTIONS=0 -DCHECKS -DPROFILE
set Game_LinkerFlags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:windows opengl32.lib -PDB:game_%random%.pdb 

set Platform_CompilerFlags=-O2 -MTd -nologo -fp:fast -GR- -EHa- -W4 -FC -Femain.exe -wd4100 -wd4530 -wd4577
set Platform_LinkerFlags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:console kernel32.lib user32.lib gdi32.lib opengl32.lib 

set Asset_CompilerFlags=-O2 -MTd -nologo -EHsc -Oi -W4 -Z7 -FC -Feasset.exe -wd4100 -Iw:\deps\
set Asset_LinkerFlags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:console

set Meta_CompilerFlags=-O2 -MTd -nologo -EHsc -Oi -W4 -Z7 -FC -Femeta.exe -wd4100 -Iw:\deps\
set Meta_LinkerFlags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:console /LIBPATH:w:\deps\clang-c libclang.lib
 
if not exist w:\build\asset.exe (
	echo compiling asset builder...
	cl %Asset_CompilerFlags% w:\src\tools\asset_builder.cpp /link %Asset_LinkerFlags%
)

if not exist w:\data\assets\assets.asset (
	echo running asset builder...
	asset.exe w:\data\assets\store.txt w:\data\assets\assets.asset
)

if not exist w:\build\meta.exe (
	echo compiling metaprogram...
	cl %Meta_CompilerFlags% w:\src\tools\meta.cpp /link %Meta_LinkerFlags%
)

echo running metaprogram...
xcopy w:\deps\clang-c\libclang.dll w:\build\ /C /Y > NUL 2> NUL
meta.exe w:\src\exile.cpp

echo compiling game lib...
cl %Game_CompilerFlags% w:\src\exile.cpp /link %Game_LinkerFlags%

if not exist w:\build\main.exe (
	echo compiling platform layer...
	cl %Platform_CompilerFlags% w:\src\platform\platform_main.cpp /link %Platform_LinkerFlags%
)

popd
