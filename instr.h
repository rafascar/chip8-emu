#include "chip8.h"

/* Instructions */
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
void op_EX9E(uint16_t opcode);
void op_EXA1(uint16_t opcode);
void op_FX07(uint16_t opcode);
void op_FX0A(uint16_t opcode);
void op_FX15(uint16_t opcode);
void op_FX18(uint16_t opcode);
void op_FX1E(uint16_t opcode);
void op_FX29(uint16_t opcode);
void op_FX33(uint16_t opcode);
void op_FX55(uint16_t opcode);
void op_FX65(uint16_t opcode);

/* Helper functions to draw on screen */
uint8_t xor_pixel(uint8_t x, uint8_t y, uint8_t p);
uint8_t get_pixel(uint8_t x, uint8_t y); 
