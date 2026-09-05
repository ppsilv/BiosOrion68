/* Dan Cristian  3-29-87 */
/*
 *
 *  Author             Date         Description
 *  --------------     --------     -----------------------------------------
 *  Andrew H. Fagg     02/19/95     Added SRAM_BASE_B_? for expanded memory
 *                                   version B.
 *                                  
 *
 *  Andrew H. Fagg     02/27/93     Added cpu_* to process table to handle
 *                                   statistics collection.
 *
 *
 */

#define MAX_PROC 16		/* maximum number of processes */

#define MAX_WAKE 8

#define BOXES 4

#define ACTIVE 1		/* schedualing states */
#define FROZEN 0
#define DEFAULT_STACK 512
#define SYS_STACK_TOP	0xa000 /* 0x2f00 /*0x2000	/* must agree with crt068.s and romstart.s */

 
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
    unsigned long time;         /* Used for timer block  - Andy  */
    unsigned long sched_interval;  /* Scheduling time interval.  */
    unsigned long cpu_time;     /* Accumulation of cpu time used. */
    unsigned long cpu_latency;  /* Accumulation of execution latency */
    unsigned short cpu_count;   /* Accumulation of number of times */
                                /*  granted cpu */
    unsigned short sched_count; /* Accumulation of number of times */
                                /*  that proc was time-scheduled.  */
    unsigned short cpu_time_state; /* True if just become runnable under a */
                                  /*  time block.  */
    unsigned short max_latency; /* Maximum latency observed.  */
} PR_STATE;			/* total size 152 bytes */
                                /*  this value must be reflected in a_exec.s */

/* types of things to block on in the mailboxes. */
#define NO_BLOCK 0x0
#define Q_BLOCK 0x1		/* queue */
#define M_BLOCK 0x2		/* mailbox */
#define S_BLOCK 0x4		/* select */
#define T_BLOCK 0x8             /* timer */   /* Andy */
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
extern int r_msleep();
extern void r_super_outp();
extern unsigned int r_super_inp();
extern void r_super_outb();
extern unsigned char r_super_inb();
extern int r_set_schedule();
extern int r_next_interval();
extern int r_supervisor();
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

				/* Defined in exec.c */

extern unsigned long last_time;	/* Time that the last process was given */
				/*   the cpu.   */
extern short last_proc;		/* Last process given the cpu. */
extern unsigned short statistics_flag; /* If TRUE, then collect stats. */


				/* Chip select info */

/* async, both, read, AS, 0wait, S/U, all IPL, avec off */
#define RAM_RD_OPT1 0x28F0 

/* async, both, read, AS, 0wait, S/U, all IPL, avec off */
#define RAM_RD_OPT2 0x48F0 

/* async, both, read, AS, 0wait, S/U, all IPL, avec off */
#define RAM_RD_OPT0 0x68F0 	

/* async, both, read/write, AS, 0wait, S/U, all IPL, avec off */
#define RAM_RDWR_OPT1 0x3830 

/* async, both, read/write, AS, 0wait, S/U, all IPL, avec off */
#define RAM_RDWR_OPT2 0x5830 

/* async, both, read, AS, 0wait, S/U, all IPL, avec off */
#define RAM_RDWR_OPT3 0x79F0 



#define SRAM_BASE 0x20000	/* Expanded memory version A */

#define SRAM_BASE_B_1 0x40000	/* Expanded memory version B */
#define SRAM_BASE_B_2 0x80000

