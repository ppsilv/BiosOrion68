#include <stdio.h>
#include "system.h"
#include "btpulib.h"
#include "68332.h"
#include "68332new.h"



/*
  * init_expanded_memory_B()
  *
  *  Initialize expanded memory version B (for use with Cypress
  * CYM1836)
  *
  *  Chip selects 8 and 9: odd and even fields of a 256K block
  * starting at SRAM_BASE_B_1.
  *
  *  Chip selects 6 and 7: odd and even fields of a 256K block
  * starting at SRAM_BASE_B_2.
  *
  */

init_expanded_memory_B()
{
  short temp;

/* DEFINE CS6 AND CS7 TO ACCESS BANK2 OF EXTERNAL MEMORY */ 
  temp = SRAM_BASE_B_2 >> 8;
  temp |= RAM_256K;                      /* MAKE BLOCK SIZE 256K */
  outp(CSBAR7,temp);
  outp(CSBAR6,temp);

/* SET OPTION REGISTER */
  temp = 0x3A30;
  outp(CSOR7,temp);
  temp = 0x5A30;
  outp(CSOR6,temp);

/* DEFINE CS8 AND CS9 TO ACCESS BANK 1 OF EXTERNAL MEMORY */ 
  temp = SRAM_BASE_B_1 >> 8;
  temp |= RAM_256K;                       /* MAKE BLOCK SIZE 256K */
  outp(CSBAR9,temp);
  outp(CSBAR8,temp);

/* SET OPTION REGISTER */
  temp = 0x3A30;
  outp(CSOR9,temp);
  temp = 0x5A30;
  outp(CSOR8,temp);

}
