/*--------------------------------------------------------------------------
 *
 *  Abstract:
 *	Download VERSABUG format over the serial lines.
 *
 *  Parameters:
 *	file: the name of the file to be downloaded.
 *
 *  Results:
 *	Returns the execution address.
 *
 *    Imports:
 *	fputs(), fputc(), fputi(), fputx(), getc();		serial.c
 *
 *    Exports:
 *	download ()
 *
 *  Bugs:
 *	Fails miserably when information coming from the host
 *	is not in VERSABUG format.
 *
 *  Design:
 *	* Print the file name from the S0 records.
 *	* Load information from S2 records into core.
 *	* Get the execution address from the S8 record.
 *
 * HISTORY
 *
 *	2-Jan-88	Dan Christian modified from popI sources.
 *
 *----------------------------------------------------------------------------
 */

#include <stdio.h>

/* conver upper/lower case hex to int */
#define	hex(c)		((c > '9') ? ((c - 'A' + 0xA) & 0xf) : (c - '0'))

#define FILESRC termin
#define FILEOUT termout
  
unsigned char byte2 (), byte ();

r_download (filename)
  char *filename;
{
    long exec_addr = 0;
    unsigned char checksum, nbytes, data;
    long addr;
    int totalbytes = 0;
    char md;
    char i;

    if (filename[0] == '!') {
	fputs (filename+1, FILEOUT); /* entire down load command (skip !) */
    } else {
	fputs ("mdl68 ", FILEOUT);	/* ask for the file */
	fputs (filename, FILEOUT);
    }
    fputs ("\n", FILEOUT);
    
    while (1) {
	/* Move to next record. */
	while ((i = byte() & 0x7f) != 'S');
	addr = 0;
	md = byte() & 0x7f;	/* record type */
	nbytes = byte2 ();	/* byte count */
/* fputs ("down1", FILEOUT); */

	switch (md) {
	  case '0':		/* Header record with name. */
/* fputs ("down2", FILEOUT); */
	    byte2 ();		/* Skip the address. */
	    byte2 ();
	    nbytes -= 3;	/* Account. */
	    while (nbytes--)	/* Translate into ascii. */
	      fputc (byte2 (), termout);
	    fputcn (termout);
	    byte2 ();           /* Eat checksum.  */
	    goto next_record;
	    
	  case '1':		/* S1 Data record. */
/* fputs ("down3", FILEOUT); */
	    addr   = byte2 ();
	    data   = byte2 ();
	    checksum = nbytes + addr + data;
	    
	    addr <<= 8;
	    addr |= data;
	    
	    nbytes -= 3;
	    totalbytes += nbytes;
	    break;
	    
	    
	  case '2':			/* S2 Data record. */
/*fputs ("down4", FILEOUT);  */
	    addr   = byte2 ();
	    data   = byte2 ();
	    checksum = nbytes + addr + data;
	    
	    addr <<= 8;
	    addr |= data;
	    checksum += (data = byte2 ());
	    addr <<= 8;
	    addr |= data;
	    
	    nbytes -= 4;
	    totalbytes += nbytes;
	    break;
	    
	  case '8':		/* Trailer record. */
/* fputs ("down5", FILEOUT); */
	    addr = (byte2 () & 0xFF) << 16;
	    addr |= (byte2 () & 0xFF) << 8;
	    addr |= (byte2 () & 0xFF);
	    byte2 ();	/* Skip checksum. */
	    exec_addr = addr;	/* Return execution addr. */
	    goto finish_dnld;
	    
	  case '9':
/* fputs ("down6", FILEOUT); */
	    addr = (byte2 () & 0xFF) << 8;
	    addr |= (byte2 () & 0xFF);
	    byte2 ();       /* Skip checksum. */
	    exec_addr = addr;	/* Return execution addr. */
	    goto finish_dnld;
	    
	  default:		/* probably an EOF */
/* fputs ("down7", FILEOUT); */
	    putc ('E', termout); putc (md, termout);
	    fputi((int) md, termout);
	    goto finish_dnld;
	}
	
/* fputs ("down8", FILEOUT); */
	if (!r_is_mem (addr) || !r_is_mem (addr + nbytes - 1) ) {
	    fputs ("\ndownload: No memory between ", termout);
	    fputx (addr, termout);
	    fputs (" and ", termout);
	    fputx (addr + nbytes, termout);
	    fputs ("\n", termout);
	    goto next_record;
	}

/* fputs ("down9", FILEOUT);	 */
	while (nbytes--) {	/* place data */
	    checksum += (data = byte2 ());
/*
	    fputs("addr : ",termout);
	    fputx(addr,termout);
	    fputs(":",termout);
	    fputx(data,termout);
	    fputs("\n",termout);
*/
	    *(char *) (addr++) = data;
	}
	
	if (byte2 () == (~checksum & 0xff)) /* verify checksum */
	  fputc ('.', termout); /* record is correct */
	else
	  fputc ('?', termout); /* checksum error */
      next_record:;
    }

/*fputs ("down10", FILEOUT); */
    /* clean up and print out status */
  finish_dnld:;
    fputs ("\nExecution address: ", termout);
    fputx (addr, termout);
    fputs (" (", termout);
    fputi (totalbytes, termout);
    fputs (" bytes).\n", termout);
    force_flush (termout);
/* fputs ("down11", FILEOUT);	    */
    while ((md = byte ()) != '\r' && md != '\n'); /* clear input */
    return (exec_addr);
}

/*
 *	routine to return a byte retrieved (in hex format)
 */

unsigned char byte2 ()
{
    char n, c;
    
    c = byte () & 0x7F;
    n = hex (c) << 4;
    c = byte () & 0x7F;
    
    return (n + hex (c));
}

unsigned char byte ()
{
    unsigned char c;
    
    /*add: timeout on characters */
    c = getc (FILESRC);
    
    return (c);
}
