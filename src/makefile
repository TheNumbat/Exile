
FLAGS= -g3 -std=c++11 -I../deps/include/ -o ../build/CaveGame 
DEFINES= -DASSERTIONS -DEMIT_LEVEL=3 -DLOG_CONSOLE
FILES=  main.cpp gl/gl.cpp glfw/glfw_manager.cpp logger/log.cpp state.cpp chunk/chunk.cpp debug/debug.cpp
LINK= -lglfw -lGL -lGLEW -ltbb -lstdc++fs -ltbbmalloc -pthread 

all:
	g++ $(FLAGS) $(DEFINES) $(FILES) $(LINK)

clean:
	rm -rf *.o
