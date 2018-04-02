@echo off

pushd w:\build

del w:\data\assets\*.asset > NUL 2> NUL
del *.ilk > NUL 2> NUL
del *.dll > NUL 2> NUL
del *.h > NUL 2> NUL
del *.i > NUL 2> NUL
del *.obj > NUL 2> NUL
del *.exe > NUL 2> NUL
del *.pdb > NUL 2> NUL
del *.opensdf > NUL 2> NUL
del *.txt > NUL 2> NUL
del *.exp > NUL 2> NUL
del *.lib > NUL 2> NUL
del *.map > NUL 2> NUL
del *.a1 > NUL 2> NUL
del *.txt > NUL 2> NUl
del w:\build\*.cpp > NUL 2> NUL

popd
