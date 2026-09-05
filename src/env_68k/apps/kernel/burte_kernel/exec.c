/* Dan Christian  3-29-87 */
/*  This file provides the basis for a real time executive  */
/*  It works in conjunction with some assembly routines in:  a_exec.s */
/*
 *
 *  Author             Date         Description
 *  --------------     --------     -----------------------------------------
 *  Andrew H. Fagg     02/26/94     Fixed bug in r_resume(): proc_state[i].time
 *                                    is set to the current time, rather
 *                                    than being left in the state when the
 *                                    process was paused.
 *  Andrew H. Fagg     04/27/93     Fixed bug in insert_runq() routine.
 *  Andrew H. Fagg     03/03/93     Added cpu usage statistics collection
 *                                   to _top() routine.
 *  Andrew H. Fagg     02/27/93     Updated msleep and set_schedule to
 *                                   handle new wallclock rate (122 usecs).
 *
 *
 */

#include <burte.h>
#include <burteio.h>
#include "system.h"
#include <trap-service.h>

PR_STATE proc_state[MAX_PROC];	/* state for all processes */
PR_STATE *run_que[MAX_PROC];

				/* Statistics collection temp vars. */
				/*  see _top(). */
unsigned long last_time; /* Time that the last process was given */
				/*   the cpu.   */
short last_proc;		/* Last process given the cpu. */
unsigned short statistics_flag; /* If TRUE, then collect stats. */

/* trap server */
trap_service(r_trap) (trap_num, arg)
  long trap_num, arg[];
  
{
    /* NOTE: The order of these calls
       MUST MATCH WITH THE CALL NUMBERS IN burte.h */
    
    const static TRAP_CALL traplist[] = { /* array of system calls */
	/* return, #arg, funct */
	0, 0, r_init,		/* INIT       0  */
	0, 2, r_fput,		/* FPUT       1  */
	1, 2, r_fget,		/* FGET       2  */
	0, 2, r_post,		/* POST       3  */
	1, 3, r_pend,		/* PEND       4  */
	1, 3, r_create,		/* CREATE     5  */
	2, 1, r_start,		/* START      6  */
	2, 1, r_kill,		/* KILL       7  */
	0, 0, r_suicide,	/* SUICIDE    8  */
	2, 1, r_wait,		/* WAIT       9  */
	2, 1, r_suspend,	/* SUSPEND    A  */
	2, 1, r_resume,		/* RESUME     B  */
	3, 2, r_priority,	/* PRIORITY   C  */
	3, 1, r_state,		/* STATE      D  */
	1, 0, r_who,		/* WHO_AM_I   E  */
	0, 0, r_freeze,		/* FREEZE     F  */
	0, 0, r_thaw,		/* THAW       10 */
        0, 1, r_msleep,         /* MSLEEP     11 */
	0, 2, r_super_outp,     /* SUPER_OUTP 12 */
	1, 1, r_super_inp,      /* SUPER_INP  13 */
	0, 1, r_set_schedule,   /* SET_SCHEDULE_INTERVAL 14  */
	0, 0, r_next_interval,  /* NEXT_INTERVAL 15 */
	1, 2, r_supervisor,     /* SUPERVISOR 16 */
	0, 2, r_super_outb,     /* SUPER_OUTB 17 */
	1, 1, r_super_inb      /* SUPER_INB  18 */
    };
    
    long val;
    TRAP_CALL *tr;
/*
    force_print("trap : ");
    force_hex(trap_num);
    force_print("\n\r");
*/
    if (bad_id (current_proc)) {
	force_print ("\007\r\nBURTE: Corrupted current proc: ");
	force_hex (current_proc);
	force_print ("\r\nCall was: "); force_hex (trap_num);
	force_print ("\r\n");
	boot ();
    }
    
    update_state = 1;		/* system calls always change state */
    
    if ((trap_num > 0) && (trap_num < NUM_SERVICE)
	&& (tr = &traplist[trap_num]) && (tr->funct)) {
	if ((tr->rtn & 0x2) && (bad_id (arg[0]))) { /* check id */
	    force_print ("\007\r\nBURTE: Bad proc id: ");
	    force_hex (arg[0]);
	    force_print ("\r\nCall was: "); force_hex (trap_num);
	    force_print ("\r\n");
	    r_suspend (current_proc);
	    new_shell ();
	}
	
	switch (tr->arg_cnt) {
	  case 0: val = (tr->funct)(); break;
	  case 1: val = (tr->funct)(arg[0]); break;
	  case 2: val = (tr->funct)(arg[0], arg[1]); break;
	  case 3: val = (tr->funct)(arg[0], arg[1], arg[2]); break;
	  case 4: val = (tr->funct)(arg[0], arg[1], arg[2], arg[3]); break;
	  default: val = (tr->funct)(arg); /* pass in arg address */
	}
	if (tr->rtn & 0x1) proc_state[current_proc].reg[0] = val;
    } else {		/* trap not fount */
	force_print ("BURTE: Invalid system call: ");
	force_hex (trap_num); force_print(", IGNORED!\r\n");
	return;
    }
}


/*------------------------Create */
/* r_create a new process, and leaves it paused */
r_create(proc, priority, stack_size)
  PFI proc;
  long priority;
  unsigned long stack_size;
{
    short new_proc, i;
    PR_STATE *pr;
    
    for (new_proc=0;
	 (new_proc < MAX_PROC) && proc_state[new_proc].status;
	 new_proc++);		/* note: NO_PROC = 0 */
    
    if (new_proc >= MAX_PROC ) {	/* too many processes ? */
	force_print("BURTE>r_create: Too many process. Create aborted.\r\n");
	return (-1);
    }
    if (stack_size == 0) stack_size = DEFAULT_STACK;
    
    pr = &proc_state[new_proc];	/* block pointer */
    pr->id = new_proc; /* create state info */
    pr->prty = (short) priority;
    pr->address = proc;
    pr->pc = proc;
    pr->status = READY | PAUSED;
    if (! bad_id (current_proc)) { /* preserve tracing mode */
	pr->s_reg = proc_state[current_proc].s_reg & 0x8000;
    } else {
	pr->s_reg = 0;
    }
    for (i=0; i < 16; i++)	/* set registers to something interesting */
      pr->reg[i] = i;
    /* set stack pointer to the top of the allocated area */
    pr->stk_sz = stack_size;
    pr->stk_top = (long)r_malloc (stack_size) + stack_size;
    if (pr->stk_top == stack_size) {
	force_print ("BURTE>r_create: no memory for stack");
	return (-1);
    }
    
    num_proc++;
    return(new_proc);
}


/*-------------------- Start */
/* Start a process from the top */
r_start (id)
  short id;
{
    short i;
    PR_STATE *pr;
    
    pr = &proc_state[id];
    pr->pc = pr->address;
    pr->reg[15] = pr->stk_top-4;
    *((int *)(pr->reg[15])) = (int)die;
    pr->wait = 0;
    pr->status = READY;
    pr->block_type = 0;
    pr->num_boxes = 0;
    
    for (i=0; i < BOXES; i++) {
	pr->mailbox[i] = 0;	/* mark mailboxes as inactive */
	pr->block[i] = 0;
    }
    insert_runq (id);
}

/*--------------------Kill */
/* kill a process */
r_kill(id)
  short id;
{
    PR_STATE *pr;
    
    pr = &proc_state[id];
    if (pr->status == 0) return; /* don't free something twice. */
    pr->status = 0;
    num_proc--;
    r_free (pr->stk_top - pr->stk_sz);
    delete_runq (id);
}

/*-------------------- suicide */
/* kill yourself (used for a process to exit. */
r_suicide()
{
    r_kill (current_proc);
}

/*-------------------- die */
/* user's suicide.  This call can be placed on the stack of process. to
   allow them to die automatically on exit */
die()
{
    system (SUICIDE);
}

/*---------------------Wait */
/* Wait for a process to die (status == 0) */
r_wait (id)
  short id;
{
    if (proc_state [id].status) { /* if process is active */
	block_one (&(proc_state[id].status), 0, B_EQ | M_BLOCK);
    } else {
	return 0;
    }
}

/*---------------------Suspend */
/* suspend a process */
r_suspend(id)
  short id;
{
    proc_state[id ].status |= PAUSED;
    delete_runq (id);
}

/*--------------------Resume */
/* resume a suspended process */
int  r_resume(id)
  short id;
{
  proc_state[id].time = wallclock;
  proc_state[id].status &= ~PAUSED;
  insert_runq (id);
}

/*---------------------Who am I */
int r_who()
{
    return (current_proc);
}

/*---------------------State */
/*  Return the state block of a process.  For advanced users only! */
int r_state(id)
  short id;
{
    return ((int)&proc_state[id]);
}

/*----------------------Priority */
/* Set the the priority of a process */

r_priority (id, pri)
  short id, pri;
{
    int tmp;
    
    tmp = proc_state[id].prty;
    proc_state[id].prty = (short) pri;
    delete_runq (id);		/* delete then reinsert at the new spot */
    insert_runq (id);
    return tmp;
}

/*----------------------Freeze */
/* suspend schedualing and interrupts */
int r_freeze()
{
    burte_state = FROZEN;
}

/*----------------------Thaw */
/* resume scheduling and interupts */
int r_thaw()
{
    burte_state = ACTIVE;
}

/*----------------------Post */
/* post a value to a mailbox */
int r_post(mailbox, value)
  long int *mailbox, value;
{
    if (r_is_mem (mailbox))
      *mailbox = value;
}

/*----------------------Pend */
/* wait for a *mailbox != block if equal == 0
 * wait for mailbox == block if equal !=  0 */
/*  the value of the mailbox is returned by the scheduler */
int  r_pend(mailbox, block, flags)
  long int *mailbox, block, flags;
{
    short i;
    PR_STATE *pr;
    unsigned int flg;
    int sr, value;
    
    if (! (r_is_mem (mailbox))) { /* check for valid memory reference. */
	sys_error = 1;
	return block;
    }				/* wait until equal or NE */
    sr = r_lock (0x2700);
    if (flags & P_TAS) {
	if (! tas (mailbox)) {
	    r_lock (sr);
	    return (*mailbox);
	} else {
	    value = *mailbox;
	}
    } else {
	value = *mailbox;
	if ((flags & P_EQUAL) ? (value == block) : (value != block)) {
	    if (flags & P_SET) *mailbox = (flags & P_EQUAL) ? ~block : block;
	    r_lock (sr);
	    return value;
	}
    }
    
    if (flags & P_CHECK) {
	r_lock (sr);
	return value;
    } else {			/* block */
	flg = M_BLOCK;
	if (flags & P_TAS)
	  flg |= B_TAS;
	else {
	    if (flags & P_EQUAL)
	      flg |= B_EQ; /* set eq and set flags */
	    if (flags & P_SET)   flg |= B_SET;
	}
	block_one (mailbox, block, flg);
	r_lock (sr);
    }
}

/*------------------r_putc---------
 * put a character into a fifo buffer
 */

r_fput (ff, value)
  FIFO_DATA *ff;
  char *value;
{
    int sr;
    
    if ((unsigned int)ff->service < 0x1000) { /* debug ?? */
	force_print ("r_fput: Invalid service func: ");
	force_hex ((int)ff->service);
	force_print ("\r\n");
	return EOF;
    }
    sr = r_lock(0x2700);	/* priorty 5, lock out serial */
    
    if (((ff->service) (ff, FPUT, value)) == EOF) /* couldn't put */
      { block_one (ff, ff->qsize, Q_BLOCK); /* wait for que to drain */
	proc_state [current_proc].reg[1] = (int)value; /* value to be queued */
    }
    r_lock (sr);		/* restore interupts */
}

/*------------------r_getc---------
 * get a character out of a fifo buffer
 */

r_fget (ff, flags)
  FIFO_DATA *ff;
  int flags;
{
    int value;
    int sr;
    
    if ((unsigned int)ff->service < 0x1000) { /* debug ?? */
	force_print ("r_fget: Invalid service func: ");
	force_hex ((int)ff->service);
	force_print ("\r\n");
	return EOF;
    }
    
    sr = r_lock(0x2700);	/* priorty 5, lock out serial */
    
    if (((ff->service) (ff, FGET, &value)) == EOF) { /* couldn't get */
	if (flags & P_CHECK)
	  return EOF;
	else
	  block_one (ff,  0, Q_BLOCK); /* wait for que to fill */
    } else return value;
    
    r_lock (sr);		/* restore interupts */
}

block_one (mailbox, block, type)
  long *mailbox;
  int block;
  unsigned char type;
{
    short i;
    PR_STATE *pr;
    
    pr = &proc_state[current_proc];
    pr->status |= WAITING;
    pr->num_boxes = 1;
    
    for (i=1; i < BOXES; i++)
      pr->mailbox[i] = 0;
    pr->mailbox[0] = mailbox;
    pr->block[0] = block;
    pr->block_type = type;
}

/*----------------------Initialize */
/* initalize everything */
r_init()
{
    short i;
    
    burte_state = FROZEN;
    for (i = 0; i < MAX_PROC; i++) {
	proc_state[i].status = 0;
	run_que[i] = 0;
    }
    num_proc = 0;
    current_proc = -1;
    sys_error = 0;
    update_state = 0;
}

/*
 * insert_runq (proc)
 *
 *  Add the new process to the run queue, which is sorted by priority.
 *
 */

insert_runq (proc)		/* insert a process into the priority que */
  short proc;
{
  short i,j;
  PR_STATE *pr = &proc_state[proc];
  
  for (i=0; i< MAX_PROC; i++)
    {
      if (! run_que[i])
	{
	  run_que[i] = pr;
	  return;
	}
      else
	if (pr->prty > run_que[i]->prty)
	  {
/*	    
	    for (j=i; (run_que[j]) && (j < MAX_PROC-1); j++)
	      {			/* shift down */
	    /*
		run_que[j+1] = run_que[j];
	      }
*/
				/* Shift down correctly (from the bottom of */
				/*  of the queue up to insertion point).  */
	    for(j = MAX_PROC-1; j>=i; --j)
	      if(run_que[j])
		run_que[j+1] = run_que[j];
	    run_que[i] = pr;
	    return;
	  }
    }
}


delete_runq (proc)		/* remove a process from the priority que */
  short proc;
{
    short i,j;
    PR_STATE *pr = &proc_state[proc];
    
    for (i=0; i< MAX_PROC; i++) {
	if (! run_que[i]) {	/* not found */
	    return (-1);
	} else if (pr == run_que[i]) {
	    for (j=i; (run_que[j]) && (j < MAX_PROC-1); j++) { /* shift up */
		run_que[j] = run_que[j+1];
	    }
	    if (j == MAX_PROC-1) run_que[j+1] = 0; /* NULL last entry */
	    return (0);
	}
    }
}


/*-------------------------Top process */
/* find the highest priority ready process and update any waiting
   /*   or processes */


  int _top() {
      PR_STATE *pr, **loop;
      short max_p, j, max_id;
      int max_wait;
      FIFO_DATA *ff;
      unsigned long wc;
      unsigned long wc_temp;

      if(statistics_flag)
	{			/* Collect statistics for completing proc */
	  if(last_proc >= 0)
	    {			/* Valid proc. */
	      wc = wallclock;	/* Compute amount of time taken since. */
	      if(wc >= last_time) /*  start of proc.                   */
		wc = wc - last_time;
	      else
		wc = wc + (0xFFFFFFFF - last_time);
				/* Add this to the accumulator for this proc */
	      proc_state[last_proc].cpu_time += wc;
	    }
	}

      update_state = 0;		/* clear update flag */
      /* update state for all processes in run_que*/
      for (loop = &run_que[0], pr = *loop;
	   pr && (loop < (run_que + MAX_PROC)); loop++, pr = *loop) {
	  if (pr->status == 0) continue;
	  pr->wait++;
	  if (pr->status & WAITING /* && !(pr->status & PAUSED) */) {
	      if (pr->block_type & M_BLOCK) { /* mailbox */
		  if (pr->block_type & B_TAS) {
		      if (! tas (pr->mailbox[0])) {
			  pr->status &= ~WAITING;
			  pr->reg[0] = *(pr->mailbox[0]);
			  pr->mailbox[0] = 0;
			  pr->num_boxes = 0;
			  pr->block_type = 0;
		      }
		  } else if ((pr->mailbox[0]) &&
			     ((pr->block_type & B_EQ) ?
			      (*(pr->mailbox[0]) == pr->block[0]) :
			      (*(pr->mailbox[0]) != pr->block[0]))) {
		      pr->status &= ~WAITING;
		      pr->reg[0] = *(pr->mailbox[0]);
		      if (pr->block_type & B_SET)
			*(pr->mailbox [0]) = (pr->block_type & B_EQ) ?
			  ~pr->block [0] : pr->block [0]; /* clear mailbox */
		      pr->mailbox[0] = 0;
		      pr->num_boxes = 0;
		      pr->block_type = 0;
		  }
	      } else if (pr->block_type & Q_BLOCK) { /* que */
		  if ((pr->mailbox[0]) &&
		      (*(pr->mailbox[0]) != pr->block[0])) {
		      pr->status &= ~WAITING;
		      ff = (FIFO_DATA *)pr->mailbox[0];
		      if (pr->block[0]) { /* qpost case */
			  (ff->service) (ff, FPUT, pr->reg[1]);
		      } else { /* qpend case */
			  (ff->service) (ff, FGET, &pr->reg[0]);
		      }
		      pr->mailbox[0] = 0;
		      pr->num_boxes = 0;
		      pr->block_type = 0;
		  }
	      } else if (pr->block_type & S_BLOCK) { /* select */
		  pr->reg[0] = 0;
		  for (j = 0; j < BOXES; j++) {
		      if ((pr->mailbox[j])
			  && (*(pr->mailbox[j]) != pr->block[j])) {
			  pr->reg[0] |= (1 << j); /* flag ready bitwise */
			  pr->mailbox[j] = 0;
		      }
		  }
		  if (pr->reg[0]) { /* if we found any ready */
		      pr->status &= ~WAITING;
		      pr->num_boxes = pr->block_type = 0;
		    }
		}
	                   /* timer block  */
	      else if (pr->block_type & T_BLOCK)  
		  {
		    wc = wallclock;
		    if(((pr->time <= wc) && ((wc - pr->time)  < LARGE)) ||
		       ((pr->time > wc) && ((pr->time - wc) > LARGE)))
		      {
			pr->block_type = 0;
			pr->status &= ~WAITING;
			pr->cpu_time_state = TRUE;
		      }
		      
		  }
	    }
      }
      
      max_p = -1000;
      max_wait = -1000;		/* find top proccess to run */
      max_id = -1;		/* default: none ready */
      for (loop = &run_que[0], pr = *loop;
	   pr && (loop < (run_que + MAX_PROC)); loop++, pr = *loop)
	if ((pr->status == READY)
	    && ((pr->prty > max_p) ||
		((pr->prty = max_p) && (pr->wait > max_wait))))
	  {
	      max_p = pr->prty;	/* find highest priority */
	      max_wait = pr->wait; /* find longest wait */
	      max_id = pr->id;
	  };
      if (max_id >= 0)		/* Any process scheduled? */
	{
	  proc_state[max_id].wait = 0;

				/* Update statistics for CPU utilization. */
	  if(statistics_flag)
	    {
	      wc = wallclock;
	      proc_state[max_id].cpu_count++; /* Number of times scheduled. */
	      if(proc_state[max_id].cpu_time_state)
		{		
				/* Process is time-scheduled. */

		  proc_state[max_id].cpu_time_state = FALSE;
		  proc_state[max_id].sched_count++;
		  if(wc >= proc_state[max_id].time)
		    wc_temp = wc - proc_state[max_id].time;
		  else
		    wc_temp = wc + (0xFFFFFFFF - proc_state[max_id].time);
		  proc_state[max_id].cpu_latency += wc_temp;
		  if(wc_temp > proc_state[max_id].max_latency)
		    proc_state[max_id].max_latency = wc_temp;
		}
	      last_time = wc;
	    }
	}
      last_proc = max_id;
      return (max_id);
  }


/* fatal system error */
burte_error(err_num)
  int err_num;
{
    volatile short i;
    
    for (i=0;i<32000;i++);	/* let print buffers empty */
    force_print ("\007\r\nBURTE: System error: ");
    force_hex (err_num);
    force_print (".  Current proc: "); force_hex (current_proc);
    force_print (" ...Dieing.\r\n");
    force_mem ((int)&err_num +4, 30); /* quick stack trace */
    boot();
}


/*---------------------Wait */
/* Wait for a process to die (status == 0) */
r_msleep (msec)
  unsigned long msec;
{
/*
  volatile long i;
  for(i=0; i < 32000;++i);
  force_print("r_msleep ");
  force_hex(msec);
  force_print("\r\n");
*/

  proc_state[current_proc].time = wallclock + (msec<<3);

  proc_state[current_proc].status |= WAITING;
  proc_state[current_proc].block_type = T_BLOCK;
}

void r_super_outp(addr, val)

     volatile unsigned short *addr;
     unsigned int val;

{
  *addr = (unsigned short) val;
}

unsigned int r_super_inp(addr)

     volatile short *addr;

{
  return((unsigned int) *addr);
}

r_set_schedule(msec)

     unsigned long msec;
{
  proc_state[current_proc].sched_interval = msec<<3;
  proc_state[current_proc].time = wallclock;
}

r_next_interval()

{
  proc_state[current_proc].time += proc_state[current_proc].sched_interval;

  proc_state[current_proc].status |= WAITING;
  proc_state[current_proc].block_type = T_BLOCK;

}

int r_supervisor(func, parms)

     int (*func)();
     char *parms;
{
  return((*func)(parms));
}

void r_super_outb(addr, val)

     volatile unsigned char *addr;
     unsigned char val;

{
  *addr = (unsigned char) val;
}

unsigned char r_super_inb(addr)

     volatile char *addr;

{
  return((unsigned char) *addr);
}
