|--------------------------------------------------------------------------
|
| ABSTRACT
|	BS ()- Do a simple byte swap.
|
| SYNOPSIS
|	bs (buffer, nbytes)
|	    char *buffer		where the bytes are
|	    int  nbytes			number of bytes
|
| RESULTS
|	None.
|
| SIDE EFFECTS
|	<nbytes / 2> words, starting at <buffer> are byte-swapped.
|
| ENVIRONMENT
|
|    Imports
|	Nothing.
|
|    Exports
|	bs ()			bs.s
|
| BUGS
|	It is assumed that buffer is word-aligned.
|
| DESIGN
|	Bring a word at a time and rotate eight times in a register.
|
| HISTORY
|  7-Nov-83  John Schlag (jfs) at Carnegie-Mellon University
|	Adapted for general use by changing dbra to subql, tstl.
|
|  1-Sep-83  Rafael Bracho (rxb) at Carnegie-Mellon University
|	Created. (Why?)
|----------------------------------------------------------------------------
|

	.text
	.even
	.globl	bs
bs:
	movl	sp@(4), a0	| get the buffer's first address
	movl	sp@(8), d0	| get the length
	addql	#1, d0		| length++
	asrl	#1, d0		| length >>=  1 (number of words)

loop:
	tstl	d0		| done yet?
	beq	ret
	movw	a0@, d1		| bring a word
	rolw	#8, d1		| swap the bytes
	movw	d1, a0@+	| put it out
	subql	#1, d0		| decrement and cycle around
	bra	loop
ret:	rts
