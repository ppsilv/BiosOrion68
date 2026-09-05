#NO_APP
.text
LC0:
	.ascii "mdl68 \0"
.text
LC1:
	.ascii "\12\0"
.text
LC2:
	.ascii "\12download: No memory between \0"
.text
LC3:
	.ascii " and \0"
.text
LC4:
	.ascii "\12Execution address: \0"
.text
LC5:
	.ascii " (\0"
.text
LC6:
	.ascii " bytes).\12\0"
.text
	.even
.globl _r_download
_r_download:
	link a6,#-4
	moveml #0x3f3c,sp@-
	movel a6@(8),a3
	clrl d6
	clrb d5
	cmpb #33,a3@
	jne L2
	movel _hostout,sp@-
	pea a3@(1)
	jbsr _fputs
	addql #8,sp
	jra L3
L2:
	movel _hostout,sp@-
	pea LC0
	lea _fputs,a2
	jbsr a2@
	movel _hostout,sp@-
	movel a3,sp@-
	jbsr a2@
	addw #16,sp
L3:
	movel _hostout,sp@-
	pea LC1
	jbsr _fputs
	addql #8,sp
	tstb d5
	jne L29
	subl a4,a4
L26:
L6:
	lea _byte,a2
	jbsr a2@
	andb #127,d0
	cmpb #83,d0
	jne L6
	clrl d3
	jbsr a2@
	moveb d0,d2
	andb #127,d2
	jbsr _byte2
	moveb d0,d4
	extw d2
	movew d2,a2
	movel a2,sp@-
	movel _termout,sp@-
	pea 1:w
	jbsr _system
	addw #12,sp
	movel a6@(-4),d0
	asll #1,d0
	moveq #-48,d7
	addl a2,d7
	movel d7,a6@(-4)
	moveq #9,d7
	cmpl a6@(-4),d7
	jcs L32
LI18:
	movew pc@(L18-LI18-2:b,d0:l),d7
	jmp pc@(2,d7:w)
L18:
	.word L9-L18
	.word L13-L18
	.word L14-L18
	.word L32-L18
	.word L32-L18
	.word L32-L18
	.word L32-L18
	.word L32-L18
	.word L15-L18
	.word L16-L18
L9:
	lea _byte2,a2
	jbsr a2@
	jbsr a2@
	subqb #3,d4
	clrl d2
	jra L10
L12:
	movel _termout,sp@-
	jbsr _byte2
	moveb d0,d2
	movel d2,sp@-
	jbsr _fputc
	addql #8,sp
L10:
	subqb #1,d4
	cmpb #255,d4
	jne L12
	movel _termout,sp@-
	jbsr _fputcn
	addql #4,sp
	jra L4
L13:
	lea _byte2,a2
	jbsr a2@
	clrl d3
	moveb d0,d3
	jbsr a2@
	moveb d0,d1
	moveb d4,d0
	addb d3,d0
	moveb d0,d2
	addb d1,d2
	asll #8,d3
	clrl d0
	moveb d1,d0
	orl d0,d3
	subqb #3,d4
	jra L30
L14:
	lea _byte2,a2
	jbsr a2@
	clrl d3
	moveb d0,d3
	jbsr a2@
	moveb d0,d1
	moveb d4,d0
	addb d3,d0
	moveb d0,d2
	addb d1,d2
	asll #8,d3
	clrl d0
	moveb d1,d0
	orl d0,d3
	jbsr a2@
	moveb d0,d1
	addb d1,d2
	asll #8,d3
	clrl d0
	moveb d1,d0
	orl d0,d3
	subqb #4,d4
L30:
	clrl d0
	moveb d4,d0
	addl d0,d6
	jra L8
L15:
	lea _byte2,a2
	jbsr a2@
	clrl d3
	moveb d0,d3
	moveq #16,d7
	asll d7,d3
	jbsr a2@
	movel a4,d7
	moveb d0,d7
	movel d7,a4
	movel a4,d0
	asll #8,d0
	orl d0,d3
	jra L31
L16:
	lea _byte2,a2
	jbsr a2@
	clrl d3
	moveb d0,d3
	asll #8,d3
L31:
	jbsr a2@
	clrl d1
	moveb d0,d1
	orl d1,d3
	jbsr a2@
	movel d3,a5
L32:
	moveb #1,d5
	jra L4
L8:
	movel d3,sp@-
	lea _r_is_mem,a2
	jbsr a2@
	addql #4,sp
	tstl d0
	jeq L20
	clrl d0
	moveb d4,d0
	movel d3,a0
	pea a0@(-1,d0:l)
	jbsr a2@
	addql #4,sp
	tstl d0
	jne L21
L20:
	movel _termout,sp@-
	pea LC2
	lea _fputs,a3
	jbsr a3@
	movel _termout,sp@-
	movel d3,sp@-
	lea _fputx,a2
	jbsr a2@
	movel _termout,sp@-
	pea LC3
	jbsr a3@
	movel _termout,sp@-
	clrl d0
	moveb d4,d0
	movel d3,a0
	pea a0@(d0:l)
	jbsr a2@
	addw #32,sp
	movel _termout,sp@-
	pea LC1
	jbsr a3@
	jra L33
L23:
	jbsr _byte2
	moveb d0,d1
	addb d1,d2
	movel d3,a0
	moveb d1,a0@+
	movel a0,d3
L21:
	subqb #1,d4
	cmpb #255,d4
	jne L23
	jbsr _byte2
	clrl d1
	moveb d0,d1
	clrl d0
	moveb d2,d0
	notl d0
	andl #255,d0
	cmpl d1,d0
	jne L24
	movel _termout,sp@-
	pea 46:w
	jra L34
L24:
	movel _termout,sp@-
	pea 63:w
L34:
	jbsr _fputc
L33:
	addql #8,sp
L4:
	tstb d5
	jeq L26
L29:
	movel _termout,sp@-
	pea LC4
	lea _fputs,a2
	jbsr a2@
	movel _termout,sp@-
	movel d3,sp@-
	jbsr _fputx
	movel _termout,sp@-
	pea LC5
	jbsr a2@
	movel _termout,sp@-
	movel d6,sp@-
	jbsr _fputi
	addw #32,sp
	movel _termout,sp@-
	pea LC6
	jbsr a2@
	movel _termout,sp@-
	jbsr _force_flush
	addw #12,sp
L27:
	jbsr _byte
	btst #0,d0
	jne L27
	movel a5,d0
	moveml a6@(-44),#0x3cfc
	unlk a6
	rts
.text
	.even
.globl _byte2
_byte2:
	link a6,#0
	movel d2,sp@-
	jbsr _byte
	andb #127,d0
	cmpb #57,d0
	jle L36
	extw d0
	extl d0
	moveq #-55,d1
	addl d1,d0
	moveq #15,d1
	andl d1,d0
	jra L37
L36:
	extw d0
	extl d0
	moveq #-48,d1
	addl d1,d0
L37:
	moveb d0,d2
	aslb #4,d2
	jbsr _byte
	andb #127,d0
	cmpb #57,d0
	jle L38
	extw d0
	extl d0
	moveq #-55,d1
	addl d1,d0
	moveq #15,d1
	andl d1,d0
	jra L39
L38:
	extw d0
	extl d0
	moveq #-48,d1
	addl d1,d0
L39:
	addb d2,d0
	movel a6@(-4),d2
	unlk a6
	rts
.text
	.even
.globl _byte
_byte:
	link a6,#0
	clrl sp@-
	movel _hostin,sp@-
	pea 2:w
	jbsr _system
	unlk a6
	rts
