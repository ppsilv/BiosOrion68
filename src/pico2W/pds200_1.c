#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "pico/stdlib.h"
#include "ringbuffer.h"
#include "ps2_keyboard.h"




// ============================================================================
// --- FUNÇÃO EXECUTADA NO CORE 1 (DEDICADO EXCLUSIVAMENTE AO TECLADO PS/2) ---
// ============================================================================
void core1_entry(void) {
    uint8_t data;
    printf("Core 1 iniciando keyboard PS2\n");
    kb_init();
    initPS2();

    while (true) {
        tight_loop_contents(); // Otimização interna do SDK para loops rápidos
    }
}

