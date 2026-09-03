// ============================================================
// libgcc_stubs.h - Funções da libgcc
// ============================================================

#ifndef __KLIBC_H__
#define __KLIBC_H__

#include <stdint.h>

long long __divdi3(long long a, long long b);
int       __divsi3(int a, int b);
long long __moddi3(long long a, long long b);
int       __modsi3(int a, int b);
long long __muldi3(long long a, long long b);
int       __mulsi3(int a, int b);
unsigned long long __udivdi3(unsigned long long a, unsigned long long b);
unsigned int       __udivsi3(unsigned int a, unsigned int b);
unsigned long long __umoddi3(unsigned long long a, unsigned long long b);
unsigned int       __umodsi3(unsigned int a, unsigned int b);
unsigned long long __umuldi3(unsigned long long a, unsigned long long b);
unsigned int       __umulsi3(unsigned int a, unsigned int b);


#endif