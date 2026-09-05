/*  strarg  --  parse a string
 *
 *  Usage:  p = strarg (ptr, brk, prompt, defalt, buf);
 *	char *p, **ptr, *brk, *prompt, *defalt, *buf;
 *
 *  Strarg will parse an argument from the string pointed to by "ptr",
 *  bumping ptr to point to the next argument in the string.
 *  The argument parsed will be copied into "buf".  If there is no
 *  argument, then getstr() will be called, and the value placed into
 *  buf.  In any event, the address of buf is returned as a value.
 *  "Brk" is the list of characters which terminate an argument;
 *  if 0, then " " is used.
 *
 *  HISTORY
 * 17-Feb-83  John Schlag (jfs) at Carnegie-Mellon University
 *	Ported to the 68000 for the Popeye system.
 *
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Rewritten for VAX.
 */

char   *nxtarg ();

char   *strarg (ptr, brk, prompt, defalt, buf)
char  **ptr,
       *brk,
       *prompt,
       *defalt,
       *buf;
{
    register char  *arg;

    arg = nxtarg (ptr, brk);	/* parse an argument */

    if (*arg && strcmp (arg, "?") != 0)
	strcpy (buf, arg);
    else
	getstr (prompt, defalt, buf);

    return (buf);
}
