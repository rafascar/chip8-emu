/* CHIP-8 EMULATOR */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "chip8.h"
#include "instr.h"
#include "digits.h"

#include "debug.c"

double time_getseconds() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    return (double) ((double)t.tv_sec + (double)t.tv_nsec / 1e9);
}

/* Reset CPU registers and load image file to memory */
void cpu_reset(FILE *file) {
    memset(reg, 0, sizeof(reg));    /* reset all data registers */
    reg_I = 0;                      /* reset address register */
    reg_PC = 0x200;                 /* programs start at 0x200 */
    stack_init();                   /* reset stack */
    memset(key, 0, sizeof(key));    /* reset input keys */
    timer_delay = 0;                /* reset delay timer */
    timer_sound = 0;                /* reset sound timer */

    srand(time(NULL));               /* set random seed for rng */

    /* All hexadecimal digits (0-9, A-F) have corresponding sprite 
     * data already stored in the memory of the interpreter.
     * We chose to store it beggining in address 0x000. */
    memcpy(&memory[FONT], digits, sizeof(digits));

    /* load image file to memory */
    uint8_t *p = &memory[reg_PC];      
    int max_read = 0xFFF - 0x200;    /* maximum file size */
    fread(p, sizeof(uint8_t), max_read, file);
}

void invalid_opcode(uint16_t opcode) {
    fprintf(stderr, "chip8: invalid opcode %04x\n", opcode);
    abort();
}

void init_sdl() {
    /* Initialize SDL with VIDEO subsystem. */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "chip8: SDL_Init error: %s\n", SDL_GetError());
        exit(1);
    }

    /* Create a 64x32 (* FACTOR) borderless window at position (10,30) */
    window = SDL_CreateWindow("CHIP-8 Emulator",
            10, 30, WIDTH * FACTOR, HEIGHT * FACTOR,
            SDL_WINDOW_BORDERLESS);

    /* Initialize the renderer that will draw to the window. */
    renderer = SDL_CreateRenderer(window, -1, 0);
}

/* Update the CPU state. 
 * It executes "cycles" number of instructions. 
 */
void cpu_update(int cycles) {
    while (cycles--) {     
        /* Fetch opcode.
         * CHIP-8 opcodes are 2-bytes, but it has a byte-addressable memory
         * thus, we need to get two consecutive bytes to fech a single 
         * opcode, hence the PC register is incremented twice. 
         * Opcodes are big-endian; so the most significant bits are shifted
         * left and OR'd with the least significant to obtain the full opcode. */
        uint16_t opcode = (uint16_t)memory[reg_PC] << 8 | memory[reg_PC+1];  
        reg_PC = reg_PC + 2;

        /* The first hexadecimal digit of an opcode dictates which instruction 
         * needs to be executed; in some cases, the last hex digit is also 
         * needed. */
        switch (opcode & 0xF000) {
            /* 0NNN (not implemented), 00E0, 00EE */
            case 0x0000:
                switch (opcode & 0x000F) {
                    case 0x0: op_00E0(opcode); break;
                    case 0xE: op_00EE(opcode); break;
                    default:
                        invalid_opcode(opcode);
                        break;
                }
                break;
            /* 1NNN */
            case 0x1000: op_1NNN(opcode); break;
            /* 2NNN */
            case 0x2000: op_2NNN(opcode); break;
            /* 3XNN */
            case 0x3000: op_3XNN(opcode); break;
            /* 4XNN */
            case 0x4000: op_4XNN(opcode); break;
            /* 5XY0 */
            case 0x5000: op_5XY0(opcode); break;
            /* 6XNN */
            case 0x6000: op_6XNN(opcode); break;
            /* 7XNN */
            case 0x7000: op_7XNN(opcode); break;
            /* 8XYN - 8XY0, 8XY1, 8XY2, 8XY3, 8XY4, 8XY5, 8XY6, 8XY7, 8XYE */
            case 0x8000: op_8XYN(opcode); break;
            /* 9XY0 */
            case 0x9000: op_9XY0(opcode); break;
            /* ANNN */
            case 0xA000: op_ANNN(opcode); break;
            /* BNNN */
            case 0xB000: op_BNNN(opcode); break;
            /* CXNN */
            case 0xC000: op_CXNN(opcode); break;
            /* DXYN */
            case 0xD000: op_DXYN(opcode); break;
            /* EX9E, EXA1 */
            case 0xE000:
                switch (opcode & 0x000F) {
                    case 0xE: op_EX9E(opcode); break;
                    case 0x1: op_EXA1(opcode); break;
                    default:
                        invalid_opcode(opcode);
                        break;
                }
                break;
            /* FX07, FX0A, FX18, FX1E, FX29, FX33, FX15, FX55, FX65 */
            case 0xF000:
                switch (opcode & 0x000F) {
                    case 0x0007: op_FX07(opcode); break;
                    case 0x000A: op_FX0A(opcode); break;
                    case 0x0008: op_FX18(opcode); break;
                    case 0x000E: op_FX1E(opcode); break;
                    case 0x0009: op_FX29(opcode); break;
                    case 0x0003: op_FX33(opcode); break;
                    case 0x0005:
                        switch (opcode & 0x00F0) {
                            case 0x0010: op_FX15(opcode); break;
                            case 0x0050: op_FX55(opcode); break;
                            case 0x0060: op_FX65(opcode); break;
                            default:
                                invalid_opcode(opcode);
                                break;
                       }
                        break;
                    default:
                        invalid_opcode(opcode);
                        break;
                }
                break;
            default:
                invalid_opcode(opcode);
                break;
        }
    }
}
    
void render() {
    /* Set the color to BLACK for clearing the screen */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    /* Now draw the pixels from frame_buffer to the screen;
     * as the screen is resized by FACTOR, each pixel is drawn
     * as a rectangle with size (w, h) = (FACTOR, FACTOR),
     * filled with color WHITE. */
    SDL_Rect pixel = { .w = FACTOR, .h = FACTOR }; 
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int x, y;
    for (x = 0; x < WIDTH; x++) {
        for (y = 0; y < WIDTH; y++) {
            if (get_pixel(x, y) == 1) {
                pixel.x = x * FACTOR;
                pixel.y = y * FACTOR;
                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }
    /* Update the screen once all the pixels are rendered. */
    SDL_RenderPresent(renderer);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: ./chip8 file nop6\n");
        exit(1);
    }

    /* Number of opcodes to fetch and execute in each emulation loop. */
    int cycles = 0;
    if (argc == 3)
        cycles = atoi(argv[2]);

    /* get image file from path in arguments and reset cpu */
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "chip8: error opening file (%s)\n", argv[1]);
        exit(1);
    }
    cpu_reset(file);
    fclose(file);

    /* Initialize SDL Window and Renderer. */
    init_sdl();

    /* begin emulation loop */
    int running = 1;
    while (running) { 
        /* Get the time at the beginning of the loop. It will be used later
         * to slow it down to 60Hz */
        double start = time_getseconds();

        /* Handle events on the event queue. It keeps processing the events on
         * the event queue until its empty. */
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            /* SDL_QUITs are generated for a variety of reasons.
             * In MacOS it will be triggered by CMD+Q; in POSIX it will 
             * handle SIGINT and SIGTERM if they are not handled elsewhere. */
            if (e.type == SDL_QUIT) {
                running = 0;
            }
        }

        /* Since we force the emulation loop to run at (approximately) 60Hz,
         * we can use it to  update the timers (decrement if less than zero). */
        if (timer_delay > 0) timer_delay--; 
        if (timer_sound > 0) timer_sound--;

        /* Update the CPU state by "cycles" instructions. This value is arbitrary
         * and must be fiddled with to achieve the right emulation speed.
         * The cycles value is the number of instructions that will execute
         * every 1/60 seconds (16 ms). */
        cpu_update(cycles);

        /* Render the frame_buffer to screen. */
        render();

        /* Force this loop to run at 60Hz (once every 16ms).
         * It does this by sleeping for the time remaining to complete 1/60 seconds
         * since the beginning of the loop; if it took more than 1/60s to run until
         * now: abort.
         * Not a very accurate implementation, but shall do the trick for now. */
        struct timespec rqtp = {
            .tv_sec = 0,
            .tv_nsec = (start + 1.0/60 - time_getseconds()) * 1e9 
        };
        nanosleep(&rqtp, NULL);
    }

    return 0;
}
