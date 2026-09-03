#include <stdint.h>

// ============================================================
// 9. __muldi3 - Multiplicação de 64 bits com sinal
// ============================================================

/**
 * __muldi3 - Multiplicação de 64 bits com sinal
 * 
 * @param a: Multiplicando (long long)
 * @param b: Multiplicador (long long)
 * @return: a * b
 * 
 * Algoritmo: Multiplicação de 32 bits em partes
 * (a_high * 2^32 + a_low) * (b_high * 2^32 + b_low)
 */
long long __muldi3(long long a, long long b) {
    // Divide em partes de 32 bits
    uint32_t a_low = (uint32_t)(a & 0xFFFFFFFF);
    uint32_t a_high = (uint32_t)(a >> 32);
    uint32_t b_low = (uint32_t)(b & 0xFFFFFFFF);
    uint32_t b_high = (uint32_t)(b >> 32);
    
    // Multiplicação usando 32 bits
    // result = a * b
    // = (a_high * 2^32 + a_low) * (b_high * 2^32 + b_low)
    // = a_high * b_high * 2^64
    // + (a_high * b_low + a_low * b_high) * 2^32
    // + a_low * b_low
    
    uint64_t result_high = 0;
    uint64_t result_low = 0;
    
    // a_low * b_low (parte 0)
    uint64_t p0 = (uint64_t)a_low * b_low;
    
    // a_high * b_low + a_low * b_high (parte 1)
    uint64_t p1 = (uint64_t)a_high * b_low + (uint64_t)a_low * b_high;
    
    // a_high * b_high (parte 2)
    uint64_t p2 = (uint64_t)a_high * b_high;
    
    // Combina os resultados
    // p0 é a parte baixa (32 bits)
    // p1 é a parte média (32 bits, com carry)
    // p2 é a parte alta (32 bits)
    
    result_low = p0;
    result_high = p2;
    
    // Adiciona p1 (parte média)
    // p1 << 32 = p1_high * 2^32 + p1_low
    uint64_t p1_high = p1 >> 32;
    uint64_t p1_low = p1 << 32;
    
    result_low += p1_low;
    if (result_low < p1_low) {
        result_high++;  // Carry
    }
    
    result_high += p1_high;
    result_high += p2;
    
    // Converte para signed (considera sinais)
    // O resultado final é unsigned, mas o retorno é signed
    long long result = (long long)((result_high << 32) | result_low);
    
    // Aplica sinal (se a ou b for negativo)
    if ((a < 0) ^ (b < 0)) {
        result = -result;
    }
    
    return result;
}
