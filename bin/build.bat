@echo off

IF NOT EXIST w:\build mkdir w:\build
pushd w:\build

set CompilerFlags=-Od -MTd -nologo -GR -EHa -Oi -WX -W4 -Z7 -FC
set LinkerFlags=/NODEFAULTLIB:MSVCRT 

set Files=w:\src\main.cpp

cl %CompilerFlags% %Files% /link %LinkerFlags%

popd