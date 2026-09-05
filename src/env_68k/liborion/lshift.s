| integer shift operations with gcc math library names.
|_lshrsi3 (a, b)
|     unsigned a, b;
|	a >> b

	.globl __lshrsi3
__lshrsi3:
	movl	a6@(8),d0
	movw	a6@(14),d1
	lsrl	d1,d0


|_lshlsi3 (a, b)
|     unsigned a, b;
|	a << b

	.globl __lshlsi3
__lshlsi3:
	movl	a6@(8),d0
	movw	a6@(14),d1
	lsll	d1,d0


|_ashrsi3 (a, b)
|     int a, b;
|	a >> b

	.globl __ashrsi3
__ashrsi3:
	movl	a6@(8),d0
	movw	a6@(14),d1
	asrl	d1,d0


|_ashlsi3 (a, b)
|     int a, b;
|	a << b

	.globl __ashlsi3
__ashlsi3:
	movl	a6@(8),d0
	movw	a6@(14),d1
	asll	d1,d0

