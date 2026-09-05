/* print carrage return-newline */

#include <stdio.h>
fputcn (fb)
  FILE *fb;
{
    putc ('\r', fb);
    putc ('\n', fb);
}
