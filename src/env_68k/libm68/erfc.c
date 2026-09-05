/*--------------------------------------------------------------------------
 *
 * ABSTRACT
 *	ERFC () - Approximate the complementary error function.
 *	ERF () - Return (1 - erfc ()).
 *
 * SYNOPSIS
 *	#include <math68.h>
 *
 *	double erfc (x)
 *	    double x;
 *
 *	double erf (x)
 *
 * RESULTS
 *	Returns the approximated error function, using Gempel's
 *	approximation.
 *
 * SIDE EFFECTS
 *	None.
 *
 * ENVIRONMENT
 *
 *    Imports
 *	exp (),
 *	fabs ()			math library.
 *
 *    Exports
 *	erfc (),
 *	erf ()			erfc.c
 *
 * BUGS
 *	None known.
 *
 * DESIGN
 *	Grabbed from the FABRICS II statistical process simulator,
 *	in EE.
 *
 * HISTORY
 * 21-Oct-83  Rafael Bracho (rxb) at Carnegie-Mellon University
 *	Gotten from Steve Lammer, thanks.
 *
 *----------------------------------------------------------------------------
 */

#include <math68.h>

#define A0	(double) 0.646521414
#define A1	(double)-0.0794409573
#define A2	(double)-0.00035947897
#define A3	(double) 0.00163053676
#define A4	(double)-0.000144389904
#define	SQRT_PI	(double) 1.772453851

double
erfc (x)
    double x;
{
    double h, y, temp;

    if (fabs (x) < 6.0)
    {
	y = fabs (x);
	h = SQRT_PI * y +
	    exp(-y * (A0 + (A1 + (A2 + (A3 + A4 * y) * y) * y) * y));
	temp = exp (-y * y) / (2 * h);
	if (x > 0)
	    return (temp);
	else
	    return (1.0 - temp);
   }
   else
   {
	if (x < 0)
	    return (1.0);
	else
	    return (0);
   }
}

/* Function approximates the ERF function */

double
erf (x)
    double x;
{
    double erfc();

    return (1.0 - erfc (x));
}
