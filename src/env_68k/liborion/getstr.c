/*
 * GETSTR ()- Get a null terminated string AND strip out the newline.
 *
 *	   If the user is interested in the terminating character, he should
 *	call getline ().
 *
 * HISTORY
 * 17-Feb-83  John Schlag (jfs) at Carnegie-Mellon University
 *	Added the definition for eoln(c) since the monitor may no longer
 *	have the function of that name.
 *
 * 28-Apr-82  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Created.
 */

#define eoln(c) ((c == '\r') || (c == '\n') || (c == '\033'))

extern char *getline ();

char *getstr (prompt, defalt, answer)
    char *prompt, *defalt, *answer;
{
    register char *p;

    answer = getline (prompt, defalt, answer);

    for (p = answer; *p != '\0'; p++);	/* go to the end of the line */

    --p;
    if (eoln (*p)) *p = '\0';	/* if there is a terminating char delete it */

    return (answer);
}
