/* quick routine to print an integer in hex format into a string */

#include <stdio.h>

char *sputx(n, s)		/* output a long hexidecimal number */
  unsigned int n;		/* return a pointer to the tail */
  char *s;
{
    char *m;
    short in=0, i, j, k;

    m = (char *)&n;

    /* Generate digits in order. */
    for (j=0, i=0; j < 8; j++) {
	if (j&1) {
	    k = *(m+i) & 0xf;
	    i++;
	}
	else {
	    k = (*(m+i) >> 4) & 0xf;
	}
	if ((k) || in) {
	    *s++ = (k < 10) ? (k + '0') : (k - 10 + 'A');
	    in++;
	}
    }
    if (in == 0) *s++ = '0';
    *s = 0;
    return (s);
}
