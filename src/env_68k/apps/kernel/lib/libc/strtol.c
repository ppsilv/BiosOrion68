// ============================================================
// strtol.c - Implementação para o kernel Orion68DOS
// ============================================================

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>

/**
 * strtol - Converte uma string para um valor long
 * 
 * @param nptr: Ponteiro para a string a ser convertida
 * @param endptr: Ponteiro para ponteiro onde será armazenado o final da string
 * @param base: Base da conversão (0, 2-36)
 * 
 * @return: Valor convertido, ou 0 em caso de erro
 * 
 * Suporta:
 * - Espaços em branco no início
 * - Sinal '+' ou '-'
 * - Prefixos: 0x (hexadecimal), 0 (octal)
 * - Bases: 2 a 36 (0 = automático)
 */
long strtol(const char *nptr, char **endptr, int base) {
    const char *s = nptr;
    long result = 0;
    int sign = 1;
    bool overflow = false;
    
    // 1. IGNORA ESPAÇOS EM BRANCO
    while (*s == ' ' || *s == '\t' || *s == '\n' || 
           *s == '\r' || *s == '\v' || *s == '\f') {
        s++;
    }
    
    // 2. VERIFICA SINAL
    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }
    
    // 3. DETERMINA BASE (se for 0)
    if (base == 0) {
        if (*s == '0') {
            s++;
            if (*s == 'x' || *s == 'X') {
                // Base 16 (hexadecimal)
                base = 16;
                s++;
            } else {
                // Base 8 (octal)
                base = 8;
                // Não avança o '0' porque ele é o primeiro dígito octal
                s--; // Volta para o '0'
            }
        } else {
            base = 10; // Decimal
        }
    }
    
    // 4. VALIDA BASE
    if (base < 2 || base > 36) {
        if (endptr) *endptr = (char *)nptr;
        return 0;
    }
    
    // 5. CONVERTE DIGITOS
    while (*s) {
        int digit;
        char c = *s;
        
        // Converte caractere para valor numérico
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'A' && c <= 'Z') {
            digit = 10 + (c - 'A');
        } else if (c >= 'a' && c <= 'z') {
            digit = 10 + (c - 'a');
        } else {
            // Caractere inválido -> para a conversão
            break;
        }
        
        // Verifica se o dígito é válido para a base
        if (digit >= base) {
            break;
        }
        
        // 6. VERIFICA OVERFLOW (antes de adicionar)
        long next_result = result * base + digit;
        
        // Overflow positivo
        if (result > LONG_MAX / base || 
            (result == LONG_MAX / base && digit > LONG_MAX % base)) {
            overflow = true;
            result = LONG_MAX;
            break;
        }
        
        // Overflow negativo (para números negativos)
        if (sign == -1) {
            if (result < LONG_MIN / base || 
                (result == LONG_MIN / base && digit > -(LONG_MIN % base))) {
                overflow = true;
                result = LONG_MIN;
                break;
            }
        }
        
        result = next_result;
        s++;
    }
    
    // 7. TRATA OVERFLOW
    if (overflow) {
        // ERANGE: valor fora do intervalo
        // (você pode definir errno = ERANGE se tiver)
    }
    
    // 8. APLICA SINAL
    if (sign == -1 && !overflow) {
        result = -result;
    }
    
    // 9. ARMAZENA PONTEIRO FINAL
    if (endptr) {
        *endptr = (char *)s;
    }
    
    return result;
}

// ============================================================
// VERSÃO SIMPLIFICADA (para uso mais comum)
// ============================================================

/**
 * atoi - Converte string para int (usando base 10)
 * 
 * @param nptr: Ponteiro para a string
 * @return: Valor inteiro (int)
 * 
 * NOTA: atoi() não detecta overflow!
 */
int atoi(const char *nptr) {
    return (int)strtol(nptr, NULL, 10);
}

/**
 * atol - Converte string para long (usando base 10)
 * 
 * @param nptr: Ponteiro para a string
 * @return: Valor long
 */
long atol(const char *nptr) {
    return strtol(nptr, NULL, 10);
}

// ============================================================
// FUNÇÕES AUXILIARES (opcionais)
// ============================================================

/**
 * strtoul - Converte string para unsigned long
 * 
 * @param nptr: Ponteiro para a string
 * @param endptr: Ponteiro para ponteiro final
 * @param base: Base (0, 2-36)
 * @return: Valor unsigned long
 * 
 * NOTA: Não suporta sinal negativo
 */
unsigned long strtoul(const char *nptr, char **endptr, int base) {
    const char *s = nptr;
    unsigned long result = 0;
    bool overflow = false;
    
    // Ignora espaços
    while (*s == ' ' || *s == '\t' || *s == '\n') s++;
    
    // Determina base (se for 0)
    if (base == 0) {
        if (*s == '0') {
            s++;
            if (*s == 'x' || *s == 'X') {
                base = 16;
                s++;
            } else {
                base = 8;
                s--; // Volta para o '0'
            }
        } else {
            base = 10;
        }
    }
    
    // Valida base
    if (base < 2 || base > 36) {
        if (endptr) *endptr = (char *)nptr;
        return 0;
    }
    
    // Converte
    while (*s) {
        int digit;
        char c = *s;
        
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'A' && c <= 'Z') digit = 10 + (c - 'A');
        else if (c >= 'a' && c <= 'z') digit = 10 + (c - 'a');
        else break;
        
        if (digit >= base) break;
        
        // Verifica overflow
        unsigned long next_result = result * base + digit;
        if (next_result < result) {
            overflow = true;
            result = ULONG_MAX;
            break;
        }
        result = next_result;
        s++;
    }
    
    if (endptr) {
        *endptr = (char *)s;
    }
    
    return result;
}

// ============================================================
// VERSÃO COM ERRO (se você tiver errno)
// ============================================================

#ifdef HAVE_ERRNO
#include <errno.h>

long strtol_with_errno(const char *nptr, char **endptr, int base) {
    long result;
    // Salva errno atual
    int saved_errno = errno;
    errno = 0;
    
    result = strtol(nptr, endptr, base);
    
    // Se houve overflow, define errno
    if (result == LONG_MAX || result == LONG_MIN) {
        errno = ERANGE;
    }
    
    // Se não houve erro, restaura errno
    if (errno == 0) {
        errno = saved_errno;
    }
    
    return result;
}
#endif
