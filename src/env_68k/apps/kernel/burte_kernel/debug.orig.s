#NO_APP
gcc_compiled.:
.text
LC0:
	.ascii " at: \0"
LC1:
	.ascii " process: \0"
LC2:
	.ascii "\15\12Ssp: \0"
LC3:
	.ascii "  Usp: \0"
LC4:
	.ascii "  status: \0"
LC5:
	.ascii "\15\12\0"
	.even
.globl _trap_mess
_trap_mess:
	link a6,#0
	moveml #0x3830,sp@-
	movel a6@(8),d3
	movel a6@(12),d4
	movel a6@(16),d2
	movew _current_proc,a0
	movel a0,sp@-
	jbsr _bad_id
	addqw #4,sp
	tstl d0
	jne L2
	movew _current_proc,a0
	movel a0,sp@-
	jbsr _r_suspend
	addqw #4,sp
L2:
	movel a6@(24),sp@-
	lea _force_print,a2
	jbsr a2@
	pea LC0
	jbsr a2@
	movel a6@(20),sp@-
	lea _force_hex,a3
	jbsr a3@
	pea LC1
	jbsr a2@
	movew _current_proc,a0
	movel a0,sp@-
	jbsr a3@
	pea LC2
	jbsr a2@
	movel d3,sp@-
	jbsr a3@
	pea LC3
	jbsr a2@
	addw #32,sp
	movel d4,sp@-
	jbsr a3@
	pea LC4
	jbsr a2@
	pea 2:w
	movel d2,sp@-
	jbsr _force_hexf
	pea LC5
	jbsr a2@
	addw #20,sp
	btst #13,d2
	jeq L3
	pea 48:w
	movel d3,sp@-
	jbsr _force_mem
	addqw #8,sp
L3:
	pea 48:w
	movel d4,sp@-
	jbsr _force_mem
	clrw d0
L6:
	addqw #1,d0
	cmpw #29999,d0
	jls L6
	moveml a6@(-20),#0xc1c
	unlk a6
	rts
LC6:
	.ascii " accessing: \0"
LC7:
	.ascii " status: \0"
	.even
.globl _addr_mess
_addr_mess:
	link a6,#0
	moveml #0x3830,sp@-
	movel a6@(8),d2
	movel a6@(12),d4
	movel a6@(16),d3
	movew _current_proc,a0
	movel a0,sp@-
	jbsr _bad_id
	addqw #4,sp
	tstl d0
	jne L9
	movew _current_proc,a0
	movel a0,sp@-
	jbsr _r_suspend
	addqw #4,sp
L9:
	movel a6@(28),sp@-
	lea _force_print,a2
	jbsr a2@
	pea LC0
	jbsr a2@
	movel a6@(20),sp@-
	lea _force_hex,a3
	jbsr a3@
	pea LC6
	jbsr a2@
	movel a6@(24),sp@-
	jbsr a3@
	pea LC1
	jbsr a2@
	movew _current_proc,a0
	movel a0,sp@-
	jbsr a3@
	pea LC2
	jbsr a2@
	addw #32,sp
	movel d2,sp@-
	jbsr a3@
	pea LC3
	jbsr a2@
	movel d4,sp@-
	jbsr a3@
	pea LC7
	jbsr a2@
	pea 2:w
	movel d3,sp@-
	jbsr _force_hexf
	pea LC5
	jbsr a2@
	pea 64:w
	movel d2,sp@-
	lea _force_mem,a2
	jbsr a2@
	addw #36,sp
	btst #13,d3
	jeq L10
	pea 64:w
	movel d4,sp@-
	jbsr a2@
L10:
	clrw d0
L13:
	addqw #1,d0
	cmpw #29999,d0
	jls L13
	moveml a6@(-20),#0xc1c
	unlk a6
	rts
LC8:
	.ascii "Trace at: \0"
LC9:
	.ascii " usp: \0"
LC10:
	.ascii "Csum at: \0"
LC11:
	.ascii " found: \0"
LC12:
	.ascii " not: \0"
	.even
.globl _trace_mess
_trace_mess:
	link a6,#0
	moveml #0x3830,sp@-
	movel a6@(8),d4
	movel a6@(12),d3
	clrl d2
	tstl _tcnt
	jle L16
	subql #1,_tcnt
	jne L16
	movew _current_proc,a1
	movel a1,sp@-
	jbsr _bad_id
	addqw #4,sp
	tstl d0
	jne L18
	movew _current_proc,a1
	movel a1,sp@-
	jbsr _r_suspend
	addqw #4,sp
L18:
	pea 9216:w
	jbsr _r_lock
	movel _termout,sp@-
	pea LC8
	lea _outs,a2
	jbsr a2@
	movel _termout,sp@-
	movel d3,sp@-
	lea _outx,a3
	jbsr a3@
	movel _termout,sp@-
	pea LC1
	jbsr a2@
	movel _termout,sp@-
	movew _current_proc,a1
	movel a1,sp@-
	jbsr a3@
	addw #36,sp
	movel _termout,sp@-
	pea LC9
	jbsr a2@
	movel _termout,sp@-
	movel d4,sp@-
	jbsr a3@
	movel _termout,sp@-
	pea LC5
	jbsr a2@
	movel _termout,sp@-
	jbsr _force_flush
	moveq #-1,d0
	jra L15
L16:
	tstl _csum_end
	jeq L19
	movel _csum_start,a0
	cmpl _csum_end,a0
	jcc L26
L23:
	moveb a0@+,d0
	extbl d0
	addl d0,d2
	cmpl _csum_end,a0
	jcs L23
L26:
	cmpl _csum,d2
	jeq L19
	movew _current_proc,a1
	movel a1,sp@-
	jbsr _bad_id
	addqw #4,sp
	tstl d0
	jne L25
	movew _current_proc,a1
	movel a1,sp@-
	jbsr _r_suspend
	addqw #4,sp
L25:
	pea 9216:w
	jbsr _r_lock
	movel _termout,sp@-
	pea LC10
	lea _outs,a2
	jbsr a2@
	movel _termout,sp@-
	movel d3,sp@-
	lea _outx,a3
	jbsr a3@
	movel _termout,sp@-
	pea LC1
	jbsr a2@
	movel _termout,sp@-
	movew _current_proc,a1
	movel a1,sp@-
	jbsr a3@
	addw #36,sp
	movel _termout,sp@-
	pea LC9
	jbsr a2@
	movel _termout,sp@-
	movel d4,sp@-
	jbsr a3@
	movel _termout,sp@-
	pea LC11
	jbsr a2@
	movel _termout,sp@-
	movel d2,sp@-
	jbsr a3@
	addw #32,sp
	movel _termout,sp@-
	pea LC12
	jbsr a2@
	movel _termout,sp@-
	movel _csum,sp@-
	jbsr a3@
	movel _termout,sp@-
	pea LC5
	jbsr a2@
	movel _termout,sp@-
	jbsr _force_flush
	movel d2,_csum
	moveq #-2,d0
	jra L15
L19:
	clrl d0
L15:
	moveml a6@(-20),#0xc1c
	unlk a6
	rts
	.even
.globl _step_break
_step_break:
	link a6,#0
	movel _old_trace_hndl,36:w
	jbsr _putbks
	tstl _csum_end
	jeq L28
	moveq #1,d0
	jra L27
L28:
	movew _current_proc,d0
	extl d0
	movel _proc_state,a0
	asll #7,d0
	andw #32767,a0@(2,d0:l)
	clrl d0
L27:
	unlk a6
	rts
LC13:
	.ascii "\12\15\0"
LC14:
	.ascii "timeout\12\15\0"
LC15:
	.ascii "larger\12\15\0"
LC16:
	.ascii " - end\12\15\0"
	.even
.globl _force_char
_force_char:
	link a6,#0
	moveml #0x3030,sp@-
	moveb a6@(11),d3
	moveq #1,d0
	movew 16776204,d2
	movew d2,d1
	andw #256,d1
L34:
	tstw d1
	jne L32
	addql #1,d0
	cmpl #799999,d0
	jle L34
L32:
	movel d0,d1
	divsll #800000,d0:d1
	tstl d0
	jne L35
	extl d2
	movel d2,sp@-
	lea _ph,a2
	jbsr a2@
	movew 16776202,d2
	pea LC13
	lea _ps,a3
	jbsr a3@
	movew d2,a0
	movel a0,sp@-
	jbsr a2@
	pea LC13
	jbsr a3@
	pea 256:w
	jbsr a2@
	pea LC13
	jbsr a3@
	pea LC14
	jbsr a3@
	addw #28,sp
	cmpw #255,d2
	jle L36
	pea LC15
	jbsr a3@
	addqw #4,sp
L36:
	movew 16776204,a0
	movel a0,sp@-
	jbsr _ph
	pea LC16
	jbsr _ps
L35:
	clrw d0
	moveb d3,d0
	movew d0,16776206
	moveml a6@(-16),#0xc0c
	unlk a6
	rts
	.even
.globl _force_print
_force_print:
	link a6,#0
	movel a2,sp@-
	movel a6@(8),a2
	clrw d0
	movew 16776204,d1
	andw #256,d1
L41:
	tstw d1
	jne L50
	addqw #1,d0
	cmpw #4999,d0
	jle L41
	jra L50
L44:
	extbl d0
	movel d0,sp@-
	jbsr _force_char
	addqw #4,sp
L50:
	moveb a2@+,d0
	jne L44
	clrw d0
	movew 16776204,d1
	andw #256,d1
L48:
	tstw d1
	jne L46
	addqw #1,d0
	cmpw #1999,d0
	jle L48
L46:
	movel a6@(-4),a2
	unlk a6
	rts
	.even
.globl _force_hex
_force_hex:
	link a6,#0
	moveml #0x2030,sp@-
	clrw d2
	lea a6@(8),a3
	movew #0,a2
L61:
	tstb a3@(a2:w)
	jne L56
	tstw d2
	jeq L54
L56:
	addqw #1,d2
	moveb a3@(a2:w),d0
	asrb #4,d0
	andw #15,d0
	cmpw #9,d0
	jgt L57
	movew d0,a0
	moveq #48,d0
	jra L63
L57:
	movew d0,a0
	moveq #55,d0
L63:
	addl a0,d0
	movel d0,sp@-
	jbsr _force_char
	moveb a3@(a2:w),d0
	andw #15,d0
	addqw #4,sp
	cmpw #9,d0
	jgt L59
	movew d0,a0
	moveq #48,d0
	jra L64
L59:
	movew d0,a0
	moveq #55,d0
L64:
	addl a0,d0
	movel d0,sp@-
	jbsr _force_char
	addqw #4,sp
L54:
	addqw #1,a2
	moveq #3,d1
	cmpw a2,d1
	jge L61
	tstw d2
	jne L62
	pea 48:w
	jbsr _force_char
L62:
	moveml a6@(-12),#0xc04
	unlk a6
	rts
	.even
.globl _force_hexf
_force_hexf:
	link a6,#0
	moveml #0x30,sp@-
	lea a6@(8),a3
	moveq #4,d1
	subw a6@(14),d1
	movew d1,a2
	jra L75
L73:
	moveb a3@(a2:w),d0
	asrb #4,d0
	andw #15,d0
	cmpw #9,d0
	jgt L69
	movew d0,a0
	moveq #48,d0
	jra L76
L69:
	movew d0,a0
	moveq #55,d0
L76:
	addl a0,d0
	movel d0,sp@-
	jbsr _force_char
	moveb a3@(a2:w),d0
	andw #15,d0
	addqw #4,sp
	cmpw #9,d0
	jgt L71
	movew d0,a0
	moveq #48,d0
	jra L77
L71:
	movew d0,a0
	moveq #55,d0
L77:
	addl a0,d0
	movel d0,sp@-
	jbsr _force_char
	addqw #4,sp
	addqw #1,a2
L75:
	moveq #3,d1
	cmpw a2,d1
	jge L73
	moveml a6@(-8),#0xc00
	unlk a6
	rts
LC17:
	.ascii "Non-existant memory between: \0"
LC18:
	.ascii " - \0"
LC19:
	.ascii "  \0"
LC20:
	.ascii " \0"
LC21:
	.ascii "   \0"
	.even
.globl _force_mem
_force_mem:
	link a6,#0
	moveml #0x3038,sp@-
	movel a6@(12),a4
	moveq #-2,d2
	andl a6@(8),d2
	lea a4@(15,d2:l),a1
	movel a1,d3
	moveq #-16,d1
	andl d1,d3
	movel d2,sp@-
	lea _r_is_mem,a2
	jbsr a2@
	addqw #4,sp
	tstl d0
	jeq L80
	movel d3,sp@-
	jbsr a2@
	addqw #4,sp
	tstl d0
	jne L79
L80:
	pea LC17
	lea _force_print,a2
	jbsr a2@
	movel d2,sp@-
	lea _force_hex,a3
	jbsr a3@
	pea LC18
	jbsr a2@
	movel d3,sp@-
	jbsr a3@
	pea LC5
	jbsr a2@
	jra L78
L79:
	movel d2,a2
	movel a2,a3
	clrw d3
	jra L96
L94:
	pea 3:w
	movel a2,sp@-
	jbsr _force_hexf
	pea LC19
	jbsr _force_print
	clrw d2
	addw #12,sp
L87:
	pea 2:w
	movew a3@+,a1
	movel a1,sp@-
	jbsr _force_hexf
	pea LC20
	jbsr _force_print
	addw #12,sp
	addqw #1,d2
	cmpw #7,d2
	jle L87
	pea LC21
	jbsr _force_print
	clrw d2
	addqw #4,sp
L93:
	cmpb #31,a2@
	jle L91
	cmpb #126,a2@
	jgt L91
	moveb a2@,d1
	extbl d1
	movel d1,sp@-
	jra L97
L91:
	pea 46:w
L97:
	jbsr _force_char
	addqw #4,sp
	addqw #1,d2
	addqw #1,a2
	cmpw #15,d2
	jle L93
	pea LC5
	jbsr _force_print
	addqw #4,sp
	addw #16,d3
L96:
	movew d3,a0
	cmpl a0,a4
	jgt L94
L78:
	moveml a6@(-20),#0x1c0c
	unlk a6
	rts
	.even
.globl _fprint_stack
_fprint_stack:
	link a6,#0
	pea 31:w
	pea a6@(12)
	jbsr _force_mem
	unlk a6
	rts
	.even
.globl _force_flush
_force_flush:
	link a6,#0
	movel a6@(8),a0
L100:
	tstl a0@
	jne L100
	unlk a6
	rts
	.even
.globl _ps
_ps:
	link a6,#0
	moveml #0x2020,sp@-
	movel a6@(8),a2
	clrl d2
	tstb a2@(d2:l)
	jeq L107
L106:
	moveb a2@(d2:l),d1
	extbl d1
	movel d1,sp@-
	jbsr _putcharacter
	addqw #4,sp
	addql #1,d2
	tstb a2@(d2:l)
	jne L106
L107:
	moveml a6@(-8),#0x404
	unlk a6
	rts
	.even
.globl _ph
_ph:
	link a6,#0
	moveml #0x2030,sp@-
	clrw d2
	lea a6@(8),a3
	movew #0,a2
L118:
	tstb a3@(a2:w)
	jne L113
	tstw d2
	jeq L111
L113:
	addqw #1,d2
	moveb a3@(a2:w),d0
	asrb #4,d0
	andw #15,d0
	cmpw #9,d0
	jgt L114
	movew d0,a0
	moveq #48,d0
	jra L120
L114:
	movew d0,a0
	moveq #55,d0
L120:
	addl a0,d0
	movel d0,sp@-
	jbsr _putcharacter
	moveb a3@(a2:w),d0
	andw #15,d0
	addqw #4,sp
	cmpw #9,d0
	jgt L116
	movew d0,a0
	moveq #48,d0
	jra L121
L116:
	movew d0,a0
	moveq #55,d0
L121:
	addl a0,d0
	movel d0,sp@-
	jbsr _putcharacter
	addqw #4,sp
L111:
	addqw #1,a2
	moveq #3,d1
	cmpw a2,d1
	jge L118
	tstw d2
	jne L119
	pea 48:w
	jbsr _putcharacter
L119:
	moveml a6@(-12),#0xc04
	unlk a6
	rts
.comm _old_trace_hndl,4
.comm _csum_end,4
.comm _csum_start,4
.comm _tcnt,4
.comm _csum,4
