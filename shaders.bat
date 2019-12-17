@echo off

pushd data

for /r %%i in (*.vert) do glslc %%i -o %%~ni.v.spv
for /r %%i in (*.frag) do glslc %%i -o %%~ni.f.spv

for /r %%i in (*.spv) do cp %%~ni.spv ../%1

rm *.spv

popd
