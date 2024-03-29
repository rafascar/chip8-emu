#include <stdlib.h>
#include <SDL.h>

/* screen dimensions */
#define WIDTH   64
#define HEIGHT  32
#define FACTOR  10      // draw screen as 640x320 window

/* callstack definitions */
#define LEVELS  12

/* base address for storing hex fonts */
#define FONT    0x000

/* 4K byte-addressable memory */
uint8_t memory[0xFFF];

/* 16 8-bit data registers V0 to VF */
uint8_t reg[16];
/* 16-bit address register I */
uint16_t reg_I;
/* 16-bit program counter PC */
uint16_t reg_PC;

/* 8-bit timers */
uint8_t timer_delay;
uint8_t timer_sound;

/* 64x32 monochrome framebuffer */
uint8_t frame_buffer[WIDTH*HEIGHT];

/* SDL window and renderer handlers */
SDL_Window *window;
SDL_Renderer *renderer;

/* input has 16 keys */
uint8_t keys[16];

/* 12 levels call stack and stack pointer */
uint16_t stack[LEVELS];
uint16_t sp;

void     stack_init();
void     stack_push(uint16_t address);
uint16_t stack_pop();

void invalid_opcode(uint16_t opcode);

