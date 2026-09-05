/* file handling functions for BURTE */
#include <burte.h>
#include <burteio.h>

file_init (fl, sz, buff)
  FILE *fl;
  int sz;
  unsigned char *buff;
{
    fl->qsize = sz;
    fl->status = 0;
    fl->data = fl->head = fl->tail = buff;
    fl->size = 0;
}

f_putc (c, fb)
  char c;
  FILE *fb;
{
    if (fb->status & Q_FULL) return (EOF);
    fb->status &= ~Q_EMPTY;
    *(fb->head) = c;	/* insert character into queue */
    fb->head++; fb->size++;
    if (fb->head >= fb->data + fb->qsize)
      fb->head = fb->data;
	
    if (fb->size >= fb->qsize) {
	fb->size = fb->qsize;
	fb->status |= Q_FULL;
    }
    return 0;
}

f_getc (fb)
  FILE *fb;
{
    int value;
    
    if (fb->size <= 0) return (EOF);
    value = *(fb->tail); /* get character from queue */
    fb->status &= ~Q_FULL;
    fb->tail++; fb->size--;
    if (fb->tail >= fb->data + fb->qsize)
      fb->tail = fb->data;
	
    if (fb->size <= 0 ) {
	fb->status |= Q_EMPTY;
    }
    return (value);
}
