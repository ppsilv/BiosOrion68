/*
 *	Header file for the input output system of REX.
 *      The QUEUEs are for interprocess communication.
 *      The FILEs are for IO communication and pipes.
 *
 *	10-28-87 Dan Christian created for REX.
 */

#ifndef IOS
#define IOS

typedef struct {
    int	size;			/* what to watch */
    int qsize;			/* what to wait on when full */
    PFI	service;		/* routine to add/remove things */
} FIFO_DATA;

typedef struct {
    int size;			/* number of chars in Q */
    int qsize;			/* size of the character buffer */
    PFI service;		/* routine to service: fill/emptying */
    int busy;			/* semiphore to resolve string access */
    unsigned short status;	/* flags: error, cooked, empty, full, */
    unsigned char *head;	/* Pointer to the head of Q */
    unsigned char *tail;	/* Pointer to the tail of Q */
    unsigned char *data;	/* Actual character buffer */
} FILE;

/*	Cases for FILE status ... */
#define FILE_ERR	0x1	/* There was an error */
#define FILE_EOF	0x2	/* The EOF condition was found */
#define BREAK		0x4	/* break seen */
#define NONULL		0x200	/* ingore null characters */
#define XONO		0x400	/* allow XON/XOFF control of output */
#define XONI		0x800	/* send XON/XOFF for input */
#define CBREAK		0x1000	/* allow ^c for break */
#define Q_EMPTY		0x2000
#define Q_FULL		0x4000
#define FTENA		0x8000	/* disallow transmit interupts (Xon/Xoff) */

/* definitions for io service routines */

#define IO_INIT 3
#define IO_DISABLE 4
#define IO_NON_EMPTY 5
#define IO_NON_FULL 6
#define IO_BEGIN 7
#define IO_RESTART 8



#define NULL 0
#define EOF -1

/* io_files and termin, etc are the same absolute locations in memory */
				/* defined in sys_vars.s */
extern FILE *termin, *termout, *hostin, *hostout, **io_files;

#endif
