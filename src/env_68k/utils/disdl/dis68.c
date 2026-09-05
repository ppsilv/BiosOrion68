/*
 * DIS68()- Routine to disassemble MC68000 machine code.
 *
 * USAGE:
 *	dis68 (buff, length, st_addr, options, fp)
 *	    unsigned short *buff;	where the code is
 *	    int length;			number of 16-bit words in <buffer>
 *	    unsigned long st_addr;	starting address for the code
 *	    char *options;		string with options:
 *					a =	print absolute addresses
 *						for relative jumps.
 *					c =	print machine code, too.
 *					d =	print constants in decimal.
 *	    FILE *fp;			where the output will go.
 *
 * HISTORY
 * 11-Mar-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Another bug killed. 'cmpl <ea>, aX' was being recognized but
 *	thought of being 'word' size rather than 'long' size.
 *
 *  2-Mar-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Big bug killed! It wouldn't recognize subtracting, or comparing,
 *	with an address register as destination (it had adding, though).
 *
 * 28-Jun-82  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	A nasty bug was fixed by using the save and use_saved
 *	flags in translate (). The bug only occurred when there
 *	was a move from memory to memory. Dis68 used to think that
 *	the first extension word was the destination address now
 *	it simply saves the modes for later decoding.
 *
 * 14-Jun-82  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Started keeping history. Taken from the p68 files and 
 *	fixed a few bugs: the register list was wrong, added the
 *	new "illegal" instruction (per version 3.0 of User's Manual).
 *	Also, the FILE pointer parameter was added so the output could
 *	be redirected. The delete package was also included so the
 *	program is interruptable.
 */

#include <stdio.h>
#include <string.h>
/*#include <del.h>*/


	struct entry {
		char	instrfmt[13];	/* instruction format		*/
		char	size[2];	/* operand size			*/
		char	extenfmt[3];	/* extension word format	*/
		char	operfmt[23];	/* mnemonic and operand format	*/
	};


	static struct entry table[] = {

		/* group 0000 */
"rd100e5s",	"",	"",	"btst	d%rd,%eas",
"rd101e7s",	"",	"",	"bchg	d%rd,%eas",
"rd110e7s",	"",	"",	"bclr	d%rd,%eas",
"rd111e7s",	"",	"",	"bset	d%rd,%eas",
"100000e6s",	"",	"bn",	"btst	#%bn,%eas",
"100001e7s",	"",	"bn",	"bchg	#%bn,%eas",
"100010e7s",	"",	"bn",	"bclr	#%bn,%eas",
"100011e7s",	"",	"bn",	"bset	#%bn,%eas",
"rd100001rs",	"w",	"w",	"movepw	d%rd,a%rs@(%w)",
"rd101001rs",	"l",	"w",	"movepl	d%rd,a%rs@(%w)",
"rd110001rs",	"w",	"w",	"movepw	a%rs@(%w),d%rd",
"rd111001rs",	"l",	"w",	"movepl	a%rs@(%w),d%rd",
"000000e8s",	"b",	"b",	"orib	#%b,%eas",
"000001e8s",	"w",	"w",	"oriw	#%w,%eas",
"000010e8s",	"l",	"l",	"oril	#%l,%eas",
"001000e8s",	"b",	"b",	"andib	#%b,%eas",
"001001e8s",	"w",	"w",	"andiw	#%w,%eas",
"001010e8s",	"l",	"l",	"andil	#%l,%eas",
"010000e7s",	"b",	"b",	"subib	#%b,%eas",
"010001e7s",	"w",	"w",	"subiw	#%w,%eas",
"010010e7s",	"l",	"l",	"subil	#%l,%eas",
"011000e7s",	"b",	"b",	"addib	#%b,%eas",
"011001e7s",	"w",	"w",	"addiw	#%w,%eas",
"011010e7s",	"l",	"l",	"addil	#%l,%eas",
"101000e8s",	"b",	"b",	"eorib	#%b,%eas",
"101001e8s",	"w",	"w",	"eoriw	#%w,%eas",
"101010e8s",	"l",	"l",	"eoril	#%l,%eas",
"110000e7s",	"b",	"b",	"cmpib	#%b,%eas",
"110001e7s",	"w",	"w",	"cmpiw	#%w,%eas",
"110010e7s",	"l",	"l",	"cmpil	#%l,%eas",
"*",		"",	"",	"",

		/* group 0001 */
"e3de1s",	"b",	"",	"movb	%eas,%ead",
"*",		"",	"",	"",

		/* group 0010 */
"e2de0s",	"l",	"",	"movl	%eas,%ead",
"*",		"",	"",	"",

		/* group 0011 */
"e2de0s",	"w",	"",	"movw	%eas,%ead",
"*",		"",	"",	"",

		/* group 0100 */
"000000e7s",	"b",	"",	"negxb	%eas",
"000001e7s",	"w",	"",	"negxw	%eas",
"000010e7s",	"l",	"",	"negxl	%eas",
"000011e7s",	"w",	"",	"movw	sr,%eas",
"001000e7s",	"b",	"",	"clrb	%eas",
"001001e7s",	"w",	"",	"clrw	%eas",
"001010e7s",	"l",	"",	"clrl	%eas",
"010000e7s",	"b",	"",	"negb	%eas",
"010001e7s",	"w",	"",	"negw	%eas",
"010010e7s",	"l",	"",	"negl	%eas",
"010011e5s",	"w",	"",	"movw	%eas,ccr",
"011000e7s",	"b",	"",	"notb	%eas",
"011001e7s",	"w",	"",	"notw	%eas",
"011010e7s",	"l",	"",	"notl	%eas",
"011011e5s",	"w",	"",	"movw	%eas,sr",
"100000e7s",	"b",	"",	"nbcd	%eas",
"100001e9s",	"l",	"",	"pea	%eas",
"100001000rd",	"w",	"",	"swap	d%rd",
"100010eBs",	"w",	"",	"movemw	%rl,%eas",
"100011eBs",	"l",	"",	"moveml	%rl,%eas",
"100010000rd",	"w",	"",	"extw	d%rd",
"100011000rd",	"l",	"",	"extl	d%rd",
"101000e7s",	"b",	"",	"tstb	%eas",
"101001e7s",	"w",	"",	"tstw	%eas",
"101010e7s",	"l",	"",	"tstl	%eas",
"101011e7s",	"b",	"",	"tas	%eas",
"101011111100",	"",	"",	"illegal",
"110010eAs",	"w",	"",	"movemw	%eas,%rl",
"110011eAs",	"l",	"",	"moveml	%eas,%rl",
"11100100v",	"",	"",	"trap	#%v",
"111001010rd",	"",	"w",	"link	a%rd,#%w",
"111001011rd",	"",	"",	"unlk	a%rd",
"111001100rd",	"l",	"",	"movl	a%rd,usp",
"111001101rd",	"l",	"",	"movl	usp,a%rd",
"111001110000",	"",	"",	"reset",
"111001110001",	"",	"",	"nop",
"111001110010",	"",	"w",	"stop	#%w",
"111001110011",	"",	"",	"rte",
"111001110101",	"",	"",	"rts",
"111001110110",	"",	"",	"trapv",
"111001110111",	"",	"",	"rtr",
"111010e9s",	"",	"",	"jsr	%eas",
"111011e9s",	"",	"",	"jmp	%eas",
"rd110e7s",	"w",	"",	"chk	%eas,d%rd",
"rd111e9s",	"l",	"",	"lea	%eas,a%rd",
"*",		"",	"",	"",

		/* group 0101 */
"3000e3s",	"b",	"",	"addqb	#%3,%eas",
"3001e2s",	"w",	"",	"addqw	#%3,%eas",
"3010e2s",	"l",	"",	"addql	#%3,%eas",
"3100e3s",	"b",	"",	"subqb	#%3,%eas",
"3101e2s",	"w",	"",	"subqw	#%3,%eas",
"3110e2s",	"l",	"",	"subql	#%3,%eas",
"010011e7s",	"b",	"",	"scc	%eas",
"010111e7s",	"b",	"",	"scs	%eas",
"011111e7s",	"b",	"",	"seq	%eas",
"000111e7s",	"b",	"",	"sf	%eas",
"110011e7s",	"b",	"",	"sge	%eas",
"111011e7s",	"b",	"",	"sgt	%eas",
"001011e7s",	"b",	"",	"shi	%eas",
"111111e7s",	"b",	"",	"sle	%eas",
"001111e7s",	"b",	"",	"sls	%eas",
"110111e7s",	"b",	"",	"slt	%eas",
"101111e7s",	"b",	"",	"smi	%eas",
"011011e7s",	"b",	"",	"sne	%eas",
"101011e7s",	"b",	"",	"spl	%eas",
"000011e7s",	"b",	"",	"st	%eas",
"100011e7s",	"b",	"",	"svc	%eas",
"101011e7s",	"b",	"",	"svs	%eas",
"010011001rd",	"",	"dw",	"dbcc	d%rd,%dw",
"010111001rd",	"",	"dw",	"dbcs	d%rd,%dw",
"011111001rd",	"",	"dw",	"dbeq	d%rd,%dw",
"000111001rd",	"",	"dw",	"dbf	d%rd,%dw",
"110011001rd",	"",	"dw",	"dbge	d%rd,%dw",
"111011001rd",	"",	"dw",	"dbgt	d%rd,%dw",
"001011001rd",	"",	"dw",	"dbhi	d%rd,%dw",
"111111001rd",	"",	"dw",	"dble	d%rd,%dw",
"001111001rd",	"",	"dw",	"dbls	d%rd,%dw",
"110111001rd",	"",	"dw",	"dblt	d%rd,%dw",
"101111001rd",	"",	"dw",	"dbmi	d%rd,%dw",
"011011001rd",	"",	"dw",	"dbne	d%rd,%dw",
"101011001rd",	"",	"dw",	"dbpl	d%rd,%dw",
"000011001rd",	"",	"dw",	"dbt	d%rd,%dw",
"100011001rd",	"",	"dw",	"dbvc	d%rd,%dw",
"101011001rd",	"",	"dw",	"dbvs	d%rd,%dw",
"*",		"",	"",	"",

		/* group 0110 */
"0000db",	"b",	"",	"bra	%db",
"0001db",	"b",	"",	"bsr	%db",
"0100db",	"b",	"",	"bcc	%db",
"0101db",	"b",	"",	"bcs	%db",
"0111db",	"b",	"",	"beq	%db",
"1100db",	"b",	"",	"bge	%db",
"1110db",	"b",	"",	"bgt	%db",
"0010db",	"b",	"",	"bhi	%db",
"1111db",	"b",	"",	"ble	%db",
"0011db",	"b",	"",	"bls	%db",
"1101db",	"b",	"",	"blt	%db",
"1011db",	"b",	"",	"bmi	%db",
"0110db",	"b",	"",	"bne	%db",
"1010db",	"b",	"",	"bpl	%db",
"1000db",	"b",	"",	"bvc	%db",
"1010db",	"b",	"",	"bvs	%db",
"000000000000",	"w",	"dw",	"bra	%dw",
"000100000000",	"w",	"dw",	"bsr	%dw",
"010000000000",	"w",	"dw",	"bcc	%dw",
"010100000000",	"w",	"dw",	"bcs	%dw",
"011100000000",	"w",	"dw",	"beq	%dw",
"110000000000",	"w",	"dw",	"bge	%dw",
"111000000000",	"w",	"dw",	"bgt	%dw",
"001000000000",	"w",	"dw",	"bhi	%dw",
"111100000000",	"w",	"dw",	"ble	%dw",
"001100000000",	"w",	"dw",	"bls	%dw",
"110100000000",	"w",	"dw",	"blt	%dw",
"101100000000",	"w",	"dw",	"bmi	%dw",
"011000000000",	"w",	"dw",	"bne	%dw",
"101000000000",	"w",	"dw",	"bpl	%dw",
"100000000000",	"w",	"dw",	"bvc	%dw",
"101000000000",	"w",	"dw",	"bvs	%dw",
"*",		"",	"",	"",

		/* group 0111 */
"rd08",		"l",	"",	"moveq	#%8,d%rd",
"*",		"",	"",	"",

		/* group 1000 */
"rd000e5s",	"b",	"",	"orb	%eas,d%rd",
"rd001e5s",	"w",	"",	"orw	%eas,d%rd",
"rd010e5s",	"l",	"",	"orl	%eas,d%rd",
"rd100e4s",	"b",	"",	"orb	d%rd,%eas",
"rd101e4s",	"w",	"",	"orw	d%rd,%eas",
"rd110e4s",	"l",	"",	"orl	d%rd,%eas",
"rd011e5s",	"w",	"",	"divu	%eas,d%rd",
"rd111e5s",	"w",	"",	"divs	%eas,d%rd",
"rd100000rs",	"b",	"",	"sbcd	d%rs,d%rd",
"rd100001rs",	"b",	"",	"sbcd	a%rs@-,a%rd@-",
"*",		"",	"",	"",

		/* group 1001 */
"rd000e1s",	"b",	"",	"subb	%eas,d%rd",
"rd001e0s",	"w",	"",	"subw	%eas,d%rd",
"rd010e0s",	"l",	"",	"subl	%eas,d%rd",
"rd011e0s",	"w",	"",	"subw	%eas,a%rd",
"rd100e2s",	"b",	"",	"subb	d%rd,%eas",
"rd101e2s",	"w",	"",	"subw	d%rd,%eas",
"rd110e2s",	"l",	"",	"subl	d%rd,%eas",
"rd111e0s",	"w",	"",	"subl	%eas,a%rd",
"rd100000rs",	"b",	"",	"subxb	d%rs,d%rd",
"rd101000rs",	"w",	"",	"subxw	d%rs,d%rd",
"rd110000rs",	"l",	"",	"subxl	d%rs,d%rd",
"rd100001rs",	"b",	"",	"subxb	a%rs@-,a%rd@-",
"rd101001rs",	"w",	"",	"subxw	a%rs@-,a%rd@-",
"rd110001rs",	"l",	"",	"subxl	a%rs@-,a%rd@-",
"*",		"",	"",	"",


		/* group 1010 */
"*",		"",	"",	"",

		/* group 1011 */
"rd000e1s",	"b",	"",	"cmpb	%eas,d%rd",
"rd001e0s",	"w",	"",	"cmpw	%eas,d%rd",
"rd010e0s",	"l",	"",	"cmpl	%eas,d%rd",
"rd011e0s",	"w",	"",	"cmpw	%eas,a%rd",
"rd100001rs",	"b",	"",	"cmpmb	a%rs@+,a%rd@+",
"rd101001rs",	"w",	"",	"cmpmw	a%rs@+,a%rd@+",
"rd110001rs",	"l",	"",	"cmpml	a%rs@+,a%rd@+",
"rd111e0s",	"l",	"",	"cmpl	%eas,a%rd",
"rd100e7s",	"b",	"",	"eorb	d%rd,%eas",
"rd101e7s",	"w",	"",	"eorw	d%rd,%eas",
"rd110e7s",	"l",	"",	"eorl	d%rd,%eas",
"*",		"",	"",	"",

		/* group 1100 */
"rd000e5s",	"b",	"",	"andb	%eas,d%rd",
"rd001e5s",	"w",	"",	"andw	%eas,d%rd",
"rd010e5s",	"l",	"",	"andl	%eas,d%rd",
"rd100e4s",	"b",	"",	"andb	d%rd,%eas",
"rd101e4s",	"w",	"",	"andw	d%rd,%eas",
"rd110e4s",	"l",	"",	"andl	d%rd,%eas",
"rd011e5s",	"w",	"",	"mulu	%eas,d%rd",
"rd111e5s",	"w",	"",	"muls	%eas,d%rd",
"rd100000rs",	"b",	"",	"abcd	d%rs,d%rd",
"rd100001rs",	"b",	"",	"abcd	a%rs@-,a%rd@-",
"rd101000rs",	"l",	"",	"exg	d%rd,d%rs",
"rd101001rs",	"l",	"",	"exg	a%rd,a%rs",
"rd110001rs",	"l",	"",	"exg	d%rd,a%rs",
"*",		"",	"",	"",

		/* group 1101 */
"rd000e1s",	"b",	"",	"addb	%eas,d%rd",
"rd001e0s",	"w",	"",	"addw	%eas,d%rd",
"rd010e0s",	"l",	"",	"addl	%eas,d%rd",
"rd011e0s",	"w",	"",	"addw	%eas,a%rd",
"rd100e4s",	"b",	"",	"addb	d%rd,%eas",
"rd101e4s",	"w",	"",	"addw	d%rd,%eas",
"rd110e4s",	"l",	"",	"addl	d%rd,%eas",
"rd111e0s",	"l",	"",	"addl	%eas,a%rd",
"rd100000rs",	"b",	"",	"addxb	d%rs,d%rd",
"rd101000rs",	"w",	"",	"addxw	d%rs,d%rd",
"rd110000rs",	"l",	"",	"addxl	d%rs,d%rd",
"rd100001rs",	"b",	"",	"addxb	a%rs@-,a%rd@-",
"rd101001rs",	"w",	"",	"addxw	a%rs@-,a%rd@-",
"rd110001rs",	"l",	"",	"addxl	a%rs@-,a%rd@-",
"*",		"",	"",	"",

		/* group 1110 */
"c000000rd",	"b",	"",	"asrb	#%c,d%rd",
"c001000rd",	"w",	"",	"asrw	#%c,d%rd",
"c010000rd",	"l",	"",	"asrl	#%c,d%rd",
"c100000rd",	"b",	"",	"aslb	#%c,d%rd",
"c101000rd",	"w",	"",	"aslw	#%c,d%rd",
"c110000rd",	"l",	"",	"asll	#%c,d%rd",
"rs000100rd",	"b",	"",	"asrb	d%rs,d%rd",
"rs001100rd",	"w",	"",	"asrw	d%rs,d%rd",
"rs010100rd",	"l",	"",	"asrl	d%rs,d%rd",
"rs100100rd",	"b",	"",	"aslb	d%rs,d%rd",
"rs101100rd",	"w",	"",	"aslw	d%rs,d%rd",
"rs110100rd",	"l",	"",	"asll	d%rs,d%rd",
"000011e4s",	"",	"",	"asr	%eas",
"000111e4s",	"",	"",	"asl	%eas",
"c000001rd",	"b",	"",	"lsrb	#%c,d%rd",
"c001001rd",	"w",	"",	"lsrw	#%c,d%rd",
"c010001rd",	"l",	"",	"lsrl	#%c,d%rd",
"c100001rd",	"b",	"",	"lslb	#%c,d%rd",
"c101001rd",	"w",	"",	"lslw	#%c,d%rd",
"c110001rd",	"l",	"",	"lsll	#%c,d%rd",
"rs000101rd",	"b",	"",	"lsrb	d%rs,d%rd",
"rs001101rd",	"w",	"",	"lsrw	d%rs,d%rd",
"rs010101rd",	"l",	"",	"lsrl	d%rs,d%rd",
"rs100101rd",	"b",	"",	"lslb	d%rs,d%rd",
"rs101101rd",	"w",	"",	"lslw	d%rs,d%rd",
"rs110101rd",	"l",	"",	"lsll	d%rs,d%rd",
"001011e4s",	"",	"",	"lsr	%eas",
"001111e4s",	"",	"",	"lsl	%eas",
"c000010rd",	"b",	"",	"roxrb	#%c,d%rd",
"c001010rd",	"w",	"",	"roxrw	#%c,d%rd",
"c010010rd",	"l",	"",	"roxrl	#%c,d%rd",
"c100010rd",	"b",	"",	"roxlb	#%c,d%rd",
"c101010rd",	"w",	"",	"roxlw	#%c,d%rd",
"c110010rd",	"l",	"",	"roxll	#%c,d%rd",
"rs000110rd",	"b",	"",	"roxrb	d%rs,d%rd",
"rs001110rd",	"w",	"",	"roxrw	d%rs,d%rd",
"rs010110rd",	"l",	"",	"roxrl	d%rs,d%rd",
"rs100110rd",	"b",	"",	"roxlb	d%rs,d%rd",
"rs101110rd",	"w",	"",	"roxlw	d%rs,d%rd",
"rs110110rd",	"l",	"",	"roxll	d%rs,d%rd",
"010011e7s",	"",	"",	"roxr	%eas",
"010111e7s",	"",	"",	"roxl	%eas",
"c000011rd",	"b",	"",	"rorb	#%c,d%rd",
"c001011rd",	"w",	"",	"rorw	#%c,d%rd",
"c010011rd",	"l",	"",	"rorl	#%c,d%rd",
"c100011rd",	"b",	"",	"rolb	#%c,d%rd",
"c101011rd",	"w",	"",	"rolw	#%c,d%rd",
"c110011rd",	"l",	"",	"roll	#%c,d%rd",
"rs000111rd",	"b",	"",	"rorb	d%rs,d%rd",
"rs001111rd",	"w",	"",	"rorw	d%rs,d%rd",
"rs010111rd",	"l",	"",	"rorl	d%rs,d%rd",
"rs100111rd",	"b",	"",	"rolb	d%rs,d%rd",
"rs101111rd",	"w",	"",	"rolw	d%rs,d%rd",
"rs110111rd",	"l",	"",	"roll	d%rs,d%rd",
"011011e7s",	"",	"",	"ror	%eas",
"011111e7s",	"",	"",	"rol	%eas",
"*",		"",	"",	"",


		/* group 1111 */
"*",		"",	"",	""

		};


	static unsigned short group[16] = {
		0,
		31,
		33,	/* these are the indices into the opcode table	*/
		35,	/* for each group of opcodes.  Groups 10 and 15 */
		37,	/* are unused, and are given values of zero.	*/
		83,	/* See the 68000 User's Manual, Pg C-1 for an	*/
		122,	/* explanation of the group breakdown		*/
		155,	/* rxb-- illegal added (per V3 of 68K user's	*/
		157,	/* manual. The codes for groups 5 - 15 are ++	*/
		168,	/* rxb-- sub[wl] with address registers added	*/
		183,	/* as well as cmp[wl] with address registers	*/
		184,	/* The codes for groups 10 and 11 are ++.	*/
		196,	/* The codes for groups 12 - 15 are (+ 4).	*/
		210,
		225,
		282
	}; 



	struct data_table{
		unsigned short	sreg;	/* source register		*/
		unsigned short	dreg;	/* destination register		*/
		unsigned short	vect;	/* vector			*/
		int	disp8;		/* 8 bit displacement		*/
		unsigned short	count;	/* count for shifts & rotates	*/
		unsigned short	num3;	/* 3 bit data			*/
		unsigned short	num8;	/* 8 bit data			*/
		char	eas[11];	/* source effective address	*/
		char	ead[11];	/* destination effective addr	*/
		unsigned short	b;	/* byte value			*/
		unsigned short	w;	/* word value			*/
		long	l;		/* long value			*/
		int	 disp16;	/* 16 bit displacement		*/
		unsigned short	bitnum;	/* bit number			*/
		char	reglist[47];	/* register list		*/
		unsigned short	predecflag;	/* TRUE if predecrement	*/
					/* addressing mode is used	*/
					/* (needed for decoding the	*/
					/* register list mask)		*/
	};

	struct data_table	data;	/* data storage table		*/

	struct option_table{
		short	hex;		/* base to print imm. data in	*/
					/* T=hex F=decimal		*/
		short	code;		/* print machine code		*/
					/* T=print F=don't print	*/
		short	absolute;	/* action for displacements	*/
					/* T=convert to absolute addr.	*/
					/* F=print as is		*/
	};

	struct option_table	flags;  /* output format option flags	*/


biti(wd,low,high)	/* returns value of bit field between low and high */
unsigned short	wd,low,high;
{
	return((wd >> low) & ~(~0 << (high-low+1)));
}

set_flags(options)
char	*options;
{
    unsigned short    loop;	/* displacement into options string */


	/* initialize flags... */
    flags.hex = 1;		/* output immediate data in hex		*/
    flags.code = 0;		/* dont't print machine code		*/
    flags.absolute = 0;		/* leave relatives displacements alone	*/


    for(loop=0; loop < strlen(options); loop++) {
	switch(*(options+loop)) {
	    case 'a': flags.absolute = 1;	/* convert displacements */
			break;			/* to absolute addresses */

	    case 'c': flags.code = 1;		/* print machine code	 */
			break;

	    case 'd': flags.hex = 0;		/* print immediate data  */
			break;			/* in decimal		 */

	    default: 
			return(-1);
	}
    }
    return(0);
}




dis68 (codestart, codelength, addr, options, fp)
    unsigned short	*codestart;
    int	codelength;
    unsigned long	addr;
    char	*options;
    FILE *fp;
{
    unsigned short    *codeptr;	/* pointer to machine code */
    unsigned short    numread;	/* number of words read to decode */
	    			/* a single instruction */

    if(set_flags(options))	/* set formatting flags. */
	return(-1);		/* abort if bad format code given */

    for(codeptr = codestart; codeptr < codestart + codelength; ) {
/**	DELRETURN;		/* abort if the user types DEL */
	fprintf (fp, "%08x\t", addr);		   /* write address */
	numread = translate(codeptr, addr, fp);	   /* write instruction */
	codeptr += numread;	/* add number of WORDS read to codeptr	*/
	addr += numread*2;	/* add number of BYTES read to addr */
    }
    return(0);	/* disassembly was successful */
}


translate (codeptr, addr, fp)
    unsigned short *codeptr;	/* points to word to be decoded */
    int	addr;			/* "address" of current instruction */
    FILE *fp;			/* where the output will go */
{

    int    index = 0;		/* opcode table index	*/
    int rdcount = 0;		/* total number of words read*/
    unsigned short    wd = 0;	/* instruction word value */
    unsigned short    *instrptr = codeptr; /* points to start of current*/
				/* instruction */
    unsigned short    loop;

    wd = *codeptr;	/* get a word */
    codeptr++;		/* point to next word to read */

    index = lookup(wd, codeptr, &rdcount);
		/* locate instruction and decode data it */
		/* contains.  rdcount is changed to reflect */
		/* the number of words read in decoding the */
		/* instruction (with extra words) and the */
		/* effective address field (if there was one)	*/

    rdcount++;		/* keep track of number of words read */
			/* while decoding this instruction */

    if (index == -1)		/* not an instruction */
	fprintf (fp, "%04x\t\t\t.word\t%d\n", wd, (short)wd);  /* data */
    else {
	if (flags.code)
		print_machinecode (instrptr, rdcount, fp);
	print_operands (table[index].operfmt, addr, fp);
    }
    return (rdcount);
}



lookup (wd, codeptr, rdcount)
    unsigned short wd;		/* value of instruction word */
    unsigned short *codeptr;	/* next word to read */
    int *rdcount;	/* no of words read in decoding */
{
    int index = 0;

    if (biti (wd, 12, 15) == 10 || biti (wd, 12, 15) == 15)
	return(-1);		/* these groups are not used */

    for(index = group[biti(wd,12,15)];table[index].instrfmt[0] != '*';index++)
	if( match (codeptr, table [index].size[0],
		table [index].extenfmt, rdcount, table [index].instrfmt, wd))
		    return (index);		/* found	*/

    return(-1);					/* not found	*/
}



match (codeptr, size, extnfmt, rdcount, format, wd)
unsigned short	*codeptr;	/* points to next word to be read	*/
				/* used in decoding effective address	*/
char		size;		/* size of instruction (b, w, or l)	*/
int *rdcount;			/* number of words read to decode	*/
				/* effective address			*/
char		*format;	/* format string to match		*/
char		*extnfmt;	/* format of the extension word		*/
unsigned short	wd;		/* word value to match			*/
{

    unsigned short    windex = 11;	/* index of current bit in word	*/
    unsigned short    mode,reg;		/* mode and register values for */
					/* effective addr translation	*/
    unsigned short    fmt = 0;		/* format code value		*/
    int save = 0,			/* flag to save mode and reg	*/
	saved = 0;			/* flag to use saved mode/reg	*/
    unsigned short    save_mode = 0,	/* where to save the mode/reg	*/
		      save_reg = 0;
    int nwords;				/* temp. for words needed.	*/

    *rdcount = 0;		/* must zero before each iteration	*/

    if (biti (wd, 12, 15) >= 1 && biti (wd, 12, 15) <= 3)
	save = -1;	/* save the reg/mode of the dest. Use source first */

    if (*extnfmt)	/* if there is an extension word */
    {
	nwords = trans_exword (extnfmt, codeptr);
	codeptr += nwords;
	(*rdcount) += nwords;
    }

    while (*format != '\0') {
	fmt = fmtvalue(format);
	if (fmt < 10)
	    switch (fmt) {
		case 0: /* bit off (0) */
		case 1: /* bit on (1)  */
			if(biti(wd,windex,windex) != (*format-'0'))
			    return(0);
			else {
			    windex--;
			    format++;
			}
			break;

		case 2: /* destination register (rd) */
			data.dreg = biti(wd,windex-2,windex);
			windex -= 3;
			format += 2;
			break;

		case 3: /* source register (rs) */
			data.sreg = biti(wd,windex-2,windex);
			windex -= 3;
			format += 2;
			break;

		case 4: /* vector (v) */
			data.vect = biti(wd,windex-3,windex);
			windex -= 4;
			format++;
			break;

		case 5: /* 8 bit displacement (db) */
			if (biti(wd,windex-7,windex) == 0) /* zero displ. */
			    return(0);
			data.disp8 = biti(wd,windex-7,windex);
			windex -= 8;
			format += 2;
			break;

		case 6: /* count for shifts & rotates (c) */
			data.count = biti(wd,windex-2,windex);
			if (data.count == 0)
			    data.count=8;	/* 0 is actually 8 */
			windex -= 3;
			format++;
			break;

		case 7: /* 3 bit data (3) */
			data.num3 = biti(wd,windex-2,windex);
			if (data.num3 == 0)
			    data.num3 = 8;	/* 0 is actually 8 */
			windex -= 3;
			format++;
			break;

		case 8: /* 8 bit data (8) */
			data.num8 = biti(wd,windex-7,windex);
			windex -= 8;
			format++;
			break;

		case 9: /* single bit wildcard value (x) */
			windex--;
			format++;
			break;
	}

    else {	/* Effective addressing fields... */
	mode = biti(wd,windex-2,windex);
	reg  = biti(wd,windex-5,windex-3);
	format += 2;

	/* the mode and register fields for a destination */
	/* effective address are reversed, so...	  */
	if (*format == 'd') {
	    reg  = biti(wd,windex-2,windex);
	    mode = biti(wd,windex-5,windex-3);
	}

	switch(fmt) {
		case 10: /* All (e0) */
			break;

		case 11: /* All except addr reg direct (e1) */
		case 15: /* Data		       (e5) */
			if (!data_mode(mode))
			    return(0);
			break;

		case 12: /* Alterable (e2) */
			if (!alter_mode(mode,reg))
			    return(0);
			break;

		case 13: /* Alterable except addr reg direct (e3) */
			if (!(alter_mode(mode,reg) && mode != 1))
			    return(0);
			break;

		case 14: /* Alterable Memory (e4) */
			if (!(alter_mode(mode,reg) && memory_mode(mode)))
			    return(0);
			break;

		case 16: /* Data except immediate (e6) */
			if (!(data_mode(mode) && !(mode == 7 && reg == 4)))
			    return(0);
			break;

		case 17: /* Data Alterable (e7) */
			if (!(data_mode(mode) && alter_mode(mode,reg)))
			    return(0);
			break;

		case 18: /* Data Alterable or Status Register (e8) */
			if (!((mode == 7 && reg == 4) || (data_mode(mode) &&
			    alter_mode(mode,reg))))
				return(0);
			break;

		case 19: /* Control (e9) */
			if (!control_mode(mode,reg))
			    return(0);
			break;

		case 20: /* Control or Postincrement (eA) */
			if (!(control_mode(mode,reg) || (mode == 3)))
			    return(0);
			data.predecflag = 0;
			trans_reglist (*codeptr++);
			(*rdcount)++;
			break;

		case 21: /* Control Alterable or Predecrement (eB) */
			if (!((mode == 4) || (control_mode(mode,reg)
			    && alter_mode(mode,reg))))
				return(0);
			if (mode == 4) data.predecflag = 1;
			else data.predecflag = 0;
			trans_reglist (*codeptr++);
			(*rdcount)++;
			break;

	    }

	/*	In the case that we have a memory to memory move, we
	 *	should save the destination mode/reg fields to be 
	 *	decoded later.
	 */

	if (!save)			/* do a normal translation */
	{
	    if (*format == 's')
		nwords = trans_ea (codeptr, size, mode, reg, data.eas);
	    else
		nwords = trans_ea (codeptr, size, mode, reg, data.ead);
	    codeptr += nwords;
	    (*rdcount) += nwords;

	    if (saved) save = -1;	/* next time use saved stuff */
	}
	else if (!saved)		/* do the saving! */
	{
	    save_mode = mode;
	    save_reg = reg;
	    saved = -1;
	    save = 0;			/* next time do a normal */
	}

	if (save && saved)		/* use the saved reg/mode */
	{
	    nwords = trans_ea (codeptr, size, save_mode, save_reg, data.ead);
	    codeptr += nwords;
	    (*rdcount) += nwords;
	    save = saved = 0;
	}

	format++;
	windex -= 6;
	}
    }
    return(1);
}


/* These functions return FALSE values if the mode is not in the group	*/
/* that is being checked						*/
/* The definition of each group can be found on page B-1 of the 68000	*/
/* User's Manual							*/

data_mode(mode)			/* TRUE if mode is a data mode		*/
unsigned short	mode;		/* mode of effective address to test	*/
{
    if (mode == 1)
	return(0);
    else
	return(1);
}

memory_mode(mode)		/* TRUE if mode is memory mode		*/
unsigned short	mode;		/* mode of effective address to test	*/
{
    if (mode == 0 || mode == 1)
	return(0);
    else
	return(1);
}

control_mode(mode,reg)		/* TRUE if mode is a control mode	*/
unsigned short	mode;		/* mode of effective address to test	*/
unsigned short	reg;		/* register of effect. addr.		*/
{
    if (mode == 0 || mode == 1 || mode == 3 || mode == 4 ||
	(mode == 7 && reg == 4))
	    return(0);
    else
    	    return(1);
}

alter_mode(mode,reg)		/* TRUE if mode is an Alterable mode	*/
unsigned short	mode;		/* mode of effective address to test	*/
unsigned short	reg;		/* register of effect. addr.		*/
{
    if (mode == 7 && (reg == 2 || reg == 3 || reg == 4))
	return(0);
    else
	return(1);
}

	


trans_ea (codeptr, size, mode, reg, str)
		/* given a mode and a register, place the	*/
		/* effective address, in the format used by	*/
		/* the assembler, into the character string	*/
		/* pointed to by str				*/

unsigned short	*codeptr;	/* next word to read (extension word for  */
				/* for effective address, if any)	  */
char		size;		/* size of instruction (b, w, or l)	  */
unsigned short	mode;		/* effective address mode		  */
unsigned short	reg;		/* effective address register		  */
char		*str;		/* stores decoded effective address here  */
{


    unsigned short    disp;	/* displacement for addressing modes with */
				/* displacement values			  */
    unsigned short    index;	/* index value for addressing modes with  */
				/* with index				  */

    switch (mode) {
	case 0:	sprintf(str,"d%d",reg);
		    return(0);
	case 1: sprintf(str,"a%d",reg);
		    return(0);
	case 2: sprintf(str,"a%d@",reg);
		    return(0);
	case 3: sprintf(str,"a%d@+",reg);
		    return(0);
	case 4: sprintf(str,"a%d@-",reg);
		    return(0);
	case 5: sprintf(str,"a%d@(0x%04x)",reg,*codeptr);
		    return(1);
	case 6: index = *codeptr;
		if (biti(index,15,15)) {
		    if (biti(index,11,11)) 
			sprintf(str,"a%d@(0x%04x,a%d:L)",
				reg,biti(index,0,7),biti(index,12,14));
		    else
			sprintf(str,"a%d@(0x%04x,a%d:W)",
				reg,biti(index,0,7),biti(index,12,14));
		}
		else {
		    if (biti(index,11,11))
			sprintf(str,"a%d@(0x%04x,d%d:L)",
				reg,biti(index,0,7),biti(index,12,14));
		    else
			sprintf(str,"a%d@(0x%04x,d%d:W)",
				reg,biti(index,0,7),biti(index,12,14));
		}
		return(1);

	case 7: switch(reg) {
		    case 0:
			if (flags.hex)
			    sprintf(str,"0x%04x",*codeptr);
			else
			    sprintf(str,"%d",*codeptr);
			return(1);

		    case 1:
			if (flags.hex)
			    sprintf(str,"0x%04x%04x",*codeptr,*(codeptr+1));
			else
			    sprintf(str,"%u",
				( (long) *codeptr << 16)
					 + (long) *(codeptr+1));
			return(2);

		    case 2:
			sprintf(str,"pc@(0x%04x)",*codeptr);
			return(1);

		    case 3:
			index = *codeptr;
			if (biti(index,15,15)) {
			    if (biti(index,11,11)) 
				sprintf(str,"pc@(0x%04x,a%d:L)",
						biti(index,0,7),
						biti(index,12,14));
			    else
				sprintf(str,"pc@(0x%04x,a%d:W)",
						biti(index,0,7),
						biti(index,12,14));
			}
			else {
			    if (biti(index,11,11)) 
				sprintf(str,"pc@(0x%04x,d%d:L)",
						biti(index,0,7),
						biti(index,12,14));
			    else
				sprintf(str,"pc@(0x%04x,d%d:W)",
						biti(index,0,7),
						biti(index,12,14));
			}
			return(1);

		    case 4:
			switch(size) {
			    case 'b':
					if (flags.hex)
					    sprintf(str,"#0x%02x",*codeptr);
					else
					    sprintf(str,"#%d",*codeptr);
					return(1);

			    case 'w':
				 	if (flags.hex)
					    sprintf(str,"#0x%04x",*codeptr);
					else
					    sprintf(str,"#%d",*codeptr);
					return(1);

			    case 'l':
					if (flags.hex)
					    sprintf(str,"#0x%04x%04x",
						    *codeptr,*(codeptr+1));
					else
					    sprintf(str,"#%u",
						( (long) *codeptr << 16)
						     + (long) *(codeptr+1));
					return(2);
			}
		}
     }
}




fmtvalue (fmt)
    char *fmt;
{

    switch (*fmt) {
	case '0': return(0);
	case '1': return(1);
	case 'r': switch (*(fmt+1)) {
			case 'd': return(2);
			case 's': return(3);
			case 'l': return(27);
			 default: return(-1);
		  }
	case 'v': return(4);
	case 'd': switch(*(fmt+1)) {
			case 'b': return(5);
			case 'w': return(25);
			 default: return(-1);
		  }
	case 'c': return(6);
	case '3': return(7);
	case '8': return(8);
	case 'x': return(9);
	case 'e': switch (*(fmt+1)) {
			case '0': return(10);
			case '1': return(11);
			case '2': return(12);
			case '3': return(13);
			case '4': return(14);
			case '5': return(15);
			case '6': return(16);
			case '7': return(17);
			case '8': return(18);
			case '9': return(19);
			case 'A': return(20);
			case 'B': return(21);
			case 'a': switch(*(fmt+2)) {
					case 's': return(28);
					case 'd': return(29);
					 default: return(-1);
				  }
			 default: return(-1);
		  }
	case 'b': switch(*(fmt+1)) {
			case 'n': return(26);
			 default: return(22);
		  }
	case 'w': return(23);
	case 'l': return(24);

    }
}


trans_exword (format, codeptr)
    char *format;		/* extension word format		*/
    unsigned short *codeptr;	/* points to current machine code word	*/
{

    if (*format == '\0')	/* no extension words	*/
	return(0);

    switch (fmtvalue(format)) {
	case 22: /* byte value */
		data.b = *codeptr;
		return(1);
		break;
			
	case 23: /* word value */
		data.w = *codeptr;
		return(1);
		break;

	case 24: /* long value */
		data.l = ( (long) *codeptr << 16)
				+ (long) *(codeptr+1);
		return(2);
		break;

	case 25: /* 16 bit displacement */
		data.disp16 = *codeptr;
		return(1);
		break;

	case 26: /* bit number */
		data.bitnum = *codeptr;
		return(1);
		break;

/*	case 27: 		This is now obsolete since now
		trans_reglist(*codeptr);  trans_reglist is called within
		return(1);	match. (It must be called BEFORE the eff.
		break;		address is resolved) -rxb */

	default: return(0);
    }
}


trans_reglist (mask)
    unsigned short	mask;	/* register list mask value */
{

	/* if addressing mode is predecrement, list mask is:	*/
	/* d0 d1 d2 d3 d4 d5 d6 d7 a0 a1 a2 a3 a4 a5 a6 a7	*/

	/* for all other addressing modes, the format is:	*/
	/* a7 a6 a5 a4 a3 a2 a1 a0 d7 d6 d5 d4 d3 d2 d1 d0	*/

	/* the addressing mode can be determined by checking		*/
	/* data.predecflag.  If it is TRUE, the the predecrement	*/
	/* mode is being used.  					*/

	/* the translated register list is stored in data.reglist	*/

    unsigned short	bit;	/* current mask bit being checked (0 - 15) */
    char		reg[5]; /* temp string for building register names */

    data.reglist[0] = '\0';		/* set register list to null */

    if (!mask)				/* move no registers! */
    {
	sprintf (data.reglist, "<NOTHING>");
	return;
    }

    if (!data.predecflag) {
	for (bit=0; bit <8; ) {
	    if (biti(mask,bit,bit)) {
		sprintf(reg,"d%d/",bit);
		strcat(data.reglist,reg);
	    }
	    bit++;
	}
	for (bit=8; bit < 16; ) {
	    if (biti(mask,bit,bit)) {
		sprintf(reg,"a%d/",bit-8);
		strcat(data.reglist,reg);
	    }
	    bit++;
	}
    }
    else {
	for (bit=0; bit < 8; ) {
	    if (biti(mask,bit,bit)) {
		sprintf (reg, "a%d/", 7 - bit);
		strcat (data.reglist, reg);
	    }
	    bit++;
	}
	for (bit=8; bit < 16; ) {
	    if (biti(mask,bit,bit)) {
		sprintf(reg,"d%d/",15-bit);
		strcat(data.reglist,reg);
	    }
	    bit++;
	}
    }
    data.reglist[strlen(data.reglist)-1] = '\0';
			/* replace trailing slash	*/
			/* with a null			*/
}


print_machinecode (instrptr, rdcount, fp)
    unsigned short *instrptr;	/* pointer to current instruction	*/
    int rdcount;		/* number of words read			*/
    FILE *fp;			/* where the output should be		*/
{
    int loop;

    for(loop = 0; loop < rdcount; loop++)
	fprintf (fp, "%04x ",*(instrptr+loop));	/* print mach. code */

    for(loop = 0; loop < (24 - 5*rdcount); loop++)
	putc (' ', fp);				/* fill w/spaces    */
}


print_operands (format, addr, fp)
    char *format;	/* operand output format		*/
    int  addr;		/* address of current instruction	*/
{
    unsigned short fmt;	/* format code value			*/

    while (*format != '\0') {
	if (*format != '%')			/* print literal	*/
	    fprintf (fp, "%c", *format++);
	else {
	    format++;				/* skip over the %	*/
	    fmt = fmtvalue(format);		/* get the format value	*/
	    switch (fmt) {
		case 2: /* destination register (rd) */
			fprintf (fp, "%d",data.dreg);
			format += 2;
			break;

		case 3: /* source register (rs) */
			fprintf (fp, "%d",data.sreg);
			format += 2;
			break;

		case 4: /* vector (v) */
			fprintf (fp, "0x%02x",data.vect);
			format++;
			break;

		case 5: /* 8 bit displacement (db) */
			if (!flags.absolute)
			    if (biti(data.disp8,7,7))	/* is sign bit set */
				fprintf (fp, "0x%02x (-)",(-data.disp8 & 255));
			    else
				fprintf (fp, "0x%02x (+)",data.disp8);
			else
			    if (biti(data.disp8,7,7))   /* is sign bit set */
				fprintf (fp, "0x%08x",
					addr + 2 - (-data.disp8 & 255));
			    else
				fprintf (fp, "0x%08x",addr + 2 + data.disp8);
			format += 2;
			break;

		case 6: /* count for shifts & rotates (c) */
			fprintf (fp, "%d",data.count);
			format++;
			break;

		case 7: /* 3 bit data (3) */
			fprintf (fp, "%d",data.num3);
			format++;
			break;

		case 8: /* 8 bit data (8) */
			if (flags.hex)
			    fprintf (fp, "0x%02x",data.num8);
			else
			    fprintf (fp, "%d",data.num8);
			format++;
			break;

		case 22: /* byte data (b) */
			if (flags.hex)
			    fprintf (fp, "0x%02x",data.b);
			else
			    fprintf (fp, "%d",data.b);
			format++;
			break;

		case 23: /* word data (w) */
			if (flags.hex)
			    fprintf (fp, "0x%04x",data.w);
			else
			    fprintf (fp, "%d",data.w);
			format++;
			break;

		case 24: /* long data (l) */
			if (flags.hex)
			    fprintf (fp, "0x%08x",data.l);
			else
			    fprintf (fp, "%u",data.l);
			format++;
			break;

		case 25: /* 16 bit displacement (dw) */
			if (!flags.absolute)
			    if (biti(data.disp16,15,15))  /* sign bit set? */
				fprintf (fp, "0x%04x (-)",
					(-data.disp16 & 32767));
			    else
				fprintf (fp, "0x%04x (+)",data.disp16);
			else
			    if (biti(data.disp16,15,15))  /* sign bit set? */
				fprintf (fp, "0x%08x",
					addr + 2 - (-data.disp16 & 32767));
			    else
				fprintf (fp, "0x%08x",addr + 2 + data.disp16);
			format += 2;
			break;

		case 26: /* bit number (bn) */
			if (flags.hex)
			    fprintf (fp, "0x%x",data.bitnum);
			else
			    fprintf (fp, "%d",data.bitnum);
			format += 2;
			break;

		case 27: /* register list (rl) */
			fprintf (fp, "%s",data.reglist);
			format += 2;
			break;

		case 28: /* source effective address (eas) */
			fprintf (fp, "%s",data.eas);
			format += 3;
			break;

		case 29: /* destination effective address (ead) */
			fprintf (fp, "%s",data.ead);
			format += 3;
			break;
	    }
	}
    }
    fprintf (fp, "\n");
}
