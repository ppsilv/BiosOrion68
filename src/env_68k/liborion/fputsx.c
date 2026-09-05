/* quick routine to print an interger in hex format to a file */

#include <stdio.h>

/* output a hexidecimal number of d digits (zero filled) */
fputsx(n, d, fb)
  unsigned int n;
  int  d;			/* characters to print */
  FILE *fb;
{
    char *m;
    short i, j, k;
    
    if (d <= 0) return;

    m = (char *)&n;

    /* Generate digits in order. */
    pend (&fb->busy, TRUE, P_SET);
    for (j=d, i=4-((d+1)>>1); j > 0; j--) {
	if (j&1) {
	    k = *(m+i) & 0xf;
	    i++;
	}
	else {
	    k = (*(m+i) >> 4) & 0xf;
	}
	putc ((k < 10) ? (k + '0') : (k - 10 + 'A'), fb);
    }
    post (&fb->busy, FALSE);
}
