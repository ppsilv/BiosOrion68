|--------------------------------------------------------------------
|
|  Abstract:
|	LREM (A, B) - signed remainder.
|
|  Parameters:
|	A - 32-bit dividend
|	B - 16-bit divisor
|
|  Results:
|	32-bit quotient, or remainder, of A / B
|
|  Side Effects:
|	None.
|
|  Environment:
|
|    Imports:
|	uldiv ()		uldiv.s
|
|    Exports:
|	lrem ()			lrem.s
|
|  Errors:
|	None known.
|
|  Design:
|	Massage the sign bits of the operands and pass on
|	to an unsigned divide routine. 'lrem()' takes advantage
|	of a known side effect in that routine to find the remainder.
|
| HISTORY
| 12-Apr-83  Rafael Bracho (rxb) at Carnegie-Mellon University
|	separated from ldiv () which takes care of sign.
|
| 11-Mar-83  Rafael Bracho (rxb) at Carnegie-Mellon University
|	Overflow detection added.
|
| 11-Jun-82  Jim McQuade (jpm) at Carnegie-Mellon University
|	Added lrem ().
|
| 08-Jun-82  Jim McQuade (jpm) at Carnegie-Mellon University
|	Created.
|--------------------------------------------------------------------

        .text
        .globl _lrem
        .globl __modsi3
__modsi3:
_lrem:
	link	a6, #0
	movl	a6@(12), sp@-
	movl	a6@( 8), sp@-
	jsr	_ldiv			| overflows are caught here!
	addql	#8, sp
	movl	d1, d0
	unlk	a6
	rts
