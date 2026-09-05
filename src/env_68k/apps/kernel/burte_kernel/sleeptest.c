/*
 *  sleeptest.c
 *
 *  Andrew H. Fagg     11/12/92    Original
 *
 *  Test the sleep function.
 *  Creates two processes : one that sleeps for 5 seconds,
 * wakes up, increments its time value, reports it to the user,
 * and then goes back to sleep again.
 *  The other process waits for input from stdin.  When a
 * 'q' is entered, it exits, causing the clockproc() to
 * exit as well.
 *
 */

#include <stdio.h>

#define SLEEP 5


/*
 * clockproc()
 *
 *  Infinite loop of wake up, increment time, report time, and then
 * go back to sleep again.
 *
 */

clockproc()
{
  int time;

  time = 0;
  while(1)
    {
      puts("The time is ");
      puti(time);
      puts("\n");
      sleep(5);
      time += SLEEP;
    }
}


/*
 * interactive()
 *
 *   Waits for input from the user (stdin).  When a 
 * 'q' is entered, the process exits.
 *
 */

interactive()
{
  char buf[100];

  while(1)
    {
      qprintf("val = ");
      gets(buf, 100);
      qprintf("entered %s\n\r", buf);
      if(buf[0] == 'q')
	return;
    }
  
}

/*
 * main()
 *
 *  Start the clockproc() and interactive() processes.  When
 * the interactive process exits, then kill the clockproc.
 *
 */

main()
{
  int clock;
  int other;
				/* Create and start the clockproc() */
  puts("Clock is");
  clock = create(clockproc, 9, 0); /* priority = 9, stack size = default */
  puti(clock);
  puts("\n");
  start(clock);

				/* Create and start the interactive process */
  other = create(interactive, 2, 0); /* priority = 2, stack size = default */
  qprintf("Other is %d\n\r", other);
  start(other);

				/* Wait for interactive() to exit. */
  wait(other);
  kill(clock);			/* Halt the clockproc() process. */
}


