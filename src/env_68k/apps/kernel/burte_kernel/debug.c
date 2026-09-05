/* routines to aid debugging */
#include <burte.h>
#include <burteio.h>
#include "system.h"
#include <Oserial.h>

extern PR_STATE *proc_state;
int csum, tcnt;			/* for tracing */
char *csum_start, *csum_end;
long old_trace_hndl;

trap_mess (super, user, status, addr, str)
  int super, user, status, addr;
  char *str;
{
    unsigned short i;

    if (! bad_id(current_proc)) r_suspend (current_proc);
    force_print (str);
    force_print (" at: "); force_hex (addr);
    force_print (" process: "); force_hex (current_proc);
    force_print ("\r\nSsp: "); force_hex (super);
    force_print ("  Usp: "); force_hex (user);
    force_print ("  status: "); force_hexf (status, 2);
    force_print ("\r\n");
    if (status & 0x2000) force_mem (super, 0x30); /* in super, print stack */
    force_mem (user, 0x30);
    for (i=0; i< 30000; i++);
}

addr_mess (super, user, status, iadd, addr, str) /* buss and address errors */
  long super, user, status, iadd, addr;
  char *str;
{
    unsigned short i;

    if (! bad_id(current_proc)) r_suspend (current_proc);
    force_print (str);
    force_print (" at: "); force_hex (iadd);
    force_print (" accessing: "); force_hex (addr);
    force_print (" process: "); force_hex (current_proc);
    force_print ("\r\nSsp: "); force_hex (super);
    force_print ("  Usp: "); force_hex (user);
    force_print (" status: "); force_hexf (status, 2);
    force_print ("\r\n");
    force_mem (super, 0x40);
    if (status & 0x2000) force_mem (user, 0x40);
    for (i=0; i< 30000; i++);
}

/*----------------trace service routine */
/* note: tracing is almost always done from user mode */
trace_mess (ustk, addr)	/* for now, checksum memory */
  long addr, ustk;
{
    char *smem;
    int  accum=0;

    if (tcnt > 0) {		/* step trace */
	if (! --tcnt) {
	    if (! bad_id(current_proc)) r_suspend (current_proc);
	    r_lock (0x2400);	/* enable serial */
	    outst ("Trace at: ");	outxt (addr);
	    outst (" process: ");	outxt (current_proc);
	    outst (" usp: ");		outxt (ustk);
	    outst ("\r\n");
	    flusht();
	    return (-1);	/* return to shell */
	}
    }
    if (csum_end) {
	for (smem = csum_start; smem < csum_end; smem++) {
	    accum += *smem;
	}
	if (csum != accum) {
	    if (! bad_id(current_proc)) r_suspend (current_proc);
	    r_lock (0x2400);	/* enable serial */
	    outst ("Csum at: ");	outxt (addr);
	    outst (" process: ");	outxt (current_proc);
	    outst (" usp: ");		outxt (ustk);
	    outst (" found: ");		outxt (accum);
	    outst (" not: ");		outxt (csum);
	    outst ("\r\n");
	    flusht();
	    csum = accum;	/* reset check sum */
	    return (-2);	/* return to shell */
	}
    }
    return 0;
}

step_break () {
    *SYS_VECT (9) = old_trace_hndl; /* set old trace handler back */
    putbks ();			/* install all breakpoints */
    if (csum_end) {		/* trace */
	return (1);		/* tell assembly routine to call trace_mess */
    } else {
	proc_state[current_proc].s_reg &= ~0x8000;
	return (0);
    }
}

/*
 *  This routine is hacked to fool the compiler into sampling TERMCTL2
 * at every time step.
 *
 */

force_char (c)
  unsigned char c;
{
    int i;
    short tmp;
    short a;

    a = 0;


    for (i=0; ((i< 200000) && (!((tmp = *TERMCTL2) & SER_TDRE))); i++);

/*
    for (i=1; ((i < 200000) && (!((tmp = *TERMCTL2) & SER_TDRE))); i++)
	  tmp = 0;
    
      if(!(i % 200000))
	{
	  ph((unsigned int)  tmp);
	  tmp = *TERMCTL; 
	  ps("\n\r");
	  ph((unsigned int)  tmp);
	  
	  ps("\n\r");
	  ph(SER_TDRE);
	  ps("\n\r");
	  ps("timeout\n\r");
	  if(tmp > 0xff)
	    ps("larger\n\r");
	  tmp = *TERMCTL2;
	  ph(tmp);
	  ps(" - end\n\r");
	}
*/

    *TERMDAT = (unsigned short) c; 
/*    *(((char *) TERMDAT)+1) = c;  */
}

force_print (str)		/* polled io string output to terminal */
  char *str;
{
    char c;
    short i;

    for (i=0; ((i< 5000) && (! (*TERMCTL2 & SER_TDRE))); i++);
    while (c = *str++) {
	force_char (c);
    }
    for (i=0; ((i< 2000) && (! (*TERMCTL2 & SER_TDRE))); i++);
}

/* output a hexidecimal number of a unknown number of bytes */
force_hex(n)
  unsigned int n;
{
    char *m;
    short j, k, nz=0;
    
    m = (char *)&n;

    /* Generate digits in order. */
    for (j=0; j < 4; j++)
      if ((*(m+j)) || nz) {
	  nz++;
	  k = (*(m+j) >> 4) & 0xf;
	  force_char ((k < 10) ? (k + '0') : (k - 10 + 'A'));
	  k = *(m+j) & 0xf;
	  force_char ((k < 10) ? (k + '0') : (k - 10 + 'A'));
      } 
    if (nz == 0) {
	force_char ('0');
    }
}

/* output a hexidecimal number of a given number of bytes (zero filled) */
force_hexf(n, b)
  unsigned int n;
  int  b;
{
    char *m;
    short j, k;
    
    m = (char *)&n;

    /* Generate digits in order. */
    for (j=4-b; j < 4; j++) {
	k = (*(m+j) >> 4) & 0xf;
	force_char ((k < 10) ? (k + '0') : (k - 10 + 'A'));
	k = *(m+j) & 0xf;
	force_char ((k < 10) ? (k + '0') : (k - 10 + 'A'));
    }
}

force_mem (start, cnt)	/* print memory, start, count */
  int start, cnt;
{
    char *mem;
    short i,j, *smem;
    int end;
    
    start &= ~1;		/* make even */
    end = (start+cnt+15) & ~15;

    if (! (r_is_mem (start)) || ! (r_is_mem (end))) {
	force_print ("Non-existant memory between: ");
	force_hex (start); force_print (" - ");
	force_hex (end);   force_print ("\r\n");
	return;
    }

    for (mem= (char *)start, smem = (short *)start, i=0; i < cnt; i += 16) {
	force_hexf (mem, 3); force_print ("  ");
	for (j=0; j< 8; j++, smem++) {
	    force_hexf (*smem, 2); force_print (" ");
	}
	force_print ("   ");
	for (j=0; j< 16; j++, mem++) {
	    if ((*mem >= ' ') && (*mem <= '~'))
	      force_char (*mem);
	    else
	      force_char ('.');
	}
	force_print ("\r\n");
    }
}

/* print out my stack */
fprint_stack (fake)		/* you needn't pass anything to this */
  int fake;
{
    force_mem ((long) &fake + 4, 31);
}

force_flush (fb)		/* for supervisor use only */
  FILE *fb;
{
    while ((volatile)fb->size);
}

#if 0
char poll_char ()
{
    while (!(*TERMCTL2 & RDRF));
    return (*TERMDAT);
}

poll_str (buff, n)		/* returns size of string */
  char *buff;			/* buffer to put string in */
  int n;			/* size of buffer including null  */
{
    char *add_in, in_char;
    int i;
    
    add_in = buff;
    while (1) {
	switch (in_char = poll_char()) {
	    
	  case '\177':		/* back space and delete */
	  case '\010':
	    if (add_in > buff) {
		add_in--;
		*add_in = 0;
		force_print ("\010 \010"); /* BS-space-BS */
	    }
	    break;
	    
	  case '\022':		/* ^R redisplay */
	    force_print ("\r\n");
	    force_print (buff);
	    break;

	  case '\025':		/* ^U erase line */
	    buff[0] = 0;
	    force_print ("#\r\n");
	    add_in = buff;
	    break;

	  case '\r':		/* execute command */
	  case '\n':
	    force_print ("\r\n");
	    *add_in = 0;
	    return ((int)(add_in-buff));
	    
	  default:	/* insert the character */
	    if (add_in < (buff + n -1)) {
		*add_in++ = in_char;
		*add_in = 0;
		force_char (in_char); /* echo character */
	    }
	}
    }
}
#endif

