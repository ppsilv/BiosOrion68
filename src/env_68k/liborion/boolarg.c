/*  boolarg  --  parse boolean from string
 *
 *  Usage:  i = boolarg (ptr,brk,prompt,defalt);
 *	int i,defalt;
 *	char **ptr,*brk,*prompt;
 *
 *  Boolarg will parse an argument from the string pointed to by "ptr",
 *  bumping ptr to point to the next argument in the string.  The
 *  argument parsed will be converted to a boolean (TRUE if it begins
 *  with 'y', 'Y', 't' or 'T'; FALSE for 'n', 'N', 'f' or 'F').  If there
 *  is no argument, or if it is not a boolean value, then getbool() will
 *  be used to ask the user for a boolean value.  In any event,
 *  the boolean value obtained (from the string or from the user) is
 *  returned.
 *  "Brk" is the list of characters which may terminate an argument;
 *  if 0, then " " is used.
 *
 *  HISTORY
 * 19-Apr-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	't', 'T', 'f' and 'F' cases added.
 *
 * 18-Apr-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Changed so it beeps at the user instead of cluttering the screen.
 *
 * 17-Feb-83  John Schlag (jfs) at Carnegie-Mellon University
 *	Ported to the 68000 for the Popeye system.
 *
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Created for VAX.
 */

#include <stdio.h>

char   *nxtarg ();

int     boolarg (ptr, brk, prompt, defalt)
char  **ptr,
       *brk,
       *prompt;
int     defalt;
{
    register char  *arg;
    register int    valu;

    valu = 2;			/* meaningless value */

    arg = nxtarg (ptr, brk);	/* parse an argument */
    if (*arg)
    {				/* there was an argument */
	switch (*arg)
	{
	    case 'n': 
	    case 'N':
	    case 'f':
	    case 'F':
		valu = FALSE;
		break;
	    case 'y': 
	    case 'Y':
	    case 't':
	    case 'T':
		valu = TRUE;
		break;
	    case '?': 
		break;
	    default: 
		putchar (CTRL ('G'));
	}
    }

    if (valu == 2)
	valu = getbool (prompt, defalt);

    return (valu);
}
