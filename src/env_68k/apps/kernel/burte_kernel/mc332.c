
mc332_init()
{
  volatile short temp;
  
/* first make sure serial channel is idle */
/* wait for xmit data reg empty */

   do
     {
       temp = inp(SCSR) & TDRE_MSK;
     } while (temp == 0);

  /* then disable xmitter and receiver */
  outp(SCCR1,inp(SCCR1) & ~TE_RE_MSK);

  /* wait for transmitter to complete any transmission in progress */
  do 
    {
      temp = inp(SCSR) & TC_MSK;
    } while (temp == 0);

  /* now that serial channel is off we can update the system clock */
  /* and the baud rate circuitry for 15.991 MHz operation */
  outp(SYNCR,SET_16_MHZ);    /* set the clock */
  outp(SCCR0,BAUD_9600);     /* set the baud rate */

  /* finally, re-enable the serial channel */
  outp(SCCR1,inp(SCCR1) | TE_RE_MSK);
}
