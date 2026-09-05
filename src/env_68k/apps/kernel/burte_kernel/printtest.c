/*
 *
 */

#include <stdio.h>


main()
{
  char c;

  while((c = 0xff & getchar()) != 13 && c != 10)
    qprintf("c : %d\n", c);
  qprintf("done\n");
}


