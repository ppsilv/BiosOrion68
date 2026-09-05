/*
 * Return the ptr in sp at which the character c appears;
 * NULL if not found
 */

#include <stdio.h>

char *index(sp, c)
  char *sp, c;
{
    do {
	if (*sp == c)
	  return(sp);
    } while (*sp++);
    return(NULL);
}
