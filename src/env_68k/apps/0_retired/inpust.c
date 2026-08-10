#include <stdio.h>
#include <stdint.h>


void ler_hex_string(void) {
    char input[] = "0x82000"; // ou apenas "82000"
    unsigned int endereco;

    // sscanf converte a string em hexadecimal para um inteiro/ponteiro
    sscanf(input, "%x", &endereco);

    printf("O endereco lido foi: 0x%X\n", endereco);
}



/**
 * Converte um único caractere ASCII hexadecimal em seu valor numérico (0 a 15).
 * Retorna -1 se o caractere for inválido.
 */
int ascii_hex_para_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1; // Caractere inválido
}

/**
 * Lê uma string hexadecimal da serial e converte para um inteiro de 32 bits (uint32_t).
 * Ideal para ler endereços de memória no 68k.
 */
uint32_t ler_uint32_hex_serial(void) {
    uint32_t resultado = 0;
    int caracteres_lidos = 0;
    
    while (caracteres_lidos < 8) { // Um inteiro de 32 bits no 68k tem no máximo 8 dígitos hex
        // Substitua pelo seu leitor de byte da UART (ex: _inbyte(1))
        int c = _inbyte(0); 
        
        if (c < 0) continue; // Espera o caractere chegar

        // Se o usuário digitar '0' seguido de 'x' ou 'X', ignora o prefixo
        if (caracteres_lidos == 1 && (c == 'x' || c == 'X') && resultado == 0) {
            resultado = 0;
            caracteres_lidos = 0;
            continue;
        }

        // Se o usuário apertar Enter (\r ou \n) ou Espaço, encerra a leitura
        if (c == '\r' || c == '\n' || c == ' ') {
            if (caracteres_lidos > 0) break;
            else continue;
        }

        int val = ascii_hex_para_val((char)c);
        if (val >= 0) {
            // Desloca o resultado 4 bits para a esquerda (multiplica por 16) 
            // e adiciona o novo dígito na base do registrador
            resultado = (resultado << 4) | (val & 0x0F);
            caracteres_lidos++;
            
            // Opcional: Ecoa o caractere de volta para o usuário ver o que digitou
            _outbyte(c); 
        }
    }
    return resultado;
}