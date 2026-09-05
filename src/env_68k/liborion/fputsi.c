/* quick routine to print a d characters of an integer to a file */
/* if the number is bigger that d characters, a * is printed in the
most significant position. */

#include <stdio.h>

fputsi(n, d, fb)		/* output an integer to a file */
  int n, d;
  FILE *fb;
{
    char string[12];
    char *i;
    short int  sign=0, j=0;

    if (d <= 0) return;
    
    if (n < 0) {	/* record sign */
	n = -n;
	sign = 1;
    }
    
    i = string;
    /* Generate digits in reverse order. */
    do {
	*(i++) = (n % 10) + '0';
	j++;
    }
    while((n /= 10) > 0);
    
    if (sign) {
	*(i++) = '-';
	j++;
    }
    
    if (j >= d) {		/* or j > d-1 */
	i = string + d - 1;
	*i++ = '*';
    } else while (j < d) {
	*i++ = ' ';
	j++;
    }

    pend (&fb->busy, TRUE, P_SET);
    while (--i >= string)	/* print back to front */
      putc (*i, fb);
    post (&fb->busy, FALSE);
    return ;
}
