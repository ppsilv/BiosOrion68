/* function version of putc.  For unix compatibility */

#include <stdio.h>

fputc (ch, fb)
  char ch;
  FILE *fb;
{
    putc (ch, fb);
}
