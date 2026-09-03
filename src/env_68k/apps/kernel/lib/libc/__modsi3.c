// ============================================================
// Divisão e módulo para 32 bits (__modsi3)
// ============================================================

/**
 * __modsi3 - Módulo para 32 bits (int)
 *
 * Retorna: a % b
 */
/*
int __modsi3(int a, int b) {
    int result = 0;
    int sign = 1;

    // Trata divisão por zero (retorna 0)
    if (b == 0) return 0;

    // Trata sinais
    if (a < 0) {
        a = -a;
        sign = -1;
    }
    if (b < 0) {
        b = -b;
    }

    // Divisão inteira simples (loop)
    while (a >= b) {
        a -= b;
    }

    return a * sign;
}
*/
// TODO this only supports a positive 32bit numerator and a positive 16bit denominator
int __modsi3(int a, int b)
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
	"clr.w	%%d2\n"		// d2 <- remainder << 16
	"add.l	%%d2, %%d0\n"	// d0 <- (remainder << 16) + low a

	"divu	%%d1, %%d0\n"

	"clr.w	%%d0\n"
	"swap	%%d0\n"
	"move.w	%%d0, %0\n"
	: "=r" (c)
	: "r" (a), "r" (b)
	: "%d0", "%d1", "%d2"
	);
	return c;
}



