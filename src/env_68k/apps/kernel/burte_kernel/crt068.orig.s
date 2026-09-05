|--------------------------------------------------------------------------
|
| ABSTRACT
|	_START ()- Startup routine for booting BURTE
|	EXIT ()- User-callable routine to end an instance of a program.
|
| SYNOPSIS
|	<_start called when jumping to a user program>.
|
|	exit (ReturnStatus)
|	    short ReturnStatus;
|
| RESULTS
|	A longword: The most significant word contains the system's
|	idea of whether it was a normal execution. The least significant
|	word is the value (necessarily short) returned by the user.
|
| SIDE EFFECTS
|	User privilege mode is entered.
|
|    Imports
|	serial_init ()		serial.c
|	main ()
|
|	<calls _alloc () and _free () via the KrnTable (see KrnTable.h)>
|
|    Exports
|	isFirst ()
|	exit ()			userstart.s
|----------------------------------------------------------------------------


|	A couple of constants ...

	.comm	__STKTOP_, 4

	.text
	.even
	.globl	_start
	.globl	_boot
_start:
	nop			| make it a popI entry point
	nop			| waiste some space to make burte happy
	nop
	nop
	nop
	nop
	movw	sr,d0
	btst	#13,d0
	beq	$1		| jump to user mode start
	movl	#0x16000, a0	| start the user's stack
	movl	a0,usp
	movl	#0x14000, sp	| set system stack
	jsr	_boot		| start burte
	bra	_start

$1:		| user mode startup
	movl	#_boot,164	| set trap 9
	trap	#9
	bra	_start			| start over

| some system constants
.globl	_system_stack_top
	_system_stack_top =	0x2000
