CXXFLAGS = -Wall -pedantic -g

.PHONY: all
all: clean main

main: server.o
	gcc -o build/main build/server.o
	chmod u+x build/main
clean:
	rm -rf build
	mkdir build
server.o: server.c
	gcc -c -o build/server.o server.c
