#define TM_STA   ((volatile unsigned char *)0xFFFF63) /* Status reg */
#define TM_CT1   ((volatile unsigned char *)0xFFFF65) /* Rd T1 Counter (MSB) */
#define TM_INT1 0x01

time_service ()
{
    unsigned char status, junk;

    status = *TM_STA;		/* get status */

    if (status & TM_INT1) junk = *TM_CT1; /* clear interupts */
}

