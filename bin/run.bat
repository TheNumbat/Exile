@echo off

IF NOT EXIST w:\data mkdir w:\data
pushd w:\data

set path=w:\deps\lib;%path%

w:\build\CaveGame.exe

popd