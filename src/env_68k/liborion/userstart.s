|--------------------------------------------------------------------------
|
| ABSTRACT
|	_START ()- Startup routine added to every file.
|	EXIT ()- User-callable routine to end an instance of a program.
|
| SYNOPSIS
|	<_start called when jumping to a user program>.
|
|	exit (ReturnStatus)
|	    short ReturnStatus;
|    Imports
|	main ()
|
|    Exports
|	exit ()			userstart.s
|
|----------------------------------------------------------------------------

	.text
	.even
	.globl	_stdin
	.globl	_stdout
	.globl	_stderr

_stdin:		.long	0	| file pointers for user process
_stdout:	.long	0
_stderr:	.long	0

	.text
	.even
	.globl	_start
	.globl	_exit
_start:
	jsr	_main		| go to user's program

_exit:				| note: a value is never returned
	movl	#8,sp@-		| suicide call
	jsr	_system
	
