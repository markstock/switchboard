CC=g++
CFLAGS=-std=c++17 -pedantic -Wall -Wextra -O3

all : switchboard.bin

switchboard.bin : switchboard.cpp lodepng.cpp hpe_autocolor.h
	$(CC) $(CFLAGS) switchboard.cpp lodepng.cpp -o $@

clean :
	rm -f *.o switchboard.bin
