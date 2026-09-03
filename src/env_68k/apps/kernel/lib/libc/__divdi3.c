// ============================================================
// Divisão para 64 bits (__divdi3 e __moddi3)
// ============================================================

/**
 * __divdi3 - Divisão de 64 bits
 *
 * Retorna: a / b
 */
long long __divdi3(long long a, long long b) {
    long long result = 0;
    long long remainder = 0;
    int sign = 1;
    int i;

    // Trata divisão por zero
    if (b == 0) return 0;

    // Trata sinais
    if (a < 0) {
        a = -a;
        sign = -sign;
    }
    if (b < 0) {
        b = -b;
        sign = -sign;
    }

    // Divisão por subtração (lenta, mas funciona)
    while (a >= b) {
        a -= b;
        result++;
    }

    return result * sign;
}
