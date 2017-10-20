#!/bin/bash

if [ ! -f asset ]; then
	echo compiling asset builder
	g++ -O2 -o asset -I../deps/ ../src/tools/asset_builder.cpp
fi

if [ ! -f ../data/assets/assets.asset ]; then
	echo running asset builder
	./asset ../data/assets/store.txt ../data/assets/assets.asset
fi

if [ ! -f meta ]; then
	echo compiling metaprogram
	g++ -O2 -o meta -I../deps/ -I/usr/lib/llvm-4.0/include/ -L/usr/lib/llvm-4.0/lib/ ../src/tools/meta.cpp -lclang 
fi

echo running metaprogram
./meta ../src/exile.cpp

echo compiling game lib...
if [ "$1" == "release" ]; then
	g++ -shared -O2 -fPIC -o game.so -I../build/ ../src/exile.cpp -pthread -lGL -fpermissive -Wno-attributes -Wno-invalid-offsetof
else 
	g++ -shared -g3 -fPIC -o game.so -I../build/ ../src/exile.cpp -pthread -lGL -fpermissive -Wno-attributes -Wno-invalid-offsetof
fi 

if [ ! -f main ]; then
	echo compiling platform layer...
	if [ "$1" == "release" ]; then
		g++ -O2 -o main ../src/platform/platform_main.cpp -Wno-attributes -lSDL -lGL
	else
		g++ -g3 -o main ../src/platform/platform_main.cpp -Wno-attributes -lSDL -lGL 
	fi
fi
