/*  MC68000 Disassembler Routine
 *  Programmer: Chad Bisk (cb29@andrew)
 *  Analyst: Dan Christian (dac@F.cs.cmu.edu)
 */

/* Implementation Note:
 *	Many parts of the source code to this dis-assembler look like they will
 *	accept illegal looking instruction codes when, in reality, they
 *	don't; this is a feature.  Either the illegal instruction code is legal
 *	when viewed in another format (in which case that format interpretation
 *	code should come first in the opcodes array) or it is actaully illegal
 *	and should never appear in usable code (The assumption is made that
 *	such illegal instructions are never handed to the dis-assembler.
 *
 *	Consequently the order of appearance of the instructions in the data
 *	array is crucial to the operation of the disasembler.  It is also
 *	probably wrong (ie. incorrectly ordered) with respect to one or two
 *	instruction precedences I didn't know about.
 */

#include <stdio.h>

#ifndef __GNU__
#define		CONST			/* const */
#else
#define		CONST			const
#endif

#define		TRUE			1
#define		FALSE			0
#define		EIGHT_BIT_MASK		~(~0 << 8)
#define		SIXTEEN_BIT_MASK	~(~0 << 16)

#define		NORMAL					0
#define		ERROR					-1
#define		ERR_EFFECTIVE_ADDRESS			-2
#define		ERR_EFFECTIVE_ADDRESS_REGISTER_PART	-423
#define		ERR_SIZE_OR_EFFECTIVE_ADDRESS		-3

struct opcode
{
  char *name;
  unsigned short mask;
  unsigned short template;
  char *args;
};

/* Kinds of Operands:
   F - fixed width constant field
   A - address register
   D - data register
   R - destination register
   r - source register - bit1: 
   E - destination effective address
   e - source effective address
   X - effective address - either way
   Q - quick immediate data
   S - size - 1 char: 0=word 1=long
   	      2 chars: 00=byte 01=word 10=long
   B - reverse direction of operation: 0=d=Dn 1=s=Dn
       (swap source and destination type: 0:=s=Dn,d=Dn  1:=s=-(An),d=-(An))
   f - direction of math operation: 0=right 1=left
   P - displacement
   H - Hex code to print: 8 - byte, 16 - word
   M - Move Size - 2 chars: 01=byte 11=word 10=long
   O - destination effective address - opposite order operands
*/

CONST struct opcode opcodes[] = 
{
  {"abcd",	0170760,	0140400,	"F4R3F5B1r3"},
  {"adda",	0170300,	0150300,	"F4A3S1F2e6"},
  {"addi",	0177400,	0003000,	"F8S2E6"},
  {"addq",	0170400,	0050000,	"F4Q3F1S2E6"},
  {"addx",	0170460,	0150400,	"F4R3F1S2F2B1r3"},
  {"add",	0170000,	0150000,	"F4D3B1S2X6"},
  {"andi ccr",	0177777,	0001074,	"F16"},
  {"andi sr",	0177777,	0001174,	"F16"},
  {"andi",	0177400,	0001000,	"F8S2E6"},
  {"and",	0170000,	0140000,	"F4D3B1S2X6"},
  {"as",	0177300,	0160300,	"F7f1F2E6"}, /* Memory */
  {"as",	0170070,	0160040,	"F4r3f1S2F3D3"}, /* Register */
  {"as",	0170070,	0160000,	"F4Q3f1S2F3D3"}, /* Quick */
  {"bccw",	0170377,	0060000,	"F4C4F8P16"},
  {"bccb", 	0170000,	0060000,	"F4C4P8"}, /* Cond Branch */
  {"bchgl",	0177770,	0004100,	"F13D3"},	/* Imm, Dn */
  {"bchgb",	0177700,	0004100,	"F10E6"},	/* Imm, Mem */
  {"bchgl",	0170770,	0000500,	"F4r3F6D3"},	/* Reg, Dn */
  {"bchgb",	0170700,	0000500,	"F4r3F3E6"},	/* Reg, Mem */
  {"bclrl",	0177770,	0004200,	"F13D3"},	/* Imm, Dn */
  {"bclrb",	0177700,	0004200,	"F10E6"},	/* Imm, Mem */
  {"bclrl",	0170770,	0000600,	"F4r3F6D3"},	/* Reg, Dn */
  {"bclrb",	0170700,	0000600,	"F4r3F3E6"},	/* Reg, Mem */
  {"braw",	0177777,	0060000,	"F16P16"},
  {"brab",	0177400,	0060000,	"F8P8"},
  {"bsetl",	0177770,	0004300,	"F13D3"},	/* Imm, Dn */
  {"bsetb",	0177700,	0004300,	"F10E6"},	/* Imm, Mem */
  {"bsetl",	0170770,	0000700,	"F4r3F6D3"},	/* Reg, Dn */
  {"bsetb",	0170700,	0000700,	"F4r3F3E6"},	/* Reg, Mem */
  {"bsrw",	0177470,	0060400,	"F16P16"},
  {"bsrb",	0177400,	0060400,	"F8P8"},
  {"btstl",	0177770,	0004000,	"F13D3"},	/* Imm, Dn */
  {"btstb",	0177700,	0004000,	"F10E6"},	/* Imm, Mem */
  {"btstl",	0170770,	0000400,	"F4r3F6D3"},	/* Reg, Dn */
  {"btstb",	0170700,	0000400,	"F4r3F3E6"},	/* Reg, Mem */
  {"chk",	0170700,	0040600,	"F4D3F3e6"},
  {"clr",	0177400,	0041000,	"F8S2E6"},
  {"cmp",	0170400,	0130000,	"F4D3F1S2e6"},
  {"cmpa",	0170300,	0130300,	"F4A3S1F2e6"},
  {"cmpi",	0177400,	0006000,	"F8S2E6"},
  {"cmpm",	0170470,	0130410,	"F4R3F1S2F3r3"},
  {"db",	0170370,	0050310,	"F4C4F5D3"},
  {"divsw",	0170700,	0100700,	"F4D3F3e6"},
  {"divuw",	0170700,	0100300,	"F4D3F3e6"},
  {"eori ccr",	0177777,	0005074,	"F16"},
  {"eori sr",	0177777,	0005174,	"F16"},
  {"eori",	0177400,	0005000,	"F8S2E6"},
  {"eor",	0170400,	0130400,	"F4r3F1S2E6"},
  {"exgl",	0170770,	0140500,	"F4D3F6D3"},	/* Dn <-> Dn */
  {"exgl",	0170770,	0140510,	"F4A3F6A3"},	/* An <-> An */
  {"exgl",	0170770,	0140610,	"F4D3F6A3"},	/* Dn <-> An */
  {"ext",	0177670,	0044200,	"F9S1F3D3"},
  {"illegal",	0177777,	0045374,	"F16"},
  {"jmp",	0177700,	0047300,	"F10E6"},
  {"jsr",	0177700,	0047200,	"F10E6"},
  {"lea",	0170700,	0040700,	"F4A3F3e6"},
  {"link",	0177770,	0047120,	"F13A3"},
  {"ls",	0177300,	0161300,	"F7f1F2E6"},	/* Memory */
  {"ls",	0170070,	0160050,	"F4r3f1S2F3D3"},/* Src Reg */
  {"ls",	0170070,	0160010,	"F4Q3f1S2F3D3"},/* Qck Imm */
  {"mulsw",	0170700,	0140700,	"F4D3F3e6"},
  {"muluw",	0170700,	0140300,	"F4D3F3e6"},
  {"nbcdb",	0177700,	0044000,	"F10E6"},
  {"neg",	0177400,	0042000,	"F8S2E6"},
  {"negx",	0177400,	0040000,	"F8S2E6"},
  {"nop",	0177777,	0047161,	"F16"},
  {"not",	0177400,	0043000,	"F8S2E6"},
  {"ori ccr",	0177777,	0000074,	"F16"},
  {"ori sr",	0177777,	0000174,	"F16"},
  {"ori",	0177400,	0000000,	"F8S2E6"},
  {"or",	0170000,	0100000,	"F4D3B1S2X6"},
  {"pea",	0177700,	0044100,	"F10e6"},
  {"reset",	0177777,	0047160,	"F16"},
  {"ro",	0177300,	0143300,	"F7f1F2E6"},	/* Memory */
  {"ro",	0170070,	0160070,	"F4r3f1S2F3D3"},/* Dn */
  {"ro",	0170070,	0160030,	"F4Q3f1S2F3D3"},/* Qck */
  {"rox",	0177300,	0162300,	"F7f1F2E6"},	/* Memory */
  {"rox",	0170070,	0160060,	"F4r3f1S2F3D3"},/* Dn */
  {"rox",	0170070,	0160020,	"F4Q3f1S2F3D3"},/* Qck */
  {"rte",	0177777,	0047163,	"F16"},
  {"rtr",	0177777,	0047167,	"F16"},
  {"rts",	0177777,	0047165,	"F16"},
  {"sbcd",	0170760,	0100400,	"F4R3F5B1r3"},
  {"s", 	0170300,	0050300,	"F4C4F2E6"},
  {"stop",	0177777,	0047162,	"F16"},
  {"suba",	0170300,	0110300,	"F4A3S1F2e6"},
  {"subi",	0177400,	0002000,	"F8S2E6"},
  {"subq",	0170400,	0050400,	"F4Q3F1S2E6"},
  {"subx",	0170460,	0110400,	"F4R3F1S2F2B1r3"},
  {"sub",	0170000,	0110000,	"F4D3B1S2X6"},
  {"swap",	0177770,	0044100,	"F13D3"},
  {"tas",	0177700,	0045300,	"F10E6"},
  {"trap",	0177760,	0147100,	"F12V4"},
  {"trapv",	0177777,	0047166,	"F16"},
  {"tst",	0177400,	0045000,	"F8S2E6"},
  {"unlk",	0177770,	0047130,	"F13A3"},
  {"move usp",	0177760,	0047140,	"F12B1A3"},
  {"move ccr",	0177700,	0042300,	"F10e6"},
  {"move sr",	0174700,	0040300,	"F5B2F3X6"},
  {"moveq",	0170400,	0070000,	"F4D3F1Q8"},
  {"movem",	0175600,	0044200,	"F5B1F3S1X6"},
  {"movep",	0170470,	0000410,	"F4D3F1B1S1F3A3"},
  {"moveaw",	0170700,	0030100,	"F4A3F3e6"},
  {"moveal",	0170700,	0020100,	"F4A3F3e6"},
  {"move",	0140000,	0000000,	"F2M2O6e6"},
  {".Data",	0,		0,		"H16"}		/* Catch errors */
};

CONST short numopcodes=sizeof(opcodes)/sizeof(opcodes[0]);

extern char		*sputs(), *sputsx(), *sputi();

unsigned short biti(wd,low,len)	/* returns value of bit field */
  unsigned short	wd;
  unsigned short	low, len;
{
    return((wd >> low) & ~(~0 << len));
}

getfieldstats(i,chr,low,len)
  unsigned short	i;
  char			chr;
  unsigned short	*low;
  unsigned short	*len;
{
    unsigned short  index = 0;
    char            t;
    
    *low = 0;
    while ((chr != (t = opcodes[i].args[index])) && (t != 0)) {
	*low = *low + atoi(opcodes[i].args + index);
	if (atoi(opcodes[i].args + index) > 9)
	  index += 2;
	else
	  index++;
    }
    if (t == 0) return (FALSE);
    *len = atoi(opcodes[i].args + index + 1);
    *low = 16 - *low - *len;
    return (TRUE);
}

dis68 (codestart, codelength, addr, options, fp)
  unsigned short	*codestart;	/* where the code is now	*/
  int			codelength;	/* number of binary code words	*/
  unsigned long		addr;		/* starting "address" for code	*/
  char			*options;	/* unused - for compatibility	*/
  FILE			*fp;
{
  unsigned short	*codeptr;	/* pointer to machine code */
  unsigned short	numread;	/* number of words read per complete instruction decoded */

  for(codeptr = codestart; codeptr < codestart + codelength;) {
      numread = translate(codeptr,addr,fp);
      codeptr += numread;	/* add number of WORDS read to codeptr	*/
      addr += numread * 2;	/* add number of BYTES read to addr	*/
  }
}

translate (codeptr, addr, fp)
  unsigned short	*codeptr;	/* points to word to be decoded */
  unsigned long		addr;		/* "address" of current instruction */
  FILE			*fp;		/* where the output will go */
{
    unsigned short	instrnum;	/* instruction index variable */
    unsigned short	rdcount = 0;	/* total number of words read */
    unsigned short	Word = 0;	/* instruction word value */
    char		*Size,		/* instruction operand size */
			*direction,	/* of math op: 0=r, 1=l */
			source[81],	/* holds Source args */
			dest[81],	/* holds Dest args (1 arg default) */
			*srcind,
			*destind;
    unsigned short	low, len, b;	/* low, length, bit chunk */
    int			i;		/* general index var */

    *source = '\0';
    *dest = '\0';
    direction = "\0";
    srcind = source;
    destind = dest;
    Word = *codeptr;			/* get a word */
    
    /* find the instruction index for the given Word */
    for (instrnum = 0; instrnum < numopcodes - 1 && ((Word & opcodes[instrnum].mask) != opcodes[instrnum].template); instrnum++);
    
    codeptr++;		/* point to next word to read */
    rdcount++;		/* keep track of number of words read while decoding this instruction */

    if(getfieldstats(instrnum, 'f', &low, &len))
      if(biti(Word,low,len)) direction = "l";
      else direction = "r";
    
    if(getfieldstats(instrnum, 'S', &low, &len)) {
	b = biti(Word,low,len);
	if (len == 2)
	  if (b == 0)
	    Size = "b";
	  else if (b == 1)
	    Size = "w";
	  else if (b == 2)
	    Size = "l";
	  else
	    Size = "Error";
	else		/* len==1, one size bit */
	  if (b == 0)
	    Size = "w";
	  else if (b == 1)
	    Size = "l";
	  else
	    Size = "Error - Incorrect Size Value";
    } else Size = "";
    fputsx((unsigned int)addr, 6, fp);	/* Prints the address word */
    fputs(":  ", fp);			/* pretty print spacing */

    /* Move Size - get the size for a move instruction */
    if(getfieldstats(instrnum, 'M',&low, &len)) {
	b = biti(Word,low,len);
	if (b == 1)
	  Size = "b";
	else if (b == 3)
	  Size = "w";
	else if (b == 2)
	  Size = "l";
	else
	  Size = "Error - Incorrect Size Value";
    } else Size = "";

    /* Data - print word in decimal form in operand field */
    if(getfieldstats(instrnum,'H',&low,&len))
      destind = sputi(Word & SIXTEEN_BIT_MASK, destind);

    if(getfieldstats(instrnum,'Q',&low,&len)) {
	srcind = sputs("#", srcind);
	srcind = sputi(biti(Word,low,len), srcind);
    }

    if(getfieldstats(instrnum,'A',&low,&len)) {
	destind = sputs("A", destind);
	destind = sputsx(biti(Word,low,len), 1, destind);
    }

    if(getfieldstats(instrnum,'D',&low,&len)) {
	destind = sputs("D", destind);
	destind = sputsx(biti(Word,low,len), 1, destind);
    }

    /* R - destination register - !!! assuming Data only */
    if(getfieldstats(instrnum,'R',&low,&len)) {
	destind = sputs("D", destind);
	destind = sputsx(biti(Word,low,len), 1, destind);
    }

    /* r - source register  (!!! assuming Data) */
    if(getfieldstats(instrnum,'r',&low,&len)) {
	srcind = sputs("D", srcind);
	srcind = sputsx(biti(Word,low,len), 1, srcind);
    }

    /* e - source effective address */
    if(getfieldstats(instrnum,'e',&low,&len))
      decodeeffectiveaddress(low, &codeptr, &rdcount, Word, Size, &srcind);
    
    /* E - destination effective address */
    if(getfieldstats(instrnum,'E',&low,&len))
      decodeeffectiveaddress(low, &codeptr, &rdcount, Word, Size, &destind);

    /* X - effective address - either way - depends on f field */
    if(getfieldstats(instrnum,'X',&low,&len)) { /* !!! need fix */
	decodeeffectiveaddress(low,&codeptr, &rdcount, Word, Size, &destind);
    }

    /* O - destination effective address - Opposite order operands */
    if(getfieldstats(instrnum,'O',&low,&len))
      decodeeffectiveaddress (low, &codeptr, &rdcount,
			      (Word && 0170077) || (biti(Word,9,3)<<6) || (biti(Word,6,3)<<9),
			      Size, &destind);

    /* Print out the words used in this instruction (1 to 4 by 16 bits) */
    for (i=0; i<rdcount; i++) {
	fputsx((unsigned int)(*(codeptr-rdcount+i)),4,fp); /* 16 bit word */
	fputs(" ",fp);					/* pretty printing */
    }
    for (i=rdcount; i<3; i++) fputs("     ", fp);
    fputs("\t",fp);
    
    fputs(opcodes[instrnum].name, fp);	/* Print instruction name */
    fputs(direction, fp);		/* direction of math op */
    fputs(Size, fp);			/* Append Size string */
    fputs("\t", fp);			/* pretty print */

    /* !!! could use some more pretty printing */
    fputs(source, fp);
    if (*source && *dest) fputs(", ", fp);
    fputs(dest, fp);

    fputs("\n", fp);
    return (rdcount);
}

decodeeffectiveaddress(low, codeptr, rdcount, Word, Size, strind)
  unsigned short	low;		/* lowest EA bit in operand word */
  unsigned short	**codeptr;	/* ptr to (current instr+1) */
  unsigned short	*rdcount;	/* # of words read for this instr */
  unsigned short	Word;		/* instruction word */
  char			*Size,		/* instruction size */
			**strind;	/* to return translated operand in */
{
    unsigned short	modepart, regpart; /* mode & register parts of EA */
    
    modepart = biti(Word,low+3,3);	/* get 1st part of eff. addr.	*/
    regpart = biti(Word,low,3);   	/* get 2nd part of eff. addr.	*/
    switch(modepart) {
      case 0:*strind=sputs("D",*strind);*strind=sputsx(regpart,1,*strind);
	break;
      case 1:*strind=sputs("A",*strind);*strind=sputsx(regpart,1,*strind);
	break;
      case 2:*strind = sputs("(A", *strind);
	*strind = sputsx(regpart, 1, *strind);
	*strind = sputs(")", *strind);
	break;
      case 3:*strind = sputs("(A", *strind);
	*strind = sputsx(regpart, 1, *strind);
	*strind = sputs(")+", *strind);
	break;
      case 4:*strind = sputs("-(A", *strind);
	*strind = sputsx(regpart, 1, *strind);
	*strind = sputs(")", *strind);
	break;
      case 5:*strind = sputs("0x", *strind);
	*strind = sputsx(**codeptr, 4, *strind);
	*strind = sputs("(A", *strind);
	*strind = sputsx(regpart, 1, *strind);
	*strind = sputs(")", *strind);
	(*codeptr)++;
	(*rdcount)++;
	break;
      case 6:*strind = sputs("0x", *strind);
	*strind = sputsx((**codeptr) & EIGHT_BIT_MASK, 2, *strind);
	*strind = sputs("(A", *strind);
	*strind = sputsx(regpart, 1, *strind);
	*strind = sputs(",X", *strind);
	if (biti(**codeptr,15,1)) *strind = sputs("A", *strind);
	else *strind = sputs("D", *strind); /* !!! may be backwards */
	/* !!! needs size from 11th bit */
	*strind = sputsx(biti(**codeptr,12,3), 1, *strind);
	*strind = sputs(")", *strind);
	(*codeptr)++;
	(*rdcount)++;
	break;
      case 7: /* Now it depends on the regpart # */
	switch(regpart) {
	  case 0: *strind = sputs("0x", *strind);	/* Absolute Short */
	    *strind = sputsx(**codeptr, 4, *strind);
	    break;
	  case 1: *strind = sputs("0x", *strind);	/* Absolute Long */
	    *strind = sputsx(**codeptr, 4, *strind);
	    (*codeptr)++;
	    (*rdcount)++;
	    *strind = sputsx(**codeptr, 4, *strind);
	    break;
	  case 2:*strind = sputs("0x", *strind); /* PC relative w/ offset */
	    *strind = sputsx(**codeptr, 2, *strind);
	    *strind = sputs("(PC)", *strind);
	    break;
	  case 3:*strind = sputs("0x", *strind); /* PC rel w/ index & offset */
	    *strind = sputsx((**codeptr) & EIGHT_BIT_MASK, 2, *strind);
	    *strind = sputs("(PC+X", *strind);
	    if (biti(**codeptr,15,1)) *strind = sputs("A", *strind);
	    else *strind = sputs("D", *strind); /* !!! may be backwards */
	    /* !!! needs size from 11th bit */
	    *strind = sputsx(biti(**codeptr,12,3), 1, *strind);
	    *strind = sputs(")", *strind);
	    break;
	  case 4:*strind = sputs("#", *strind);	/* Immediate */
	    if (Size == "b") *strind = sputsx((**codeptr) & EIGHT_BIT_MASK, 2, *strind);
	    else if (Size == "w") *strind = sputsx(**codeptr, 4, *strind);
	    else if (Size == "l") {
		*strind = sputsx(**codeptr, 4, *strind);
		(*codeptr)++;
		(*rdcount)++;
		*strind = sputsx(**codeptr, 4, *strind);
	    }
	    else {
		*strind = sputs("ERR_SIZE_UNKOWN", *strind);
		return(ERR_SIZE_OR_EFFECTIVE_ADDRESS);
	    }
	    break;
	  default: return(ERR_EFFECTIVE_ADDRESS_REGISTER_PART);
	}
	(*codeptr)++;
	(*rdcount)++;
	break;
      default: return(ERR_EFFECTIVE_ADDRESS);
    }
    return(NORMAL);
}
