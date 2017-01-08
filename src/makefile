
FLAGS= -g3 -std=c++11 -I../deps/include/ -o CaveGame
DEFINES= -DASSERTIONS -DEMIT_LEVEL=3 -DLOG_CONSOLE
FILES=  main.cpp gl/gl.cpp glfw_manager/glfw_manager.cpp logger/log.cpp
LINK= -lglfw -lGL -pthread

all:
	g++ $(FLAGS) $(DEFINES) $(FILES) $(LINK)

clean:
	rm -rf *.o
