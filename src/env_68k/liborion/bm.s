|--------------------------------------------------------------------------
|
|  Abstract:
|	bm (to, from, nbytes)  - Fast block move.
|
|  Parameters:
|	a0 holds the source byte address
|	a1 holds the destination byte address
|	d0 holds the byte count
|
|  Returns:
|	Nothing.
|
|  Side Effects:
|	Source array copied to destination array.
|
|  Errors:
|	Quite a few.  Does NO error checking.
|	Pass it garbage and garbage you will get.
|	The source and destination arrays CANNOT overlap.
|
|  Design:
|	Brute force.
|
| HISTORY
|  7-Nov-83  John Schlag (jfs) at Carnegie-Mellon University
|	Fixed bug: changed dbra to standard subql, tstl.
|
| 19-Oct-83  John Schlag (jfs) at Carnegie-Mellon University
|	Fixed bug: check for zero byte count.
|
| 20-Sep-83  John Schlag (jfs) at Carnegie-Mellon University
|	Adapted for the new popeye (conan) library.
|
| 19-Jan-83  Rafael Bracho (rxb) at Carnegie-Mellon University
|	Fixed bug (discovered by McQuade) that was pushing the wrong
|	registers (d4, a6 and a7) and retrieving the right ones (a0, a1
|	and d0).
|
| 11-Mar-82  Jim McQuade (jpm) at Carnegie-Mellon University
|	Created.
|----------------------------------------------------------------------------
|
	.text
	.even
	.globl	_bm
	.globl	bm

_bm:
bm:
	movl	sp@(12), d0		| byte count
	movl	sp@(4), a0		| destination
	movl	sp@(8), a1		| source
loop:	tstl	d0			| byte count == 0 ?
	beq	ret
	movb	a1@+, a0@+		| copy byte
	subql	#0x01, d0		| minus one
	bra	loop			| loop while d0 >= zero
ret:	rts				| done
