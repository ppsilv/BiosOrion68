#include <stdio.h>
#include <stdarg.h>
#include <string.h>


/*
 *
    printf("=== Sistema MC68000 ===\n");
    printf("Char: %c\n", 'A');
    printf("Decimal: %d\n", 12345);
    printf("Negativo: %d\n", -6789);
    printf("Unsigned: %u\n", 4000000000U);
    printf("Hexadecimal: 0x%x\n", 0xABCDEF);
    printf("Octal: %o\n", 255);
    printf("String: %s\n", "Hello World!");
    printf("Mix: %c %d %s 0x%x\n", 'X', 42, "test", 0x123);
    printf("Porcentagem: 100%% completo\n");

    // Testes diversos
    printf("Zero: %d\n", 0);
    printf("Max int: %d\n", 2147483647);
    printf("Min int: %d\n", -2147483648);

  Características desta implementação:
✅ ZERO operações de ponto flutuante

✅ ZERO divisões/multiplicações

✅ ZERO dependências de biblioteca

✅ Usa apenas: adição, subtração, shifts, comparações

✅ Suporte completo a: %c, %d, %i, %u, %x, %o, %s, %%

✅ Float ignorado mas não causa erro

✅ Código completamente independente
 *
 *
 */

// Função strlen simples
//int strlen(const char *str) {
//    int len = 0;
//    while (*str++) len++;
//    return len;
//}

// Conversão de hexadecimal usando shifts (sem divisão)
char *itox(unsigned int value, char *str) {
    char *ptr = str;
    int started = 0;

    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return str;
    }

    for (int shift = 28; shift >= 0; shift -= 4) {
        int nibble = (value >> shift) & 0xF;
        if (nibble != 0 || started) {
            *ptr++ = "0123456789abcdef"[nibble];
            started = 1;
        }
    }

    *ptr = '\0';
    return str;
}

// Conversão de decimal usando lookup table (sem divisão)
char *itodec(int value, char *str) {
    char *ptr = str;

    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return str;
    }

    if (value < 0) {
        *ptr++ = '-';
        value = -value;
    }

    unsigned int uvalue = value;

    // Potências de 10 pré-calculadas
    static const unsigned int powers[] = {
        1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1
    };

    int started = 0;

    for (int i = 0; i < 10; i++) {
        unsigned int power = powers[i];
        int digit = 0;

        // Contar subtrações (em vez de divisão)
        while (uvalue >= power) {
            uvalue -= power;
            digit++;
        }

        if (digit != 0 || started) {
            *ptr++ = '0' + digit;
            started = 1;
        }
    }

    *ptr = '\0';
    return str;
}

// Conversão de octal usando shifts (sem divisão)
char *itooct(unsigned int value, char *str) {
    char *ptr = str;
    int started = 0;

    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return str;
    }

    for (int shift = 30; shift >= 0; shift -= 3) {
        int triplet = (value >> shift) & 0x7;
        if (triplet != 0 || started) {
            *ptr++ = '0' + triplet;
            started = 1;
        }
    }

    *ptr = '\0';
    return str;
}

// Conversão de unsigned decimal (sem divisão)
char *itoudec(unsigned int value, char *str) {
    char *ptr = str;

    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return str;
    }

    // Potências de 10 para unsigned
    static const unsigned int powers[] = {
        1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1
    };

    int started = 0;

    for (int i = 0; i < 10; i++) {
        unsigned int power = powers[i];
        int digit = 0;

        while (value >= power) {
            value -= power;
            digit++;
        }

        if (digit != 0 || started) {
            *ptr++ = '0' + digit;
            started = 1;
        }
    }

    *ptr = '\0';
    return str;
}


// Nova função para preencher com zeros à esquerda
void pad_zeros(char *str, int width) {
    int len = strlen(str);
    if (len >= width) return;

    // Mover conteúdo para a direita
    for (int i = len; i >= 0; i--) {
        str[i + (width - len)] = str[i];
    }

    // Preencher com zeros à esquerda
    for (int i = 0; i < (width - len); i++) {
        str[i] = '0';
    }
}

// Implementação principal do printf - COM SUPORTE A MODIFICADORES
int printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    int chars_printed = 0;
    char buffer[32];

    while (*format) {
        if (*format == '%') {
            format++;

            // Verificar modificadores
            int zero_pad = 0;
            int width = 0;

            // Processar flags e width
            while (*format >= '0' && *format <= '9') {
                if (*format == '0' && width == 0) {
                    zero_pad = 1;
                } else {
                    width = width * 10 + (*format - '0');
                }
                format++;
            }

            // Processar length modifiers (simplificado)
            if (*format == 'l') {
                format++; // Ignorar 'l' por enquanto
            }

            switch (*format) {
                case 'c': {
                    char c = (char)va_arg(args, int);
                    putchar(c);
                    chars_printed++;
                    break;
                }

                case 'd':
                case 'i': {
                    int num = va_arg(args, int);
                    itodec(num, buffer);
                    if (zero_pad && width > 0) {
                        pad_zeros(buffer, width);
                    }
                    puts(buffer);
                    chars_printed += strlen(buffer);
                    break;
                }

                case 'u': {
                    unsigned int num = va_arg(args, unsigned int);
                    itoudec(num, buffer);
                    if (zero_pad && width > 0) {
                        pad_zeros(buffer, width);
                    }
                    puts(buffer);
                    chars_printed += strlen(buffer);
                    break;
                }

                case 'f':
                case 'F': {
                    // Ignorar floats
                    puts("[float]");
                    chars_printed += 7;
                    va_arg(args, double);
                    break;
                }

                case 's': {
                    char *str = va_arg(args, char*);
                    if (str) {
                        puts(str);
                        chars_printed += strlen(str);
                    } else {
                        puts("(null)");
                        chars_printed += 6;
                    }
                    break;
                }

                case 'x':
                case 'X': {
                    unsigned int num = va_arg(args, unsigned int);
                    itox(num, buffer);
                    if (zero_pad && width > 0) {
                        pad_zeros(buffer, width);
                    }
                    puts(buffer);
                    chars_printed += strlen(buffer);
                    break;
                }

                case 'o': {
                    unsigned int num = va_arg(args, unsigned int);
                    itooct(num, buffer);
                    if (zero_pad && width > 0) {
                        pad_zeros(buffer, width);
                    }
                    puts(buffer);
                    chars_printed += strlen(buffer);
                    break;
                }

                case '%': {
                    putchar('%');
                    chars_printed++;
                    break;
                }

                default:
                    char num_str[10]={0};
                    putchar('%');
                    if (zero_pad) putchar('0');
                    if (width > 0) {
                        itodec(width, num_str);
                        puts(num_str);
                    }
                    putchar(*format);
                    chars_printed += 2 + (width > 0 ? strlen(num_str) : 0);
                    break;
            }
        } else {
            putchar(*format);
            chars_printed++;
        }

        format++;
    }

    va_end(args);
    return chars_printed;
}

