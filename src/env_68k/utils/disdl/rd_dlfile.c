/*----------------------------------------------------------------------------
 *
 * rd_dlfile(fp, buffer, wordcount) - Read in a .dl file in Motorola S format.
 *
 * Arguments:
 * fp:		pointer to the .dl file.
 * buffer:	pointer to a returned pointer buffer of
 *                       unsigned short integers for the code.
 * wordcount:	pointer to an integer where we return the wordcount.
 *
 * HISTORY
 * 14-Sept-87 Dan Christian (dac) at Carnegie-Mellon U.
 *      Added dynamic memory allocation and improve error messages.
 *
 * 11-Jun-82  John Schlag (jfs) at Carnegie-Mellon University
 *	Separated this routine from disdl(1) and added interpretation
 *	of S2 records.
 *----------------------------------------------------------------------------
 */

#include <stdio.h>
#define BUFFINC 512

char *pchar();			/* defined in rd_dlbytes */

rd_dlfile(fp, bufferin, wordcount)
  FILE *fp;
  unsigned short **bufferin;
  int *wordcount;
{
    int		addr = -1;
    char		c;
    unsigned short *buffer;
    int i, nbytes, nwords = 0, done = 0, bufin = 0;
    
    int buffroom = BUFFINC, buffsize = BUFFINC;
    
				/* initialize buffer */
    buffer = (unsigned short *) malloc (buffsize * sizeof (short));
    if (buffer == NULL) {
	fprintf (stderr, "Unable to allocate memory for reading file.\n");
	exit(1);
    }
    
    while (!done)
     {
	 while ( fgetc(fp) != 'S');	/* wait for the next record */
	 c = fgetc(fp);
	 switch (c)
	  {
	    case '0':
	      break;		/* skip header record */
	      
	    case '1':
	      nbytes = byte2(fp) - 3;
	      nwords += nbytes / 2;
	      if (addr == -1)
	       addr = byte4(fp);	/* 1st S1 record: set addr */
	      else
	       byte4(fp);		/* else skip over addr */
	      for (i = 0 ; i < (nbytes / 2) ; ++i) {
		  if (buffroom <= 0) {
		      buffsize += BUFFINC;
		      buffer = (unsigned short *) realloc
		       (buffer, buffsize * sizeof (short));
		      if (buffer == NULL) {
			  fprintf (stderr,
				   "Ran out of memory reading file.\n");
			  exit(1);
		      }
		      buffroom += BUFFINC;
		  }
		  buffer[bufin++] = (unsigned short)byte4(fp);
		  buffroom--;
	      }
	      byte2(fp);			/* skip checksum value */
	      break;
	      
	    case '2':
	      nbytes = byte2(fp) - 4;
	      nwords += nbytes / 2;
	      if (addr == -1)
	       addr = byte6(fp);	/* 1st S2 record: set addr */
	      else
	       byte6(fp);		/* else skip over addr */
	      for (i = 0 ; i < (nbytes / 2) ; ++i) {
		  if (buffroom <= 0) {
		      buffsize += BUFFINC;
		      buffer = (unsigned short *) realloc
		       (buffer, buffsize * sizeof (short));
		      if (buffer == NULL) {
			  fprintf (stderr,
				   "Ran out of memory reading file.\n");
			  exit(1);
		      }
		      buffroom += BUFFINC;
		  }
		  buffer[bufin++] = (unsigned short)byte4(fp);
		  buffroom--;
	      }
	      byte2(fp);			/* skip checksum value */
	      break;
	      
	    case '8':		/* skip last record */
	      done = 1;
	      break;
	      
	    case '9':		/* skip last record */
	      done = 1;
	      break;
	      
	    default:		/* error */
	      fprintf(stderr,
		      "Unknown record type: S%s in .dl file.\n",
		      pchar(c));
	      break;
	  }
     }
    
    *wordcount = nwords;	/* return the number of words */
    *bufferin = buffer;	/* return the created buffer */
    return(addr);		/* return the starting address */
}
