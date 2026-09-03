

// ============================================================
// 1. __divsi3 - Divisão de 32 bits (int)
// ============================================================

/**
 * __divsi3 - Divisão de 32 bits com sinal
 *
 * @param a: Dividendo
 * @param b: Divisor
 * @return: a / b
 *
 * NOTA: Se b == 0, retorna 0 (evita divisão por zero)
 */
/*
int __divsi3(int a, int b) {
    int result = 0;
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
        sign = -sign;
    }

    // Divisão por subtração (simples, mas funciona)
    while (a >= b) {
        a -= b;
        result++;
    }

    return result * sign;
}
*/

// TODO this only supports a positive 32bit numerator and a positive 16bit denominator
int __divsi3(int a, int b)
{
	int c;

	asm(
	"clr.l	%%d0\n"
	"clr.l	%%d1\n"
	"move.w	%1, %%d0\n"	// d0 <- low a
	"move.w	%2, %%d1\n"	// d1 <- low b
	"move.l %1, %%d2\n"
	"clr.w	%%d2\n"
	"swap	%%d2\n"		// d2 <- high a

	"divu	%%d1, %%d2\n"
	"move.w %%d2, %%d3\n"
	"swap	%%d3\n"
	"clr.w	%%d3\n"		// d3 <- high result
	"clr.w	%%d2\n"		// d2 <- remainder << 16
	"add.l	%%d2, %%d0\n"	// d0 <- (remainder << 16) + low a

	"divu	%%d1, %%d0\n"

	"swap	%%d0\n"
	"clr.w	%%d0\n"
	"swap	%%d0\n"		// clear upper d0 (throw away remainder)

	"add.l	%%d3, %%d0\n"
	"move.l	%%d0, %0\n"
	: "=r" (c)
	: "r" (a), "r" (b)
	: "%d0", "%d1", "%d2", "%d3"
	);
	return c;
}




