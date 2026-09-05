/*  intarg  --  parse integer argument
 *
 *  Usage:  i = intarg (ptr,brk,prompt,min,max,default)
 *    int i,min,max,default;
 *    char **ptr,*brk,*prompt;
 *
 *  Will attempt to parse an argument from the string pointed to
 *  by "ptr", incrementing ptr to point to the next arg.  If
 *  an arg is found, it is converted into an integer.  If there is
 *  no arg or the value of the arg is not within the range
 *  [min..max], then "getint" is called to ask the user for an
 *  integer value.
 *  "Brk" is the list of characters which may terminate an argument;
 *  if 0, then " " is used.
 *
 *  HISTORY
 * 18-Apr-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Changed so it beeps at the user instead of cluttering the screen.
 *
 * 17-Feb-83  John Schlag (jfs) at Carnegie-Mellon University
 *	Ported to the 68000 for the Popeye system.
 *
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Rewritten for VAX.
 */

#include <stdio.h>

#define isdigit(c) ((c <= '9') && (c >= '0'))

char   *nxtarg ();

int     intarg (ptr, brk, prompt, min, max, defalt)
char  **ptr;
char   *brk,
       *prompt;
int     min,
        max,
        defalt;
{
    register int    i;
    register char  *arg,
                   *p;

    arg = nxtarg (ptr, brk);

    if (*arg != '\0')			/* if there was an arg */
    {
	for (p = arg; *p && (isdigit (*p) || *p == '-' || *p == '+'); p++);
	if (*p)
	{
	    if (strcmp (arg, "?") != 0)
		putchar (CTRL ('G'));
	}
	else
	{
	    i = atoi (arg);
	    if (i < min || i > max)
		putchar (CTRL ('G'));
	}
    }

    if (*arg == '\0' || *p != '\0' || i < min || i > max)
	i = getint (prompt, min, max, defalt);

    return (i);
}
