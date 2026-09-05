/*  getbool -- ask user a yes/no question
 *
 *  Usage:  i = getbool (prompt, defalt);
 *
 *  Example:  do {...} while (getbool ("More?",1));
 *
 *  Prints prompt string, asks user for response.  Defalt is
 *  0 (no) or 1 (yes), and is used if user types just carriage return.
 *
 *  HISTORY
 * 26-Apr-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Changed so it accepts 't' and 'T' as TRUE. 'f' and 'F' as FALSE.
 *
 * 19-Mar-82  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Rewritten for the 68000, most of the old code is gone
 *
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Rewritten for VAX.  Possible changes for the future: accept "t" (true)
 *	and "f" (false), 0 and 1, etc.
 */
#include <stdio.h>

getbool (prompt, defalt)
    char *prompt;
    int defalt;
{
    char input[128];

    if (defalt)			/* if not 0 (FALSE) */
	defalt = TRUE;

    while (1)
    {
	printf ("%s  [%s]  ", prompt, (defalt ? "yes" : "no"));

	if ((gets (input, 128) == NULL) || *input == '\0')
	    return (defalt);

	if (*input == 'y' || *input == 'Y' || *input == 't' || *input == 'T')
	    return (TRUE);

	if (*input == 'n' || *input == 'N' || *input == 'f' || *input == 'F')
	    return (FALSE);

	putchar (CTRL ('G'));	/* error!, sound the bell */
    } 
}
