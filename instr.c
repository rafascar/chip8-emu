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

/* 6XNN     Store number NN in register VX.
 */
void op_6XNN(uint16_t opcode) {
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;

    reg[x] = nn;
}

/* 7XNN     Add the value NN to register VX.
 */
void op_7XNN(uint16_t opcode) {
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;

    reg[x] = reg[x] + nn;
}

/* 8XYN - 8XY0, 8XY1, 8XY2, 8XY3, 8XY4, 8XY5, 8XY6, 8XY7, 8XYE
 *
 * 8XY0     Store the value of register VY in register VX.
 *
 * 8XY1     Set VX to VX OR VY.
 * 8XY2     Set VX to VX AND VY.
 * 8XY3     Set VX to VX XOR VY.
 *
 * 8XY4     Add the value of register VY to register VX
 *          Set VF to 01 if a carry occurs
 *          Set VF to 00 if a carry does not occur
 *
 * 8XY5     Subtract the value of register VY from register VX
 *          Set VF to 00 if a borrow occurs
 *          Set VF to 01 if a borrow does not occur
 *
 * 8XY6     Store the value of register VY shifted right one bit in register VX
 *          Set register VF to the least significant bit prior to the shift
 *
 * 8XY7     Set register VX to the value of VY minus VX
 *          Set VF to 00 if a borrow occurs
 *          Set VF to 01 if a borrow does not occur
 *
 * 8XYE     Store the value of register VY shifted left one bit in register VX
 *          Set register VF to the most significant bit prior to the shift
 */
void op_8XYN(uint16_t opcode) {
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t y  = (opcode & 0x00F0) >> 4;

    /* The 8XY4 opcode needs the previous value of register X to compute
     * if a carry will occur; hence this declaration here, once it is not
     * possible to declare inside a switch statement. */
    uint8_t prevx;

    switch (opcode & 0x000F) {
        /* 8XY0 VX = VY */
        case 0x0:
            reg[x] = reg[y];
            break;
        /* 8XY1 VX = VX OR VY */
        case 0x1:
            reg[x] = reg[x] | reg[y];
            break;
        /* 8XY2 VX = VX AND VY */
        case 0x2:
            reg[x] = reg[x] & reg[y];
            break;
        /* 8XY3 VX = VX XOR VY */
        case 0x3:
            reg[x] = reg[x] ^ reg[y];
            break;
        /* 8XY4 VX = VX + VY (carry on VF) */
        case 0x4:
            prevx = reg[x];
            reg[x] = reg[x] + reg[y];
            /* If carry occurs, the current value of VX is smaller than its
             * previous value; so we set the carry flag on register VF. */
            reg[0xF] = (reg[x] < prevx) ? 0x1 : 0x0;
            break;
        /* 8XY5 VX = VX - VY (borrow on VF) */
        case 0x5:
            /* If the value of VY (subtrahend) is greater than the value of
             * VX (minuend), a borrow will occur; so we set the borrow flag
             * on register VF. */
            reg[0xF] = (reg[y] > reg[x]) ? 0x1 : 0x0;
            reg[x] = reg[x] - reg[y];
            break;
        /* 8XY6 VX = VY >> 1 (LSB on VF) */
        case 0x6:
            reg[0xF] = reg[y] & 0x1;
            reg[x] = reg[y] >> 1;
            break;
        /* 8XY7 VX = VY - VX (borrow on VF) */
        case 0x7:
            /* Same as 8XY5, but inverted. */
            reg[0xF] = (reg[x] > reg[y]) ? 0x1 : 0x0;
            reg[x] = reg[y] - reg[x];
            break;
        /* 8XYE VX = VY << 1 (MSB on VF) */
        case 0xE:
            reg[0xF] = (reg[y] & 0x80) >> 7;
            reg[x] = reg[y] << 1;
            break;
        default:
            invalid_opcode(opcode);
            break;
    }
}

/* 9XY0     Skip the following instruction if the value of register 
 *          VX is not equal to the value of register VY.
 */
void op_9XY0(uint16_t opcode) {
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t y  = (opcode & 0x00F0) >> 4;

    uint8_t vx = reg[x];
    uint8_t vy = reg[y];
    if (vx != vy)   
        reg_PC = reg_PC + 2;
}

/* ANNN     Store memory address NNN in register I.
 */
void op_ANNN(uint16_t opcode) {
    uint16_t nnn = opcode & 0x0FFF;
    reg_I = nnn;
}

/* BNNN     Jump to address NNN + V0.
 */
void op_BNNN(uint16_t opcode) {
    uint16_t nnn = opcode & 0x0FFF;
    reg_PC = nnn + reg[0];
}

/* CXNN     Set VX to a random number with a mask of NN.
 */
void op_CXNN(uint16_t opcode) {
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;

    /* It generated a random number between 00 and FF; it then logical
     * ANDs this value with a byte mask in order to reduce the size of
     * the set of random numbers capable of being returned from this func. */
    reg[x] = (uint8_t)(rand() % 0xFF) & nn;
    print_registers();
}
