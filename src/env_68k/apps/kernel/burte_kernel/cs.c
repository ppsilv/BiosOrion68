#include <stdio.h>
#include "system.h"
#include "btpulib.h"
#include "68332.h"
#include "68332new.h"

/*
 * init_expanded_memory()
 *
 *   Set up 128K memory starting at SRAM_BASE (0x20000 in this case).
 *
 */

init_expanded_memory()
{
  unsigned short temp;

  /* CS07 set up for RAM 16-bit port lower byte READ */
  /* Set CS base address and block size */

  temp = inp(CSPAR1);
/*
  temp &= ~CS06MSK; 
*/
  temp &= ~CS07MSK;
  temp &= ~CS08MSK;
  temp &= ~CS09MSK;
/*
  temp &= ~CS10MSK;
*/
  outp(CSPAR1, temp);

  temp = ((SRAM_BASE>>8) & CSBAR_MSK) | RAM_128K;
  outp(CSBAR7, temp);

  temp = RAM_RDWR_OPT1;
  outp(CSOR7, temp );

  temp = inp(CSPAR1);
  temp &= ~CS07MSK;
  temp |= CS07MSK_16BIT;
  outp(CSPAR1, temp);


  /* CS08 set up for RAM 16-bit port upper byte READ */
   /* Set CS base address and block size */

  temp = ((SRAM_BASE>>8) & CSBAR_MSK) | RAM_128K;
  outp(CSBAR8, temp);
  /* Set CS options  to read only from upper bytems  */
  temp = RAM_RDWR_OPT2;
  outp(CSOR8, temp);
  /* Activate chip select register */
  temp = inp(CSPAR1);
  temp &= ~CS08MSK;
  temp |= CS08MSK_16BIT;
  outp(CSPAR1, temp);

}

