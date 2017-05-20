@echo off

IF NOT EXIST w:\build mkdir w:\build
pushd w:\build

del *.pdb > NUL 2> NUL

set Game_DebugCompilerFlags=-Od -MTd -nologo -Gr -EHa -Oi -W4 -Z7 -FC -Fegame.dll -LD -DDEBUG 
set Game_ReleaseCompilerFlags=-O2 -MTd -nologo -Gr -EHa -W4 -FC -Fegame.dll -LD -EXPORT:start_up 
set Game_LinkerFlags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:console opengl32.lib -PDB:game_%random%.pdb -EXPORT:start_up -EXPORT:main_loop -EXPORT:shut_down

set Platform_DebugCompilerFlags=-Od -MTd -nologo -Gr -EHa -Oi -W4 -Z7 -FC -Femain.exe -DDEBUG
set Platform_ReleaseCompilerFlags=-O2 -MTd -nologo -Gr -EHa -W4 -FC -Femain.exe
set Platform_LinkerFlags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:console kernel32.lib user32.lib gdi32.lib opengl32.lib 

cl %Game_DebugCompilerFlags% w:\src\game_main.cpp /link %Game_LinkerFlags%

if "%1"=="platform" goto platform
goto :eof

:platform
cl %Platform_DebugCompilerFlags% w:\src\platform_main.cpp /link %Platform_LinkerFlags%

popd