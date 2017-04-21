@echo off
pushd w:\src\
w:\odin\odin.exe build w:\src\cave.odin
del *.ll > NUL 2> NUL
del *.bc > NUL 2> NUL
del *.obj > NUL 2> NUL
xcopy cave.exe w:\build\cave.exe /q /y
del cave.exe
popd
