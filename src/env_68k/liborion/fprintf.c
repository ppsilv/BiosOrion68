/*--------------------------------------------------------------------------
 *
 * ABSTRACT
 *	FPRINF ()- Send formatted output to a FILE.
 *	SPRINTF ()- Put formatted output on a string buffer,
 *		with appended NULL.
 *	PRINTF ()- Send formatted output to the standard output.
 *
 * SYNOPSIS
 *	#include <stdio68>
 *
 *	fprintf (fp, CtlStr, arg1, arg2, ...)
 *	    FILE *fp;			where to send the information.
 *	    char *CtlStr;		Control String (see below)
 *	    ???  arg1, arg2, ...	Arguments (defined in CtlStr).
 *
 *	char *sprintf (buffer, CtlStr, arg1, arg2, ...)
 *	    char *buffer;		where to put the string
 *
 *	printf (CtlStr, arg1, arg2, ...)
 *
 * RESULTS
 *	Sprintf returns buffer. Fprintf and printf don't return anything.
 *
 * SIDE EFFECTS
 *	The formatted output is sent to the file specified. If printf is
 *	called, puts () is invoked. For fprintf (), fputs () is called.
 *
 * CONTROL STRING
 *	The control string specifies the output. All characters, except for
 *	'%' are used verbatim. Standard C quotes (beginning with '\') work
 *	normally. If a '%' is encountered in the control string, the following
 *	character (s) are examined to determine the type of argument that
 *	is on the stack, as well as the printing format desired.
 *
 *	The following codes are recognized:
 *		%Ff	'double' precision (currently 64 bits) float.
 *		%Fe	'double' precision float, exponential form.
 *		%Fg	'double' precision float. Smallest of %f or %e above.
 *		%Fd	'int' precision (currently 32 bits) decimal, signed.
 *		%Fu	'int' precision decimal, unsigned.
 *		%Fx	'int' precision hex, unsigned.
 *		%Fo	'int' precision octal, unsigned.
 *		%Fb	'int' precision binary, unsigned.
 *		%Fs	string
 *		%Fc	character
 *		%%	percent
 *
 *	In the previous codes, <F> is an optional field description
 *	consisting of the following: (1) An optional minus sign specifying
 *	that the data is to be right justified within the field; (2) an
 *	optional '0' specifying that the unused places of the field are to
 *	be filled with zeroes (the default is spaces); (3) a field-length
 *	code (n) specifying the minimum number of places the field should
 *	occupy; and (4) a precision code (n) --separated from the
 *	field-length code by a period,-- specifying the number of decimal
 *	places (for a %f, %g or %e) to be used (default six), or the
 *	maximum number of characters to be printed (for %s) within the field.
 *
 *	In the field-length and precision specifications, <n> may be either
 *	an integer or the character 'v' which means that the specification
 *	is the next argument in the call.
 *
 * ENVIRONMENT
 *
 *    Imports
 *	puts		stdio.h
 *	fputs		fputs.c
 *
 *    Exports
 *	fprintf (),
 *	sprintf (),
 *	printf ()		fprintf.c
 *
 * BUGS
 *	printf () will fail if more than 256 characters are sent at one
 *	time.
 *
 * DESIGN
 *	Straight forward, once you realize that the number of arguments
 *	is variable. It must pull them from the stack only when it needs
 *	them. It is possible to underrun the stack if not enough arguments
 *	are pushed. Popeyekrn will detect the situation, though.
 *
 * HISTORY
 * 03-Jun-93 Jim Montgomery (jfm) at University of Southern California
 *      Modified code to make use of the 68881 chip.  Added the include
 *      file <math-68881.h>.  This file contains macros that gcc uses
 *      to generate inline floating point instructions that implement
 *      various math library routines such as sin or modf.  Replaced the
 *      call to fix() with a call to floor().  *WARNING* This software will
 *      only work in a system that makes use of the 68881 FPU Coprocessor 
 *
 * 22-Nov-87 Dan Christian (dac) at CMU
 *	addapted to REX.
 *
 * 25-Apr-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	fprintf finally brought to life.
 *
 * 15-Apr-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	printfutils deleted (as well as iprintf ()). It was a hack
 *	anyhow.
 *
 * 15-Apr-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Fields allowed for %c codes. Again, we are just emulating the
 *	Vaxen's printf.
 *
 * 14-Apr-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Finally got %g working! It behaves pretty much like the Vaxen %g
 *	(as far as my tests go). 
 *
 * 13-Apr-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	printfutils created so iprintf and printf may be used from the
 *	same program without getting a lot of stuff twice.
 *
 * 13-Apr-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Floating point stuff added. Also the buffers were made globals
 *	so they don't have to be downloaded (or use the stack). Note
 *	that %g is VERY slow since it must use both conversions and
 *	decide which one uses fewer characters.
 *
 *  6-Mar-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Flushright and zero filling added trying to conform more with
 *	Kernighan and Ritchie's printf specification. Also modified the
 *	way it handles unknown codes, it simply quotes the next
 *	character.
 *
 *  3-Mar-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Imported for the user/supervisor kernel. This header page added.
 *	Binary mode (%b) and decimal unsigned (%u) added as well.
 *	fp_printf () is no longer in effect, it has been changed to
 *	Printf (), a static that all the other printf's call.
 *
 * 27-Jan-82  Jim McQuade (jpm) at Carnegie-Mellon University
 *	Changed printf to fp_printf which takes as an additional arguement
 *	a function pointer to the output function.  Changed the 'args'
 *	parameter from an integer to an integer pointer so fp_printf
 *	can run down the runtime stack and get its arbitrary list of
 *	arguements.  Procedures printf and printf2 now invoke fp_printf
 *	with putchar and put2, respectively, and also pass the arguement
 *	list pointer.
 *
 *----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <math-68881.h>

#define	ESC		'%'

#define REAL		0	/* cases for putfloat (%f, %e and %g) */
#define EXP		1
#define SMALLEST	2

#define BUFSIZE		256	/* Buffer for printf */
char printfbuf [BUFSIZE];

#define FLOATDIGS	50	/* 50 digits in a huge double */
char temp [FLOATDIGS];		/* used by cvt () */
unsigned char fbuf [FLOATDIGS + 10];
unsigned char ebuf [FLOATDIGS + 10];


/*
 *	These are called by the user ...
 */

printf (ctl, arg1)
    char *ctl;
{
    _Printf (printfbuf, ctl, &arg1);
    puts (printfbuf);
}

char *
sprintf (buffer, ctl, arg1)
    char *buffer;
    char *ctl;
{
    _Printf (buffer, ctl, &arg1);
    return buffer;
}

fprintf (fp, ctl, arg1)
    register FILE *fp;
    char *ctl;
{
    _Printf (printfbuf, ctl, &arg1);
    fputs (printfbuf, fp);
}

/*
 *	All flavors of printf call this routine.
 */

_Printf (out, ctl, argp)
    register char  *out;
    register char  *ctl;
    register long  *argp;	/* 1st argument's address */
{
    register char  *cc = ctl;	/* Control string */
    register char   ch;
    register char   *p;		/* a working pointer */
    int Arg1, Arg2, Right;
    char Zfill;
    char *RdArg (),		/* read an numerical argument */
	*putnum (),		/* put a numerical value */
	*putch (),		/* put a character */
	*putstring (),		/* put a string */
	*putfloat ();		/* or put a float */

    while (ch = *cc++)
    {
	if (ch != ESC) {
	    *out++ = ch;
	    continue;
	}

	Arg1 = Arg2 = -1;
	Right = 0;
	Zfill = ' ';

	if (*cc == '-') {
	    Right = -1;
	    cc++;
	}

	if (*cc == '0') {
	    Zfill = '0';
	    cc++;
	}

	if (((ch = *cc++) >= '0' && ch <= '9') || (ch == 'v'))
	{
	    if (ch == 'v')	/* minimum field is an argument */
		Arg1 = *argp++;
	    else		/* it is a constant, read it */
		cc = RdArg (--cc, &Arg1);

	    ch = *cc++;
	}

	if (ch == '.')		/* 'n.n' precision fields */
	{
	    if (*cc == 'v') {
		Arg2 = *argp++;
		cc++;
	    }
	    else
		cc = RdArg (cc, &Arg2);

	    ch = *cc++;
	}

	switch (ch)
	{
	    case 0:
		*out++ = ESC;	/* % at the end of a string */
		break;

	    case 'd':		/* decimal signed */
		out = putnum (*argp++, Arg1, Right, 10, 1, Zfill, out);
		break;

	    case 'u':		/* decimal unsigned */
		out = putnum (*argp++, Arg1, Right, 10, 0, Zfill, out);
		break;

	    case 'o':		/* octal unsigned */
		out = putnum (*argp++, Arg1, Right, 8, 0, Zfill, out);
		break;

	    case 'x':		/* hexadecimal unsigned */
		out = putnum (*argp++, Arg1, Right, 16, 0, Zfill, out);
		break;

	    case 'b':		/* binary unsigned */
		out = putnum (*argp++, Arg1, Right, 2, 0, Zfill, out);
		break;

	    case 'c':		/* character */
		out = putch (*argp++, Arg1, Right, Zfill, out);
		break;

	    case 's':		/* string */
		out = putstring (*argp++, Arg1, Arg2, Right, Zfill, out);
		break;

	    case 'f':		/* float */
		out = putfloat (*(double *)argp, Arg1, Arg2, Right,
				Zfill, REAL, out);
		argp += 2;	/* doubles use two longs in the stack */
		break;

	    case 'e':		/* exponential float */
		out = putfloat (*(double *)argp, Arg1, Arg2, Right,
				Zfill, EXP, out);
		argp += 2;
		break;

	    case 'g':		/* use the smallest possible output */
		out = putfloat (*(double *)argp, Arg1, Arg2, Right,
				Zfill, SMALLEST, out);
		argp += 2;
		break;

	    default: 
		*out++ = ch;	/* quote the next character */
		break;
	}
    }
    *out = '\0';		/* null terminate the string */
}

/*
 *	Read a decimal argument from the string <*ptr>
 *	and put it in <*where>. Return the new value of <ptr>.
 *	Used to pull constant fields from the control string.
 */


static char *
RdArg (ptr, where)
    register char  *ptr;
    register int   *where;
{
    register int val = 0;

    while (*ptr >= '0' && *ptr <= '9')
	val = ((val << 3) + (val << 1)) + *ptr++ - '0';

    *where = val;

    return ptr;
}

/*
 *	Routine to put the number <val> in the buffer <*out>. Use
 *	<fmt> places (min), radix <radix> (2, 8, 10 or 16), make it
 *	signed if <sign> is set, right justify the number if <right>
 *	and fill unused places with the <fill> character. Returns
 *	the new value of <out>.
 */


static char *
putnum (val, fmt, right, radix, sign, fill, out)
    register int    val;
    int    fmt;			/* -1 if no minimum field was specified */
    int    right;
    register int    radix;
    int    sign;
    char   fill;
    register char   *out;
{
    char    digbuf[32],		/* max 32 chars (%b on long) */
            negflag = 0;

    register int ndig = 0;
    int     spaces = fmt;	/* maximum number of spaces == all */

    if (sign && val < 0) {	/* make it positive */
	if ((val = -val) < 0)	/* we have the -0 case! */
	    val = 0;
	negflag++;
    }

    switch (radix) {

	case 10:
	    ndig = putn2 (val, digbuf);
	    break;

	case 16:
	    do {
		digbuf[ndig++] = val & 0XFL;
		val = (val >> 4) & 0X0FFFFFFF;
	    }
	    while (val);
	    break;

	case 8:
	    do {
		digbuf[ndig++] = val & 0X7L;
		val = (val >> 3) & 0X1FFFFFFF;
	    }
	    while (val);
	    break;

	case 2:
	    do {
		digbuf[ndig++] = val & 0X1L;
		val = (val >> 1) & 0X7FFFFFFF;
	    }
	    while (val);
	    break;
    }

    spaces = fmt - ndig - negflag;	/* adjust number of spaces */

    if (!right)
	while (spaces-- > 0)
	    *out++ = fill;

    if (negflag)
	*out++ = '-';

    while (ndig)
    {
	register int temp;

	if ((temp = digbuf[--ndig]) >= 10)
	    temp += 'A' - '0' - 10;
	*out++ = temp + '0';
    }

    if (right)
	while (spaces-- > 0)
	    *out++ = ' ';

    return out;
}

/*
 *	Routine called to put a decimal value <val> in the character
 *	buffer <*digs>. It returns the number of digits in the buffer.
 */


static unsigned long DecTab[] = {
    1000000000L, 100000000L, 10000000L, 1000000L,
    100000L, 10000L, 1000L, 100L, 10L, 1L, 0L
};


static int
putn2 (val, digs)
    register unsigned long val;
    register char  *digs;
{
    register unsigned long *lp = DecTab;
    register int    ndigs = 10;		/* max 10 decimals in 32 bits */
    register int    i = 0;
    register int    dig;

    if (val == 0) {		/* put at least a zero */
	digs[0] = 0;
	return 1;
    }

    while (*lp > val) {		/* search for the first table value */
	lp++;
	ndigs--;
    }

    while (ndigs)		/* for all the significant digits */
    {
	dig = 0;

	do {
	    if (val >= *lp) {
		val -= *lp;
		dig++;
	    }
	    else {
		lp++;		/* point to the next table value */
	        break;		/* out of the loop */
	    }
	} while (1);
				/* dig has the number of times this
				   table value fits in val	 */
	digs[--ndigs] = dig;
	i++;
    }

    return i;
}

/*
 *	Put the character <c> in the character buffer <*out>. Use
 *	a minimum of <fmt> places, right justify if <right>. If
 *	left justified, use <fill> as the filling character.
 *	Returns the new value of <out>.
 */


static char *
putch (c, fmt, right, fill, out)
    char c;
    register int fmt;
    int right;
    register char fill;
    register char *out;
{
    if (fmt == -1) {
	*out++ = c;
	return out;
    }

    if (!right) {
	register int i;

	for (i = fmt - 1; i > 0; i--)
	    *out++ = fill;
    }

    *out++ = c;

    if (right) {
	register int i;

	for (i = fmt - 1; i > 0; i--)
	    *out++ = ' ';
    }

    return out;
}

/*
 *	Put the string <*str> in the character buffer <*out>. Use
 *	a minimum of <fmt1> places, write a maximum of <fmt2>
 *	characters and right justify if <right> is not zero.
 *	Returns the new value of <out>.
 */


static char *
putstring (str, fmt1, fmt2, right, fill, out)
    register char  *str;
    register int  fmt1;
    int  fmt2,  right;
    register char fill;
    register char  *out;
{
    register int len = strlen (str);

    if (fmt2 != -1 && len > fmt2)
	len = fmt2;

    fmt1 -= len;		/* number of places to fill */

    if (!right) {
	register int i;

	for (i = fmt1; i > 0; i--)
	    *out++ = fill;
    }

    while (*str && len--)
	*out++ = *str++;

    if (right) {
	register int i;

	for (i = fmt1; i > 0; i--)
	    *out++ = ' ';
    }

    return out;
}

/*
 *	Routine to put the double <val> in the buffer <*out>. Use
 *	<fmt1> places (min) with <fmt2> digits after the decimal
 *	point. Right justify the number if <right> and fill unused
 *	spaces (to the left) with <fill>. If <mode> is EXP, use exponential
 *	format; if REAL, use float format; if SMALLEST, use whichever uses
 *	less space. Returns the updated value of <out>.
 */

static char *
putfloat (val, fmt1, fmt2, right, fill, mode, out)
    double val;
    int fmt1, fmt2;
    int right;
    char fill;
    int mode;
    register char *out;
{
    int DecExp;			/* position of the decimal point if mode is
				   REAL. If mode is EXP, has the exponent */
    int sign;			/* set to one if negative */
    register int i, j;
    register int eused, fused;	/* for the %g case */
    int efracc, ffracc;		/* flags: true iff fraccional part used */
    register unsigned char *p;
    register unsigned char *q;
    char *cvt ();		/* routine that performs the conversion */

    if (fmt2 == -1)		/* default: six characters */
	fmt2 = 6;
    if (mode == EXP)		/* make space for the integer */
	fmt2++;

    if (mode != REAL)		/* if exponential format is desired */
    {
	q = ebuf;
	p = (unsigned char *)
	    cvt (val, fmt2, &DecExp, &sign, &efracc, 1, temp);

	if (sign)
	    *q++ = '-';
	if (*p == '0')		/* adjust for zero */
	    DecExp++;
	*q++ = *p++;		/* output the integer and decimal point */
	*q++ = '.';
				/* now save the number */
	for (i = fmt2; --i > 0; *q++ = *p++);

	*q++ = 'e';
	DecExp--;
	if (DecExp >= 0)
	    *q++ = '+';
	else {
	    *q++ = '-';
	    DecExp = -DecExp;
	}

				/* save the exponent */
	*q++ = (unsigned char) DecExp / 10 + '0';
	*q++ = (unsigned char) DecExp % 10 + '0';

	eused = q - ebuf;
    }

    if (mode != EXP)	/* normal floating format */
    {
	q = fbuf;
	p = (unsigned char *)
	    cvt (val, fmt2, &DecExp, &sign, &ffracc, 0, temp);

	if (sign)
	    *q++ = '-';
	if ((i = DecExp) <= 0)
	    *q++ = '0';
	else
	    do *q++ = *p++;
	    while (--i);

	if (i = fmt2)
	     *q++ = '.';

	if ((DecExp = -DecExp) > 0)
	    while (--i >= 0) {
		*q++ = '0';
		if (--DecExp <=0) break;
	    }

	if (i > 0)
	    while (--i >= 0)
		*q++ = *p++;

	fused = q - fbuf;
    }

    /*	It is time to output the buffer */

    if (mode == SMALLEST)	/* must choose the smallest output */
    {
	for (p = fbuf + fused - 1; p >= fbuf; fused--, p--)
	{
	    if (ffracc && (*p != '0'))
		break;
	    if (!ffracc && (*p == '.')) {
		fused--;	/* don't output the decimal point */
		break;
	    }
	}
	if (ffracc && (*p == '.'))	/* force exponential! */
	    fused = FLOATDIGS;

	q = ebuf + eused - 4;	/* point to the 'e' character */
	for (p = ebuf + eused - 5; p >= ebuf; eused--, p--)
	{
	    if (efracc && (*p != '0')) {
		p++;		/* output this decimal value */
		break;
	    }
	    if (!efracc && (*p == '.')) {
		eused--;
		break;
	    }
	}
	if (p != q)		/* if there were deletions */
	    for (i = 4; i; i--)
		*p++ = *q++;


	if (eused < fused) {
	    p = ebuf;
	    q = ebuf + eused;
	}
	else {
	    p = fbuf;
	    q = fbuf + fused;
	}
    }
    else			/* simply let p point to the proper buffer */
	p = (mode == REAL) ? fbuf : ebuf;

    /*
     *	p is pointing to the beginning of the buffer to be sent out.
     *	q points to the last valid character of such buffer.
     */

    if ((i = fmt1 - (q - p)) < 0)
	i = 0;		/* no need for filling characters */

    if (right == 0)
	while (i-- > 0)
	    *out++ = fill;

    for (j = (q - p); j; j--)
	*out++ = *p++;

    if (right)
	while (i-- > 0)
	    *out++ = ' ';

    return out;
}

/*
 *	Routine to convert to decimal <ndigits> of the double <arg>.
 *	On exit, <decpt> is set to the position of decimal point.
 *	<sign> is set to 0 for positive and 1 for negative. <fracc>
 *	is set to 0 if the number is an even integer. The data
 *	will be in <buf>. If <eflag>, exponential format will be used.
 */

static char *
cvt (arg, ndigits, decpt, sign, fracc, eflag, buf)
    double arg;
    int ndigits, *decpt, *sign, *fracc;
    int eflag;
    char *buf;
{
    register int r2;
    double fi, fj;
    register char *p, *p1;
    double modf ();
    double floor();
    double floor_val;

    if (ndigits < 0)
	ndigits = 0;
    if (ndigits >= FLOATDIGS - 1)
	ndigits = FLOATDIGS-2;

    r2 = 0;
    *sign = *fracc = 0;
    p = &buf [0];

    if (0.0 > arg) {
	*sign = 1;
	arg = -arg;
    }

    if ((arg = modf (arg, &fi)) != 0.0)
	*fracc = 1;
    p1 = &buf [FLOATDIGS];

    /*
     * Do integer part
     */

    if (fi != 0)
    {
	p1 = &buf [FLOATDIGS];
	while (fi != 0)
	{
	    fj = modf (fi / 10, &fi);
            floor_val = fj*10.1+0.5;
	    *--p1 = floor (floor_val) + '0';
	    r2++;
	}
	while (p1 < &buf [FLOATDIGS])
	    *p++ = *p1++;
    }
    else if (arg > 0)
    {
	while ((fj = arg * 10) < 1)
	{
	    arg = fj;
	    r2--;
	}
    }

    p1 = &buf[ndigits];
    if (eflag==0)
	p1 += r2;
    *decpt = r2;

    if (p1 < &buf[0]) {
	buf[0] = '\0';
	return(buf);
    }

    /*
     *	Do fractional part
     */

    while (p<=p1 && p<&buf[FLOATDIGS])
    {
	arg *= 10;
	arg = modf(arg, &fj);
	*p++ = (int)fj + '0';
    }

    if (p1 >= &buf[FLOATDIGS])
    {
	buf[FLOATDIGS-1] = '\0';
	return(buf);
    }

    p = p1;
    *p1 += 5;
    while (*p1 > '9')
    {
	*p1 = '0';
	if (p1>buf)
	    ++*--p1;
	else
	{
	    *p1 = '1';
	    (*decpt)++;
	    if (eflag==0)
	    {
		if (p>buf)
		    *p = '0';
		p++;
	    }
	}
    }

    *p = '\0';
    return (buf);
}
