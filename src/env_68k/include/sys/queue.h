
typedef struct {
    int size;			/* number of chars in Q */
    unsigned short qsize;	/* size of the character buffer */
    unsigned char data_sz;	/* size of data in buffer (1,2,4)*/
    unsigned char flags;	/* mode flags */
    unsigned char *head;	/* Pointer to the head of Q */
    unsigned char *tail;	/* Pointer to the tail of Q */
    unsigned char *data;	/* Actual character buffer */
} QUEUE;

/* Cases for flags */
#define Q_EMPTY		0x1
#define Q_FULL		0x2
#define Q_OVERWRITE	0x4
