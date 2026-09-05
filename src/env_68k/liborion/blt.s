|Block transfer subroutine: blt(destination, source, count), returns count
|
|   5/22/91  Andrew H. Fagg  Changed branch location names so it would compile
|
	.globl	_blt
	.text

_blt:	movl	sp@(4),a0	|destination
	movl	sp@(8),a1	|source
	movl	a0,d0		|destination
	andl	#0x1,d0		|see if word aligned
	movl	a1,d1		|source
	andl	#0x1,d1		|see if long word aligned?
	cmpl	d0,d1		|do they agree?
	beq	jmp1		|yes, can do something interesting
	movl	sp@(12),d1	|count
	bra	jmp5		|ho, hum, just do byte moves
jmp1:	movl	sp@(12),d1	|count
	tstl	d0		|are we on a long boundry?
	beq	jmp3		|yes, don't worry about fudge
	negl	d0		|complement
	addql	#2,d0		|2 - adjustment = fudge
	cmpl	d1,d0		|is count bigger than fudge
	bge	jmp5		|no, must be 3 bytes or less
	subl	d0,d1		|shrink remaining count by this much
	subql	#1,d0
jmp2:	movb	a1@+,a0@+	|move bytes to get to long boundry
	subl	#1,d0
	bge	jmp2		|while alignment count
jmp3:	movl	d1,d0		|copy remaining count
	andl	#~3,d0		|count mod 4 is number of long words
	beq	jmp5		|hmm, must not be any
	subl	d0,d1		|long words moved * 4 = bytes moved
	asrl	#2,d0		|number of long words
	cmpl	#12,d0		|do we have a bunch to do?
	blt	jmp38		|no, just do normal moves
	moveml	#0x7F3E,sp@-	|save some registers
jmp34:	moveml	a1@+,#0x7CFE	|block move via various registers
	moveml	#0x7CFE,a0@
	addl	#48,a0		|moveml won't let me auto inc a destination
	subl	#12,d0		|we moved twelve longs worth
	cmpl	#12,d0		|do we have another 12 to go
	bge	jmp34		|yes, keep at it
	moveml	sp@+,#0x7CFE	|restore registers
	tstl	d0		|any long's left
	beq	jmp5		|no, nothing but a few random bytes
jmp38:	subql	#1,d0
jmp4:	movl	a1@+,a0@+	|copy as many long words as possible
	subl	#1,d0
	bge	jmp4		|while long word count
jmp5:	tstl	d1		|anything left to do?
	beq	jmp7		|nothing left
	subql	#1,d1
jmp6:	movb	a1@+,a0@+	|copy any residual bytes
	subl	#1,d1
	bge	jmp6		|while byte count
jmp7:	movl	sp@(12),d0	|just return the count
	rts
