/*
//Function #1
int kbd_getchar(void) {
    int result;
    asm volatile (
        "MOVE.W #1, %%D1\n\t"   // CCONIN
        "TRAP #1\n\t"
        "MOVE.L %%D1, %0"
        : "=g" (result)
        :
        : "d0", "d1", "cc", "memory"
    );
    return result & 0xFF; // Retorna apenas o byte inferior
}
*/
//Function #2
/*
void putchar(char c) {
    asm volatile (
        "movem.l %%d0/%%d1/%%a0,-(%%sp)\n\t"  // Save modified registers
        "move.b %0, %%d0\n\t"                 // Put character in D0
        "move.w #2, %%d1\n\t"                 // OUTCH trap function code
        "trap #1\n\t"                         // Call TUTOR function
        "movem.l (%%sp)+,%%d0/%%d1/%%a0"      // Restore registers
        :
        : "r" (c)
        : "d0", "d1", "a0", "cc", "memory"
    );
}
*/
void putchar(char c) {
    asm volatile (
        "movem.l %%d0/%%d1/%%a0,-(%%sp)\n\t"  // Save modified registers
        "move.b %0, %%d0\n\t"                 // Put character in D0
        "move.w #2, %%d1\n\t"                 // Put function code in D1
        "trap #1\n\t"                         // Call TRAP #1
        "movem.l (%%sp)+,%%d0/%%d1/%%a0"      // Restore registers
        :
        : "r" (c)
        : "d0", "d1", "a0", "cc", "memory"
    );
}
