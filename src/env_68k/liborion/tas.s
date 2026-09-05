|  test and set instruction
|  tas (addr)

.globl _tas
.text
.even

_tas:	
	moveq	#0,d0
	movl	sp@(4),a0
	tas	a0@
	bnes	1f
	rts
1:
	moveq	#-1,d0
	rts
