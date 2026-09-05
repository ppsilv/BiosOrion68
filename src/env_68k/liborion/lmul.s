| 32-bit multiply routines with gcc calling names

.text
	.even
	.globl __mulsi3
	.globl _lmul
_lmul:
__mulsi3:
	link a6,#0
	moveml #0x3e00,sp@-
	movel a6@(8),d3		|d3 = mltcand = b
	movel d3,d5		|d5 = tmp2 = b
	movel d5,d6		|d6 = tmp3 = b
	movel a6@(12),d2	|d2 = mltplier = a
	movel d2,d4		|d4 = tmp1 = a

		swap	d4	|swap halves of tmp2
		swap	d5	|swap halves of tmp1
		mulu	d2,d3	|cand *= plier
		mulu	d5,d2	|plier *= tmp2
		mulu	d4,d6	|tmp3 *= tmp1
		mulu	d4,d5	|tmp2 *= tmp1
		swap	d3	|swap cand
		addw	d2,d3	|cand += plier
		clrl	d4	|tmp1 = 0
		addxl	d4,d5	|tmp2 += tmp1
		addw	d6,d3	|cand += tmp3
		addxl	d4,d5	|tmp2 += tmp1
		swap	d3	|swap cand
		clrw	d2	|low plier = 0
		swap	d2	|swap plier
		clrw	d6	|low tmp3 = 0
		swap	d6	|swap tmp3
		addl	d6,d2	|plier += tmp3
		addl	d5,d2	|plier += tmp2

	tstl a6@(12)		|b < 0
	jge L3
	subl a6@(8),d2		|plier -= a
L3:
	tstl a6@(8)		|a < 0
	jge L4
	subl a6@(12),d2		|plier -= a
L4:
	tstl d3			|cand >= 0
	jlt L5
	tstl d2
	jeq L7

		orb	#2,cc	|set overflow
		trapv		|overflow error trap

L6:
	jra L7
L5:
	moveq #-1,d1		|plier != -1
	cmpl d2,d1
	jeq L7

		orb	#2,cc	|set overflow
		trapv		|overflow error trap

L7:
	movel d3,d0		|return cand

	moveml a6@(-20),#0x7c
	unlk a6
	rts
