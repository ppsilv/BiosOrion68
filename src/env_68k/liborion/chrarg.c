/*  chrarg  --  parse character and return its index
 *
 *  Usage:  i = chrarg (ptr,brk,prompt,legals,defalt);
 *	int i;
 *	char **ptr,*brk,*prompt,*legals,defalt;
 *
 *  Chrarg will parse an argument from the string pointed to by "ptr",
 *  bumping ptr to point to the next argument.  The first character
 *  of the arg will be searched for in "legals", and its index
 *  returned; if it is not found, or if there is no argument, then
 *  getchr() will be used to ask the user for a character.
 *  "Brk" is the list of characters which may terminate an argument;
 *  if it is 0, then " " is used.
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

char   *index (), *nxtarg ();

int     chrarg (ptr, brk, prompt, legals, defalt)
char  **ptr,
       *brk,
       *prompt,
       *legals,
        defalt;
{
    register int    i;
    register char  *arg,
                   *p;

    i = -1;			/* bogus value */

    arg = nxtarg (ptr, brk);	/* parse argument */

    if (*arg)			/* there was an arg */
    {
	p = index (legals, *arg);
	if (p)
	    i = p - legals;
	else
	    if (strcmp ("?", arg) != 0)
		putchar (CTRL ('G'));
    }

    if (i < 0)
	i = getchr (prompt, legals, defalt);

    return (i);
}
