/*  doublearg  --  parse double argument
 *
 *  Usage:  i = doublearg (ptr,brk,prompt,min,max,default)
 *    double i,min,max,default;
 *    char **ptr,*brk,*prompt;
 *
 *  Will attempt to parse an argument from the string pointed to
 *  by "ptr", incrementing ptr to point to the next arg.  If
 *  an arg is found, it is converted into an double.  If there is
 *  no arg or the value of the arg is not within the range
 *  [min..max], then "getdouble" is called to ask the user for an
 *  double value.
 *  "Brk" is the list of characters which terminate an argument;
 *  if 0, then " " is used.
 *
 *  HISTORY
 * 18-Apr-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Changed so it beeps at the user instead of cluttering the screen.
 *
 *  8-Mar-83  John Schlag (jfs) at Carnegie-Mellon University
 *	Ported to the 68000 for the Popeye system.
 *
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Created for VAX.
 */

#include <stdio.h>

double  getdouble (), atof();
char   *nxtarg (), *skipover ();

double
doublearg (ptr, brk, prompt, min, max, defalt)
    char  **ptr;
    char   *brk,
           *prompt;
    double  min,
            max,
            defalt;
{
    register double i;
    register char  *arg,
                   *p;

    arg = nxtarg (ptr, brk);

    if (*arg != '\0')		/* if there was an arg */
    {
	p = skipover (arg, "0123456789+-.eE");
	if (*p)
	{
	    if (strcmp (arg, "?") != 0)
		putchar (CTRL ('G'));
	}
	else
	{
	    i = atof (arg);
	    if (i < min || i > max)
	    {
		putchar (CTRL ('G'));
	    }
	}
    }

    if (*arg == '\0' || *p != '\0' || i < min || i > max)
	i = getdouble (prompt, min, max, defalt);

    return (i);
}
