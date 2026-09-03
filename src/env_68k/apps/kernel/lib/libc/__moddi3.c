

/**
 * __moddi3 - Módulo de 64 bits
 *
 * Retorna: a % b
 */
long long __moddi3(long long a, long long b) {
    int sign = 1;

    // Trata divisão por zero
    if (b == 0) return 0;

    // Trata sinais
    if (a < 0) {
        a = -a;
        sign = -sign;
    }
    if (b < 0) {
        b = -b;
    }

    // Módulo por subtração
    while (a >= b) {
        a -= b;
    }

    return a * sign;
}
