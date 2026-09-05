|  BURTE  executive support routines
|   Dan Christian 4-8-87
|  these routines support C calls for BURTE
|
|	5/18/93		Andrew H. Fagg		Set up NXM_flag so that it
|       					  is in RAM.
|       2/27/93         Andrew H. Fagg          Update size of process table
|                                                 entry (from 136 to 152)
|       5/21/92         Andrew H. Fagg          Updated exception frame
|						  length in _clear_traps,
|						  run_a_proc, and execute.
|	3/21/92		Andrew H. Fagg		Updated for new version of gcc



	MAX_PR	=	16		| maximum number of processes

	ACTIVE	=	1		| schedualing states
	FROZEN	=	0

|  data structure definitions
| PR_STATE offsets
	SID	=	0
	SSREG	=	2
	SPC	=	4
	SREG	=	8
	EREG	=	SREG + 64
	SZSTATE = 	152 | 136 | 128

| ------------------imports
|		_r_trap
|		_top
|		_burte_error
|	variables
|		_proc_state
|		_current_proc
|		_burte_state

| ------------------- exports
|	_r_start
|	_trap_service
|	_r_lock
|	_r_save
|	_bad_id
|	_system

| external variables
.globl	_proc_state, _current_proc, _burte_state

|**  Initialy starts the scheduler
|	a process must have been already created by boot (in start.c)
|**  never returns.
	.text
	.even
	.globl	_r_boot
_r_boot:
	movl	#_system_stack_top,sp	|set supervisor stack
	movl	#ACTIVE,_burte_state
	jmp	new_proc


|------------------ new_proc
| call a new process.  This never returns!!!!!
|  It is assumed that there is nothing on the stack.
|      The original trap call must be cleared before _new_proc
|	or execute is called. (you must jmp to this routine)
|  final stack (simulated trap call)
|	return addr	ssp@(2)
|	status reg	ssp@

|*  The register usage
|	a0	restore state base
|	a1	register restore conter and scratch
|	d0	the process id to execute (passed in)
|	d1	scratch

	.even
	.globl	clear_trap
	.globl	new_proc
clear_trap:
	addl	#0x08,sp  	|remove trap call from stack  (Andy 6->8)
new_proc:
	movw	#0x2700,sr	|lock out interupts during update
	jsr	__top
	tstl	d0		| invalid proc = -1
	bge	execute

	movw	#0x2000,sr	|allow pending interupts to be serviced
	bra	new_proc	|cycle until something is ready.

execute:
	tstw	d0		| proc to execute is in d0
	blts	9f		| test < 0
	cmpw	#MAX_PR,d0	| is not a valid ID
	bges	9f
	movw	#SZSTATE,d1	|indexing constant
	muls	d1,d0		|note: d0,d1 must really be 16bit
	movl	#_proc_state,a0	|base address
	addl	d0,a0		|a0 is pointer to process state

	movw	#0x2700,sr	|lock out interupts during state restoration
	movw	a0@(SID),_current_proc
	movw	#0xA4,sp@-	|push format/vector   (Andy added)
	movl	a0@(SPC),sp@-	|push return pc
	movw	a0@(SSREG),sp@-	|push return status
	addl	#SREG,a0 	|a0 is now  base of registers
	movl	a0, a1
	moveml	a1@+,#0x00ff	|d0-d7
	addql	#8,a1		|skip over a0, a1
	moveml	a1@+,#0x7c00	|a2-a6
	movl	a0@(60),a1	|get usp
	movl	a1,usp		|set usp
	movl	a0@(36),sp@-	|push a1
	movl	a0@(32),sp@-	|push a0
	moveml	sp@+,#0x0300	|restore a0-a1
	rte	|execute process (restores priority level and interupts)

9:	|bad current proc
	movl	#11,sp@-
|	movw	#0x2700,sr	|disallow interupts
	jsr	_burte_error	|which calls exit()


|************ handle returns from exceptions
	.globl	run_a_proc
run_a_proc:
	addl	#8,sp 		|clear trap state (Andy : 6->8 to handle
				|   new exception format)
|	movw	0x2700,sr	|make sure that there are no changes
				| to update_state that we don't know about
	tstl	_update_state
	beq	7f
	jmp	new_proc	|run schedualer

7:		|restart previous process
	movw	_current_proc, d0
	jmp	execute		| restart previous process

|
| Set status register (you must be in supervisor mode to do this)
| returns a old status register
	.globl	_r_lock
_r_lock:
	clrl	d0
	movw	sr,d0
	movl	sp@(4),d1
	movw	d1,sr
	rts

|** save for trap and interupt calls
|**  system stack must only have trap calling info on it, as
|**    shown here
|** All state and registers are saved
|	trap return	sp@(24)
|	trap sr 	sp@(22)
|	save return	sp@(18)
|	sr	sp@(16)
|	a0	sp@(12)
|	a1	sp@(8)
|	d0	sp@(4)
|	d1	sp

	.globl	_r_save
_r_save:	| executive save
	movw	sr,sp@-			|lock out interupts
	movw	#0x2700,sr
	moveml	#0x00c0,sp@-		|a0-a1
	moveml	#0xc000,sp@-		|do-d1
	movw	_current_proc,d1	|array subscript
	blts	2f			|if d1 <0 or > MAX_PROC then this
	cmpw	#MAX_PR,d1		| is not a valid ID
	bges	2f

	movl	#SZSTATE,d0		|indexing constant
	muls	d1,d0
	movl	#_proc_state,a0
	addl	d0, a0			|particular state

	movl	sp@(24),a0@(SPC)	| pc from trap call
	movw	sp@(22),a0@(SSREG)	| sr from trap call
	movl	a0, a1			|copy base address
	addl	#(EREG-4),a1		| tail of register block at reg[14]
	moveml	sp@+,#0x0003		| restore d0-d1
	moveml	#0xfffe,a1@-		|save everything, a0,a1 are wrong
	movl	usp,a1			|get user stack pointer
	movl	a1,a0@(SREG+60)		|save user stack to a7
	movl	sp@+,a0@(SREG+32)	|save real a0
	movl	sp@+,a0@(SREG+36)	|save real a1
	movw	sp@+,sr			|restore interupt level
	rts

2:		|invalid id in _current_proc
	movw	#0x2700,sr
	movl	#12,sp@-	|error 1
	jsr	_burte_error	|die with a system error


|** user save. allows d0-d1, a0 to be trashed
|** The current sr flags will not be saved correctly
|	save return	sp

	.globl	_user_save
_user_save:	|user save
	movw	_current_proc,d1	|array subscript
	blts	3f			|if d1 <=0 or > MAX_PR then this
	cmpw	#MAX_PR,d1		| is not a valid ID
	bges	3f

	movl	#SZSTATE,d0		|indexing constant
	muls	d1,d0
	movl	#_proc_state,a0
	addl	d0, a0

	movl	sp,a0@(SPC)		| pc from trap call
	movw	sr,a0@(SSREG)
	addl	#EREG,a0		| tail of register block
	moveml	#0xffff, a0@-		|save everything
	rts

3:		|invalid id in _current_proc
	movl	#1,sp@-		|error 2
	jsr	_burte_error	|die with a system error
	rts


| checks for a valid id, register d0 is modified
|	sp@(4)	id
|	sp	return

	.globl	_bad_id
_bad_id:
	movl	sp@(4),d0		|id to check
	blts	4f			|if d1 <0 or >= MAX_PR then this
	cmpl	#MAX_PR,d0		| is not a valid ID
	bges	4f
	moveq	#0, d0			|valid id
	rts

4:			|bad id
	moveq	#-1,d0
	rts


	.globl	_r_is_mem	|test for the presece of memory
				|stolen from popI code (util.s)
	.data

|NXM_flag:	.skip	1
|NXM_flag:	.long
	.lcomm NXM_flag,1	|Force flag to bss segment.
	.text
	.even
_r_is_mem:
	BUSTRAP  = 0x08		| first trap location (bus error)

	movl	BUSTRAP, a1		| save the old bus error routine
	movl	#forget, BUSTRAP	| ignore bus errors
	clrb	NXM_flag
	movl	#-1, d0			| assume there is R/W memory

	movl	sp@(4), a0		| get the address to test
	movb	a0@, d1			| see if there is read memory
|	nop				| synchronize the bus error!
	btst	#0, NXM_flag
	beq	read_ok			| there is read memory!

	clrb	NXM_flag
	clrl	d0
	movw	#-1, d0			| assume there is write memory

read_ok:
	movb	d1, a0@
	nop				| synchronize the bus error!
	btst	#0, NXM_flag
	beq	write_ok		| I could write something

	clrw	d0			| tell the world, no write memory

write_ok:
	movl	a1, BUSTRAP		| restore the old bus error routine
	rts

|	Ignore a bus error.

forget:				| trap buss error, set flag
	movb	#1, NXM_flag		| set Non-eXistent-Memory flag
	addql	#8, sp			| flush stack
	rte				| return

