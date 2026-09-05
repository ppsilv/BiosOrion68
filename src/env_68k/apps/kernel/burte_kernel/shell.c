/* the command interpreter for the BURTE kernel */
/*
 *
 *  Author             Date         Description
 *  --------------     --------     -----------------------------------------
 *  Andrew H. Fagg     02/26/94     Fixed bug in statistics command that
 *                                   caused the % cpu to be mis-computed.
 *  Andrew H. Fagg     03/03/93     Added the statistics and wait commands.
 *
 *
 */

#include <stdio.h>
#include <rline.h>
#include "system.h"

char *skip_words();

int help(), quick_help(), run(), cont(), brun(), bcont(), set(), clear();
int show(), memory(), bset(), wset(), lset();
int load(), send(), reboot();
int create_proc(), print_reg(),  print_st(), bug_level();
int disassemble(), transp(), show_active(), kill_proc();
int watch(), proc_trace();
int hbaud(), tbaud();
int statistics();
int wait_pid();

extern PR_STATE  proc_state[];	/* in exec.c */
extern int csum, tcnt;		/* in debug.c */
extern char *csum_start, *csum_end;
extern long old_trace_hndl;
extern PFI step_break_h();	/* in ihandle.s */

extern int end;			/* The end of BURTE. */

typedef struct {
    PFI funct;			/* function to call */
    char *comm;			/* command to match */
    char *help;			/* help string */
} SHELL_COMMAND;

const SHELL_COMMAND comms [] = {
    help, "help",
    " [command] \t=> Help on some commands",
    quick_help,	    "?",
    "\t\t=> Show commands",
    load, "dload",
    " [!<command>] <name> \t=> Download from host",
    transp, "talk",
    " \t\t=> talk to the host",
    send, "!",
    " <command> \t=> Send command to the host",
    hbaud, "hbaud",
    " <rate> \t=> Set host buad rate",
    tbaud, "tbaud",
    " <rate> \t=> Set terminal buad rate",
    run, "run",
    " <addr> \t=> Run a program",
    brun, "brun",
    " <addr> \t=> Run a program in bg",
    cont, "continue",
    " <pid> \t=> Continue a process",
    bcont, "bcontinue",
    " <pid> \t=> Continue a proc in bg",
    kill_proc, "kill",
    " <pid> \t=> Kill a proc",
    show_active, "ps",
    "\t\t=> show all active proc",
    print_reg, "registers",
    " <pid> \t=> print registers for proc",
    print_st, "state",
    " <pid> \t=> print state for a proc",
    set, "set",
    " <addr> \t=> set a breakpoint",
    clear, "clear",
    " <addr> \t=> clear a breakpoint",
    show, "show",
    "\t\t=> show all breakpoints",
    statistics, "statistics",
    " [secs (5)] \t=> collect process statistics",
    hidebks, "hide",
    "\t\t=> Hide breakpoints in memory",
    watch, "watch",
    " <laddr>  [haddr] \t=> Watch for memory",
    proc_trace, "trace",
    " <pid> [steps (1)] \t=> execute steps instructions",
    memory, "memory",
    " <addr> [count (20)] \t=> display memory",
    bset, "bset",
    " <addr> \t=> set a byte memory location",
    wset, "wset",
    " <addr> \t=> set a word memory location",
    wait_pid, "wait",
    " <pid> \t=> force shell to wait for process to exit",
    lset, "lset",
    " <addr> \t=> set a long memory location",
    disassemble, "disassem",
    " <addr> [count (8)] \t=> disassemble mem",
    reboot, "reboot",
    "\t\t=> reboot BURTE",
    0,0,0
  };

static char buff[80];		/* that oooo so often used input buffer */

shell()
{
    short i,j, run;
    char *clean, *cp, *args;

    while (1) {
	if (hostin->size > 0) print_from_host();
	fputs ("8) ", termout);
	fgets (buff, sizeof buff, termin, termout);

	clean = skip_words (buff, 0); /* kill initial whitespace */
	if (*clean == 0) {
	    continue;
	}

	for (args = clean; *args ; args++) { /* parse off first word */
	    if ((*args == ' ') || (*args == '\t')
		|| (*args == '\r') || (*args ==  '\n')) {
		break;
	    }
	}
	if (*args)
	  *args++ = 0;		/* args points to rest of string */
	else
	  *args = 0;		/* at end of input string */
	    
	for (i=0, j=0; comms[i].comm; i++) { /* find all matches */
	    if (stlmatch (comms[i].comm, clean)) {
		j++;
		fputs (comms[i].comm, termout); fputs (": ", termout);
		run = i;
	    }
	}
	fputcn(termout);
	
	if (j == 1) {
	    (comms[run].funct)(skip_words (args, 0));
	} else if (j == 0) {
	    fputs ("Unknown command: ", termout);
	    fputs (clean, termout);
	    fputcn(termout);
	} else {
	    fputs ("Ambiguous command: ", termout);
	    fputs (clean, termout);
	    fputcn(termout);
	}
    }
}

print_from_host()		/* echo pending input to terminal */
{
    fputs ("From Host:\n", termout);
    while ((volatile)hostin->size > 0) {
	putc (getc (hostin), termout);
    }
    fputcn (termout);
}

help (cl)				/* long help */
  char *cl;
{
    SHELL_COMMAND *ct;

    if (! *cl)
      fputs ("Unique abreviations are acceptable for all commands\n",
	     termout);
    for (ct=comms; ct->comm; ct++) { /* find all matches */
	if (stlmatch (ct->comm, cl)) {
	    fputs (ct->comm, termout);
	    fputs (ct->help, termout);
	    fputcn(termout);
	}
    }
}

quick_help ()			/* print possible commands */
{
    SHELL_COMMAND *ct;

    for (ct=comms; ct->comm; ct++) {
	fputs (ct->comm, termout);
	fputs (", ", termout);
    }
    fputcn(termout);
}

run (cl)			/* start a process */
  char *cl;
{
    int pr;

    pr = brun (cl);
    if (pr >= 0) wait (pr);
}

brun (cl)			/* start a process in background */
  char *cl;
{
    int pr;
    long *st;


    st = (long *) atox (cl);
/*    if (st < (long *)SYS_STACK_TOP) { */
				/* Make sure address is above BURTE. */
    if (st < ((0xFFFFF000 & (unsigned int) &end) + 0x1000))
      {
	fputs ("Invalid address: too low in memory!\n", termout);
	return -1;
    }
    *st++ = (long)termin;
    *st++ = (long)termout;
    *st++ = (long)termout;

    pr = create (st, 10, 0);
    if (pr < 0) {
	fputs ("Unable to create process\n", termout);
	return -1;
    }
    fputs ("Starting proc: ", termout); fputx (pr, termout); fputcn(termout);
    if (csum_end) {		/* trace */
	proc_state[pr].s_reg |= 0x8000;
    } else {
	proc_state[pr].s_reg &= ~0x8000;
    }
    putbks ();
    if (hide_one (proc_state[pr].pc)) {
	fputs ("Hiding break point\n", termout);
	old_trace_hndl = *SYS_VECT (9);
	*SYS_VECT (9) = (long) step_break_h;
	proc_state[pr].s_reg |= 0x8000;
    }
    start (pr);
    return pr;
}

cont (cl)			/* continue a stopped process */
  char *cl;
{
    int pr;

    pr = bcont (cl);
    if (pr >= 0) wait (pr);
}

bcont (cl)
  char *cl;
{
    int pr;

    if ((pr = get_pid (cl)) < 0) return -1;

    if (! (proc_state[pr].status & PAUSED)) {
	fputs ("Process is already running: ", termout);
	fputx (pr, termout);
	fputcn (termout);
	return -1;
    }
    if (proc_state[pr].status == 0) {
	fputs ("Dead process, can\'t continue: ", termout);
	fputx (pr, termout);
	fputcn(termout);
	return -1;
    }
    if (csum_end) {		/* trace */
	proc_state[pr].s_reg |= 0x8000;
    } else {
	proc_state[pr].s_reg &= ~0x8000;
    }
    putbks ();
    if (hide_one (proc_state[pr].pc)) {	/* next instruction is a break pnt */
	fputs ("Hiding break point\n", termout);
	old_trace_hndl = *SYS_VECT (9);
	*SYS_VECT (9) = (long) step_break_h;
	proc_state[pr].s_reg |= 0x8000;
    }

    resume (pr);
    return pr;
}

print_reg (cl)			/* print process registers */
  char *cl;
{
    int pr;

    if ((pr = get_pid (cl)) < 0) return;
    r_print_regs (pr);
}

print_st (cl)			/* print total process state */
  char *cl;
{
    int pr;

    if ((pr = get_pid (cl)) < 0) return;
    r_print_state (pr);
}

show_active (cl)		/* list all active processes */
  char *cl;
{
    short i, stat;
    PR_STATE *p;

    fputs ("PID\tStart\tPC\tStatus\n", termout);
    for (p = &proc_state[0], i=0; p < &proc_state [MAX_PROC]; p++, i++)
      if (p->status) {
	  fputx ((int)i, termout); fputs ("\t", termout);
	  fputx ((int)p->address, termout); fputs ("\t", termout);
	  fputx ((int)p->pc, termout); fputs ("\t", termout);
	  stat = p->status;
	  if (stat & STOPPED) putc ('S', termout);
	  if (stat & PAUSED) putc ('P', termout);
	  if (stat & WAITING) putc ('W', termout);
	  if (stat & READY) putc ('R', termout);
	  fputcn (termout);
      }
}

kill_proc (cl)
  char *cl;
{
    int pr;

    if ((pr = get_pid (cl)) < 0) return;

    if (pr != 0) {
	kill (pr);
    } else {
	fputs ("Process 0 is shell!  Do not kill!\n", termout);
    }
}


reboot ()
{
    trap (10);			/* which calls boot */
}

set (cl)			/* set a breakpoint */
  char *cl;
{
    if (! setbk ( atox (cl))) {
	fputs ("Couldn't set breakpoint\n", termout);
    }
}

clear (cl)			/* remove a breakpoint */
  char *cl;
{
    int addr = atox (cl);

    if (stlmatch ("all", cl)) {
	hidebks ();
	clearall();
    } else {
	if (hide_one (addr)) clearbk (addr);
    }
}

show ()				/* list all breakpoints */
{
    prbks();
}

watch (cl)			/* set region of memory to watch */
  char *cl;
{
    int a;
    char *smem;

    a = atox (cl);
    if (! a) {
	csum_start = csum_end = 0;
	return;
    }
    csum_start = (char *)a;
    a = atox (skip_words (cl, 1));
    if ((char *)a >= csum_start) {
	csum_end = (char *)a+1;
    } else {
	if (a == 0) a = 1;
	csum_end = csum_start+a; /* just watch one char */
    }
    for (smem = csum_start, a=0; smem < csum_end; smem++) { /* set csum */
	a += *smem;
    }
    csum = a;
    fputs ("Checksum is: ", termout);
    fputx (csum, termout);
    fputcn (termout);
}

proc_trace (cl)
  char *cl;
{
    int n, p;

    if ((p = get_pid (cl)) < 0) return;
    n = atox (skip_words(cl,1));
    if (! n) n = 1;
    tcnt = n;
    proc_state[p].s_reg |= 0x8000; /* set trace bit */
    if (bcont (cl) >= 0) {
	wait (p);		/* wait until done or shell restarted */
    }
}

/*
 * display_decimal(x,y)
 *
 *  Display the ratio x/y, with accuracy to 1/10th.
 *
 */

display_decimal(x,y)
     unsigned int x,y;
{
  unsigned int temp;
  temp = x/y;
  fputi((int) temp, termout);
  fputs(".", termout);
  fputi((int) ((x*10)/y - temp*10), termout);
}

/*
 * statistics (cl)
 *
 *   Collect process running-time statistics and report them to the
 * user.  The routine first clears out the statistics-collecting
 * data-structures, then turns on statistics collection and sleeps
 * for a specified amount of time.  Then, statistics collection is
 * turned off, and the values are reported.  The reported values are:
 *  PID - pid of process
 *  Time - total execution time (in units of 122 usecs).
 *  Sched - number of times the process was scheduled by the scheduler.
 *  S. Avg - Average amount of time used each time the process was
 *      scheduled.
 *  T. Run - Number of times that the process was Time-Scheduled
 *      (via a sleep() or next_interval() system call).
 *  T. Avg - Average amount of time used by the process for each
 *      time it was Time-Scheduled.
 *  Latency - The sum of the amount of time between when the process
 *      becomes runnable (for time-scheduled processes) and the time
 *      that it actually executes.
 *  L. Avg - Average latency for each time the process was time-scheduled.
 * 
 *
 */

statistics (cl)
     char *cl;
{
  int secs;
  PR_STATE *p;
  short i;
  unsigned int total_time;

  secs = atoi(cl);
  if(!secs)
    secs = 5;
  if(secs > 60 || secs < 1)
    {
      fputs("[secs] can only be between 1 and 60.\n", termout);
      return;
    }
				/* Initialize process data structures. */
  for (p = &proc_state[0], i=0; p < &proc_state [MAX_PROC]; p++, i++)
    {
      p->cpu_time = 
	p->cpu_latency =
	p->cpu_count =
	p->sched_count = 0;
      p->cpu_time_state = FALSE;
      p->max_latency = 0;
    }
  last_proc = -1;
				/* Turn on statistics collection */
  statistics_flag = TRUE;
				/* Wait specified amount of time */
  sleep(secs);
  statistics_flag = FALSE;	/* Turn off collection */

				/* Report */

  fputs("PID\tTime\tSched\tS. Avg\tT. Run\tT. Avg\tLatency\tL. Avg\tL. Max\n",termout);
  total_time = 0;
  for (p = &proc_state[0], i=0; p < &proc_state [MAX_PROC]; p++, i++)
    {
      if (p->status)		/* For each process */
	{
	  fputi((int)i, termout); fputs("\t", termout);
	  fputi((int)p->cpu_time, termout); fputs("\t", termout);
	  total_time += p->cpu_time;
	  fputi((int)p->cpu_count, termout); fputs("\t", termout);
	  if(p->cpu_count > 0)
	    display_decimal(p->cpu_time, p->cpu_count);
	  fputs("\t", termout);
	  fputi((int)p->sched_count, termout); fputs("\t", termout);
	  if(p->sched_count > 0)
	    display_decimal(p->cpu_time, p->sched_count);
	  fputs("\t", termout);
  	  fputi((int)p->cpu_latency, termout); fputs("\t", termout);
	  if(p->sched_count > 0)
	    display_decimal(p->cpu_latency, p->sched_count);
	  fputs("\t", termout);
	  fputi((int)p->max_latency, termout);
	  fputs("\n", termout);
	}
    }
				/* Display total time. */
  fputs("\t------\n\t", termout);
  fputi((int) total_time, termout);
  total_time /= secs;		/* Scale to one-second. */
  fputs("\n\t", termout);
				/* Percent CPU usage. */
  display_decimal(total_time, 80);
  fputs("%\n", termout);
}

memory (cl)
  char *cl;
{
    int start, finish;
    char *mem;
    short i,j, *smem;

    start = atox (cl);
    if (! start) {
	fputs ("usage: memory start <count>\n", termout);
	return;
    }
    start &= ~0x1;		/* round down to even */
    finish = atox ( skip_words (cl, 1));
    if ( !finish) finish = 24;

    for (mem= (char *)start, smem = (short *)start, i=0; i < finish;) {
	if (! r_is_mem (mem)) {
	    mem += 2;
	    i+=2;
	    continue;
	}
	fputsx (mem, 6, termout); fputs ("  ", termout);
	for (j=0; j< 8; j++, smem++) {
	    fputsx (*smem, 4, termout); fputs (" ", termout);
	}
	fputs ("   ", termout);
	for (j=0; j< 16; j++, i++, mem++) {
	    if ((*mem >= ' ') && (*mem <= '~'))
	      putc (*mem, termout);
	    else
	      putc ('.', termout);
	}
	fputcn(termout);
    }
}

bset (cl)
  char *cl;
{
    char *cp, *clean;

    cp = (char *)atox (cl);
    while (r_is_mem (cp)) {	/* while in valid memory */
	fputsx (cp, 6, termout); fputs ("  <", termout);
	fputsx (*cp, 2, termout); fputs (">  : ", termout);
	fgets (buff, sizeof buff, termin, termout);
	if (! buff[0]) return;	/* empty line returns */
	*cp++ = atox (buff);
    }
    fputs ("Nonexistant memory location: ", termout);
    fputx (cp, termout);
}
 
wset (cl)
  char *cl;
{
    char *clean;
    short *sp;

    sp = (short *)(atox (cl) & ~1L);
    while (r_is_mem (sp) && r_is_mem (sp+1)) {	/* while in valid memory */
	fputsx (sp, 6, termout); fputs ("  <", termout);
	fputsx ((int)*sp, 4, termout); fputs (">  : ", termout);
	fgets (buff, sizeof buff, termin, termout);
	if (! buff[0]) return;	/* empty line returns */
	*sp++ = atox (buff);
    }
    fputs ("Nonexistant memory location: ", termout);
    fputx (sp, termout);
}
 
lset (cl)
  char *cl;
{
    char *clean;
    long *lp;

    lp = (long *)(atox (cl) & ~1L);
    while (r_is_mem (lp) && r_is_mem (lp+3)) {	/* while in valid memory */
	fputsx (lp, 6, termout); fputs ("  <", termout);
	fputsx ((int)*lp, 8,  termout); fputs (">  : ", termout);
	fgets (buff, sizeof buff, termin, termout);
	if (! buff[0]) return;	/* empty line returns */
	*lp++ = atox (buff);
    }
    fputs ("Nonexistant memory location: ", termout);
    fputx (lp, termout);
}

disassemble (cl)
  char *cl;
{
    int start, finish;

    start = atox (cl);
    if (! start) {
	fputs ("usage: memory start <count>\n", termout);
	return;
    }
    start &= ~0x1;		/* round down to even */

    finish = atox ( skip_words (cl, 1));
    if ( !finish) finish = 10;

    dis68 (start, finish, start, "", termout);
}


load(cl)
  char *cl;
{
    hostin->status |= NONULL;	 /* kill incoming padding */
    r_download (cl);
}

send (cl)			/* send command to host */
  char *cl;
{
    fputs (cl, hostout);
    putc ('\n', hostout);
    fflush (hostout);
}

trans2() {			/* half of the transparent mode */
    while (1) {
	putc (getc (hostin), termout); /* host to terminal */
    }
}

transp()			/* transparent communications to host */
{
    int pr;
    short tstat, hstat;

    pr = create (trans2, 10, 64);
    if (pr < 0) {
	fputs ("Couldn't make sub proc\n", termout);
	return;
    }
    start (pr);
    clearerr (termin);
    tstat = termin->status;
    hstat = hostin->status;
    termin->status &= ~(CBREAK | XONO);
    hostin->status &= ~(CBREAK | XONO);
    hostin->status |= NONULL;	/* kill incoming padding */
    while (! ((volatile)(termin->status) & (FILE_ERR | FILE_EOF | BREAK))) {
	putc (getc (termin), hostout); /* exit on break or error */
    }

    clearerr (termin);
    termin->status = tstat;
    hostin->status = hstat;
    kill (pr);
    fputcn (termout);
}

hbaud (cl)			/* set host (port 1) buad rate */
  char *cl;
{
    if (! r_set_baud (1, atoi (cl))) {
	fputs ("Could not set host baud ", termout);
	fputi (atoi (cl), termout);
	fputcn (termout);
    }
}

tbaud (cl)			/* set terminal (port 0) buad rate */
  char *cl;
{
    if (! r_set_baud (0, atoi (cl))) {
	fputs ("Could not set terminal baud ", termout);
	fputi (atoi (cl), termout);
	fputcn (termout);
    }
}

/*--------------------- Print state */
/* print out all state for a given process */
r_print_state (id)
  int id;
{
    PR_STATE *pr;
    if (bad_id (id)) {
	fputs ("Invalid id: ", termout); fputx (id, termout);
	fputcn(termout);
	return (-1);
    }

    pr = &proc_state[id];
    fputs ("\nid: ", termout);  fputx (pr->id, termout);
    fputs ("  prty: ", termout);  fputi (pr->prty, termout);
    fputs ("  addr: ", termout);  fputx (pr->address, termout);
    fputs ("  stat: ", termout);  fputx (pr->status, termout);
    fputs ("\nstack top: ", termout); fputx (pr->stk_top, termout);
    fputs (" stk size: ", termout); fputi (pr->stk_sz, termout);
    fputcn(termout);
    r_print_wait(id);
    r_print_regs(id);
}

r_print_regs (id)
  int id;
{
    PR_STATE *pr;
    long *reg;

    if (bad_id (id)) {
	fputs ("Invalid id: ", termout); fputx (id, termout);
	fputcn(termout);
	return (-1);
    }

    pr = &proc_state[id];
    fputs ("pc = ", termout);     fputsx (pr->pc, 6, termout);
    fputs ("  sr = ", termout);   fputsx (pr->s_reg, 4, termout);
    fputcn(termout);

    reg = &pr->reg[0];
    fputs ("d0 = ", termout);   	fputsx (*reg++, 8, termout);
    fputs ("  d1 = ", termout); 	fputsx (*reg++, 8, termout);
    fputs ("  d2 = ", termout); 	fputsx (*reg++, 8, termout);
    fputs ("  d3 = ", termout); 	fputsx (*reg++, 8, termout);
    fputs ("\nd4 = ", termout); 	fputsx (*reg++, 8, termout);
    fputs ("  d5 = ", termout); 	fputsx (*reg++, 8, termout);
    fputs ("  d6 = ", termout); 	fputsx (*reg++, 8, termout);
    fputs ("  d7 = ", termout); 	fputsx (*reg++, 8, termout);
    fputs ("\na0 = ", termout); 	fputsx (*reg++, 8, termout);
    fputs ("  a1 = ", termout); 	fputsx (*reg++, 8, termout);
    fputs ("  a2 = ", termout); 	fputsx (*reg++, 8, termout);
    fputs ("  a3 = ", termout); 	fputsx (*reg++, 8, termout);
    fputs ("\na4 = ", termout); 	fputsx (*reg++, 8, termout);
    fputs ("  a5 = ", termout); 	fputsx (*reg++, 8, termout);
    fputs ("  a6 = ", termout); 	fputsx (*reg++, 8, termout);
    fputs ("  a7 = ", termout); 	fputsx (*reg++, 8, termout);
    fputcn(termout);
    return (0);
}

/*---------------------- Print wait */
/* print what we are waiting on. */
r_print_wait (id)
  int id;
{
    PR_STATE *pr;

    pr = &proc_state[id];
    fputs ("wait: ", termout);  fputi (pr->wait, termout);
    fputs ("  Btype: ", termout);	fputx (pr->block_type, termout);
    fputs ("  Nbox: ", termout);	fputx (pr->num_boxes, termout);
    fputs ("\nmlb0: ", termout);	fputx (pr->mailbox[0], termout);
    fputs (" \tmlb1: ", termout);	fputx (pr->mailbox[1], termout);
    fputs (" \tmlb2: ", termout);	fputx (pr->mailbox[2], termout);
    fputs (" \tmlb3: ", termout); 	fputx (pr->mailbox[3], termout);
    fputs ("\nblk0: ", termout); 	fputi (pr->block[0], termout);
    fputs (" \tblk1: ", termout); 	fputi (pr->block[1], termout);
    fputs (" \tblk2: ", termout); 	fputi (pr->block[2], termout);
    fputs (" \tblk3: ", termout); 	fputi (pr->block[3], termout);
    fputcn(termout);
}

get_pid (s)			/* get a valid pid from a string */
  char *s;
{
    int pr;

    pr = atox (s);
    if (bad_id (pr)) {
	fputs ("Invalid id: ", termout); fputx (pr);
	return -1;
    }
    return pr;
}

/*
 * wait_pid (cl)
 *
 *   Force the shell process to wait for some process that is
 * currently running.  This is useful when breaking out of a
 * process to access the shell commands, and then returning
 * to the original process (e.g. so that keyboard input goes to
 * that process instead of the shell).
 *
 */

wait_pid (cl)
     char *cl;
{
  int pid;

  if ((pid = get_pid (cl)) < 0) return -1;
  if(proc_state[pid].status)
    {				/* Legal proc */
      if(pid == 0)		/* Don't allow wait on shell */
	{
	  fputs("Can't wait on the shell process!\n", termout);
	}
      else
	wait(pid);
    }
  else
    fputs("Invalid PID\n", termout);
}
