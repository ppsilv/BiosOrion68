/* output a string to a string, and return a pointer to the
   end of the string. */

#include <stdio.h>

char *sputs (str, s)
  char *str, *s;
{
    while (*s++ = *str++);
    return (--s);
}
