/*
 *  nxtarg -- strip off arguments from a string
 *
 *  Usage:  p = nxtarg (&q,brk);
 *	char *p,*q,*brk;
 *	extern char _argbreak;
 *
 *	q is pointer to next argument in string
 *	after call, p points to string containing argument,
 *	q points to remainder of string
 *
 *  Leading blanks and tabs are skipped; the argument ends at the
 *  first occurence of one of the characters in the string "brk".
 *  When such a character is found, it is put into the external
 *  variable "_argbreak", and replaced by a null character; if the
 *  arg string ends before that, then the null character is
 *  placed into _argbreak;
 *  If "brk" is 0, then " " is substituted.
 *
 *  HISTORY
 * 12-Oct-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Bug fixed by Steve Shafer in the unix version ported. In
 *	eliminating the trailing blanks, we should check for back >=
 *	front.
 *
 * 17-Feb-83  John Schlag (jfs) at Carnegie-Mellon University
 *	Ported to the 68000 for the Popeye system.
 *
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Rewritten for VAX.  By popular demand, a table of break characters
 *	has been added (implemented as a string passed into nxtarg).
 *
 *  Originally	from klg (Ken Greer); IUS/SUS UNIX.
 */

char    _argbreak;
char   *skipto ();

char *
nxtarg (q, brk)
    char  **q,
           *brk;
{
    register char  *front, *back;

    /* start of string */

    front = *q;

    /* leading blanks and tabs */

    while (*front && (*front == ' ' || *front == '\t'))
	front++;

    /* find break character at end */

    if (brk == 0)
	brk = " ";
    back = skipto (front, brk);
    _argbreak = *back;
    *q = (*back ? back + 1 : back);	/* next arg start location */

    /* eliminate trailing blanks and tabs */

    back -= 1;
    while ((back >= front) && (*back == ' ' || *back == '\t'))
	back--;
    back++;
    if (*back)				/* null terminate */
	*back = '\0';
    return (front);
}
