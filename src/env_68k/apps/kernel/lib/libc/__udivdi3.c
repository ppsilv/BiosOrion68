// ============================================================
// 6. __udivdi3 - Divisão de 64 bits sem sinal
// ============================================================

/**
 * __udivdi3 - Divisão de 64 bits sem sinal
 * 
 * @param a: Dividendo (unsigned long long)
 * @param b: Divisor (unsigned long long)
 * @return: a / b
 * 
 * Algoritmo: Divisão longa (shift-subtract)
 */
unsigned long long __udivdi3(unsigned long long a, unsigned long long b) {
    // Trata divisão por zero
    if (b == 0) return 0;
    
    // Se a < b, o resultado é 0
    if (a < b) return 0;
    
    // Se b é potência de 2, usa shift (otimização)
    if ((b & (b - 1)) == 0) {
        // Conta quantos bits de shift
        int shift = 0;
        unsigned long long temp = b;
        while (temp > 1) {
            temp >>= 1;
            shift++;
        }
        return a >> shift;
    }
    
    // Algoritmo de divisão longa (shift-subtract)
    unsigned long long remainder = a;
    unsigned long long divisor = b;
    unsigned long long quotient = 0;
    int shift = 0;
    
    // Encontra o maior shift onde divisor <= remainder
    while (divisor <= remainder) {
        divisor <<= 1;
        shift++;
    }
    
    // Volta um passo
    divisor >>= 1;
    shift--;
    
    // Divide
    while (shift >= 0) {
        if (remainder >= divisor) {
            remainder -= divisor;
            quotient |= (1ULL << shift);
        }
        divisor >>= 1;
        shift--;
    }
    
    return quotient;
}