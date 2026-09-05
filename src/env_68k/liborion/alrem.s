|J. Test	1/81
|addressed signed long remainder: *dividend = *dividend % divisor
|
| HISTORY
| 11-Mar-83  Rafael Bracho (rxb) at Carnegie-Mellon University
|	Changed so it calls lrem (in ldiv.s) and thus get full 32 bit
|	precision and overflow detection.
|
	.globl	alrem
	.text

alrem:	link	a6,#0
	movl	a6@(8), a0	| a0 = dividend pointer
	movl	a6@(12), sp@-	| push divisor
	movl	a0@, sp@-	| push dividend
	jsr	lrem
	addql	#8, sp		| flush the stack
	movl	a6@(8), a0
	movl	d0, a0@		| store the result
	unlk	a6
	rts
