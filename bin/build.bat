@echo off
pushd w:\src\
w:\odin\odin.exe build w:\src\main.odin
del *.ll > NUL 2> NUL
del *.bc > NUL 2> NUL
del *.obj > NUL 2> NUL
xcopy main.exe w:\build\main.exe /q /y
del main.exe
popd
