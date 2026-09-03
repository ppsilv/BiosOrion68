
// ============================================================
// 8. __umoddi3 - Módulo de 64 bits sem sinal
// ============================================================

/**
 * __umoddi3 - Módulo de 64 bits sem sinal
 *
 * @param a: Dividendo (unsigned long long)
 * @param b: Divisor (unsigned long long)
 * @return: a % b
 *
 * Algoritmo: Divisão longa (shift-subtract)
 * Implementação eficiente para 64 bits
 */
unsigned long long __umoddi3(unsigned long long a, unsigned long long b) {
    // Trata divisão por zero
    if (b == 0) return 0;

    // Se a < b, o resto é a
    if (a < b) return a;

    // Se b é potência de 2, usa máscara (otimização)
    if ((b & (b - 1)) == 0) {
        return a & (b - 1);
    }

    // Algoritmo de divisão longa (shift-subtract)
    unsigned long long remainder = a;
    unsigned long long divisor = b;
    int shift = 0;

    // Encontra o maior shift onde divisor <= remainder
    while (divisor <= remainder) {
        divisor <<= 1;
        shift++;
    }

    // Volta um passo
    divisor >>= 1;
    shift--;

    // Subtrai repetidamente
    while (shift >= 0) {
        if (remainder >= divisor) {
            remainder -= divisor;
        }
        divisor >>= 1;
        shift--;
    }

    return remainder;
}
