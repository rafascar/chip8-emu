all:
	gcc -Wall -g -o chip8 chip8.c instr.c stack.c `sdl2-config --cflags --libs`
