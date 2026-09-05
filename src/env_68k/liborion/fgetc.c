/* function version of getc.  For unix compatibility */

#include <stdio.h>

fgetc (fb)
  FILE *fb;
{
    return getc (fb);
}
