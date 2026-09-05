/* quick routine to print a d characters of an integer to a file */
/* if the number is bigger that d characters, a * is printed in the
most significant position.  Otherwise, spaces are insterted to make
the string d characters long.
A pointer to the last (null) position in the string is returned. */

#include <stdio.h>

char *sputsi(n, d, s)		/* output an integer into a string */
  int n, d;
  char *s;
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

    while (--i >= string)	/* print back to front */
      *s++ = *i;

    *s = 0;
    return (s);
}
