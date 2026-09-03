#include <stdint.h>

// ============================================================
// 10. __umuldi3 - Multiplicação de 64 bits sem sinal
// ============================================================

/**
 * __umuldi3 - Multiplicação de 64 bits sem sinal
 * 
 * @param a: Multiplicando (unsigned long long)
 * @param b: Multiplicador (unsigned long long)
 * @return: a * b
 */
unsigned long long __umuldi3(unsigned long long a, unsigned long long b) {
    // Divide em partes de 32 bits
    uint32_t a_low = (uint32_t)(a & 0xFFFFFFFF);
    uint32_t a_high = (uint32_t)(a >> 32);
    uint32_t b_low = (uint32_t)(b & 0xFFFFFFFF);
    uint32_t b_high = (uint32_t)(b >> 32);
    
    // Multiplicação usando 32 bits
    // (igual à versão com sinal, mas sem tratar sinais)
    
    uint64_t result_high = 0;
    uint64_t result_low = 0;
    
    // a_low * b_low
    uint64_t p0 = (uint64_t)a_low * b_low;
    
    // a_high * b_low + a_low * b_high
    uint64_t p1 = (uint64_t)a_high * b_low + (uint64_t)a_low * b_high;
    
    // a_high * b_high
    uint64_t p2 = (uint64_t)a_high * b_high;
    
    result_low = p0;
    result_high = p2;
    
    // Adiciona p1 (parte média)
    uint64_t p1_high = p1 >> 32;
    uint64_t p1_low = p1 << 32;
    
    result_low += p1_low;
    if (result_low < p1_low) {
        result_high++;  // Carry
    }
    
    result_high += p1_high;
    result_high += p2;
    
    return (result_high << 32) | result_low;
}