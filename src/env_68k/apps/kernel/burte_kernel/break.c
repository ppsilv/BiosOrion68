/*--------------------------------------------------------------------------
 *
 * ABSTRACT
 *	SETBK (addr)- Mark a place to have a breakpoint.
 *	CLEARBK (addr)- Unmark (delete) a breakpoint from a place.
 *	CLEARALL ()- Delete all breakpoints.
 *	PRBKS ()- Print all active breakpoints.
 *	SEARCHBK (addr)- Look for an active breakpoint.
 *	PUTBKS ()- Install the active breakpoints.
 *	HIDEBKS ()- Hide all active breakpoints.
 *
 * RESULTS
 *	setbk () and clearbk () return TRUE iff succesful.
 *	searchbk () returns the index of the beakpoint or -1 if the
 *	breakpoint does not exist.
 *
 *	putbks(), hidebks(), clearall(), and prbk() don't return anything.
 *
 * SIDE EFFECTS
 *	The places are 'marked' via setbk (). They are unmarked
 *	with a called of clearbk (). 
 *	Breakpoints are installed (via an illegal instruction) by a
 *	call to putbks (). They are restored to their previous values
 *	by a call to hidebks ().
 *	Note that clearbk () has no side effects and thus may
 *	be called when in doubt to delete a possible breakpoint.
 *	This feature is used by the 'c'ontinue command in krn68 ().
 *
 *	The breakpoints call breaksrv() (in a_exec.s).  This is installed
 *	at startup.
 *
 *    Imports
 *	fputs ()
 *	fputx ()	libburte.a
 *	r_is_mem ()	a_exec.s
 *
 *    Exports
 *	initbk ()
 *	setbk ()
 *	clearbk ()
 *	clearall ()
 *	putbks ()
 *	hidebks ()
 *	searchbk ()
 *	prbk ()			break.c
 *
 * BUGS
 *
 *	Sixteen breakpoints are supported.
 *
 * DESIGN
 *	Brute force. Just keep a couple of arrays with the addresses
 *	of the breakpoints and the corresponding instructions. An address
 *	of zero is illegal (no breakpoint).
 *
 * HISTORY
 *	1-Jan-88	Dan Christian modified from PopI for BURTE.
 *
 *----------------------------------------------------------------------------
 */

#include <burte.h>
#include <burteio.h>

#define FALSE 0
#define TRUE 1

#define ILLEGAL		0x4AFC	/* 68000's illegal instruction */

int setbk(), searchbk(), clearbk(), prbks();

extern int csum, tcnt;
extern char *csum_start, *csum_end;

init_break ()
{
    clearall();
    csum = tcnt = 0;
    csum_start = csum_end = 0;
}

static short  Instrs [16];
static long Bkpoints [16];

setbk (addr)
    long addr;
{
    short i;

    addr &= ~1;			/* round down to an even address */
    if (addr < 0x100) {
	fputs ("You cannot break in trap vectors.\n", termout);
	return FALSE;
    }
    if (! (r_is_mem (addr))) {
	fputs ("No memory at: ", termout);
	fputx (addr,termout); fputs ("\n", termout);
	return FALSE;
    }
    if (searchbk (addr) >= 0) {
	fputs ("Trap exists: ", termout);
	fputx (addr,termout); fputs ("\n", termout);
	return FALSE;
    }

    for (i=0; i < 16; i++) {
	if (Bkpoints [i] == 0) {
	    Bkpoints [i] = addr;
	    Instrs [i] = *(short *) addr;
	    return TRUE;
	}
    }
    fputs ("No space for trap\n");
    return FALSE;		/* no more space for breakpoints */
}

searchbk (addr)
    long addr;
{
    short i;

    for (i=0; i < 16; i++)
      if (Bkpoints [i] == addr) break;

    if (i == 16) return (-1);
    return (i);
}

clearbk (addr)
    long addr;
{
    short i;

    if (addr < 0x100) return FALSE;
    i = searchbk (addr);
    if ( i < 0) return FALSE;
    Bkpoints [i] = 0;
    return TRUE;
}

clearall ()
{
    short i;

    for (i=0; i< 16; i++)
      Bkpoints [i] = 0;
}

prbks ()
{
    short i;
    
    fputs ("\nActive breakpoints:\n", termout);
    for (i = 0; i < 16; i++) {
	if (Bkpoints [i])
	  {
	      fputs ("Address  ", termout);
	      fputx (Bkpoints [i], termout);
	      fputs ("\n", termout);
	  }
    }
}

putbks ()
{
    short i;

    for (i = 0; i < 16; i++) {
	if (Bkpoints [i] != 0) {
	    *(short *) (Bkpoints [i]) = ILLEGAL;
	}
    }
}

hidebks ()
{
    short i;

    for (i = 0; i < 16; i++) {
	if (Bkpoints [i] != 0) {
	    *(short *) Bkpoints [i] = Instrs [i];
	}
    }
}

hide_one (addr)
  int addr;
{
    short i;
    
    if ((i = searchbk (addr)) >= 0) {
	*(short *) Bkpoints [i] = Instrs [i]; /* clear the next brkpnt */
	return (1);
    }
    return (0);
}
