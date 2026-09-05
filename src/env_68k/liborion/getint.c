/*  getint --  prompt user for int
 *
 *  Usage:  i = getint (prompt,min,max,default)
 *	int i,min,max,default;
 *	char *prompt;
 *
 *  Getint prints the message:  prompt  (min to max)  [defalt]
 *  and accepts a line of input from the user.  If the input
 *  is out of range, a CTRL-G is sounded to signal the user, and
 *  the prompt string is displayed again. When a valid integer
 *  is entered, it is returned.
 *
 *  HISTORY
 * 25-Jan-82  John Schlag (jfs) at Carnegie-Mellon University
 *	Rewritten for the M68000. Not much left of the original code.
 *	Changed error message to Ctrl-G. Don't like someone else scribbling
 *	on my screen.
 *
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Rewritten for VAX.
 */
#include <stdio.h>

getint (prompt, min, max, defalt)
    char   *prompt;
    int     min,
            max,
            defalt;
{
    register int    i;
    char input[128];

    while (1)
    {
	qprintf ("%s  (%d to %d)  [%d]  ", prompt, min, max, defalt);
	if ((gets (input, 128) == NULL) || *input == '\0')
	    return (defalt);
	i = atoi (input);
	if ((i >= min) && (i <= max))
	    return (i);
	putchar (CTRL ('G'));	/* CTRL-G on error */
    }
}
