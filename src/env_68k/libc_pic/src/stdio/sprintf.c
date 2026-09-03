#include <stdio.h>
#include <stdarg.h>
#include <string.h>


extern char *itodec(int value, char *str);
extern char *itooct(unsigned int value, char *str);
extern char *itoudec(unsigned int value, char *str);
extern char *itox(unsigned int value, char *str);
extern void pad_zeros(char *str, int width);

int sprintf(char *str, const char *format, ...) {
    va_list args;
    va_start(args, format);

    char *original_str = str;
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

            switch (*format) {
                case 'c': {
                    char c = (char)va_arg(args, int);
                    *str++ = c;
                    break;
                }

                case 'd':
                case 'i': {
                    int num = va_arg(args, int);
                    itodec(num, buffer);
                    if (zero_pad && width > 0) {
                        pad_zeros(buffer, width);
                    }
                    char *temp = buffer;
                    while (*temp) {
                        *str++ = *temp++;
                    }
                    break;
                }

                case 'u': {
                    unsigned int num = va_arg(args, unsigned int);
                    itoudec(num, buffer);
                    if (zero_pad && width > 0) {
                        pad_zeros(buffer, width);
                    }
                    char *temp = buffer;
                    while (*temp) {
                        *str++ = *temp++;
                    }
                    break;
                }

                case 'f':
                case 'F': {
                    // Ignorar floats
                    char *temp = "[float]";
                    while (*temp) {
                        *str++ = *temp++;
                    }
                    va_arg(args, double);
                    break;
                }

                case 's': {
                    char *s = va_arg(args, char*);
                    if (!s) s = "(null)";
                    while (*s) {
                        *str++ = *s++;
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
                    char *temp = buffer;
                    while (*temp) {
                        *str++ = *temp++;
                    }
                    break;
                }

                case 'o': {
                    unsigned int num = va_arg(args, unsigned int);
                    itooct(num, buffer);
                    if (zero_pad && width > 0) {
                        pad_zeros(buffer, width);
                    }
                    char *temp = buffer;
                    while (*temp) {
                        *str++ = *temp++;
                    }
                    break;
                }

                case '%': {
                    *str++ = '%';
                    break;
                }

                default:
                    *str++ = '%';
                    *str++ = *format;
                    break;
            }
        } else {
            *str++ = *format;
        }

        format++;
    }

    *str = '\0'; // Terminar a string
    va_end(args);

    return str - original_str; // Retornar número de caracteres escritos
}
