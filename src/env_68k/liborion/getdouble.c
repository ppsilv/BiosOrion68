/*  getdouble --  prompt user for double
 *
 *  Usage:  f = getdouble (prompt,min,max,defalt)
 *	double f,min,max,defalt;
 *	char *prompt;
 *
 *  Getdouble prints the message:  prompt  (min to max)  [defalt]
 *  and accepts a line of input from the user.  If the input
 *  is not null or numeric, an error message is printed; otherwise,
 *  the value is converted to a double (or the value "defalt" is
 *  substituted if the input is null).  Then, the value is
 *  checked to ensure that is lies within the range "min" to "max".
 *  If it does not, an error message is printed.  As long as
 *  errors occur, the cycle is repeated; when a legal value is
 *  entered, this value is returned by getdouble.
 *  On end-of-file or read error in the standard input, the default
 *  is returned.
 *
 *  HISTORY
 * 19-Apr-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	external declaration of atof () added.
 *
 *  8-Mar-83  John Schlag (jfs) at Carnegie-Mellon University
 *	Ported to the 68000 for the Popeye system.
 *
 * 23-Oct-82  Steven Shafer (sas) at Carnegie-Mellon University
 *	Added code to return default on EOF or error in standard input.
 *
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Created for VAX.
 */

#include <stdio.h>

extern double atof ();

double
getdouble (prompt, min, max, defalt)
    double  min, max, defalt;
    char   *prompt;
{
    register char  *p;
    register int    err;
    double  i;
    char input[128];

    extern char  *skipover ();

    while (1)
    {
	printf ("%s  (%g to %g)  [%g]  ", prompt, min, max, defalt);

	if ((gets (input, 128) == NULL) || *input == '\0')
	{
	    i = defalt;
	    if (i < min || i > max)
	    {
		putchar (CTRL ('G'));
		continue;
	    }
	    else
		return (defalt);
	}

	p = skipover (input, "0123456789+-.eE");

	if (*p)			/* non-numeric */
	{
	    putchar (CTRL ('G'));
	    continue;
	}
	else
	{
	    if (*input)
		i = atof (input);
	    else
		i = defalt;
	    if (i < min || i > max)
	    {
		putchar (CTRL ('G'));
		continue;
	    }
	    else
		return (i);
	}
    }
}
