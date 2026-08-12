#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

char welcome[]="Tinybasic converter (c)2026 pdsilva aka pgodao\n" \
                "Program to convert a simple text file into tinybasic format\n";




int main(int argc, char *argv[]) {
    puts(welcome);
    if (argc < 3) {
        printf("Uso: %s <entrada.txt> <saida.bas>\n", argv[0]);
        return 1;
    }

    FILE *fin = fopen(argv[1], "r");
    if (!fin) {
        perror("Erro ao abrir entrada");
        return 1;
    }

    FILE *fout = fopen(argv[2], "wb");
    if (!fout) {
        perror("Erro ao criar saída");
        fclose(fin);
        return 1;
    }

    char buffer[512];
    uint16_t auto_line = 10;

    while (fgets(buffer, sizeof(buffer), fin)) {
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len - 1] == '\r' || buffer[len - 1] == '\n' || buffer[len - 1] == ' ')) {
            buffer[--len] = '\0';
        }

        char *ptr = buffer;
        while (*ptr == ' ' || *ptr == '\t') ptr++;

        if (*ptr == '\0') continue;

        uint16_t line_num = auto_line;

        if (isdigit((unsigned char)*ptr)) {
            line_num = (uint16_t)strtoul(ptr, &ptr, 10);
            auto_line = line_num + 10;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
        } else {
            auto_line += 10;
        }

        if (*ptr == '\0') continue;

        // Converter para MAIÚSCULAS (mantém strings dentro de aspas se necessário, mas para o BASIC tudo em caixa alta garante execução)
        for (char *p = ptr; *p; p++) {
            *p = (char)toupper((unsigned char)*p);
        }

        uint8_t text_len = (uint8_t)strlen(ptr);

        // Estrutura: 2 bytes (linha) + 1 byte (tamanho) + text_len + 1 byte (0x0A)
        uint8_t raw_total = 2 + 1 + text_len + 1;
        uint8_t padding = (raw_total % 2 != 0) ? 1 : 0;
        uint8_t total_line_size = raw_total + padding;

        // 1. Número da linha em Big-Endian (2 bytes)
        uint8_t line_bytes[2];
        line_bytes[0] = (uint8_t)((line_num >> 8) & 0xFF);
        line_bytes[1] = (uint8_t)(line_num & 0xFF);
        fwrite(line_bytes, 1, 2, fout);

        // 2. Tamanho TOTAL do bloco da linha (1 byte)
        fwrite(&total_line_size, 1, 1, fout);

        // 3. Texto ASCII (sem o número da linha)
        fwrite(ptr, 1, text_len, fout);

        // 4. Terminador 0x0A
        uint8_t eol = 0x0A;
        fwrite(&eol, 1, 1, fout);

        // 5. Byte de alinhamento par (se necessário)
        if (padding) {
            fwrite(&eol, 1, 1, fout);
        }
    }

    // --- MARCADOR DE FIM DE PROGRAMA (EOF) ---
    // Linha 0x0000 e tamanho 0x00 para indicar ao RUN que o programa acabou
    //uint8_t eof_marker[4] = {0x0a, 0x00};
    //fwrite(eof_marker, 1, 2, fout);

    fclose(fin);
    fclose(fout);
    printf("Binario gerado com sucesso com marcador de fim de programa.\n");
    return 0;
}
