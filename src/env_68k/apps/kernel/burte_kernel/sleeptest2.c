/*
 *  sleeptest.c
 *
 *  Andrew H. Fagg     11/12/92    Original
 *
 *  Test the time-based scheduling capability.
 *  Creates two processes : one that wakes up every 5 seconds,
 * increments its time value, reports it to the user,
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
 * go back to sleep again.  Because set_schedule_interval() and
 * next_interval() are used, this process is assured that it will
 * be scheduled on the specified intervals, with no drift due to 
 * various delays. 
 */

clockproc()
{
  int time;

  time = 0;
  set_schedule_interval(5000);	/* Specify how often the process */
				/*   is to wake up in msecs. */
  while(1)
    {
      puts("The time is ");
      puti(time);
      puts("\n");
      next_interval();		/* Wait for next time interval */
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

  puts("Clock is");              /* Create and start the clockproc() */
  clock = create(clockproc, 9, 0); /* priority = 9, stack size = default */
  puti(clock);
  puts("\n");
  start(clock);
                                 /* Create and start the interactive process */
  other = create(interactive, 2, 0);   /* priority = 2, stack size = default */
  qprintf("Other is %d\n\r", other);
  start(other);
				/* Wait for interactive() to exit. */
  wait(other);
  kill(clock);			/* Halt the clockproc() process. */
}


