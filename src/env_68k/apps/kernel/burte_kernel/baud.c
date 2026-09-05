/*--------------------------------------------------------------------------
 *
 * ABSTRACT
 *	_SET_RATE()- Routine to set the host's baud rate.
 *
 * SYNOPSIS
 *	set_baud (port, rate)
 *	short port
 *	port 0 = terminal
 *	port 1 = host
 *
 *	    short rate;		baud rate: 50, 75, 110, 134, 150, 300,
 *				600, 1200, 1800, 2000, 2400, 3600, 4800,
 *     				7200, 9600, 19200.
 *
 * RESULTS
 *	Returns true if sucessful.
 *
 * SIDE EFFECT
 *	The port's baud rate is set to the approtriate rate. If the rate
 *	is unknown, the baud rate is not changed.
 *
 * ENVIRONMENT
 *
 *    Imports
 *	None.
 *
 *    Exports
 *	set_rate ()		baud.c
 *
 * DESIGN
 *	Brute force.  Note: the baud is garbage until explicitely set.
 *
 * HISTORY
 *	1-2-87	Dan Christian modifed from PopI kernel sources.
 *
 *	3/21/92		Andrew H. Fagg		Set up for 68332, only port 0
 *
 *----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <Oserial.h>

#define NUM_BAUD 9

r_set_baud (port, rate)
  int port, rate;
{
    
    const static short Rate [] = {
	/* 50, 75, 110, 134, */
	110, 300, 600, 1200,
        /* 1800, 2000, */
        2400,/* 3600,*/ 4800, 
	/* 7200, */
        9600, 19200
      };
    const static short Convert[] = {
      4766, 1748, 874, 437, 218, 109, 55, 27};

    char i;
    static unsigned char code;
    
    if ((port != 0) /* && (port != 1) */) 
      return (FALSE); /* bad port select */
    
    for (i = 0; i < NUM_BAUD; i++)
      if (Rate [i] == (short)rate) break;
    
    if (i == NUM_BAUD) return (FALSE); /* not found! */
    
    /* User port 0 (terminal) uses upper 4 bits. */
    
    if (port) {			/* host */
      /*
	code &= 0xf0;
	code |= i;
      */
    } else {			/* terminal */
      *SER_SCCR0 = Convert[i];
    }
    
    /* set the rate */
    /* *SLBAUD = code; */    
    return (TRUE);
}
