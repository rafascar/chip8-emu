/* Implementation of all 35 CHIP-8 instructions 
 *
 * NNN refers to a hexadecimal memory address;
 * NN refers to a hexadecimal byte;
 * N refers to a hexadecimal nibble;
 * X and Y refer to registers.
 * */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "instr.h"
#include "debug.c"

/* 00E0     Clear the screen.
 */
void op_00E0(uint16_t opcode) {
    int screen_size = WIDTH * HEIGHT;
    memset(frame_buffer, 0, screen_size);
}

/* 00EE     Return from a subroutine.
 */
void op_00EE(uint16_t opcode) {
    reg_PC = stack_pop();
}

/* 1NNN     Jump to address NNN.
 */
void op_1NNN(uint16_t opcode) {
    uint16_t nnn = opcode & 0x0FFF;
    assert(nnn >= 0x200 && nnn <= 0xFFF);
    reg_PC = nnn;
}

/* 2NNN     Execute subroutine starting at NNN.
 */
void op_2NNN(uint16_t opcode) {
    uint16_t nnn = opcode & 0x0FFF;
    assert(nnn >= 0x200 && nnn <= 0xFFF);
    /* First, push incremented PC to stack so we can return from subroutine 
     * later; only then jump to subroutine. */
    stack_push(reg_PC);
    reg_PC = nnn;
}

/* 3XNN     Skip the following instruction if the value of register VX equals NN.
 */
void op_3XNN(uint16_t opcode) {
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;

    uint8_t vx = reg[x];
    if (vx == nn)   
        reg_PC = reg_PC + 2;
}

/* 4XNN     Skip the following instruction if the value of register VX 
 *          is not equal to NN.
 */
void op_4XNN(uint16_t opcode) {
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;

    uint8_t vx = reg[x];
    if (vx != nn)   
        reg_PC = reg_PC + 2;
}

/* 5XY0     Skip the following instruction if the value of register VX 
 *          is equal to the value of register VY.
 */
void op_5XY0(uint16_t opcode) {
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t y  = (opcode & 0x00F0) >> 4;

    uint8_t vx = reg[x];
    uint8_t vy = reg[y];
    if (vx == vy)   
        reg_PC = reg_PC + 2;
}
