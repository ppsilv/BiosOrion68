#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

unsigned long get_system_tick_nmi_safe(void);

//static inline unsigned int m68k_disable_interrupts(void) __attribute__((always_inline));
static inline void m68k_restore_interrupts(unsigned int old_sr) __attribute__((always_inline));
static inline void m68k_enable_all_interrupts(void) __attribute__((always_inline));
static inline void m68k_disable_all_interrupts(void) __attribute__((always_inline));


// Restaura o Status Register para o estado anterior
static inline void m68k_restore_interrupts(unsigned int old_sr) {
    __asm__ __volatile__ (
        "move.w %0, %%sr"
        :
        : "d" (old_sr)
        : "cc"
    );
}

// Esta função lê o SR, mascara APENAS até o nível 2, e retorna o SR antigo
static inline unsigned int m68k_disable_level2(void) {
    unsigned int sr;
    __asm__ __volatile__ (
        "move.w %%sr, %0\n\t"
        "ori.w #0x0200, %%sr"  // Desabilita níveis 1 e 2 (onde está seu tick)
        : "=d" (sr) : : "cc"
    );
    return sr;
}
static inline unsigned int m68k_disable_level2_perfect(void) {
    unsigned int sr;
    __asm__ __volatile__ (
        "move.w %%sr, %0\n\t"      // 1. Salva o SR atual para retornar
        "move.w %0, %%d0\n\t"      // 2. Copia para um registrador temporário
        "andi.w #0xF8FF, %%d0\n\t" // 3. Limpa os bits I2, I1, I0 (força nível 0 temporário)
        "ori.w #0x0200, %%d0\n\t"  // 4. Seta exatamente o nível 2 (010)
        "move.w %%d0, %%sr"        // 5. Atualiza o SR real
        : "=d" (sr)
        :
        : "d0", "cc"               // Avisa o GCC que usamos o d0
    );
    return sr;
}
// Esta função lê o SR, mascara até o nível 3, e retorna o SR antigo
static inline unsigned int m68k_disable_level3(void) {
    unsigned int sr;
    __asm__ __volatile__ (
        "move.w %%sr, %0\n\t"
        "ori.w #0x0300, %%sr"  // Desabilita níveis 1, 2 e 3
        : "=d" (sr) 
        : 
        : "cc"
    );
    return sr;
}
// Versão blindada: Seta estritamente o nível 3 no SR
static inline unsigned int m68k_disable_level3_perfect(void) {
    unsigned int sr;
    __asm__ __volatile__ (
        "move.w %%sr, %0\n\t"      // 1. Salva o SR atual
        "move.w %0, %%d0\n\t"      // 2. Copia para d0
        "andi.w #0xF8FF, %%d0\n\t" // 3. Limpa as máscaras antigas (I2, I1, I0 = 0)
        "ori.w #0x0300, %%d0\n\t"  // 4. Seta exatamente o nível 3 (011)
        "move.w %%d0, %%sr"        // 5. Atualiza o SR
        : "=d" (sr)
        :
        : "d0", "cc"
    );
    return sr;
}


// Força o SR a aceitar TODAS as interrupções (Nível 0)
static inline void m68k_enable_all_interrupts(void) {
    __asm__ __volatile__ (
        "andi.w #0xF8FF, %%sr"  // Zera os bits de máscara (I0, I1, I2), liberando geral
        : : : "cc"
    );
}

// Força o SR a mascarar até o Nível 2 (Bloqueia o seu Tick de 5ms/10ms)
static inline void m68k_disable_all_interrupts(void) {
    __asm__ __volatile__ (
        "ori.w #0x0200, %%sr"   // Seta a máscara estritamente para o nível 2
        : : : "cc"
    );
}

unsigned long get_system_tick(void);

#endif