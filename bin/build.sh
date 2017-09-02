#!/bin/bash

pushd ../build

# set Game_DebugCompilerFlags=-Od -MTd -nologo -Gr -EHa -Oi -W4 -Z7 -FC -Fegame.dll -LD -wd4100 -wd4201 -Iw:/build/
# set Game_ReleaseCompilerFlags=-O2 -MT -nologo -Gr -EHa -W4 -FC -Z7 -Fegame.dll -LD -wd4100 -wd4201 -Iw:/build/
# set Game_LinkerFlags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:windows opengl32.lib -PDB:game_%random%.pdb -EXPORT:start_up -EXPORT:main_loop -EXPORT:shut_down -EXPORT:on_reload -EXPORT:on_unload

# set Platform_DebugCompilerFlags=-Od -MTd -nologo -Gr -EHa -Oi -W4 -Z7 -FC -Femain.exe -wd4100
# set Platform_ReleaseCompilerFlags=-O2 -MT -nologo -Gr -EHa -W4 -FC -Femain.exe -wd4100
# set Platform_LinkerFlags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:console kernel32.lib user32.lib gdi32.lib opengl32.lib 


# set Meta_CompilerFlags=-O2 -MT -nologo -Gr -EHa -Oi -W4 -Z7 -FC -Femeta.exe -wd4100 -Iw:/deps/
# set Meta_LinkerFlags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:console /LIBPATH:w:/deps/clang-c libclang.lib

if [ ! -f asset ]; then
	echo compiling asset builder
	g++ -O2 -o asset -I../deps/ ../src/asset_builder.cpp
fi

if [ ! -f ../data/assets/assets.asset ]; then
	echo running asset builder
	./asset ../data/assets/store.txt ../data/assets/assets.asset
fi

# if [ ! -f meta ]; then
	echo compiling metaprogram
	g++ -O2 -o meta -I/usr/lib/llvm-4.0/include/ -L/usr/lib/llvm-4.0/lib/ ../src/meta.cpp -lclang 
# fi

echo running metaprogram
./meta ../src/game.cpp

echo compiling game lib...
if [ "$1" == "release" ]; then
	g++ -shared -O2 -o game -I../build/ ../src/game.cpp -pthread -lGL
else 
	g++ -shared -g3 -o game -I../build/ ../src/game.cpp -pthread -lGL
fi

if [ ! -f main ]; then
	echo compiling platform layer...
	if [ "$1" == "release" ]; then
		g++ -g3 -o platform ../src/platform/platform_main.cpp
	else
		g++ -g3 -o platform ../src/platform/platform_main.cpp
	fi
fi

popd