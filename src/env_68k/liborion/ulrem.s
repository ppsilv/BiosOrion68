|--------------------------------------------------------------------
|
|  Abstract:
|	ULREM (A, B) - unsigned 32-bit remainder
|
|  Parameters:
|	A - 32-bit dividend
|	B - 16-bit divisor
|
|  Results:
|	32-bit remainder of an unsigned divide.
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
|	ulrem ()		ulrem.s
|
|  Errors:
|	None known.
|
|  Design:
|	Take advantage of a known side-effect of uldiv ().
|
| HISTORY
| 11-Mar-83  Rafael Bracho (rxb) at Carnegie-Mellon University
|	Overflow detection is performed by uldiv ().
|
| 11-Jun-82  Jim McQuade (jpm) at Carnegie-Mellon University
|	Created.
|--------------------------------------------------------------------

	.text
        .globl _ulrem
        .globl __umodsi3
__umodsi3:
_ulrem:
	link	a6, #0
	movl	a6@(12), sp@-
	movl	a6@( 8), sp@-
	jsr	_uldiv
	addql	#8, sp
	movl	d1, d0
	unlk	a6
	rts
