@echo off

IF NOT EXIST w:\build mkdir w:\build
pushd w:\build

set DebugCompilerFlags=-Od -MTd -nologo -Gr -EHa -Oi -W4 -Z7 -FC 
set ReleaseCompilerFlags=-O2 -MT -nologo -Gr -EHa -W4 -FC 
set LinkerFlags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:console kernel32.lib user32.lib gdi32.lib opengl32.lib

cl %DebugCompilerFlags% -Fegame.dll w:\src\game_main.cpp -LD /link %LinkerFlags% -PDB:game_%random%.pdb

cl %DebugCompilerFlags% -Femain.exe w:\src\platform_main.cpp /link %LinkerFlags%

popd