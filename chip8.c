/* CHIP-8 EMULATOR */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 4K byte-addressable memory */
uint8_t memory[0xFFF];

/* 16 8-bit data registers V0 to VF */
uint8_t reg[16];
/* 16-bit address register I */
uint16_t reg_I;
/* 16-bit program counter PC */
uint16_t reg_PC;

/* TODO: stack */

/* Reset CPU registers and load image file to memory */
void cpu_reset(FILE *file) {
    memset(reg, 0, sizeof(reg));    /* reset all data registers */
    reg_I = 0;                      /* reset address register */
    reg_PC = 0x200;                 /* programs start at 0x200 */

    /* load image file to memory */
    uint8_t *p = &memory[reg_PC];      
    int max_read = 0xFFF - 0x200;    /* maximum file size */
    fread(p, sizeof(uint8_t), max_read, file);
}
    


int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: ./chip8 file\n");
        exit(1);
    }

    /* get image file from path in arguments and reset cpu */
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "chip8: error opening file (%s)\n", argv[1]);
        exit(1);
    }
    cpu_reset(file);
    fclose(file);


    /* begin emulation loop */
    int running = 1;
    while (running) {
        /* Fetch instruction.
         * CHIP-8 instructions are 2-bytes, but it has a byte-addressable memory
         * thus, we need to get two consecutive bytes to fech a single 
         * instruction, hence the PC register is incremented twice. 
         * Instructions are big-endian; so the most significant bits are shifted
         * left and OR'd with the least significant to obtain the full instr. */
        uint16_t instr = (uint16_t)memory[reg_PC] << 8 | memory[reg_PC+1];  
        reg_PC = reg_PC + 2;
        /* The first hexadecimal digit of an opcode dictates which instruction 
         * needs to be executed; in some cases, the last hex digit is also 
         * needed. */
        switch (instr & 0xF000) {
            /* 0NNN (not implemented), 00E0, 00EE */
            case 0x0000:
                switch (instr & 0x000F) {
                    case 0x0:
                        break;
                    case 0xE:
                        break;
                    default:
                        abort();
                        break;
                }
                break;
            /* 1NNN */
            case 0x1000:
                break;
            /* 2NNN */
            case 0x2000:
                break;
            /* 3XNN */
            case 0x3000:
                break;
            /* 4XNN */
            case 0x4000:
                break;
            /* 5XY0 */
            case 0x5000:
                break;
            /* 6XNN */
            case 0x6000:
                break;
            /* 7XNN */
            case 0x7000:
                break;
            /* 8XY0, 8XY1, 8XY2, 8XY3, 8XY4, 8XY5, 8XY6, 8XY7, 8XYE */
            case 0x8000:
                switch (instr & 0x000F) {
                    case 0x0:
                        break;
                    case 0x1:
                        break;
                    case 0x2:
                        break;
                    case 0x3:
                        break;
                    case 0x4:
                        break;
                    case 0x5:
                        break;
                    case 0x6:
                        break;
                    case 0x7:
                        break;
                    case 0xE:
                        break;
                    default:
                        fprintf(stderr, "chip8: invalid opcode\n");
                        abort();
                        break;
                }
                break;
            /* 9XY0 */
            case 0x9000:
                break;
            /* ANNN */
            case 0xA000:
                break;
            /* BNNN */
            case 0xB000:
                break;
            /* CXNN */
            case 0xC000:
                break;
            /* DXYN */
            case 0xD000:
                break;
            /* EX9E, EXA1 */
            case 0xE000:
                switch (instr & 0x000F) {
                    case 0xE:
                        break;
                    case 0x1:
                        break;
                    default:
                        fprintf(stderr, "chip8: invalid opcode");
                        abort();
                        break;
                }
                break;
            /* FX07, FX0A, FX18, FX1E, FX29, FX33, FX15, FX55, FX65 */
            case 0xF000:
                switch (instr & 0x000F) {
                    case 0x0007:
                        break;
                    case 0x000A:
                        break;
                    case 0x0008:
                        break;
                    case 0x000E:
                        break;
                    case 0x0009:
                        break;
                    case 0x0003:
                        break;
                    case 0x0005:
                        switch (instr & 0x00F0) {
                            case 0x0010:
                                break;
                            case 0x0050:
                                break;
                            case 0x0060:
                                break;
                            default:
                                fprintf(stderr, "chip8: invalid opcode");
                                abort();
                                break;
                        }
                    default:
                        fprintf(stderr, "chip8: invalid opcode");
                        abort();
                        break;
                }
                break;
            default:
                fprintf(stderr, "chip8: invalid opcode");
                abort();
                break;
        }
    }

    return 0;
}

