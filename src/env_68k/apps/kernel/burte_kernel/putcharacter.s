|  putcharacter instruction for the 68332
|  putcharacter(char) - sends to output port
|
|   5/22/91  Andrew H. Fagg   Original
|
|

.globl _putcharacter
.text
.even

_putcharacter:
	movel sp@(4),d0      |  get byte and put it on the top of stack
	moveb d0,sp@-
	trap #15	|  call output function
	.word 0x20
	movel #1,d0	|  return a true
	rts


