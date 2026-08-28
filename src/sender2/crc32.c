#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
uint32_t crc32_update(uint32_t crc, const uint8_t *buffer, size_t length) {
    for (size_t i = 0; i < length; i++) {
        crc = crc32_table[(crc ^ buffer[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc;
}

// Função para ler o arquivo do disco e calcular o CRC-32
uint32_t crc32_from_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Erro ao abrir arquivo");
        return 0;
    }

    uint32_t crc = 0xFFFFFFFFUL;
    uint8_t buffer[4096]; // Bloco de leitura de 4 KB
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            crc = crc32_table[(crc ^ buffer[i]) & 0xFF] ^ (crc >> 8);
        }
    }

    fclose(file);
    return crc ^ 0xFFFFFFFFUL;
}

/**
 * Função para calcular o CRC-32 final de um buffer completo.
 */
uint32_t crc32_calculate(const uint8_t *buffer, size_t length) {
    // Inicia com 0xFFFFFFFF e faz o XOR final com 0xFFFFFFFF
    return crc32_update(0xFFFFFFFFUL, buffer, length) ^ 0xFFFFFFFFUL;
}

/*
int main(){

crc32_init(); // No startup do m68k

uint32_t crc_m68k = 0xFFFFFFFFUL;

for (size_t i = 0; i < tamanho_arquivo; i++) {
    uint8_t byte_lido = ler_byte_do_pico();
    buffer_ram[i] = byte_lido;

    // Atualiza o CRC byte a byte rapidamente
    crc_m68k = crc32_table[(crc_m68k ^ byte_lido) & 0xFF] ^ (crc_m68k >> 8);
}

// Finaliza o cálculo
crc_m68k ^= 0xFFFFFFFFUL;

// No final, leia os 4 bytes do CRC enviados pelo Pico e compare:
if (crc_m68k == crc_pico) {
    // Arquivo 100% íntegro!
}
}
*/
