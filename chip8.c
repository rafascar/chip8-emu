/* CHIP-8 EMULATOR */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

#include "chip8.h"
#include "instr.h"
#include "digits.h"


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
    memset(keys, 0, sizeof(keys));  /* reset input keys */
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

SDL_AudioSpec audio_desired, audio_obtained;
SDL_AudioDeviceID audio_devid;

int samples_per_second = 0;     // obtained sample rate to be filled after opening audio device
int sample_nr = 0;              // used to keep track of the current sample


/* userdata: application-specific paremeter saved in userdata field
 * stream  : pointer to the audio data buffer
 * len     : lenfgt of that buffer in bytes
 */
void mix_audio(void* userdata, uint8_t *stream, int len) {
    int i;
    for (i = 0; i < len; i++, sample_nr++) {
        /* Calculate the time for the next sample. */ 
        double t = (double)sample_nr / (double)samples_per_second; 
        /* Calculate a single sample for time t. */
        int8_t sample = 127 * cos(2.0 * M_PI * 440.0 * t);  // 440Hz sine wave
        /* Fill the audio buffer with the calculated sample. */
        stream[i] = sample;
    }
}

void init_sdl() {
    /* Initialize SDL with VIDEO and AUDIO subsystems. */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "chip8: SDL_Init error: %s\n", SDL_GetError());
        exit(1);
    }

    /* Create a 64x32 (* FACTOR) borderless window at position (10,30) */
    window = SDL_CreateWindow("CHIP-8 Emulator",
            10, 30, WIDTH * FACTOR, HEIGHT * FACTOR,
            SDL_WINDOW_BORDERLESS);

    /* Initialize the renderer that will draw to the window. */
    renderer = SDL_CreateRenderer(window, -1, 0);

    /* Configure audio parameters. */
    audio_desired.freq = 44100;         // samples per second 44,100 Hz
    audio_desired.format = AUDIO_S8;    // 8-bit bit depth (-128 to 127)
    audio_desired.channels = 1;         // mono
    audio_desired.samples = 4096;       // size of the audio buffer in sample frames
    audio_desired.callback = mix_audio;
    audio_desired.userdata = NULL;

    audio_devid = SDL_OpenAudioDevice(NULL, 0, &audio_desired, &audio_obtained, 0);
    if (audio_devid == 0) {
        fprintf(stderr, "chip8: SDL_OpenAudioDevice error %s\n", SDL_GetError());
        exit(1);
    }
    samples_per_second = audio_obtained.freq;
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

void keys_update() {
    /* Get a snapshot of the current state of the keyboard
     * and update keys[16] with it. */
    const uint8_t *keyboard_state = SDL_GetKeyboardState(NULL);

    /* 1 2 3 C                    1 2 3 4 */
    keys[1]   = keyboard_state[SDL_SCANCODE_1];
    keys[2]   = keyboard_state[SDL_SCANCODE_2];
    keys[3]   = keyboard_state[SDL_SCANCODE_3];
    keys[0xC] = keyboard_state[SDL_SCANCODE_4];
    /* 4 5 6 D                    Q W E R */
    keys[4]   = keyboard_state[SDL_SCANCODE_Q];
    keys[5]   = keyboard_state[SDL_SCANCODE_W];
    keys[6]   = keyboard_state[SDL_SCANCODE_E];
    keys[0xD] = keyboard_state[SDL_SCANCODE_R];
    /* 7 8 9 E                    A S D F */
    keys[7]   = keyboard_state[SDL_SCANCODE_A];
    keys[8]   = keyboard_state[SDL_SCANCODE_S];
    keys[9]   = keyboard_state[SDL_SCANCODE_D];
    keys[0xE] = keyboard_state[SDL_SCANCODE_F];
    /* A 0 B F                    Z X C V */
    keys[0xA] = keyboard_state[SDL_SCANCODE_Z];
    keys[0]   = keyboard_state[SDL_SCANCODE_X];
    keys[0xB] = keyboard_state[SDL_SCANCODE_C];
    keys[0xF] = keyboard_state[SDL_SCANCODE_V];
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: ./chip8 file nop6\n");
        exit(1);
    }

    /* Number of opcodes to fetch and execute in each emulation loop. */
    int cycles_per_frame = 0;
    if (argc == 3)
        cycles_per_frame = atoi(argv[2]);

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
    
        /* Reset all input keys; if any of them is set, it will be handled
         * by the event handler. */
        memset(keys, 0, sizeof(keys));
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

        /* Get keyboard state and update keys[16] array that is used by the 
         * instructions to know about the keyboard input. */
        keys_update();

        /* Since we force the emulation loop to run at (approximately) 60Hz,
         * we can use it to  update the timers (decrement if less than zero). */
        if (timer_delay > 0) timer_delay--; 
        if (timer_sound > 0) timer_sound--;

        /* Update the CPU state by "cycles" instructions. This value is arbitrary
         * and must be fiddled with to achieve the right emulation speed.
         * The cycles value is the number of instructions that will execute
         * every 1/60 seconds (16 ms). */
        cpu_update(cycles_per_frame);

        /* SDL_PauseAudioDevice(devid, 0) will start playing; non-zero will pause.
         * So we can pass directly the negated timer_sound value: while it's higher
         * than 0 will continue playing, and once it reaches 0 it will pause. */ 
        SDL_PauseAudioDevice(audio_devid, !timer_sound);

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
