/* Simple implementation of a stack */

#include <assert.h>
#include <string.h>
#include "chip8.h"

void stack_init() {
    /* reset stack elements and stack pointer */
    memset(stack, 0, LEVELS);
    sp = 0;
}

void stack_push(uint16_t address) {
    assert(sp < LEVELS);
    stack[sp++] = address;
}

uint16_t stack_pop() {
    assert(sp > 0);
    return stack[--sp];
}
