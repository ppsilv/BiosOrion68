// ============================================================
// int_to_string.c - Conversão de inteiros para strings
// Para o kernel Orion68DOS (m68k)
// ============================================================

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

/**
 * int_to_string - Converte um inteiro para string
 * 
 * @param value: Valor a ser convertido
 * @param str: Buffer para armazenar a string (DEVE ter tamanho suficiente!)
 * @param base: Base da conversão (2-36)
 * 
 * @return: Ponteiro para a string convertida (str)
 * 
 * Suporta:
 * - Bases: 2 a 36
 * - Números negativos (apenas base 10)
 * - Strings terminadas em '\0'
 * 
 * Exemplos:
 *   int_to_string(123, buffer, 10) -> "123"
 *   int_to_string(255, buffer, 16) -> "ff"
 *   int_to_string(-42, buffer, 10) -> "-42"
 *   int_to_string(42, buffer, 2)   -> "101010"
 */

char *int_to_string(int value, char *str, int base);

/**
 * long_to_string - Converte um long para string
 * 
 * @param value: Valor a ser convertido
 * @param str: Buffer para armazenar a string
 * @param base: Base da conversão (2-36)
 * 
 * @return: Ponteiro para a string convertida (str)
 */
char *long_to_string(long value, char *str, int base) {
    // Valida a base
    if (base < 2 || base > 36) {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }
    
    char *ptr = str;
    char *start = str;
    long num = value;
    bool is_negative = false;
    
    // Trata números negativos (apenas base 10)
    if (num < 0 && base == 10) {
        is_negative = true;
        num = -num;
    }
    
    // Caso especial: zero
    if (num == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return str;
    }
    
    // Converte dígitos (do último para o primeiro)
    while (num > 0) {
        int digit = num % base;
        if (digit < 10) {
            *ptr++ = '0' + digit;
        } else {
            *ptr++ = 'a' + (digit - 10);
        }
        num /= base;
    }
    
    // Adiciona sinal negativo
    if (is_negative) {
        *ptr++ = '-';
    }
    
    *ptr = '\0';  // Termina a string
    
    // Inverte a string (porque foi construída ao contrário)
    ptr--;
    while (start < ptr) {
        char temp = *start;
        *start++ = *ptr;
        *ptr-- = temp;
    }
    
    return str;
}

/**
 * unsigned_long_to_string - Converte unsigned long para string
 * 
 * @param value: Valor a ser convertido
 * @param str: Buffer para armazenar a string
 * @param base: Base da conversão (2-36)
 * 
 * @return: Ponteiro para a string convertida (str)
 */
char *unsigned_long_to_string(unsigned long value, char *str, int base) {
    if (base < 2 || base > 36) {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }
    
    char *ptr = str;
    char *start = str;
    unsigned long num = value;
    
    // Caso especial: zero
    if (num == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return str;
    }
    
    // Converte dígitos (do último para o primeiro)
    while (num > 0) {
        int digit = num % base;
        if (digit < 10) {
            *ptr++ = '0' + digit;
        } else {
            *ptr++ = 'a' + (digit - 10);
        }
        num /= base;
    }
    
    *ptr = '\0';  // Termina a string
    
    // Inverte a string (porque foi construída ao contrário)
    ptr--;
    while (start < ptr) {
        char temp = *start;
        *start++ = *ptr;
        *ptr-- = temp;
    }
    
    return str;
}

// ============================================================
// FUNÇÕES DE CONVENIÊNCIA
// ============================================================

/**
 * int_to_hex - Converte int para string hexadecimal
 * 
 * @param value: Valor a ser convertido
 * @param str: Buffer para armazenar a string
 * @param uppercase: true = letras maiúsculas (ABCDEF)
 * 
 * @return: Ponteiro para a string convertida
 */
char *int_to_hex(int value, char *str, bool uppercase) {
    char *result = long_to_string((long)value & 0xFFFFFFFF, str, 16);
    
    // Converte para maiúsculas se solicitado
    if (uppercase) {
        for (char *p = str; *p; p++) {
            if (*p >= 'a' && *p <= 'f') {
                *p = *p - 'a' + 'A';
            }
        }
    }
    
    return str;
}

/**
 * int_to_binary - Converte int para string binária
 * 
 * @param value: Valor a ser convertido
 * @param str: Buffer para armazenar a string (mínimo 33 bytes)
 * 
 * @return: Ponteiro para a string convertida
 */
char *int_to_binary(int value, char *str) {
    int bits = sizeof(int) * 8;
    char *p = str;
    
    for (int i = bits - 1; i >= 0; i--) {
        *p++ = (value & (1 << i)) ? '1' : '0';
    }
    *p = '\0';
    
    return str;
}

/**
 * int_to_hex_with_prefix - Converte int para string hexadecimal com prefixo "0x"
 * 
 * @param value: Valor a ser convertido
 * @param str: Buffer para armazenar a string
 * @param uppercase: true = letras maiúsculas
 * 
 * @return: Ponteiro para a string convertida
 */
char *int_to_hex_with_prefix(int value, char *str, bool uppercase) {
    str[0] = '0';
    str[1] = 'x';
    int_to_hex(value, str + 2, uppercase);
    return str;
}

// ============================================================
// VERSÃO PARA SISTEMAS SEM MALLOC (KERNEL)
// ============================================================

/**
 * int_to_string_static - Versão com buffer estático (para kernel)
 * 
 * ATENÇÃO: Não é reentrante! Use apenas em contexto único.
 * 
 * @param value: Valor a ser convertido
 * @param base: Base da conversão (2-36)
 * 
 * @return: Ponteiro para buffer estático com a string
 */
char *int_to_string_static(int value, int base) {
    static char buffer[64];  // Buffer estático no kernel
    return int_to_string(value, buffer, base);
}

/**
 * long_to_string_static - Versão com buffer estático (para kernel)
 * 
 * ATENÇÃO: Não é reentrante! Use apenas em contexto único.
 * 
 * @param value: Valor a ser convertido
 * @param base: Base da conversão (2-36)
 * 
 * @return: Ponteiro para buffer estático com a string
 */
char *long_to_string_static(long value, int base) {
    static char buffer[64];
    return long_to_string(value, buffer, base);
}

char *int_to_string(int value, char *str, int base) {
    return long_to_string((long)value, str, base);
}
