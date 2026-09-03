#include <stdlib.h>
#include <stddef.h>

extern char * __utoa(unsigned value, char *str,  int base);

char * __itoa (int value, char *str, int base){
  unsigned uvalue;
  int i = 0;

  /* Check base is supported. */
  if ((base < 2) || (base > 36))
    {
      str[0] = '\0';
      return NULL;
    }

  /* Negative numbers are only supported for decimal.
   * Cast to unsigned to avoid overflow for maximum negative value.  */
  if ((base == 10) && (value < 0))
    {
      str[i++] = '-';
      uvalue = (unsigned)-value;
    }
  else
    uvalue = (unsigned)value;

  __utoa (uvalue, str, base);
  return str;
}

char * itoa (int value, char *str, int base){
  return __itoa (value, str, base);
}
