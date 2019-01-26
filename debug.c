/* ANSI formatting for drawing B/W background (a pixel) */
const char *pixel[2] = {
    "\x1b[40m  \x1b[0m",    /* black */
    "\x1b[47m  \x1b[0m"     /* white */
};

void print_memory(uint16_t start, uint16_t n) {
    printf("*** Memory (start=0x%03x, n=%d)\n", start, n);
    int i;
    for (i = start; i < start+n; i++) {
        printf("0x%03x\t%02x\n", i, memory[i]);
    }
    printf("\n");
}

void print_stack() {
    printf("*** Stack (sp=%d)\n", sp);
    int i = 0;
    for (i = 0; i < sp; i++) {
        printf("[%d]\t%x\n", i, stack[i]);
    }
    printf("\n");
}

void print_registers() {
    printf("*** Registers\n");
    printf("PC\t%x\n", reg_PC);
    printf("I\t%x\n", reg_I);
    int i;
    for (i = 0; i < 16; i++) {
        printf("V%x\t%x\n", i, reg[i]);
    }
    printf("\n");
}

void print_screen() {
    printf("*** Screen (%dx%d)\n", WIDTH, HEIGHT);
    int h, w;
    for (h = 0; h < HEIGHT; h++) {
        for (w = h * WIDTH; w < (h+1)*WIDTH; w++) {
           printf("%s", pixel[frame_buffer[w]]);
           //printf("%d", frame_buffer[w]);
        }
        printf("\n");
    }
}
