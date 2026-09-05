/* quick routine to print an interger in hex format to a file */

#include <stdio.h>

fputx(n, fb)			/* output a long hexidecimal number */
  unsigned int n;
  FILE *fb;
{
    char *m;
    short in=0, i, j, k;

    m = (char *)&n;

    /* Generate digits in order. */
    pend (&fb->busy, TRUE, P_SET);
    for (j=0, i=0; j < 8; j++) {
	if (j&1) {
	    k = *(m+i) & 0xf;
	    i++;
	}
	else {
	    k = (*(m+i) >> 4) & 0xf;
	}
	if ((k) || in) {
	    putc ((k < 10) ? (k + '0') : (k - 10 + 'A'), fb);
	    in++;
	}
    }
    if (in == 0) putc ('0', fb);
    post (&fb->busy, FALSE);
    return ;
}
