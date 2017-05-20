@echo off

IF NOT EXIST w:\build mkdir w:\build
pushd w:\build

del *.pdb > NUL 2> NUL

set Game_DebugCompilerFlags=-Od -MTd -nologo -Gr -EHa -Oi -W4 -Z7 -FC -Fegame.dll -LD -DDEBUG -wd4100
set Game_ReleaseCompilerFlags=-O2 -MTd -nologo -Gr -EHa -W4 -FC -Fegame.dll -LD -wd4100
set Game_LinkerFlags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:windows opengl32.lib -PDB:game_%random%.pdb -EXPORT:start_up -EXPORT:main_loop -EXPORT:shut_down -EXPORT:on_load -EXPORT:on_unload

set Platform_DebugCompilerFlags=-Od -MTd -nologo -Gr -EHa -Oi -W4 -Z7 -FC -Femain.exe -DDEBUG -wd4100
set Platform_ReleaseCompilerFlags=-O2 -MTd -nologo -Gr -EHa -W4 -FC -Femain.exe -wd4100
set Platform_LinkerFlags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:console kernel32.lib user32.lib gdi32.lib opengl32.lib 

cl %Game_DebugCompilerFlags% w:\src\game_main.cpp /link %Game_LinkerFlags%

if "%1"=="platform" goto platform
goto :eof

:platform
cl %Platform_DebugCompilerFlags% w:\src\platform_main.cpp /link %Platform_LinkerFlags%

popd