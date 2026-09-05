/* routine to do input with editing */
/* Dan Christian 1-Dec-87 */

#include <stdio.h>
#include <rline.h>
#include <term.h>

/* set char *s1 to point to the null in s2 */
#define find_end(s1,s2)    s1 = s2; while (*s1) s1++

/*------------------- read_line 
 * reads in a line of text with fancy editing and previous command subst.
 *
 * buff_ring is a character array: buff_ring[ring_size][buff_size]
 *  buff_ring holds commands to be substituted using ^P ^N;
 *
 * c_buffer points to the initial buffer to use.
 *
 * prompt is the prompt to be displayed
 * emacs syle commands supported ctl-A B D E F L
 * unix variants supported ctl-U R W H <delete>
 */
char *read_line (fin, fout, prompt,
		 c_buffer, buff_size, buff_ring, ring_size, flags)
  FILE *fin, *fout;
  char *prompt, *c_buffer, *buff_ring;
  unsigned int buff_size, ring_size, flags;
{
    short i, no_ring = 1;
    unsigned int ring_pos;
    char in_char, *add_in, *cp, nc, tc;
    
    for (ring_pos=0; ring_pos < ring_size; ring_pos++)
      if ((char *)(buff_ring +ring_pos*buff_size) == c_buffer) {
	  no_ring = 0;
	  break;
      }

    if ((!(flags &  NO_NEXT)) && (! no_ring)) {
	    if (++ring_pos >= ring_size) ring_pos = 0;
	    c_buffer = (char *)(buff_ring +ring_pos*buff_size);
	}
    if (flags & NO_CLEAR) {
	find_end (add_in, c_buffer);
    } else {
	add_in = c_buffer;
	*add_in = 0;
    }
    
    fputs (prompt, fout); fputs (c_buffer, fout);
    while (1){
	in_char = getc (fin);
	switch (in_char) {
	  case '\r':		/* note: accepts empty lines */
	  case '\n':
	    fputs ("\r\n", fout);
	    return (c_buffer);
	    break;
	    
	  case '\177':		/* back space and delete */
	  case '\010':
	    if (add_in <= c_buffer) {
		putc ('\007', fout); /* signal can't back up */
		break;
	    }
	    add_in--;
	    putc ('\010', fout); /* BS fall through to ^D */
	  case '\004':		/* ^D delete */
	    for (cp = add_in; *cp; cp++) {
		*cp = *(cp+1);	/* shift left */
		if (*cp) putc (*cp, fout);
		else putc (' ', fout);
	    }			/* BS to insertion point */
	    while (--cp >= add_in) putc ('\010', fout);
	    break;

	  case '\001':		/* ^A start of line */
	    while (--add_in >= c_buffer) putc ('\010', fout);
	    add_in++;
	    break;

	  case '\005':		/* ^E end of line */
	    while (*add_in) putc (*(add_in++), fout);
	    break;

	  case '\002':		/* ^B backward character */
	    if (add_in <= c_buffer) {
		putc ('\007', fout); /* signal can't back up */
		break;
	    }
	    add_in--;
	    putc ('\010', fout);
	    break;
	    
	  case '\006':		/* ^f forward character */
	    if (!(*add_in)) {
		putc ('\007', fout); /* signal can't go forward */
		break;
	    }
	    putc (*(add_in++), fout);
	    break;
	    
	  case '\027':		/* ^w erase word */
	    while ((--add_in >= c_buffer) && (*add_in == ' '))
	      for (cp = add_in; *cp; cp++) *cp = *(cp+1);
	    add_in++;
	    while ((--add_in >= c_buffer) && (*add_in != ' '))
	      for (cp = add_in; *cp; cp++) *cp = *(cp+1);
	    add_in++;
	    redisplay (fout, prompt, c_buffer);
	    find_end (cp, c_buffer);
	    while (--cp >= add_in) putc ('\010', fout);
	    break;
	    
	    /*  command ring substitutions */
	  case '\020':		/* ^p previous in ring */
	    if (no_ring) break;
	    if (--ring_pos < 0) ring_pos = ring_size-1;
	    strcpy (c_buffer, (char *)(buff_ring +ring_pos*buff_size));
	    find_end (add_in, c_buffer);
	    redisplay (fout, prompt, c_buffer);
	    break;
	    
	  case '\016':		/* ^n next in ring */
	    if (no_ring) break;
	    if (++ring_pos >= ring_size) ring_pos = 0;
	    strcpy (c_buffer, (char *)(buff_ring +ring_pos*buff_size));
	    find_end (add_in, c_buffer);
	    redisplay (fout, prompt, c_buffer);
	    break;
	    
	    /* standard input editing unix commands */
	  case '\022':		/* ^R  redisplay*/
	    redisplay (fout, prompt, c_buffer);
	    find_end (cp, c_buffer);
	    while (--cp >= add_in) putc ('\010', fout);
	    break;
	    
	  case '\014':		/* ^L clear screen */
	    clear_screen (fout);
	    fputs (prompt, fout); fputs (c_buffer, fout);
	    break;
	    
	  case '\025':		/* ^U erase line */
	    c_buffer[0] = '\0';
	    add_in = c_buffer;
	    clear_line (fout); fputs (prompt, fout);
	    break;
	    
	  case EOF:
	    return (c_buffer);
	    break;
	    
	  default:	/* insert the character */
	    if (add_in >= (c_buffer + buff_size-1)) {
		putc ('\007', fout);
		break;
	    }

	    for (cp = add_in, nc = in_char; nc; cp++) {
		putc (nc, fout);
		tc = *cp; *cp = nc; nc = tc; /* swap np *cp */
	    }
	    *cp = 0;		/* mark end null */
	    add_in++;		/* increment insert point */
	    while (--cp >= add_in) putc ('\010', fout);	/* BS to ins pnt */
	}
    }
}
