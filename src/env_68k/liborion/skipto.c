/************************************************************************
 *  skipover and skipto -- skip over characters in string
 *
 *  Usage:	p = skipto (string,charset);
 *		p = skipover (string,charset);
 *
 *  char *p,*charset,*string;
 *
 *  Skipto returns a pointer to the first character in string which
 *  is in the string charset; it "skips until" a character in charset.
 *  Returns a pointer to the null-terminating character if it never finds
 *  a character in charset.
 *
 *  Skipover returns a pointer to the first character in string which
 *  is not in the string charset; it "skips over" characters in charset.
 *  Returns a pointer to the null-terminating character if all characters
 *  are in charset.
 *
 * HISTORY
 * 29-Mar-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	With John Schlag fixed a small bug. Also documented that it
 *	returns a pointer to the null-terminating character if special
 *	conditions are met.
 *
 * 17-Feb-83  John Schlag (jfs) at Carnegie-Mellon University
 *	Ported to the 68000 for the Popeye system. Completely reimplemented
 *	to save space rather than time.
 *
 * 26-Jun-81  David Smith (drs) at Carnegie-Mellon University
 *	Skipover, skipto rewritten to avoid inner loop at expense of space.
 *
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Skipover, skipto adapted for VAX from skip() and skipx() on the PDP-11
 *	(from Ken Greer).  The names are more mnemonic.
 *
 *	Sindex adapted for VAX from indexs() on the PDP-11 (thanx to Ralph
 *	Guggenheim).  The name has changed to be more like the index()
 *	and rindex() functions from Bell Labs; the return value (pointer
 *	rather than integer) has changed partly for the same reason,
 *	and partly due to popular usage of this function.
 */
#include <stdio.h>

char *skipto (string, charset)
    char  *string,
          *charset;
{
    register char *p;

    for (p = string ; *p && index (charset, *p) == NULL ; p++);

    return (p);
}

char *skipover (string, charset)
    char  *string,
	  *charset;
{
    register char *p;

    for (p = string ; *p && index (charset, *p) != NULL ; p++);

    return (p);
}
