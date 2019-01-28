/* CHIP-8 EMULATOR */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "chip8.h"
#include "instr.h"
#include "digits.h"

/* Reset CPU registers and load image file to memory */
void cpu_reset(FILE *file) {
    memset(reg, 0, sizeof(reg));    /* reset all data registers */
    reg_I = 0;                      /* reset address register */
    reg_PC = 0x200;                 /* programs start at 0x200 */
    stack_init();                   /* reset stack */
    memset(key, 0, sizeof(key));    /* reset input keys */

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
    
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: ./chip8 file nop6\n");
        exit(1);
    }

    /* DEBUGGING: Number of opcodes to fetch and execute. */
    int nops = 0;
    if (argc == 3)
        nops = atoi(argv[2]);


    /* get image file from path in arguments and reset cpu */
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "chip8: error opening file (%s)\n", argv[1]);
        exit(1);
    }
    cpu_reset(file);
    fclose(file);

    /* begin emulation loop */
    //int running = 1;
    //while (running) { 
    while (nops--) {     /* debugging */
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
                    case 0x0:
                        op_00E0(opcode);
                        break;
                    case 0xE:
                        op_00EE(opcode);
                        break;
                    default:
                        invalid_opcode(opcode);
                        break;
                }
                break;
            /* 1NNN */
            case 0x1000:
                op_1NNN(opcode);
                break;
            /* 2NNN */
            case 0x2000:
                op_2NNN(opcode);
                break;
            /* 3XNN */
            case 0x3000:
                op_3XNN(opcode);
                break;
            /* 4XNN */
            case 0x4000:
                op_4XNN(opcode);
                break;
            /* 5XY0 */
            case 0x5000:
                op_5XY0(opcode);
                break;
            /* 6XNN */
            case 0x6000:
                op_6XNN(opcode);
                break;
            /* 7XNN */
            case 0x7000:
                op_7XNN(opcode);
                break;
            /* 8XYN - 8XY0, 8XY1, 8XY2, 8XY3, 8XY4, 8XY5, 8XY6, 8XY7, 8XYE */
            case 0x8000:
                op_8XYN(opcode);
                break;
            /* 9XY0 */
            case 0x9000:
                op_9XY0(opcode);
                break;
            /* ANNN */
            case 0xA000:
                op_ANNN(opcode);
                break;
            /* BNNN */
            case 0xB000:
                op_BNNN(opcode);
                break;
            /* CXNN */
            case 0xC000:
                op_CXNN(opcode);
                break;
            /* DXYN */
            case 0xD000:
                op_DXYN(opcode);
                break;
            /* EX9E, EXA1 */
            case 0xE000:
                switch (opcode & 0x000F) {
                    case 0xE:
                        op_EX9E(opcode);
                        break;
                    case 0x1:
                        op_EXA1(opcode);
                        break;
                    default:
                        invalid_opcode(opcode);
                        break;
                }
                break;
            /* FX07, FX0A, FX18, FX1E, FX29, FX33, FX15, FX55, FX65 */
            case 0xF000:
                switch (opcode & 0x000F) {
                    case 0x0007:
                        op_FX07(opcode);
                        break;
                    case 0x000A:
                        op_FX0A(opcode);
                        break;
                    case 0x0008:
                        op_FX18(opcode);
                        break;
                    case 0x000E:
                        op_FX1E(opcode);
                        break;
                    case 0x0009:
                        op_FX29(opcode);
                        break;
                    case 0x0003:
                        op_FX33(opcode);
                        break;
                    case 0x0005:
                        switch (opcode & 0x00F0) {
                            case 0x0010:
                                op_FX15(opcode);
                                break;
                            case 0x0050:
                                op_FX55(opcode);
                                break;
                            case 0x0060:
                                op_FX65(opcode);
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
                break;
            default:
                invalid_opcode(opcode);
                break;
        }
    }

    return 0;
}
