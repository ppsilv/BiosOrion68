#ifndef __SHOW_REGISTERS__
#define __SHOW_REGISTERS__


#include <stdio.h>
#include <stdint.h>

// Estrutura para organizar os dados na memória
typedef struct {
    uint32_t d[8]; // D0 a D7
    uint32_t a[8]; // A0 a A7 (A7 é o seu Stack Pointer!)
} regs_t;

static inline void dump_registradores(void) __attribute__((always_inline));

static inline void dump_registradores(void) {
    regs_t r;
    int i;

    // Captura os registradores no exato ponto da linha de execução
    __asm__ __volatile__ (
        "movem.l %%d0-%%d7, %0\n\t"
        "movem.l %%a0-%%a7, %1\n\t"
        : "=m" (r.d), "=m" (r.a)
        :
        : "memory"
    );

    // Agora joga os dados direto no printf que cospe na sua PicoVGA
    printf("\n================= DUMP DE REGISTRADORES 68K =================\n");
    
    for(i = 0; i < 8; i++) {
        printf("  D%d: 0x%08X", i, (unsigned int)r.d[i]);
        if (i == 3 || i == 7) printf("\n");
    }
    
    for(i = 0; i < 8; i++) {
        if (i == 7) {
            // Como é inline, este A7 é o stack pointer REAL do ponto da chamada!
            printf("  SP: 0x%08X\n", (unsigned int)r.a[i]); 
        } else {
            printf("  A%d: 0x%08X", i, (unsigned int)r.a[i]);
        }
        if (i == 3) printf("\n");
    }
    printf("=============================================================\n");
}

#endif