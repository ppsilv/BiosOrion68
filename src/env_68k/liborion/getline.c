/*
 *  getline --  prompt user for a newline-terminated string
 *
 *  Usage:  p = getline (prompt,defalt,answer);
 *	char *p,*prompt,*defalt,*answer;
 *
 *  Getstr prints this message:  prompt  [defalt]
 *  and accepts a line of input from the user.  This line is
 *  entered into "answer", which must be a big char array;
 *  if the user types just carriage return, then the string
 *  "defalt" is copied into answer.
 *  Value returned by getline is just the same as answer,
 *  i.e. pointer to result string.
 *
 *  HISTORY
 *	3-DEC-87	Dan Christian stole from popI, killed history.
 */

#include <stdio.h>

char *
getline (prompt, defalt, answer)
    char   *prompt,
           *defalt,
           *answer;
{
    char input[128];

    extern char *strcpy ();

    printf ("%s  [%s]  ", prompt, defalt);
    if (fgets (input, 128, stdin, stdout) == NULL)
	strcpy (answer, defalt);
    else if (input [0] == '\r' || input [0] == '\n')
	strcpy (answer, defalt);
    else
	strcpy (answer, input);
    return (answer);
}
