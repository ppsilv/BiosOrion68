/*
 *  timetest.c
 *
 *  Andrew H. Fagg     11/12/92    Original
 *
 *  Test the wallclock.
 *
 */

#include <stdio.h>
#include <burte.h>

/*
 * main()
 *
 *   Loop 10 times, each time wait for 1 second to pass on the
 * wallclock.
 *   wallclock is a predefined integer that is updated every
 * msec, with its units being msecs.
 *
 */

main()
{
  unsigned long i;
  int j;
				/* Query wallclock a couple times. */
  i = wallclock;
  qprintf("wall = %d\n\r", i);
  i = wallclock;
  qprintf("wall = %d\n\r", i);

				/* Loop 10 times */
  for(j = 0; j < 10; ++j)
    {
                                /* Wait for 1 sec.  */
      while(i + 1000 > wallclock);

				/* Report time. */
      i = wallclock;
      qprintf("wall = %d\n\r", i);
    }
  qprintf("done\n\r");
}
