/* quick routine to print an integer in hex format to a string */

#include <stdio.h>

/* output a hexidecimal number of d digits (zero filled) */
/* return a pointer to the last (null) character. */
char *sputsx(n, d, s)		/* buffer s must be at least d+1 long */
  unsigned int n;
  int  d;			/* characters to print */
  char *s;
{
    char *m;
    short i, j, k;
    
    if (d <= 0) return (s);

    m = (char *)&n;

    /* Generate digits in order. */
    for (j=d, i=4-((d+1)>>1); j > 0; j--) {
	if (j&1) {
	    k = *(m+i) & 0xf;
	    i++;
	}
	else {
	    k = (*(m+i) >> 4) & 0xf;
	}
	*s++ = (k < 10) ? (k + '0') : (k - 10 + 'A');
    }
    *s = 0;
    return (s);
}
