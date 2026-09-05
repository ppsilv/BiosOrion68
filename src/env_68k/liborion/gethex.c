/*
 * GETHEX()- C routine that prompts the user for an int in hex notation
 *
 * Usage:  x = gethex (prompt, min, max, defalt)
 *	int x, min, max, defalt;
 *	char *prompt;
 *	
 * Gethex prints the message: <prompt (min to max) [defalt]>
 * and accepts a line of input from the user. If the input is 
 * out of range, a CTRL-G is sounded to signal the user and the
 * prompt string is displayed again. When a valid integer is 
 * entered, it is returned.
 *
 * HISTORY
 * 03-Mar-82  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Made from getint.c 
 *	The following history logs are from that routine (getint())
 *
 * 25-Jan-82  John Schlag (jfs) at Carnegie-Mellon University
 *	Rewritten for the M68000. Not much left of the original code.
 *	Changed error message to Ctrl-G. Don't like someone else scribbling
 *	on my screen.
 *
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Rewritten for VAX.
 */
#include <stdio.h>

gethex (prompt, min, max, defalt)
    int     min,
            max,
            defalt;
    char   *prompt;
{
    register int    x;
    char input[128];

    while (1)
    {
	printf ("%s  (%x to %x)  [%x]  ", prompt, min, max, defalt);

	if ((gets (input, 128) == NULL) || *input == '\0')
	    return (defalt);

	x = atox (input);
	if ((x >= min) && (x <= max))
	    return (x);

	putchar (CTRL ('G'));	/* CTRL-G on error */
    }
}
