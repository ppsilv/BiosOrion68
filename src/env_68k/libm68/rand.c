 /*
 * RAND()- C routine that generates pseudo-random numbers.
 * It utilizes the linear congruential method which generates
 * the k-th random number x(k) based on the equation:
 *
 * 		x(k) = ( A * x(k-1) + C ) MOD M
 *
 * "M" is chosen to be a power of 2 (2**32) so the MOD function
 * is performed automatically by letting the other operations
 * overflow.
 * 
 * "A" should end with the bit pattern 101 (octal 5) and should not
 * exhibit any simple pattern. The pattern chosen was:
 *
 *   A = 11000110 10011110 01010011 01001101 = 3,332,264,781
 *
 * "C" should be an odd number as close as possible to:
 *
 *		Y = M * ( 1 - 1 / SQR(3) ) / 2
 *
 * For M = (2**32), Y = 907,633,385.7 then C = 907,633,385.
 * 
 * SRAND()- C routine that calls rand() after initializing (seeding)
 * the random number generator with <seed>.
 *
 * The numbers produced by these routines will range from 0 to
 * (2**31)-1, inclusive. They will have a period of 2**32.
 *
 * NRAND()- C routine that calls rand() and then normalizes the random
 * number so it lies in the range: [0 - N]. The normalization is done
 * by forming the integer part of the product: ((N + 1) * x(k))/(2**32).
 *
 * HISTORY
 * 15-Jan-82  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Improved so it is completely compatible with Unix rand(3).
 *
 * 14-Jan-82  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Created
 *
 */

#define	A_Ran	3332264781
#define C_Ran	907633385

static unsigned long random = 1;	/* Seed = 1 */

long rand()
{
    register unsigned long dummy; /* Variable to force logical shift */
    dummy = __rand();
    dummy >>= 1;
    return (dummy);		/* Return random [0 - ((2**31)-1)] */
}


long srand(seed)
    int seed;
{
    random = (long) seed;	/* seed the generator */
    return(rand());
}



long nrand(N)
    long N;
{
    return (__nrand (N, __setrng (N)));
}



/* 
 * The utilities that follow shouldn't be seen by the user.
 */



static int __setrng(N)
    register unsigned long N;
{
    register int shift;

    shift = 0;
    N++;
    while (N)
    {
	N >>= 1;		/* Make a LOGICAL right shift */
	shift++;		/* Count this bit */
    };
    return(shift);
}


static long __nrand(N,shift)
    register long N;
    register int shift;		/* number (n) of signif. bits in <N> */
{
    register unsigned long ran;

    ran = __rand();		/* Get a random number */
    ran >>= shift;		/* Avoid overflow */
    ran = ran * (++N);
    ran >>= (32 - shift);	/* Get the integer */
    return(ran);
}



static long __rand()
{
    random = __lmul(random,A_Ran);
    random += C_Ran;		/* random = A_Ran * random + C_Ran */
    return (random);
}



static long __lmul(a,b)
    long a, b;
{
    register long mltplier, mltcand, tmp1, tmp2, tmp3;

/* 
 * For the assembly language, we have: mltplier (d7), mltcand (d6),
 * tmp1 (d5), tmp2 (d4) and tmp3 (d3).
 */

    tmp3 = tmp2 = mltcand = a;
    tmp1 = mltplier = b;

    asm("	swap	d4");	/* Swap the halves of tmp2 */
    asm("	swap	d5");	/* as well as tmp1 */

/* 
 * Generate the four partial products
 */

    asm("	mulu	d7,d6");	/* pp#1 in d6 */
    asm("	mulu	d4,d7");	/* pp#2 in d7 */
    asm("	mulu	d5,d3");	/* pp#3 in d3 */
    asm("	mulu	d5,d4");	/* pp#4 in d4 */
    asm("	swap	d6");		/* sum#1 = pp#2(low)... */
    asm("	addw	d7,d6");	/* ... + pp#1(high) */
    asm("	clrl	d5");
    asm("	addxl	d5,d4");	/* propagate carry in pp#4 */
    asm("	addw	d3,d6");	/* sum#2 = sum#1 + pp#3(low) */
    asm("	addxl	d5,d4");	/* propagate carry in pp#4 */
    asm("	swap	d6");		/* low product in right order */
    asm("	clrw	d7");		/* Prepare for sum#3 */
    asm("	swap	d7");
    asm("	clrw	d3");
    asm("	swap	d3");
    asm("	addl	d3,d7");	/* sum#3 = pp#2(low) + pp#3(high) */
    asm("	addl	d4,d7");	/* sum#4 = sum#3 + pp#4 */

/*
 * At this point, mltcand (d6) and mltplier (d7) have the UNSIGNED
 * 64-bit result (mltplier has the most sign. 32 bits).
 *
 * Now let's modify the result to make it SIGNED.
 */

    if (b < 0) mltplier -= a;
    if (a < 0) mltplier -= b;

    return(mltcand);
}

