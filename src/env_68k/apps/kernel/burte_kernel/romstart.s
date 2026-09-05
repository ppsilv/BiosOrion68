| startup vectors for BURTE in rom
	.text
	.even
	.globl _boot, _system_stack_top

_start:
        nop                     | make it a popI entry point
        nop                     | waiste some space to make burte happy
        nop
        nop
        nop
        nop
        movw    sr,d0
        btst    #13,d0
        beq     $1              | jump to user mode start
        movl    #0x05000, a0    |0xf000| start the user's stack
        movl    a0,usp
        movl    #0x03000, sp    |0x2f00 | set system stack
        jsr     _boot           | start burte
        bra     _start

$1:             | user mode startup
        movl    #_boot,164      | set trap 9
        trap    #9
        bra     _start          | start over

| some system constants
|	.long	_system_stack_top
|	.long	_boot

.globl  _system_stack_top
        _system_stack_top =     0x3000 |0xa000 |2f00  | 0x2000

