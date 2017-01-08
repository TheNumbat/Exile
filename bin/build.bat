@echo off

set CommonCompilerFlags=-Od -MTd -nologo -GR -EHa -Zo -Oi -WX -W4 -FC -Z7 -I..\deps\include\ -Fe:CaveGame.exe
set CommonCompilerFlags= -DASSERTIONS -DEMIT_LEVEL=3 -DLOG_CONSOLE %CommonCompilerFlags% 
set CommonLinkerFlags= -LIBPATH:w:\deps\lib\ /NODEFAULTLIB:MSVCRT -incremental:no -opt:ref opengl32.lib glfw3dll.lib

set FILES= w:\src\main.cpp w:\src\glfw_manager\glfw_manager.cpp w:\src\logger\log.cpp w:\src\gl\gl.cpp

IF NOT EXIST w:\build mkdir w:\build
pushd w:\build

cl %CommonCompilerFlags% %FILES% /link %CommonLinkerFlags%

popd
