/* start-up program to begin execution of the Executive */
/* Dan Christian 6-28-87 */

 /*
 *
 *  Author             Date         Description
 *  --------------     --------     -----------------------------------------
 *  Andrew H. Fagg     02/19/95     Added EXPAND_B for expanded memory
 *                                   version B.
 *                                  
 *
 */

/*#define DEBUG  */
/*#define DEBUG2 */

#include <stdio.h>
#include "system.h"
#include <service.h>
#include <trap-service.h>
#include <Oserial.h>
#define HEAP_START 0x3000	/*0x0f000   /*  0x16000*/
#define HEAP_SIZE 0x2000	/*0x1000     /*  0x9fff */

extern r_boot();		/* in a_exec.s */
extern shell();			/* in shell.c */
extern int bus_error(), addr_error(), illegal(), zero(); /* in ihandler.s */
extern int chk_trap(), trapv(), privelege(), trace(), line1010();
extern int line1111(), trap_error(), ubug();
extern int time_service();	/* in time.c */

extern PR_STATE proc_state[];	/* in exec.c */
extern int end;			/* The end of BURTE. */

boot ()				/* must run in supervisor mode */
{
  int temp;

/*  mc332_init();  */
  tpu_init();			/* Configure TPU and other IO */

#ifdef EXPAND
  init_expanded_memory();
#endif
#ifdef EXPAND_B
  init_expanded_memory_B();
#endif
  force_print("booting ...\n\r");
  set_traps ();
  /* insure that old timers arent active */
  tm_init();
  serial_init ();
  pr_stats_init();
  /* hostin->status |= NONULL; */	/* kill incoming padding */
#ifdef DEBUG
  force_print("boot1\n\r");
#endif
    r_init();

#ifdef DEBUG
  force_print("boot2\n\r");
#endif

    r_reset_free();
#ifdef DEBUG
  force_print("boot3\n\r");
#endif
    r_init_free (HEAP_START, HEAP_SIZE);
#ifdef DEBUG
  force_print("boot4\n\r");
#endif
  make_rtable();
#ifdef DEBUG
  force_print("boot5\n\r");
#endif
    init_break ();		/* initialize breakpoints */
#ifdef DEBUG
  force_print("boot6\n\r");
#endif
    temp = r_create(shell, 10, 0); /* stack is default size */
#ifdef DEBUG
  force_hex(proc_state[0].stk_top);
  force_print(":");
  force_hex(proc_state[0].stk_sz);
  force_print("\r\n");
  force_print("boot7\n\r");
#endif
    r_start (temp);
#ifdef DEBUG
  force_print("boot8\n\r");
#endif
    r_lock (0x2000);
#ifdef DEBUG
  force_print("boot9\n\r");
#endif

  outst ("  \r\n-----------\r\nBURTE V2.12  \tbuilt " __DATE__ "\r\n\n");
#ifdef DEBUG2
  while(1); 
  force_print("boot10\n\r");
#endif
  outst ("(c) 1988 Dan Christian\r\n");
  outst ("MC68332 port by Andrew H. Fagg\r\n\n");
#ifdef EXPAND
  outst ("EXPANDED MEMORY VERSION (starts at ");
  outxt(SRAM_BASE);
  outst(")\r\n");
#endif
#ifdef EXPAND_B
  outst ("EXPANDED MEMORY VERSION B (starts at ");
  outxt(SRAM_BASE_B_1);
  outst(")\r\n");
#endif
  if ((unsigned int)boot > 0x60000)
    outst ("Rom version\r\n");
  outst("\n\r");
#ifdef DEBUG2
  force_print("boot11\n\r");
#endif
  outst("Free memory starts at ");
  outxt((0xFFFFF000 & (unsigned int) &end) + 0x1000);
  outst(".\n\r\n\r");
#ifdef DEBUG2
  force_print("boot12\n\r");
#endif
  outst ("Hey Burte!  Ya Ernie?\r\n");
/*    hostin = termin; hostout = termout; */
  r_boot();
#ifdef DEBUG2
  force_print("boot13\n\r");
#endif

}

restart ()			/* must run in supervisor mode */
{
    short i;

    r_lock (0x2700);
    burte_state = FROZEN;
    for (i=0; i< MAX_PROC; i++) { /* pause all active processes */
	if (proc_state[i].status)
	  proc_state[i].status |= PAUSED;
      }
    serial_init ();
    hostin->status |= NONULL;	/* kill incoming padding */
    make_rtable();
    set_traps ();
    r_start (0);		/* this should restart the shell */
    r_lock (0x2000);
    outst ("Restarting...\r\n"); flusht();
    r_boot();		/* never returns */
}

new_shell()			/* must run in supervisor mode */
{
    r_lock (0x2700);
    burte_state = FROZEN;

				/* unlock print buffers */
    io_files[0]->busy = io_files[1]->busy =
      io_files[2]->busy = io_files[3]->busy = FALSE;
    r_start (0);
    r_boot();			/* boot sets interupt level */
}

set_traps ()
{
    *SYS_VECT (0) = (long) 0x2000; /* initial stack used to be 2000 */
    *SYS_VECT (1) = (long) boot; /* boot */

    *SYS_VECT (2) = (long) bus_error;
    *SYS_VECT (3) = (long) addr_error;
    *SYS_VECT (4) = (long) illegal;
    *SYS_VECT (5) = (long) zero;
    *SYS_VECT (6) = (long) chk_trap;
    *SYS_VECT (7) = (long) trapv;
    *SYS_VECT (8) = (long) privelege;
    *SYS_VECT (9) = (long) trace;
    *SYS_VECT (10) = (long) line1010;
    *SYS_VECT (11) = (long) line1111;
    *SYS_VECT (15) = (long) trap_error;
    *SYS_VECT (24) = (long) trap_error;


    *INT_VECT (1) = (long) trap_error;
    *INT_VECT (2) = (long) trap_error;
    *INT_VECT (3) = (long) trap_error;
    *INT_VECT (4) = (long) ubug;
    /* int 5 is set by serial init */
    *INT_VECT (6) = (long) ubug;
    /* int 7 is set by timer init */
    *TRAP_VECT(9) = (long) r_trap;
    *TRAP_VECT(10) = (long) boot;
}


/*
 * pr_stats_init()
 *
 *  Initialize the process statictics data structures.
 *
 */

pr_stats_init()
{
  statistics_flag = FALSE;
  last_proc = -1;
}
