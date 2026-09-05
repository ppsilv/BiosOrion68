|--------------------------------------------------------------------------
|
| ABSTRACT
|	WS ()- Do a word swap and byte swap.
|
| SYNOPSIS
|	ws (buffer, nbytes)
|	    char *buffer		where the bytes are
|	    int  nbytes			number of bytes
|
| RESULTS
|	None.
|
| SIDE EFFECTS
|	<nbytes / 4> longs, starting at <buffer> are (word/byte)-swapped.
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
|	It is assumed that <buffer> is long-aligned.
|
| DESIGN
|	Straightforward bit fiddling.
|
| HISTORY
|  7-Nov-83  John Schlag (jfs) at Carnegie-Mellon University
|	Created by hacking bs(), the byteswapping utility.
|----------------------------------------------------------------------------
|

	.text
	.even
	.globl	ws
ws:
	movl	sp@(4), a0	| get the buffer's first address
	movl	sp@(8), d0	| get the length
	addql	#3, d0		| round to multiple of four
	asrl	#2, d0

loop:
	tstl	d0		| done yet?
	beq	ret
	movl	a0@, d1		| bring a long
	rolw	#8, d1		| swap bytes 0 & 1
	swap	d1		| bring 3 & 4 to 0 & 1
	rolw	#8, d1		| swap the bytes
	movl	d1, a0@+	| put it out
	subql	#1, d0		| decrement and cycle around
	bra	loop
ret:	rts
