/* Implementation of all 35 CHIP-8 instructions 
 *
 * NNN refers to a hexadecimal memory address;
 * NN refers to a hexadecimal byte;
 * N refers to a hexadecimal nibble;
 * X and Y refer to registers.
 * */

#include <stdio.h>
#include <string.h>

#include "instr.h"

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
