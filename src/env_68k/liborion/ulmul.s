| 32-bit multiply routines with gcc calling names

	.text
	.even
	.globl _ulmul
	.globl __umulsi3
_ulmul:
__umulsi3:
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

	tstl d2			|plier != 0
	jeq $1

		orb	#2,cc	|set overflow
		trapv		|overflow error trap

$1:
	movel d3,d0		|return cand

	moveml a6@(-20),#0x7c
	unlk a6
	rts
