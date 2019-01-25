#include <stdlib.h>

#define WIDTH   64
#define HEIGHT  32

/* 4K byte-addressable memory */
uint8_t memory[0xFFF];

/* 16 8-bit data registers V0 to VF */
uint8_t reg[16];
/* 16-bit address register I */
uint16_t reg_I;
/* 16-bit program counter PC */
uint16_t reg_PC;

/* 64x32 monochrome framebuffer */
uint8_t frame_buffer[64*32];
