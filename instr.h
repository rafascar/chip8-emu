#include "chip8.h"

void op_00E0(uint16_t opcode);
void op_00EE(uint16_t opcode);
void op_1NNN(uint16_t opcode);
void op_2NNN(uint16_t opcode);
void op_3XNN(uint16_t opcode);
void op_4XNN(uint16_t opcode);
void op_5XY0(uint16_t opcode);
void op_6XNN(uint16_t opcode);
void op_7XNN(uint16_t opcode);
void op_8XYN(uint16_t opcode);
void op_9XY0(uint16_t opcode);
void op_ANNN(uint16_t opcode);
void op_BNNN(uint16_t opcode);
void op_CXNN(uint16_t opcode);
void op_DXYN(uint16_t opcode);
