/* quick routine to print a integer to a file */

#include <stdio.h>

fputi(n, fb)			/* output an integer to a file */
  int n;
  FILE *fb;
{
    char string[12];
    char *i;
    short int  sign=0;
    
    if (n < 0) {	/* record sign */
	n = -n;
	sign = 1;
    }
    
    i = string;
    /* Generate digits in reverse order. */
    do
      { *(i++) = (n % 10) + '0'; }
    while((n /= 10) > 0);
    
    if (sign)
      *(i++) = '-';
    
    pend (&fb->busy, TRUE, P_SET);
    while (--i >= string)	/* print back to front */
      putc (*i, fb);
    post (&fb->busy, FALSE);
    return ;
}
