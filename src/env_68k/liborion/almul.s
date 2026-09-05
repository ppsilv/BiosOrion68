|J. Test	1/81
|addressed signed long multiply routine: *a = *a * b
|
| HISTORY
|  9-Mar-83  Rafael Bracho (rxb) at Carnegie-Mellon University
|	Changed to call lmul (in lmul.c) thus providing full 32 bits of
|	precision as well as overflow detection.
|

	.globl	almul
	.text

almul:	link	a6,#0
	movl	a6@(8), a0		| a0 = pointer to a
	movl	a6@(12), sp@-		| push 'b'
	movl	a0@, sp@-		| push 'a'
	jsr	lmul			| call lmul (result in d0)
	addql	#8, sp			| flush the stack
	movl	a6@(8), a0		| a0 = pointer to 'a'
	movl	d0, a0@			| store the result
	unlk	a6
	rts
