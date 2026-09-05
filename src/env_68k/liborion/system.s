| fast trap routine for most system calls
|**  maintrap service
| i.e.   system(CREATE, proc, priority)
	.even
	.text
	.globl	_system
_system:
	trap	#9
	rts
