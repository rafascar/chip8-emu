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

    return 0;
}

