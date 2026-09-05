/*--------------------------------------------------------------------------
 *
 * ABSTRACT
 *	FPRINF ()- Send formatted output to a FILE.
 *	SQPRINTF ()- Put formatted output on a string buffer,
 *		with appended NULL.
 *	QPRINTF ()- Send formatted output to the standard output.
 *
 * SYNOPSIS
 *	#include <stdio68>
 *
 *	fqprintf (fp, CtlStr, arg1, arg2, ...)
 *	    FILE *fp;			where to send the information.
 *	    char *CtlStr;		Control String (see below)
 *	    ???  arg1, arg2, ...	Arguments (defined in CtlStr).
 *
 *	char *sqprintf (buffer, CtlStr, arg1, arg2, ...)
 *	    char *buffer;		where to put the string
 *
 *	qprintf (CtlStr, arg1, arg2, ...)
 *
 * RESULTS
 *	Sqprintf returns buffer. Fqprintf and qprintf don't return anything.
 *
 * SIDE EFFECTS
 *	The formatted output is sent to the file specified. If qprintf is
 *	called, puts () is invoked. For fqprintf (), fputs () is called.
 *
 * CONTROL STRING
 *	The control string specifies the output. All characters, except for
 *	'%' are used verbatim. Standard C quotes (beginning with '\') work
 *	normally. If a '%' is encountered in the control string, the following
 *	character (s) are examined to determine the type of argument that
 *	is on the stack, as well as the printing format desired.
 *
 *	The following codes are recognized:
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
 *	fqprintf (),
 *	sqprintf (),
 *	qprintf ()		fqprintf.c
 *
 * BUGS
 *	qprintf () will fail if more than 256 characters are sent at one
 *	time.
 *
 * DESIGN
 *	Straight forward, once you realize that the number of arguments
 *	is variable. It must pull them from the stack only when it needs
 *	them. It is possible to underrun the stack if not enough arguments
 *	are pushed. Popeyekrn will detect the situation, though.
 *
 * HISTORY
 * 22-Nov-87 Dan Christian (dac) at CMU
 *	addapted to REX.
 * 12-Apr-93 Andrew H. Fagg
 *       Added lock to qprintfbuf to avoid multi-process contention problems.
 *
 *----------------------------------------------------------------------------
 */

#include <stdio.h>

#define	ESC		'%'

#define REAL		0	/* cases for putfloat (%f, %e and %g) */
#define EXP		1
#define SMALLEST	2

#define BUFSIZE		256	/* Buffer for qprintf */
char qprintfbuf [BUFSIZE];

int qprintf_lock = FALSE;	/* Lock for qprintfbuf */

/*
 *	These are called by the user ...
 */

qprintf (ctl, arg1)
    char *ctl;
{
  pend(&qprintf_lock, TRUE, P_SET);
  _Qprintf (qprintfbuf, ctl, &arg1);
  puts (qprintfbuf);
  post(&qprintf_lock, FALSE);
}

char *
sqprintf (buffer, ctl, arg1)
    char *buffer;
    char *ctl;
{
  _Qprintf (buffer, ctl, &arg1);
  return buffer;
}

fqprintf (fp, ctl, arg1)
    FILE *fp;
    char *ctl;
{
  pend(&qprintf_lock, TRUE, P_SET);
  _Qprintf (qprintfbuf, ctl, &arg1);
  fputs (qprintfbuf, fp);
  post(&qprintf_lock, FALSE);
}


/*
 *	All flavors of qprintf call this routine.
 */

_Qprintf (out, ctl, argp)
    char  *out;
    char  *ctl;
    long  *argp;	/* 1st argument's address */
{
    char  *cc = ctl;	/* Control string */
    char   ch;
    char   *p;		/* a working pointer */
    int Arg1, Arg2, Right;
    char Zfill;
    char *RdArg (),		/* read an numerical argument */
	*putnum (),		/* put a numerical value */
	*putch (),		/* put a character */
	*putstring ();		/* put a string */

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
    char  *ptr;
    int   *where;
{
    int val = 0;

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
    int    val;
    int    fmt;			/* -1 if no minimum field was specified */
    int    right;
    int    radix;
    int    sign;
    char   fill;
    char   *out;
{
    char    digbuf[32],		/* max 32 chars (%b on long) */
            negflag = 0;

    int ndig = 0;
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
	int temp;

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


const static unsigned long DecTab[] = {
    1000000000L, 100000000L, 10000000L, 1000000L,
    100000L, 10000L, 1000L, 100L, 10L, 1L, 0L
};


static int
putn2 (val, digs)
    unsigned long val;
    char  *digs;
{
    unsigned long *lp = DecTab;
    int    ndigs = 10;		/* max 10 decimals in 32 bits */
    int    i = 0;
    int    dig;

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
    int fmt;
    int right;
    char fill;
    char *out;
{
    if (fmt == -1) {
	*out++ = c;
	return out;
    }

    if (!right) {
	int i;

	for (i = fmt - 1; i > 0; i--)
	    *out++ = fill;
    }

    *out++ = c;

    if (right) {
	int i;

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
    char  *str;
    int  fmt1;
    int  fmt2,  right;
    char fill;
    char  *out;
{
    int len = strlen (str);

    if (fmt2 != -1 && len > fmt2)
	len = fmt2;

    fmt1 -= len;		/* number of places to fill */

    if (!right) {
	int i;

	for (i = fmt1; i > 0; i--)
	    *out++ = fill;
    }

    while (*str && len--)
	*out++ = *str++;

    if (right) {
	int i;

	for (i = fmt1; i > 0; i--)
	    *out++ = ' ';
    }

    return out;
}
