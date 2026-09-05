| various simple system interupt servers
|	3/21/92		Andrew H. Fagg		Updated for new version of gcc

.text
bus:	.asciz	"Bus error"
addr:	.asciz	"Address error"
trp:	.asciz	"Uninitialized trap"
illeg:	.asciz	"Breakpoint"
zro:	.asciz	"Divide by 0"
chkt:	.asciz	"chk trap"
trv:	.asciz	"Trapv trap"
prv:	.asciz	"privelege violation"
trc:	.asciz	"trace"
ln0:	.asciz	"line 1010 emulate trap"
ln1:	.asciz	"line 1111 emulate trap"
ub:	.asciz	"programmer break"

.text
.even
.globl	_bus_error
.globl	_addr_error
.globl	_trap_error
.globl	_illegal
.globl	_zero
.globl	_chk_trap
.globl	_trapv
.globl	_privelege
.globl	_trace
.globl	_step_break_h
.globl	_line1010
.globl	_line1111
.globl	_ubug
.globl _addr_mess	| in debug.c
.globl _trap_mess
.globl	_step_break
.globl	_restart, _new_shell	| in start.c

_bus_error:
	btst	#5,sp@		| test state
	bne	9f		| in super, dont save state
	jsr	_r_save		| save user state
9:	pea	bus		| push error message

add_trap:
	movl	sp@(6),sp@-	| push address accessed
	movl	sp@(18),sp@-	| push instr address
	clrl	d0
	movw	sp@(20),d0
	movl	d0,sp@-		| push status
	movl	usp,a0		| push user stack
	movl	a0,sp@-
5:	pea	sp@(34)		| push super stack
6:	jsr	_addr_mess
	movl	#_system_stack_top,sp
	jsr	_restart	| never returns


_addr_error:
	btst	#5,sp@		| test state
	bne	9f		| in super, dont save state
	jsr	_r_save		| save user state
9:	pea	addr		| push error message
	bras	add_trap

_trap_error:
	btst	#5,sp@		| test state
	bne	9f		| in super, dont save state
	jsr	_r_save		| save user state
9:	pea	trp		| push error message

do_trap:	| bulk of most of the trap routines
	movl	sp@(6),sp@-	| push address of error
	clrl	d0
	movw	sp@(8),d0	| push status
	movl	d0,sp@-
	movl	usp,a0		| push user stack
	movl	a0,sp@-
	pea	sp@(22)		| push super stack
	jsr	_trap_mess
	addl	#20,sp
	jsr	_new_shell	| never returns

_illegal:
	btst	#5,sp@		| test state
	bne	9f		| in super, dont save state
	jsr	_r_save		| save user state
9:	pea	illeg		| push error message
	bra	do_trap

_zero:
	btst	#5,sp@		| test state
	bne	9f		| in super, dont save state
	jsr	_r_save		| save user state
9:	pea	zro		| push error message
	bra	do_trap

_chk_trap:
	btst	#5,sp@		| test state
	bne	9f		| in super, dont save state
	jsr	_r_save		| save user state
9:	pea	chkt		| push error message
	bra	do_trap

_trapv:
	btst	#5,sp@		| test state
	bne	9f		| in super, dont save state
	jsr	_r_save		| save user state
9:	pea	trv		| push error message
	bra	do_trap

_privelege:
	btst	#5,sp@		| test state
	bne	9f		| in super, dont save state
	jsr	_r_save		| save user state
9:	pea	prv		| push error message
	bra	do_trap

_trace:
	moveml	#0xc0c0, sp@-	| save d0,d1,a0,a1
	movl	sp@(18),sp@-	| push address of trace
	movl	usp, a0
	movl	a0, sp@-	| push user stack
	jsr	_trace_mess
	addql	#8,sp
	tstl	d0
	bne	8f
	moveml	sp@+,#0x0303	| restore d0,d1,a0,a1
	rte
8:	| trace error, return to shell
	moveml	sp@+,#0x0303	| restore d0,d1,a0,a1
	btst	#5,sp@		| test state
	bne	9f		| in super, dont save state
	jsr	_r_save		| save user state
9:	jsr	_new_shell	| go back to shell


_step_break_h:
	moveml	#0xc0c0, sp@-	| save d0,d1,a0,a1
	jsr	_step_break
	tstl	d0
	bne	8f
	moveml	sp@+,#0x0303	| restore d0,d1,a0,a1
	rte
8:	| trace on, call trace routine
	moveml	sp@+,#0x0303	| restore d0,d1,a0,a1
	movel	36,sp@-		| get trace routine address
	rts			| jump to trace routine


_line1010:
	btst	#5,sp@		| test state
	bne	9f		| in super, dont save state
	jsr	_r_save		| save user state
9:	pea	ln0		| push error message
	bra	do_trap

_line1111:
	btst	#5,sp@		| test state
	bne	9f		| in super, dont save state
	jsr	_r_save		| save user state
9:	pea	ln1		| push error message
	bra	do_trap

_ubug:			| debugging interupt
	btst	#5,sp@		| test state
	bne	9f		| in super, dont save state
	jsr	_r_save		| save user state
9:	moveml	#0xc0c0, sp@-	| save d0,d1,a0,a1
	pea	ub		| push error message
	movl	sp@(22),sp@-	| push address
	clrl	d0
	movw	sp@(24),d0
	movl	d0,sp@-		| push status
	movw	sr,d0
	movl	usp,a0		| push user stack
	movl	a0,sp@-
	pea	sp@(38)		| push super stack
	jsr	_trap_mess
	addl	#20,sp
	moveml	sp@+,#0x0303	| restore d0,d1,a0,a1
	jsr	_restart
	rte
