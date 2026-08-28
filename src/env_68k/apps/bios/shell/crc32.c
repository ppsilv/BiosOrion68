#include <stdint.h>
#include <stddef.h>


//crc32 **********************************************************
// Tabela de 1 KB gerada em tempo de execução
static uint32_t crc32_table[256];

// Polinômio padrão IEEE 802.3 (Refletido)
#define CRC32_POLYNOMIAL 0xEDB88320UL

/**
 * Inicializa a tabela de busca do CRC-32.
 * Chame esta função UMA VEZ no início do programa (main).
 */
void crc32_init(void) {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int k = 0; k < 8; k++) {
            c = (c & 1) ? (CRC32_POLYNOMIAL ^ (c >> 1)) : (c >> 1);
        }
        crc32_table[i] = c;
    }
}

/**
 * Atualiza o CRC a cada bloco ou byte recebido.
 * Pode ser chamado iterativamente se você receber o arquivo em partes.
 */
//uint32_t crc32_update(uint32_t crc, const uint8_t *buffer, size_t length) {
//    for (size_t i = 0; i < length; i++) {
//        crc = crc32_table[(crc ^ buffer[i]) & 0xFF] ^ (crc >> 8);
//    }
//    return crc;
//}



uint32_t crc32_update(uint32_t crc, const uint8_t *buffer, size_t length) {
    // Processa blocos de 4 bytes por iteração
    while (length >= 4) {
        crc = crc32_table[(crc ^ buffer[0]) & 0xFF] ^ (crc >> 8);
        crc = crc32_table[(crc ^ buffer[1]) & 0xFF] ^ (crc >> 8);
        crc = crc32_table[(crc ^ buffer[2]) & 0xFF] ^ (crc >> 8);
        crc = crc32_table[(crc ^ buffer[3]) & 0xFF] ^ (crc >> 8);
        buffer += 4;
        length -= 4;
    }
    // Processa os bytes restantes
    while (length--) {
        crc = crc32_table[(crc ^ *buffer++) & 0xFF] ^ (crc >> 8);
    }
    return crc;
}

/**
 * Função para calcular o CRC-32 final de um buffer completo.
 */
uint32_t crc32_calculate(const uint8_t *buffer, size_t length) {
    // Inicia com 0xFFFFFFFF e faz o XOR final com 0xFFFFFFFF
    return crc32_update(0xFFFFFFFFUL, buffer, length) ^ 0xFFFFFFFFUL;
}









/*
; ====================================================================
; uint32_t crc32_update_asm(uint32_t crc, const uint8_t *buf, size_t len)
; Entradas (ABI padrão GCC/68k):
;   Stack: 4(SP) = crc (32 bits), 8(SP) = buf (32 bits), 12(SP) = len (32 bits)
; Retorno: D0 (crc final)
; ====================================================================

    .global crc32_update_asm
crc32_update_asm:
    MOVEM.L D2/A2, -(SP)        ; Preserva registradores utilizados
    
    MOVE.L  12(SP), D0          ; D0 = crc inicial
    MOVEA.L 16(SP), A0          ; A0 = buffer
    MOVE.L  20(SP), D2          ; D2 = length
    BEQ     .done               ; Se length == 0, encerra

    LEA     crc32_table, A1     ; A1 = ponteiro para a tabela (1 KB)

.loop:
    MOVE.B  (A0)+, D1           ; D1.B = *buffer++
    EOR.B   D0, D1              ; D1.B = (crc ^ buffer[i])
    ANDI.W  #$00FF, D1          ; Limpa byte superior
    LSL.W   #2, D1              ; Multiplica o índice por 4 (offset do uint32_t)
    
    LSR.L   #8, D0              ; D0 = (crc >> 8)
    EOR.L   (A1, D1.W), D0      ; D0 = (crc >> 8) ^ crc32_table[index]
    
    SUBQ.L  #1, D2              ; length--
    BNE     .loop               ; Mantém o laço enquanto length > 0

.done:
    MOVEM.L (SP)+, D2/A2        ; Restaura registradores
    RTS



*/