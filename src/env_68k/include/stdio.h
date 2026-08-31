#ifndef STDIO_H
#define STDIO_H

#include <stddef.h>

void putchar(char c);
char getchar(void);

int printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);

void puts(const char *str);
char* gets_s( char* str, int n );
char kbdhit(void);


#endif
