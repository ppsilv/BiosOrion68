/*
 * LSQRT()- C routine that obtains the square root of a 32-bit
 * integer. It uses the method of successive approximations.
 * Returns the best 16-bit result after two successive 
 * approximations are equal or separated by one.
 *
 * HISTORY
 * 03-Feb-82  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Trivial solution added.
 *
 * 22-Jan-82  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Created
 *
 */

lsqrt(a)
    register unsigned a;			/* d7 */
{
    register unsigned short last_try;		/* d6 */
    register unsigned short new_try;		/* d5 */
    register int diff;				/* d4 */
    register unsigned temp1;			/* d3 */
    register unsigned temp2;			/* d2 */

    if (!a) return(0);			/* Trivial solution */

    temp1 = a;
    asm("	divu	#200.,d3");
    last_try = (short) temp1;
    last_try += 2;			/* last_try = (a / 200) + 2 */

    while(1)
    {
	temp1 = a;
	asm("	divu	d6,d3");
	new_try = (short) temp1;		/* new_try = a / last_try */
	new_try = (last_try + new_try) >> 1;
	if(new_try == last_try)
	    break;
	if((diff = last_try - new_try) == 1)
	    break;
	last_try = new_try;
    }

    temp1 = new_try * new_try;
    new_try++;
    temp2 = new_try * new_try;
    if ( a - temp1 <= temp2 - a ) return (--new_try);
    else return (new_try);
}

