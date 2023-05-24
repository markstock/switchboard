CC=g++
CFLAGS=-ansi -pedantic -Wall -Wextra -O3

all : switchboard.bin

%.bin : lodepng.cpp %.cpp
	$(CC) $(CFLAGS) $^ -o $@


