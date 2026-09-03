// ============================================================
// kprintf.c - Printf para o kernel Orion68DOS
// Versão CORRETA e COMPILÁVEL!
// ============================================================

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <kernel.h>

// ============================================================
// FUNÇÃO DE SAÍDA (substitua pelo seu hardware!)
// ============================================================

static void putchar_kernel(char c) {
    // ============================================================
    // SUBSTITUA AQUI PELA SUA SAÍDA!
    // Exemplo: video_putchar(c) ou uart_putchar(c)
    // ============================================================
    
    // TODO: Implemente sua saída real aqui!
    // Por enquanto, faz nada (evita warning)
    (void)c;
}

static void puts_kernel(const char *s) {
    while (*s) {
        putchar_kernel(*s++);
    }
}

// ============================================================
// CONVERSÃO DE NÚMEROS
// ============================================================

static char *itoa_kernel(int value, char *str, int base) {
    char *ptr = str;
    char *start = str;
    int num = value;
    bool negative = false;
    
    if (base < 2 || base > 36) {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }
    
    if (num < 0 && base == 10) {
        negative = true;
        num = -num;
    }
    
    if (num == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return str;
    }
    
    while (num > 0) {
        int digit = num % base;
        if (digit < 10) {
            *ptr++ = '0' + digit;
        } else {
            *ptr++ = 'a' + (digit - 10);
        }
        num /= base;
    }
    
    if (negative) {
        *ptr++ = '-';
    }
    
    *ptr = '\0';
    
    // Inverte a string
    ptr--;
    while (start < ptr) {
        char temp = *start;
        *start++ = *ptr;
        *ptr-- = temp;
    }
    
    return str;
}

static char *ultoa_kernel(unsigned long value, char *str, int base) {
    char *ptr = str;
    char *start = str;
    unsigned long num = value;
    
    if (base < 2 || base > 36) {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }
    
    if (num == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return str;
    }
    
    while (num > 0) {
        int digit = num % base;
        if (digit < 10) {
            *ptr++ = '0' + digit;
        } else {
            *ptr++ = 'a' + (digit - 10);
        }
        num /= base;
    }
    
    *ptr = '\0';
    
    // Inverte
    ptr--;
    while (start < ptr) {
        char temp = *start;
        *start++ = *ptr;
        *ptr-- = temp;
    }
    
    return str;
}

// ============================================================
// kprintf - A FUNÇÃO PRINCIPAL (COMPILÁVEL!)
// ============================================================

void kprintf(const char *fmt, ...) {
    va_list args;
    char buffer[32];
    const char *p;
    
    va_start(args, fmt);
    
    for (p = fmt; *p; p++) {
        if (*p != '%') {
            putchar_kernel(*p);
            continue;
        }
        
        p++;
        if (!*p) break;
        
        switch (*p) {
            case 'c': {
                char c = (char)va_arg(args, int);
                putchar_kernel(c);
                break;
            }
            
            case 'd':
            case 'i': {
                int val = va_arg(args, int);
                itoa_kernel(val, buffer, 10);
                puts_kernel(buffer);
                break;
            }
            
            case 'u': {
                unsigned int val = va_arg(args, unsigned int);
                ultoa_kernel(val, buffer, 10);
                puts_kernel(buffer);
                break;
            }
            
            case 'x': {
                unsigned int val = va_arg(args, unsigned int);
                ultoa_kernel(val, buffer, 16);
                puts_kernel(buffer);
                break;
            }
            
            case 'X': {
                unsigned int val = va_arg(args, unsigned int);
                ultoa_kernel(val, buffer, 16);
                char *s = buffer;
                while (*s) {
                    if (*s >= 'a' && *s <= 'f') {
                        *s = *s - 'a' + 'A';
                    }
                    s++;
                }
                puts_kernel(buffer);
                break;
            }
            
            case 'p': {
                void *ptr = va_arg(args, void*);
                putchar_kernel('0');
                putchar_kernel('x');
                ultoa_kernel((unsigned long)ptr, buffer, 16);
                puts_kernel(buffer);
                break;
            }
            
            case 's': {
                char *str = va_arg(args, char*);
                if (str == NULL) {
                    puts_kernel("(null)");
                } else {
                    puts_kernel(str);
                }
                break;
            }
            
            case '%': {
                putchar_kernel('%');
                break;
            }
            
            default: {
                putchar_kernel('%');
                putchar_kernel(*p);
                break;
            }
        }
    }
    
    va_end(args);
}

void kputc(char c) {
    putchar_kernel(c);
}

void kputs(const char *s) {
    puts_kernel(s);
    putchar_kernel('\n');
}