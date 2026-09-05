/*
 *  tpu.c
 *
 *   Initialization routine for TPU registers.
 *
 *   Author             Date        Notes
 *   -----------------  -------  ---------------------------------
 *   Andrew H. Fagg     5/23/92  Original
 *
 */



#include <stdio.h>
#include "system.h"
#include "68332.h"
#include "68332new.h"

tpu_init()
{
  *TPU_TMCR = 0;           /*  Place into user mode.  */

				/* Port F is being used for compass IO. */
/*  *PEPAR = 0; /* Set all pins as PORTF pins instead of INT */

}
