/* output a string to a file */
/* NOTE: unlike Unix, neither puts(s) or fputs(s,fb) append a
 CR-NL to the string. */

#include <stdio.h>

fputs (str, fb)
  FILE *fb;
  char *str;
{
    char c;
    short i;

    pend (&fb->busy, TRUE, P_SET);
    while (c = *str++) {
	if (c == '\n') {
	    putc ('\r', fb);
	    putc ('\n', fb);
	} else
	  putc (c, fb);
    }
    post (&fb->busy, FALSE);
}
