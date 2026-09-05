/*  hexarg  --  parse hexadecimal integer argument
 *
 *  Usage:  i = hexarg (ptr,brk,prompt,min,max,default)
 *    unsigned int i,min,max,default;
 *    char **ptr,*brk,*prompt;
 *
 *  Will attempt to parse an argument from the string pointed to
 *  by "ptr", incrementing ptr to point to the next arg.  If
 *  an arg is found, it is converted into an hexadecimal integer.  If there is
 *  no arg or the value of the arg is not within the range
 *  [min..max], then "gethex" is called to ask the user for an
 *  hexadecimal integer value.
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
 *	Created for VAX.
 */

#include <stdio.h>

char   *nxtarg ();
unsigned int    atox ();

int     hexarg (ptr, brk, prompt, min, max, defalt)
char  **ptr;
char   *brk,
       *prompt;
unsigned int    min,
                max,
                defalt;
{
    register unsigned int   i;
    register char  *arg,
                   *p;

    arg = nxtarg (ptr, brk);

    if (*arg != '\0')			/* if there was an arg */
    {
	if (arg[0] == '0' && (arg[1] == 'x' || arg[1] == 'X'))
	    strcpy (arg, arg + 2);
	for (p = arg; *p && ((*p >= '0' && *p <= '9') ||
		    (*p >= 'a' && *p <= 'f') ||
		    (*p >= 'A' && *p <= 'F')); p++);
	if (*p)
	{
	    if (strcmp (arg, "?") != 0)
		putchar (CTRL ('G'));
	}
	else
	{
	    i = atox (arg);
	    if (i < min || i > max)
		putchar (CTRL ('G'));
	}
    }

    if (*arg == '\0' || *p != '\0' || i < min || i > max)
	i = gethex (prompt, min, max, defalt);

    return (i);
}
