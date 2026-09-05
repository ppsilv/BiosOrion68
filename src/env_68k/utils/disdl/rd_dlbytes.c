/*----------------------------------------------------------------------------
 * These are the subroutines needed by the upper level rd_dl routines
 * to read 8 bit, 16 bit and 24 bit quantities from a download file.
 *
 * HISTORY
 * 14-Sept-87 Dan Christian (dac) at Carnegie-Mellon U.
 *      Added dynamic memory allocation and improved error messages.
 *      Added control printing routine: pchar.
 *
 * 11-Jun-82  John Schlag (jfs) at Carnegie-Mellon University
 *	Separated these routines from disdl(1) and rd_dlfile(3).
 *
 *----------------------------------------------------------------------------
 */

#include <stdio.h>

char *pchar();

byte2(fp)
  FILE *fp;
{
    int	num, c;
    
    c = getc(fp);
    if ( ('0' <= c) && (c <= '9'))
     c -= '0';
    else {
	if ( ('A' <= c) && (c <= 'F'))
	 c += 10 - 'A';
	else {
	    fprintf (stderr, "Non-hex digit encountered: %s\n", pchar(c));
	    c = 0;
	}
    }
    num = c << 4;
    c = getc(fp);
    if ( ('0' <= c) && (c <= '9') )
     c -= '0';
    else {
	if ('A' <= c && c <= 'F')
	 c += 10 - 'A';
	else {
	    fprintf (stderr, "Non-hex digit encountered: %s\n", pchar(c));
	    c = 0;
	}
    }
    num = num + c;
    return(num);
}

byte4(fp)
  FILE *fp;
{
    int temp, result;
    
    result = byte2(fp) << 8;	/* get upper byte */
    temp = byte2(fp);		/* get lower byte */
    result += temp;
    return(result);
}

byte6(fp)
  FILE *fp;
{
    int temp, result;
    
    result = byte4(fp) << 8;
    result += byte2(fp);
    return(result);
}

/* returns a printable string of a single character.  Control codes are
 *   returned as ^_.  The returned string is internal to the routine,
 *   and should not be freed or used as a constant.
 */
char *pchar(ch)
  char ch;
{
    static char out[3];
    if (ch < ' ') {
	out[0] = '^'; out[1] = ch + '@'; out[2] = 0;
    } else {
	out[0] = ch; out[1]= 0;
    };
    return (out);
}
