#include <stdint.h>
#include <stdbool.h>
#include "drv_ps2.h"

#define RING_BUFFER_SIZE 256 // Deve ser 256 para o wrap-around automático do uint8_t

typedef struct {
    volatile uint8_t buffer[RING_BUFFER_SIZE];
    volatile uint8_t head; // Índice de escrita (alterado na INT3)
    volatile uint8_t tail; // Índice de leitura (alterado no SO)
} RingBuffer;

static RingBuffer kb_buf;

void kb_init(void) {
    kb_buf.head = 0;
    kb_buf.tail = 0;
}

// 1. Escreve no Buffer (Chamar dentro da trata_int3_handler)
bool kb_put(uint8_t data) {
    
    uint8_t next_head = kb_buf.head + 1; // 255 + 1 vira 0 automaticamente!

    // Se o próximo head alcançar o tail, o buffer encheu
    if (next_head == kb_buf.tail) {
        return false; // Buffer cheio (descarta byte)
    }

    kb_buf.buffer[kb_buf.head] = data;
    kb_buf.head = next_head;
    return true;
}

// 2. Lê do Buffer (Chamar no Loop Principal / SO)
bool kb_get(uint8_t *data) {
    // Se head == tail, não há dados novos
    if (kb_buf.head == kb_buf.tail) {
        return false; // Buffer vazio
    }

    *data = kb_buf.buffer[kb_buf.tail];
    kb_buf.tail++; // 255 + 1 vira 0 automaticamente!
    return true;
}

// 3. Verifica se tem caractere disponível
bool kb_available(void) {
    return (kb_buf.head != kb_buf.tail);
}

unsigned int ps2_getchar(){
    uint8_t data; 
    kb_get((uint8_t *)data);
    return (unsigned int)data; 
}
