@echo off
pushd w:\src\
w:\odin\odin.exe build w:\src\text.odin
del *.ll > NUL 2> NUL
del *.bc > NUL 2> NUL
del *.obj > NUL 2> NUL
xcopy text.exe w:\build\text.exe /q /y
del text.exe
popd
