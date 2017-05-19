@echo off

IF NOT EXIST w:\build mkdir w:\build
pushd w:\build

set CompilerFlags=-Od -MTd -nologo -GR -EHa -Oi -W4 -Z7 -FC 
set LinkerFlags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:console kernel32.lib user32.lib gdi32.lib opengl32.lib

set Files=w:\src\main.cpp

cl %CompilerFlags% %Files% /link %LinkerFlags%

popd