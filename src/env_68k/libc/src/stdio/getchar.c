
int getchar(void){
    int result;
    asm volatile (
        "MOVE.W #1, %%D1\n\t"   // CCONIN
        "TRAP #1\n\t"
        "MOVE.L %%D0, %0"
        : "=g" (result)
        :
        : "d0", "d1", "cc", "memory"
    );
    return result;// & 0xFF; // Retorna apenas o byte inferior
}
