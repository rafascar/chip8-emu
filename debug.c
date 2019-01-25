void print_memory(uint16_t start, uint16_t n) {
    printf("*** Memory (start=0x%x, n=%d)\n", start, n);
    int i;
    for (i = start; i < start+n; i++) {
        printf("0x%x\t%x\n", i, memory[i]);
    }
    printf("\n");
}

void print_stack() {
    int i = 0;
    printf("*** Stack (sp=%d)\n", sp);
    for (i = 0; i < sp; i++) {
        printf("[%d]\t%x\n", i, stack[i]);
    }
    printf("\n");
}

void print_registers() {
    int i;
    printf("*** Registers\n");
    printf("PC\t%x\n", reg_PC);
    printf("I\t%x\n", reg_I);
    for (i = 0; i < 16; i++) {
        printf("V%x\t%x\n", i, reg[i]);
    }
    printf("\n");
}
