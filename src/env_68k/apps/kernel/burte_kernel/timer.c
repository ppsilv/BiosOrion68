
/*
 *  History
 *
 *  Author             Date         Description
 *  --------------     --------     -----------------------------------------
 *  Andrew H. Fagg     03/03/93     Removed tm_read(), tm_set(), tm_scale()
 *  Andrew H. Fagg     02/27/93     Updated wallclock to run at 122 usec rate
 *  Andrew H. Fagg     03/01/95     Initialize wall_clock_count inside 
 *                                   tm_init() so we can put into ROM.
 *
 *  Timer usages:
 *  	wall_clock, updated every milisecond.
 *
 */

#include <burte.h>
#include <service.h>
#include <Otimer.h>
#include <68332.h>
#include <68332new.h>

int wall_clock_count;

reentrant (time_service)	/* interupt handler */
{
  wallclock++;
  wall_clock_count = (1 + wall_clock_count) % 8;
  if(!wall_clock_count)
      wall_clock++;
}

/* Sets initial values on all the timers */
tm_init()
{
/*
  t2_proc = 0;
    t3_proc = 0;
*/
  wallclock = 0;
  wall_clock = 0;
  wall_clock_count = 0;
    *PITR = 0x01;		/* Update every 122 usecs. */
    *INT_VECT (42) = (long) time_service;  /* timer */
    *PICR = 0x242;

}

/* Reads Timer and returns 16 bit time elapsed (in usecs) */
/*
unsigned int tm_read (timer)
    int timer;
{
    unsigned short time_elapsed;
    unsigned char *tpointer = (unsigned char *) &time_elapsed;

    switch (timer) {
      case 1:
	*tpointer++ = *TM_CT1;
	break;
      case 2:
	*tpointer++ = *TM_CT2;
	break;
      case 3:
	*tpointer++ = *TM_CT3;
	break;
      default:
	return -1;		/* error */
/*
    }
    *tpointer = *TM_LSB;	/* read the elapsed time */
/*

    return ((unsigned int)time_elapsed); /* the timer counts down */
/*
}
*/


/* set at timer (1,2, or 3).
a count, the timer, and the proper control register is passed in.
*/
/*
tm_set (usecs, timer, mode)
    unsigned short usecs;
    int timer;
    unsigned char mode;		/* mode to pass in */
/*
{
    static unsigned char t2mode;

    if (timer==1) {
	t2mode |= 0x1;		/* set t1 select bit */
/*
    } else if (timer == 3) {
	t2mode &= ~0x1;		/* clear t1 select bit */
/*
    }
    
    if (timer==1) {
	*TM_CR2 = t2mode;
	*TM_CR1 = mode;
    } else if (timer == 2) {
	*TM_CR2 = mode;
	t2mode = mode;
    } else if (timer ==3) {
	*TM_CR2 = t2mode;
	*TM_CR3 = mode;
    }

    *TM_MSB = (usecs >> 8) & 0xff;
    switch (timer) {		/* write low word (which causes the load) */
/*
      case 1:
	*TM_LA1 = usecs & 0xff;
	break;
      case 2:
	*TM_LA2 = usecs & 0xff;
	break;
      case 3:
	*TM_LA3 = usecs & 0xff;
	break;

      default:
	return -1;
    }
    return 0;
}
*/

/*
 * STATIC ROUTINE.  SCALE ().
 *
 *	scale (usecs, pcount) - compute the scale given a number of
 *	microseconds. It returns the number to be downloaded in the
 *	scale timer, zero if too many microseconds.
 *
 * DESIGN
 *	Given a number of microseconds, compute the necessary frequency
 *	division (a multiple of two) so that the period is smaller than
 *	64K. The frequency division is returned as ``scale'' and the new
 *	period is updated by the pointer.
 */

/*
tm_scale (usecs, pcount)
    long usecs;
    short *pcount;		/* RETURNED */
/*
{
    int scale_factor;

/* If usecs is less than 0xffff then we can represent the number exactly */
/* otherwise we have to approximate it */
/*

    if (usecs <= 0xffff)
    {
	*pcount = 0;
	return(usecs);
    }

/* It's bigger than 0xffff so we have to approximate */
/*
    for (scale_factor = 1; usecs > 0xFFFF; scale_factor <<= 1)
	usecs >>= 1;

    *pcount = usecs;
    return (scale_factor >> 1);	/* take toggle into account */
/*
}
*/
