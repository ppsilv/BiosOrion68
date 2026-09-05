#include <stdio.h>
#include <term.h>

				/* strings for clear screen, clear line */
char *cl_str = "#\r\n", *cs_str = "#\r\n\n\n\n\n\n"; /* defaults */
 
redisplay (fl, prompt, command_line)
FILE *fl;
char *prompt, *command_line;
{
    if (fl == NULL) return;
    clear_line (fl);
    fputs (prompt, fl); fputs (command_line, fl);
}

/* limited terminal support.  Since, we only need two control strings, we */
/* don't bother with the fat, ugly mess of termcap. */
set_term (ttype)
  char *ttype;
{
    cl_str = "#\r\n";		/* default */
    cs_str = "#\r\n\n\n\n\n\n";

    if (!ttype) return;

    if (match (ttype, "vt100")){
	cs_str = "\033[;H\033[2J"; /* clear screen */
	cl_str = "\r\033[K"; /* clear line */
	return;
    }

    if (match (ttype, "h19")) {
	cs_str = "\033E";
	cl_str = "\r\033K";
	return;
    }
}
