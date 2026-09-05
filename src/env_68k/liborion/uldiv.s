|--------------------------------------------------------------------
|
|  Abstract:
|	ULDIV (A, B) - unsigned divide
|
|  Parameters:
|	A - 32-bit dividend
|	B - 16-bit divisor
|
|  Results:
|	Integer quotient of A / B .
|
|  Side Effects:
|	Remainder is available in data register 1.
|
|  Environment:
|
|    Imports:
|	None.
|
|    Exports:
|	uldiv			uldiv.s
|
|  Errors:
|	None known.
|
|  Design:
|	From 'The 68000: Principles and Programming' by Leo J. Scanlon
|
| HISTORY
| 11-Mar-83  Rafael Bracho (rxb) at Carnegie-Mellon University
|	Overflow protection added.
|
| 02-Jun-82  Jim McQuade (jpm) at Carnegie-Mellon University
|	Created.
|--------------------------------------------------------------------

	.text
        .globl _uldiv
        .globl __udivsi3
__udivsi3:
_uldiv:
	link	a6, #0			| Context swap.
	moveml	#0x3000, sp@-

	movl	a6@( 8), d0		| Get parameters.
	movl	a6@(12), d1		| d1 - divisor; d0 - dividend.
	swap	d1
	tstw	d1			| should be zero!
	bne	overflow

	swap	d1
	clrw	d3			| zero low word.
	divu	d1, d0			| Has overflow occured ?
					| (Side effect: if divisor was
					| zero an exception was generated).

	bvcs	format			|   No. Format results.

	movw	d0, d2			|   Yes. Copy Y0 to d2.
	clrw	d0			| d0 changes from Y1-Y0 to Y1-0.
	swap	d0			| d0 contains 0-Y1.
	divu	d1, d0			| divide puts R1-Q1 into d0.
	movw	d0, d3			| d3 contains Q1.
	movw	d2, d0			| d0 changes from R1-Q1 to R1-Y0.
	divu	d1, d0			| divide puts R0-Q0 into d0.
|
|  Format quotient (d0) and remainder (d1).
|
format:
	movl	d0, d1			| d1 contains R-Q or R0-Q0.
	swap	d0			| d0 contains Q-R or Q0-R0.
	movw	d3, d0			| d0 contains Q-0 or Q0-Q1.
	swap	d0			| d0 contains 0-Q or Q1-Q0.
	clrw	d1			| d1 contains R-0 or R0-0.
	swap	d1			| d1 contains 0-R or 0-R0.

	moveml	sp@+, #0x000C		| Restore context.
	unlk	a6
	rts

overflow:				| more than 16 bits of divisor!
	orb	#2, cc			| set the overflow bit
	trapv				| and trap it
