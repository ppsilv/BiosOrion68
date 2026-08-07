#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

static uint32_t crc32_table[256];
#define CRC32_POLYNOMIAL 0xEDB88320UL

// Gerador da tabela de busca
void crc32_init(void) {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int k = 0; k < 8; k++) {
            c = (c & 1) ? (CRC32_POLYNOMIAL ^ (c >> 1)) : (c >> 1);
        }
        crc32_table[i] = c;
    }
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
/*
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <caminho_do_arquivo>\n", argv[0]);
        return 1;
    }

    crc32_init();

    uint32_t crc = crc32_from_file(argv[1]);
    printf("CRC32: 0x%08X\n", crc);

    return 0;
}
*/
