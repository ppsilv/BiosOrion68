extern double fabs(), floor(), ceil(), fmod(), ldexp(), frexp();
extern double sqrt(), hypot(), atof();
extern double sin(), cos(), tan(), asin(), acos(), atan(), atan2();
extern double exp(), log(), log10(), pow();
extern double sinh(), cosh(), tanh();
extern double j0(), j1(), jn(), y0(), y1(), yn();
extern double erfc(), erf();

extern double getdouble (), doublearg ();

#define HUGE	1.701411733192644270e38
#define HUGEINT	0x7FFFFFFF
#define	pi	3.1415926535897932384626434

#define DEG(r)	( (double) (r * 360.0) / (2.0 * pi) )
#define RAD(d)	( (double) (d * pi) / 180.0 )
