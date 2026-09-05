
#define MAX_PROC 16		/* maximum number of processes */

#define MAX_WAKE 8

#define BOXES 4

#define ACTIVE 1		/* schedualing states */
#define FROZEN 0
#define DEFAULT_STACK 512
#define SYS_STACK_TOP	0x2000	/* must agree with crt068.s and romstart.s */

 
/* process state */
typedef struct {
    short id;			/* a number for 0 to MAX_PROC */
    short s_reg;		/* status register */
    PFI  pc;			/* current pc */
    long reg[16];		/* register block */
    int status;			/* process status, see above */
    long wait;			/* time since last executed */
    short prty;			/* 0 = lowest priority */
    signed char num_boxes;	/* number of mailboxes to reference */
    unsigned char block_type;	/* types of the boxes waited on */
    long block[BOXES];		/* mailbox blocks while no */
    long *mailbox[BOXES];	/* block[i] == *mailbox[i] */
    PFI address;		/* only used to start the process */
    long stk_top;		/* initial stack value */
    long stk_sz;		/* size of stack (bytes) */
} PR_STATE;			/* total size 128 bytes */

/* types of things to block on in the mailboxes. */
#define NO_BLOCK 0x0
#define Q_BLOCK 0x1		/* queue */
#define M_BLOCK 0x2		/* mailbox */
#define S_BLOCK 0x4		/* select */
#define B_EQ	0x10		/* wait until equal, otherwise until NE */
#define B_SET	0x20		/* set box to block afterward */
#define B_TAS	0x40		/* use test-and-set */

/* not used now, may change later */
/* wake_up state */		
typedef struct {		
    long period;		/* delay between posts, (in ticks) */
				/* 0 if not periodic. */
    long count;			/* counts until post */
    long *mailbox;		/* mailbox to post to */
    long value;			/* value to post */
} WAKE_CALL;

#define outst(s) outs(s,termout)
#define flusht() force_flush(termout)
#define outxt(n) outx(n,termout)

extern short current_proc, num_proc; /* in sys_vars.s */

extern int outs(), outc(), outi(), outx(); /* in serial.c */
extern int trap_service(), r_lock(), r_boot(); /* in a_exec.c */

extern int r_init(), r_trap();	/* in exec.c */
extern int r_create(), r_start (), die();
extern int r_kill(), r_suicide (), r_wait();
extern int r_suspend(), r_resume();
extern int r_priority(), r_state(), r_who();
extern int r_freeze(), r_thaw();
extern int r_post(), r_pend();
extern int r_fput(), r_fget();
extern int block_one();

extern char *r_malloc(), *r_realloc(); /* in alloc.c */
extern int r_free(), r_init_free(), r_reset_free(), r_tell_free();

extern int r_is_mem();		/* in a_exec.c */
extern int r_set_baud();	/* in baud.c */
extern int r_download();	/* in download.c */

extern int boot(), restart(), new_shell(); /* in start.c */
extern int r_print_state(), r_print_wait(); /* in debug.c */
extern int force_flush(), force_hex(), force_print(), force_char();
extern int force_mem(), force_hexf(), fprint_stack(), show_where();

extern int init_break(), setbk(), clearbk(), clearall(); /* in break.c */
extern int prbks(), putbks(), hidebks(), hide_one();

extern int make_rtable();	/* in romcalls.c */

extern int timer_service(), tm_init(), tm_set(), tm_scale(); /* in timer.c */
extern unsigned int tm_read();

extern int system_stack_top, system_code_top; /* in sys_vars.s */

#define SYS_VECT(n)	((long *)(4*n))	/* for setting system traps */
