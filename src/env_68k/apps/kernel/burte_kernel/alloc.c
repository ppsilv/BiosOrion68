/*--------------------------------------------------------------------------
 *
 * ABSTRACT
 *	r_malloc ()- Allocate space from the dynamically-managed heap.
 *	r_FREE ()- Free previously allocated space.
 *	r_REALLOC ()- Reallocate space changing its size.
 *
 *	r_RESET_FREE ()- Reset the package.
 *	r_INIT_FREE ()- Give the package a heap.
 *	r_TELL_FREE ()- Tell the available free space left.
 *
 *	DUMP_FR_LIST ()- Print the free list on the terminal.
 *	DUMP_HEAD ()- Print the header of a block.
 *
 * SYNOPSIS
 *	Reset_free should be called before any other routine.
 *
 *	char *r_malloc (nbytes)
 *	    unsigned nbytes;		size wanted.
 *
 *	r_free (space)
 *	    char *space;		previously allocated
 *
 *	char *r_realloc (space, nbytes)
 *	    char *space;		previously allocated
 *	    unsigned nbytes;		new size.
 *
 *	r_reset_free ()
 *
 *	r_init_free (space, nbytes)
 *	    unsigned long space;	address of first byte
 *	    unsigned nbytes		number of bytes.
 *
 *	r_tell_free ()
 *
 *	dump_fr_list ()
 *
 *	dump_head (space)
 *	    char *space;		previously allocated
 *
 * RESULTS
 *	r_malloc (), realloc ():
 *	    Return a pointer to at least nbytes in core. This space
 *	    is guaranteed to be properly alligned for any data types.
 *	    realloc () guarantees that the original data is not changed,
 *	    even if the space is moved somewhere else. (Provided the data
 *	    fits, of course). They will return NULL if there was no more
 *	    contiguous memory of this size.
 *	    realloc () may return NULL if it thinks that the pointer
 *	    passed to it is bogus.
 *
 *	free ():
 *	    Returns TRUE if the space was added to the free list.
 *
 *	init_free ():
 *	    Returns TRUE if successful, FALSE otherwise.
 *
 *	tell_free ():
 *	    Returns the number of bytes still available in the heap.
 *
 *	reset_free (), dump_fr_list (), dump_head ():
 *	    Nothing.
 *
 * SIDE EFFECTS
 *	None that concern the user. It is very important to use pointers
 *	returned by alloc (or realloc) when calling free () or the whole
 *	package will get hopelessly lost. This possibility has been reduced
 *	somewhat by the addition of the magic number. It is nevertheless
 *	there.
 *
 *	The calls to reset_free () and init_free () are supervisor calls.
 *
 *	The debugging routines are only available if the package is compiled
 *	with the -DDEBUG flag.
 *
 * ENVIRONMENT
 *
 *    Imports
 *	None. (printf () for debugging).
 *
 *    Exports
 *	r_malloc (), r_free (), r_realloc (),
 *	reset_free (), init_free (), tell_free (),
 *	dump_fr_list (), dump_head ()	alloc.c
 *
 * DESIGN
 *	Copied out of Kernighan and Ritchie (page 175). It keeps the
 *	free list in a ring with a pointer (allocp) to it. Each free
 *	block keeps a header containing the number of bytes available in
 *	the block and the pointer to the next block on the ring.
 *
 *	Alloc () moves around the ring looking for a block with enough
 *	memory. If it finds one with more than enough, strips the remaining
 *	part from the top and returns a pointer to the bottom (after creating
 *	the appropriate header). It returns NULL if it can't find a block big
 *	enough.
 *
 *	Free () first checks the integrity of the header structure of the
 *	block. It looks at the pointer and decides whether the block is
 *	contiguous to other free block (in which case it updates the lowest
 *	memory header). Otherwise it links the isolated block to the ring.
 *
 *	Realloc () checks the integrity of the header structure of the block.
 *	Then it checks to see whether the size requested is less than the
 *	space already available or not. If it is, it simply strips off the
 *	tail, creates an appropriate header for it, and calls free ().
 *	Otherwise, it will free itself and call alloc () for the appropriate
 *	size. If alloc () returns the same pointer it had before, it does
 *	nothing. Otherwise it copies the data from the old place to the
 *	new one. 
 *
 *	The rest of the routines are brute force.
 *
 * HISTORY
 *  5-Nov-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Magic number added so the probability of freeing bogus pointers,
 *	or freeing something twice, is small.
 *
 * 17-Jan-82  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Copied from the C programming language book. Page 175. Changed to
 *	reflect the fact that we don't have an operating system
 *
 */

#define NULL 0
#define TRUE -1
#define FALSE 0

	/* The MAGIC number is not a valid address, ascii string,
	   instruction, or symmetric bit pattern */
#define	MAGIC	( (union header *) 0xAF00F00A )

typedef short ALIGN;		/* Forces alignment on 68000 */

union header {			/* free block header */
    struct {
	union header *ptr;	/* next free block */
	unsigned int size;	/* size of this free block */
    } s;
    ALIGN x;			/* force alignment of blocks */
};

typedef union header HEADER;	/* on the 68000 this is 8 bytes */

static HEADER	*allocp;	/* points to top of block ring */

/*
 * r_malloc()- Returns a character pointer pointing to space with
 * at least <nbytes> free.  The space is not zeroed.
 */

char *r_malloc (nbytes)
unsigned nbytes;
{
    HEADER *p, *q;
    unsigned nunits;
				/* round up to multiple of HEADER */
    nunits = (nbytes + 7) / 8;

    nunits++;			/* allocate space for this block's HEADER */

    if ((q = allocp) == NULL) {
	return (NULL);		/* hasn't been initialized! */
    }

    for (p = q -> s.ptr; ; q = p, p = p -> s.ptr)
    {
	if (p -> s.size >= nunits)	/* Big enough */
	{
	    if(p -> s.size == nunits)	/* Exactly */
		q -> s.ptr = p -> s.ptr;
	    else
	    {			/* allocate tail end */
		p -> s.size -= nunits;
		p += p -> s.size;
		p -> s.size = nunits;
	    }
	    p -> s.ptr = MAGIC;
	    allocp = q;		/* move allocp to improve next try */
	    return((char *)(p+1));
	}
	if (p == allocp) {	/* wrapped around free list */
	    return (NULL);
	}
    }	
}

/* 
 * r_free()- C routine that puts the space pointed to by the 
 * character pointer <ap> into the free list.
 * Assumes that <ap> was returned by 'alloc()' at some point.
 */

r_free (ap)
    char *ap;
{
    HEADER *p, *q;

    p = (HEADER *)ap - 1;	/* point to the header */
    if (p -> s.ptr != MAGIC) {
	force_print ("free: Invalid pointer returned!\n");
	return FALSE;
    }

    for (q = allocp; !(p > q && p < q -> s.ptr); q = q -> s.ptr)
	if (q >= q -> s.ptr && (p > q || p < q -> s.ptr))
	    break;		/* at one end or other */

    if (p + p -> s.size == q -> s.ptr)
    {				/* join to upper neighbor */
	p -> s.size += q -> s.ptr -> s.size;
	p -> s.ptr = q -> s.ptr -> s.ptr;
    }
    else
	p -> s.ptr = q -> s.ptr;

    if (q + q -> s.size == p)
    {				/* join to lower neighbor */
	q -> s.size += p -> s.size;
	q -> s.ptr = p -> s.ptr;
    }
    else
	q -> s.ptr = p;

    allocp = q;

    return TRUE;
}

/*
 *  r_realloc ()- Change the size of a block. If the new size is
 *  smaller, simply free () the rest. Otherwise, free the block
 *  passed (so there is more space in the package) and allocate
 *  the new size. Check the returned pointer (it may be the same)
 *  and copy the data if it differs from the one to be reallocated.
 */

char *r_realloc (ap, nbytes)
    char *ap;
    unsigned nbytes;
{
    HEADER *p, *q;
    unsigned unitsWanted;
    unsigned unitsNow;

    p = (HEADER *) ap - 1;	/* point to the header */
    if (p -> s.ptr != MAGIC) {
	force_print ("realloc: Invalid pointer returned!\n");
	return NULL;
    }
				/* get nearest multiple of HEADERs */
    unitsWanted = nbytes + sizeof (HEADER) - 1;
    unitsWanted = unitsWanted / 8;
    unitsWanted++;		/* plus space for the block's HEADER */

    if (p -> s.size == unitsWanted) {
	return (ap);
    }

    if (p -> s.size > unitsWanted)
    {
	q = p + unitsWanted;	/* where to make a header */
	q -> s.size = p -> s.size - unitsWanted;
	r_free (q + 1);		/* free the spare space */
	p -> s.size = unitsWanted;
	return (ap);
    }

    unitsNow = ((HEADER *) ap - 1) -> s.size;

    r_free (ap);			/* must re-allocate */

    if ((p = (HEADER *) r_malloc (nbytes)) == NULL) {
	return (NULL);
    }

    if (p < (HEADER *) ap)		/* new block at lower address */
    {
	int i;

	q = (HEADER *) ap;		/* beginning of old data */
	ap = (char *) p;		/* save the new value to return */

	for (i = unitsNow - 1; i; i--)
	    *p++ = *q++;		/* copy the data */
    }
    else if (p > (HEADER *) ap)		/* new block at higher address */
    {
	int i;

	q = (HEADER *) ap + unitsNow;	/* ending of old data (plus one) */
	ap = (char *) p;		/* save the new value to return */
	p += unitsNow;			/* ending of new data (plus one) */

	for (i = unitsNow - 1; i; i--)
	    *(--p) = *(--q);		/* copy the data */
    }

    return (ap);
}

/*
 *  INIT_FREE ()- Places arbitrary blocks on the free list.
 *  'cp' must be on an even boundary.  If it's not 1 unit will be
 *  subtracted from 'nu' and 1 unit will be added to cp.
 *  'nu' should be an integral number of HEADER's.  Any fractional
 *  part will be truncated.  'init_free()' will return FALSE if 
 *  zero blocks are freed.
 */

r_init_free (cp, nu)
char *cp;			/* start of block */
unsigned int nu;		/* number op chars in block */
{
    HEADER *up;

    if ((long) cp & 1) {
	cp++;			/* round up to a short boundry */
	nu--;			/* adjust available space */
    }

    nu = nu / 8;

    if (!nu) {
	return (FALSE);
    }

    up = (HEADER *) cp;
    up -> s.size = nu;

    up -> s.ptr = allocp = up;

    return (TRUE);
}

/*
 *	RESET_FREE ()- Simply reset the allocp pointer to NULL.
 *
 *	TELL_FREE ()- Return the number of bytes in the free
 *	list. It will return FALSE if the package was in use.
 */

r_reset_free() {
    allocp = NULL;
}

r_tell_free ()
{
    HEADER *p = allocp;
    long nbytes = 0;

    do {
	nbytes += p -> s.size;
	p = p -> s.ptr;
    } while (p != allocp);	/* remember it is a ring */

    nbytes *= 8 ;		/* *8 to yeild bytes */

    return nbytes;
}

/*
 *	The following code is only useful to debug the dynamic
 *	space allocator. It should not be compiled unless a problem
 *	with it is suspected
 */

#ifdef DEBUG

dump_fr_list ()
{
    HEADER *p;

    printf ("\r\n[");

    p = allocp;

    do {
	printf ("\tptr: %x, link: %x, size: %d\r\n",
		p, p -> s.ptr, p -> s.size);
	p = p -> s.ptr;
    } while (p != allocp);

    printf ("]");

}


dump_head (ptr)
    HEADER *ptr;
{
    HEADER *p;

    p = ptr - 1;

    printf ("\r\n[ ");
    printf ("\thead: %x, link: %x, size: %d",
	    p, p -> s.ptr, p -> s.size);
    printf (" ]");
}

#endif
