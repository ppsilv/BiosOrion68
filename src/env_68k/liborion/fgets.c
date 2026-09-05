/* WARNING:  This is different that unix gets */
/* read in a string with basic editing and echoing terminated by CR or NL */
/* returns the number of characters in the string (excluding NULL)*/
/* The CR or NL is not added to the string.*/

#include <stdio.h>

fgets (buff, n, fin, fout)
  char *buff;			/* buffer to put string in */
  int n;			/* size of buffer including null  */
  FILE *fin;			/* file to get input from */
  FILE *fout;			/* file to echo to (NULL if no echo) */
{
    char *add_in, in_char;
    int i;
    
    add_in = buff;
    while (1) {
	switch (in_char = getc (fin) & 0x7f) { /* mask high bit */
	    
	  case '\177':		/* back space and delete */
	  case '\010':
	    if (add_in > buff) {
		add_in--;
		*add_in = 0;
		if (fout) fputs ("\010 \010", fout); /* BS-space-BS */
	    }
	    break;
	    
	  case '\027':		/* ^w erase word */
	    while ((--add_in >= buff) && (*add_in == ' '))
	      if (fout) fputs ("\010", fout);	/* BS */
	    add_in++;
	    while ((--add_in >= buff) && (*add_in != ' '))
	      if (fout) fputs ("\010 \010", fout); /* BS-space-BS */
	    *(++add_in) = 0;
	    break;
	    
	  case '\025':		/* ^U erase line */
	    buff[0] = 0;
	    if (fout)
	      while (--add_in >= buff)
		fputs ("\010 \010", fout); /* BS-space-BS */

	    add_in = buff;
	    break;

	  case '\022':		/* ^R redisplay */
	    if (fout) {
		fputs ("\r\n", fout);
		fputs (buff, fout);
		break;
	    }

	  case EOF:
	    if (add_in < (buff + n -1)) {
		*add_in++ = EOF;
		*add_in = 0;
	    }
	    return ((int)(add_in-buff-1));
	    
	  case '\r':		/* execute command */
	  case '\n':
	    if (fout) fputs ("\r\n", fout);
	    *add_in = 0;
	    return ((int)(add_in-buff));
	    
	  default:	/* insert the character */
	    if (add_in < (buff + n -1)) {
		*add_in++ = in_char;
		*add_in = 0;
		if (fout) putc (in_char, fout); /* echo character */
	    }
	}
    }
}
