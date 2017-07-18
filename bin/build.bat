@echo off

if not exist w:\build mkdir w:\build
pushd w:\build

del *.pdb > NUL 2> NUL

set Game_DebugCompilerFlags=-Od -MTd -nologo -Gr -EHa -Oi -W4 -Z7 -FC -Fegame.dll -LD -wd4100 -wd4201 -Iw:\build\ 
set Game_ReleaseCompilerFlags=-O2 -MT -nologo -Gr -EHa -W4 -FC -Fegame.dll -LD -wd4100 -wd4201 -Iw:\build\
set Game_LinkerFlags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:windows opengl32.lib -PDB:game_%random%.pdb -EXPORT:start_up -EXPORT:main_loop -EXPORT:shut_down -EXPORT:on_reload -EXPORT:on_unload

set Platform_DebugCompilerFlags=-Od -MTd -nologo -Gr -EHa -Oi -W4 -Z7 -FC -Femain.exe -wd4100
set Platform_ReleaseCompilerFlags=-O2 -MT -nologo -Gr -EHa -W4 -FC -Femain.exe -wd4100
set Platform_LinkerFlags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:console kernel32.lib user32.lib gdi32.lib opengl32.lib 

set Asset_CompilerFlags=-O2 -MTd -nologo -Gr -EHa -Oi -W4 -Z7 -FC -Feasset.exe -wd4100 -Iw:\deps\
set Asset_LinkerFlags=/SUBSYSTEM:console

set Meta_CompilerFlags=-O2 -MTd -nologo -Gr -EHa -Oi -W4 -Z7 -FC -Femeta.exe -Iw:\deps\ -wd4100
set Meta_LinkerFlags=/SUBSYSTEM:console /LIBPATH:w:\deps\clang-c libclang.lib

if not exist w:\build\asset.exe (
	echo compiling asset builder...
	cl %Asset_CompilerFlags% w:\src\asset_builder.cpp /link %Asset_LinkerFlags%
)

if not exist w:\data\assets\assets.asset (
	echo running asset builder...
	pushd w:\data\assets
	w:\build\asset.exe store.txt assets.asset
	popd
)

if not exist w:\build\meta.exe (
	echo compiling metaprogram...
	cl %Meta_CompilerFlags% w:\src\meta.cpp /link %Meta_LinkerFlags%
)

echo running metaprogram...
meta.exe w:\src\game.cpp

echo compiling game lib...
if "%1"=="release" (
	cl %Game_ReleaseCompilerFlags% w:\src\game.cpp /link %Game_LinkerFlags%
) else if "%2"=="release" (
	cl %Game_ReleaseCompilerFlags% w:\src\game.cpp /link %Game_LinkerFlags%
) else (
	cl %Game_DebugCompilerFlags% w:\src\game.cpp /link %Game_LinkerFlags%
)

if not exist w:\build\main.exe (
echo compiling platform layer...
	if "%2"=="release" (
		cl %Platform_ReleaseCompilerFlags% w:\src\platform_main.cpp /link %Platform_LinkerFlags%
	) else (
		cl %Platform_DebugCompilerFlags% w:\src\platform_main.cpp /link %Platform_LinkerFlags%
	)
)

popd