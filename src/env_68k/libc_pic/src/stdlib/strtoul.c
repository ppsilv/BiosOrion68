#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>
#include <ctype.h>
#include <limits.h>

unsigned long strtoul(const char *nptr, char **endptr, int base) {
    const char *s = nptr;
    unsigned long acc = 0;
    int c;
    unsigned long cutoff;
    int neg = 0, any = 0, cutlim;

    // 1. Ignora espaços em branco iniciais (Feito na mão para não usar isspace)
    do {
        c = *s++;
    } while (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f');

    // 2. Trata o sinal
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else if (c == '+') {
        c = *s++;
    }

    // 3. Detecta ou valida a base numérica
    if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0) {
        base = (c == '0') ? 8 : 10;
    }

    // 4. Calcula os limites para evitar Overflow
    cutoff = ULONG_MAX / (unsigned long)base;
    cutlim = ULONG_MAX % (unsigned long)base;

    // 5. Converte os caracteres em números (Sem usar isdigit ou isalpha)
    for (;; c = *s++) {
        if (c >= '0' && c <= '9') {
            c -= '0';
        } else if (c >= 'a' && c <= 'z') {
            c = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'Z') {
            c = c - 'A' + 10;
        } else {
            break; // Caractere inválido, para o laço
        }

        if (c >= base) {
            break; // O dígito não pertence à base numérica atual
        }

        if (any < 0) {
            continue;
        }

        if (acc > cutoff || (acc == cutoff && c > cutlim)) {
            any = -1;
            acc = ULONG_MAX;
        } else {
            any = 1;
            acc = acc * base + c;
        }
    }

    // 6. Aplica o sinal
    if (neg && any > 0) {
        acc = -acc;
    }

    // 7. Atualiza o ponteiro de fim
    if (endptr != 0) {
        *endptr = (char *)(any ? s - 1 : nptr);
    }

    return acc;
}
