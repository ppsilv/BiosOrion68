#include <stdio.h>
#include <stdint.h>


void puts(const char *str) {
    while (*str) {
        putchar(*str++);
    }
}

