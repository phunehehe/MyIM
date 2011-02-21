CFLAGS = -Wall -pedantic -g

.PHONY: all
all: clean main

clean:
	rm -rf build
	mkdir build

main: server.o
	$(CC) -o build/main build/server.o
	chmod u+x build/main

server.o: server.c
	$(CC) -c -o build/server.o server.c
