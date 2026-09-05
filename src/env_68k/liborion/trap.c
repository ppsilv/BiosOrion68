/*--------------------------------------------------------------------------
 *
 * ABSTRACT
 *	TRAP ()- Cause a trap
 *
 *
 *	trap (trap_no, trap_action, arg1, arg2, ... )
 *	    int trap_no;
 *	    int trap_action;		action within the trap.
 *	    int arg1, arg2, ... ;	arguments for the FUNCTION.
 *
 * RESULTS
 *    trap:
 *	Returns whatever the trapped routine does.
 *
 * ENVIRONMENT
 *
 *    Imports
 *	None.
 *
 *    Exports
 *	trap ()			user_traps.c
 *
 * DESIGN
 *	Force the correct trap using assembly.
 *
 *	The trap handler knows how to find the arguments that were
 *	passed to trap().
 *
 *
 *----------------------------------------------------------------------------
 */
/* Existing PopI traps use the fact that the trap_no is in the register. */

trap (trap_no, trap_action, arg1)
  register int trap_no;
{
    
    switch (trap_no)
      {
	case 0:  asm ("	trap	#0"); break;
	case 1:  asm ("	trap	#1"); break;
	case 2:  asm ("	trap	#2"); break;
	case 3:  asm ("	trap	#3"); break;
	case 4:  asm ("	trap	#4"); break;
	case 5:  asm ("	trap	#5"); break;
	case 6:  asm ("	trap	#6"); break;
	case 7:  asm ("	trap	#7"); break;
	case 8:  asm ("	trap	#8"); break;
	case 9:  asm ("	trap	#9"); break;
	case 10: asm ("	trap	#10"); break;
	case 11: asm ("	trap	#11"); break;
	case 12: asm ("	trap	#12"); break;
	case 13: asm ("	trap	#13"); break;
	case 14: asm ("	trap	#14"); break;
	case 15: asm ("	trap	#15"); break;
	default: force_print ("Invalid trap\r\n");
      }
    
    return;				/* returns whatever was in d0 */
}
