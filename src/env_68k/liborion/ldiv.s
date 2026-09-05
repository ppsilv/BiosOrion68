|--------------------------------------------------------------------
|
|  Abstract:
|	LDIV (A, B) - signed divide.
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
|	ldiv ()			ldiv.s
|
|  Errors:
|	None known.
|
|  Design:
|	Massage the sign bits of the operands and pass on
|	to an unsigned divide routine.
|
| HISTORY
| 24-Apr-92  Andrew H Fagg (ahfagg@pollux.usc.edu) fixed .bss reference
|
| 12-Apr-83  Rafael Bracho (rxb) at Carnegie-Mellon University
|	lrem () taken out of here. It now makes sure that both d0 and d1
|	have the right sign.
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

	|.bss
	.data
	.even
sign:	.skip 4

	.text
	.globl _ldiv
        .globl __divsi3
__divsi3:
_ldiv:
	link	a6, #0
	movl	#0, sign		| assume positive

	movl	a6@(8), d0
	bpl	ldiv1

	notl	sign
	negl	d0
ldiv1:
	movl	a6@(12), d1		| get the full divisor (32 bits)
	swap	d1			| check the sign of divisor
	bpl	ldiv3
	cmpw	#0xFFFF, d1		| should be negative sign extension!
	bne	overflow		| nope
ldiv2:
	notl	sign
	swap	d1
	negl	d1			| make it a positive long!
	bra	ldiv4
ldiv3:
	tstw	d1			| should be positive sign extension!
	bne	overflow		| nope
	swap	d1
ldiv4:
	movl	d1, sp@-
	movl	d0, sp@-
	jsr	_uldiv
	addql	#8, sp

	tstl	sign
	beq	ldiv5

	negl	d0
	negl	d1
ldiv5:
	unlk	a6
	rts

overflow:
	orb	#2, cc			| set the overflow flag
	trapv				| and trap it!
