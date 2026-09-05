/* io system calls for BURTE */
/* Created 12-Nov-87 by Dan Christian */
/*
 *  3/21/92		Andrew H. Fagg		Modified for 68332
 *
 *  MRESET -> SER_RESET
 *  RENA -> SER_REI
 *  TENA -> SER_TIE (should also be TCIE?)
 *  RDRF -> SER_RDRF
 *  TDRE -> SER_TDRE
 *  FRERR -> SER_FE
 *  
 *  4/24/92            Andrew H. Fagg           Changed def of SERIAL_STATE
 *                                               to handle 332 serial
 *                                               (char -> short)
 */


#ifndef IO_BUFFER_SIZE
#define IO_BUFFER_SIZE 512
#endif

#include <burte.h>
#include <burteio.h>
#include <service.h>
#include <Oserial.h>

#define LOLIM 63		/* send ^q when this much room is available */
#define HILIM 4			/* send ^s when less than this room left */

int file_init(), r_serial_service();
int r_serial();

typedef struct {
    volatile unsigned short *cport;  /* char->short by Andrew Fagg for 332 */
    volatile short *dport;
    unsigned short  creg;	/* copy of the control register */
    char  must_send;		/* high prority characters */
    char  in_xoff;		/* have send ^s */
    char  in_brk;		/* just saw a break */
} SERIAL_STATE;

static FILE _realflio[4];	/* actual term in/out, host in/out */
static SERIAL_STATE host_state, term_state;
static unsigned char _iobuff[4][IO_BUFFER_SIZE];

set_iflags (fl, ss)
  FILE *fl;
  SERIAL_STATE *ss;
{
    if (fl->size <= 0) fl->status |= Q_EMPTY;
    if (fl->size >  0) fl->status &= ~Q_EMPTY;
    if (fl->size >= fl->qsize) fl->status |= Q_FULL;
    if (fl->size <  fl->qsize) fl->status &= ~Q_FULL;

    if (fl->status & Q_FULL) {
	ss->creg &= ~SER_REI;
    } else {
	ss->creg |= SER_REI;
    }
      if (fl->status & Q_EMPTY) {
	ss->creg &= ~SER_TIE;
    } else {
	ss->creg |= SER_TIE;
    }
  *ss->cport = ss->creg;
}

/* system initialization of serial driver and files */
serial_init ()
{
    short i;
    FILE *stfp;

    io_files = &termin;		/* io_files overwrite termin/out etc */
				/* see sys_var.s for details */
				/* set baud rates to default */
/*
    if (! (r_set_baud (0, 9600) && r_set_baud (1, 9600))) {
	*SLBAUD = (char)0xee;
	force_print ("Error setting baud. Hard set to 9600.\r\n");
    }
*/
    for (i=0, stfp = &_realflio[0]; i < 4; i++, stfp++) {
	file_init (stfp, IO_BUFFER_SIZE, _iobuff[i]);
	stfp->service = r_serial_service;
	stfp->status = XONO | CBREAK;
	stfp->busy = 0;
	io_files[i] = stfp;
    }
    return (r_serial_service (NULL, IO_BEGIN, 0) < 0);
}


r_serial_service (fb, action, arg)
  FILE *fb;
  int action;
  int arg;
{
    SERIAL_STATE *serstate= NULL;
    int value=0;
    unsigned short temp;

    serstate = &term_state;
/*
    if ((fb == io_files[0]) || (fb == io_files[1])) {
	serstate = &term_state;
    }
    else
      {
	force_print("r_serial_service : file error\n\r");
	return;
      }
*/
/*
 else if ((fb == io_files[2]) || (fb == io_files[3])) {
	serstate = &host_state;
    }
*/

    switch (action) {

      case IO_INIT:
	break;

      case FGET:
	*(int *)arg = value = f_getc (fb);
	
      case IO_NON_FULL:
	if (! serstate) break;
	if ((serstate->in_xoff) && ((fb->qsize - fb->size) > LOLIM)) {
	    serstate->in_xoff = 0;
	    serstate->must_send = 021; /* ^q */
	    serstate->creg |= SER_TIE;	/* allow trans interupt */
	    *serstate->cport = serstate->creg;
	}
	if (! (serstate->creg & SER_REI)) {
	    serstate->creg |= SER_REI;
	    *serstate->cport = serstate->creg;
	}
	break;

      case FPUT:
	value = f_putc (arg, fb);
	
      case IO_NON_EMPTY:
	if (! serstate) break;
	if (!(fb->status & FTENA) && !(serstate->creg & SER_TIE))
	  {
	    serstate->creg |= 0x0088; /* SER_TIE; */
	    *serstate->cport = serstate->creg;     /*  CRASHES_HERE */
	  }
	break;

      case IO_BEGIN:
	*TERMCTL = 0;
	temp = *TERMCTL2;
	temp = *TERMDAT;
/*	*TERMCTL = SER_RESET; MRESET*/	/* reset the bugger */
/*	*HOSTCTL = MRESET;	    */  /* reset the bugger */
	*(QMCR) = SER_CONFIG;
 	*(QILR) = 0x2540;           /* *(QILR) = SER_INT_CONFIG; */
	*TERMCTL = /*SER_RESET |*/ SER_TE | SER_RE;
	*INT_VECT(40) = (long)r_serial; 

      case IO_RESTART: /* set flags and interupts */
	host_state.creg = term_state.creg = DISINT;
	host_state.in_xoff = term_state.in_xoff = 0;
	host_state.in_brk = term_state.in_brk = 0;
	host_state.must_send = term_state.must_send = 0;
/*	
        host_state.cport = HOSTCTL;
	host_state.dport = HOSTDAT;
*/
	term_state.cport = TERMCTL;
	term_state.dport = TERMDAT;
	set_iflags (io_files[0], &term_state);
	set_iflags (io_files[1], &term_state);
/*	set_iflags (io_files[2], &host_state);
	set_iflags (io_files[3], &host_state);
*/
	break;

      default:
	force_print ("Invalid IOCTL: ");
	force_hex (action); force_print ("\r\n");
	return EOF;
    }
    return value;
}


i_service (r_serial)		/* handle serial line interupts */
{
    SERIAL_STATE *ssp;	/* pointer to state record to use */
    char c;
    unsigned short status;
    short n_buff;
    FILE *fl;

/*    force_print("r_serial enter\r\n");  */
/*
    while (1)
      {			/* get all possible reqests*/
/*
        if (((status = *HOSTCTL) & IREQ))
	  {
	    n_buff = 2;
	    ssp = &host_state;
	  }
	else
	  if (((status = *TERMCTL) & IREQ))
	    {
	      n_buff = 0;
	      ssp = &term_state;
	    }
	  else
	    {
	      return;		/* nothing ready: exit */
/*
	    }
*/

    ssp = &term_state;
    n_buff = 0;
    status = *SER_SCSR;
	
	if (status & SER_FE) {	/* break */
	    io_files[n_buff]->status |= BREAK;
	    c = *ssp->dport;	/* clear flag */
	    ssp->in_brk = 1;
	    return; /* continue; */
	} else if ((status & RCVERR) && (status & SER_FE)) { /* error */
	    io_files[n_buff]->status |= FILE_ERR;
	    c = *ssp->dport;	/* clear flag, signal error */
	    return; /* continue; */
	} else if (status & TMTERR) { /* (lost CTS) disable transmit */
	    ssp->creg &= ~SER_TIE;
	    *ssp->cport = ssp->creg;
	    return; /* continue; */
	} else if ((ssp->must_send) && (status & SER_TDRE)) { /* must output */
	    fl = io_files[n_buff+1];
	    *ssp->dport = ssp->must_send;
	    ssp->must_send = 0;
	    if ((fl->status & FTENA) && (ssp->creg & SER_TIE)) {
		ssp->creg &= ~SER_TIE; /* disable trans if paused */
		*ssp->cport = ssp->creg;
	    }

	    return; /* continue; */  
	    /* RECEIVE a character */
	    /* add: hi/low input XON/XOFF */
	} else if ((ssp->in_brk) && (status & SER_RDRF)) { /* clean up break */
	    ssp->in_brk = 0;
	    c = *ssp->dport;
	    return; /* continue; */
	} else if ((status & SER_RDRF) && (ssp->creg & SER_REI)) {
	    fl = io_files[n_buff];
	    c = *ssp->dport;

	    /* nulls are used as padding, eliminate them. */
	    if ((c == 0) && (fl->status & NONULL)) return; /* continue; */
	    if ((c == '\003') && (fl->status & CBREAK)) { /* check ^C */
		force_print ("Break\r\n");
		new_shell();	/* temperary way to pause, and run shell */
	    }
	    if (fl->status & XONO) { /* allow ^S^Q control */
		if (c == '\023') {	/* ^S */
		    io_files[n_buff+1]->status |= FTENA; /* mark Xon/Xoff */
		    ssp->creg &= ~SER_TIE;
		    *ssp->cport = ssp->creg;
		    return; /* continue; */
		} else if (c ==	'\021') { /* ^Q */
		    io_files[n_buff+1]->status &= ~FTENA;
		    if (io_files[n_buff+1]->size > 0) {
			ssp->creg |= SER_TIE;
			*ssp->cport = ssp->creg;
		    }
		    return; /* continue; */
		}
	    }
	    
	    if (ssp->in_xoff) {
		if ((fl->qsize - fl->size) > LOLIM) {
		    ssp->in_xoff = 0;
		    ssp->must_send = 021; /* ^Q */
		}
	    } else if ((fl->qsize - fl->size) < HILIM) {
		ssp->in_xoff = 1;
		ssp->must_send = 023; /* ^S */
	    }

	    if (fl->status &  Q_EMPTY) { /* see if this will change state */
		update_state = 1;
		fl->status &= ~Q_EMPTY;
	    }

	    *(fl->head) = c;	/* insert character into queue */
	    fl->head++; fl->size++;
	    if (fl->head >= fl->data + fl->qsize)
	      fl->head = fl->data; /* wrap around buffer */
	    
	    if (fl->size >= fl->qsize) {
		fl->status |= Q_FULL;
		ssp->creg &= ~SER_REI; /* disable receive */
		*ssp->cport = ssp->creg;
	    }

	    /* TRANSMIT a character */
	} else if ((status & SER_TDRE) && (ssp->creg & SER_TIE)) {
	    n_buff++;
	    fl = io_files[n_buff];
	    if (fl->size <=0) goto receive_off;
	    if (fl->status & Q_FULL) { /* see if this will change state */
		update_state = 1;
		fl->status &= ~Q_FULL;
	    }
	    
	    *(ssp->dport) = *(fl->tail); /* get character from queue */
	    fl->tail++; fl->size--;
	    if ((unsigned long)fl->tail - (unsigned long)fl->data >= fl->qsize)
	      fl->tail = fl->data;
	    
	    if (fl->size <= 0 ) {
	      receive_off:
		fl->size = 0;
		fl->status |= Q_EMPTY;
		ssp->creg &= ~SER_TIE; /* disable transmit  */
		*ssp->cport = ssp->creg;
	    }
	} else
	  *ssp->cport = ssp->creg;
/*    }  */
}

/*-----------------------outs */
/* queued output for supervisor functions */
outs (str, fb)			/* print null terminated string */
  char *str;
  FILE *fb;
{
    char c;

    while (c = *str) {
	outc (c, fb);
	str++;
    }
}

/*-----------------------outc */
/* queued output for supervisor functions */
outc (c, fb)
  char c;
  FILE *fb;
{
    while (((volatile)fb->status) & Q_FULL); /* wait for spot in queue */
    (fb->service) (fb, FPUT, c);
}

/* for supervisor use */
outx(n, fb)			/* output a long hexidecimal number */
  unsigned int n;
  FILE *fb;
{
    char *m;
    short in=0, i, j, k;

    m = (char *)&n;

    /* Generate digits in order. */
    for (j=0, i=0; j < 8; j++) {
	if (j&1) {
	    k = *(m+i) & 0xf;
	    i++;
	}
	else {
	    k = (*(m+i) >> 4) & 0xf;
	}
	if ((k) || in) {
	    outc ((k < 10) ? (k + '0') : (k - 10 + 'A'), fb);
	    in++;
	}
    }
    if (in == 0) outc ('0', fb);
    return ;
}
