/* quick routine to print a integer to a string */

#include <stdio.h>

/* output an integer to a string */
/* return a pointer to the last (null) character. */
char *sputi(n, s)
  int n;
  char *s;
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
    
    while (--i >= string)	/* print back to front */
      *s++ = *i;
    *s = 0;
    return (s);
}
