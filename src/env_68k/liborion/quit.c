/*--------------------------------------------------------------------------
 *
 * ABSTRACT
 *	QUIT () - Exit with an error code and a ``printf'' message.
 *
 * SYNOPSIS
 *	quit (ecode, mess, Arg1, ... )
 *	    unsigned short ecode;	error code
 *	    char *mess;			control string for printf
 *	    ??? Arg1, ...;		arguments for printf.
 *
 * RESULTS
 *	Never returns.
 *
 * SIDE EFFECTS
 *	The program is terminated and the terminal receives the message.
 *
 * ENVIRONMENT
 *
 *    Imports
 *	_Printf ()		fprintf.c
 *	exit ()			<startup file>
 *	char printfbuf []	fprintf.c
 *
 *    Exports
 *	quit ()			quit.c
 *
 *----------------------------------------------------------------------------
 */

#include <stdio.h>

extern char printfbuf [];

quit (ecode, ctl, arg1)
    short ecode;
    char *ctl;
{
    _Printf (printfbuf, ctl, &arg1);
    outs (printfbuf, termout);
    force_flush (termout);
    exit (ecode);
}
