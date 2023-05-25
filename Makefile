CC=g++
CFLAGS=-std=c++11 -pedantic -Wall -Wextra -O3

all : switchboard.bin

switchboard.bin : switchboard.cpp lodepng.cpp ryb_autocolor.h
	$(CC) $(CFLAGS) switchboard.cpp lodepng.cpp -o $@


