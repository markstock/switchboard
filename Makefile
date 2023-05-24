CC=g++
CFLAGS=-std=c++11 -pedantic -Wall -Wextra -O3

all : switchboard.bin

%.bin : lodepng.cpp %.cpp
	$(CC) $(CFLAGS) $^ -o $@


