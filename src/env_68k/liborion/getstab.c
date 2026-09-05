/*  getstab  --  ask user for string in table
 *
 *  Usage:  i = getstab (prompt,table,defalt)
 *	int i;
 *	char *prompt, **table, *defalt;
 *
 *  Getstab prints the messge:	prompt  [defalt]
 *  and asks the user to type in a line.  This input text
 *  is compared to all the strings in the table to see which
 *  (if any) it matches; the stablk() routine is used
 *  for the matching.  If the string is ambiguous or invalid
 *  (i.e. matches zero strings, or more than one), the cycle
 *  is repeated.  When a valid string is typed, the index
 *  of the string it matches is returned.  If the user just
 *  types carriage return, the default string is used for matching.
 *  The default is also used on error or EOF in standard input.
 *  The string table may be declared in this way:
 * 	char *table[] = {"string1","string2",...,0};
 *
 *  HISTORY
 *  8-Mar-83  John Schlag (jfs) at Carnegie-Mellon University
 *	Ported to the 68000 for the Popeye system. This code didn't return
 *	anything before. Luckily, stablk()'s result in D0 was untouched.
 *
 * 23-Oct-82  Steven Shafer (sas) at Carnegie-Mellon University
 *	Added code to use default value on error or EOF in standard input.
 *
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Rewritten for VAX.  Now lets stablk() do the error-message printing.
 *
 *  Originally by klg (Ken Greer) on IUS/SUS UNIX.
 */

#include <stdio.h>

getstab (prompt, table, defalt)
    char   *prompt,
          **table,
           *defalt;
{
    char    input[128];
    register int    ix;

    do
    {
	printf ("%s  [%s]  ", prompt, defalt);
	if ((gets (input, 128) == NULL) || *input == '\0')
	    strcpy (input, defalt);
	ix = stablk (input, table, FALSE);
    }
    while (ix < 0);

    return (ix);
}
