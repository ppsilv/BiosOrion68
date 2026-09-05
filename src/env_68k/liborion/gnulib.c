/* Subroutines needed by GCC output code on some machines.  */
/* Compile this file with the Unix C compiler!  */
/* set up for popI/rex use */

union double_di { double d; int i[2]; };
union flt_or_int { int i; float f; };

#define HIGH 1
#define LOW 0

/*#ifdef L_fixunsdfdi*/
double
_fixunsdfdi (a)
     double a;
{
  union double_di u;
  u.i[LOW] = (unsigned int) a;
  u.i[HIGH] = 0;
  return u.d;
}
/*#endif

#ifdef L_fixdfdi*/
double
_fixdfdi (a)
     double a;
{
  union double_di u;
  u.i[LOW] = (int) a;
  u.i[HIGH] = (int) a < 0 ? -1 : 0;
  return u.d;
}
/*#endif

#ifdef L_floatsidf*/
double
_floatsidf (a)
     int a;
{
  return (double) a;
}
/*#endif

#ifdef L_floatdidf*/
double
_floatdidf (u)
     union double_di u;
{
  register double hi
    = ((double) u.i[HIGH]) * (double) 0x10000 * (double) 0x10000;
  register double low = (unsigned int) u.i[LOW];
  return hi + low;
}
/*#endif*/

#define INTIFY(FLOATVAL)  (intify.f = (FLOATVAL), intify.i)

/*#ifdef L_addsf3*/
int
_addsf3 (a, b)
     union flt_or_int a, b;
{
  union flt_or_int intify;
  return INTIFY (a.f + b.f);
}
/*#endif

#ifdef L_negsf2*/
int
_negsf2 (a)
     union flt_or_int a;
{
  union flt_or_int intify;
  return INTIFY (-a.f);
}
/*#endif

#ifdef L_subsf3*/
int
_subsf3 (a, b)
     union flt_or_int a, b;
{
  union flt_or_int intify;
  return INTIFY (a.f - b.f);
}
/*#endif

#ifdef L_cmpsf2*/
int
_cmpsf2 (a, b)
     union flt_or_int a, b;
{
  union flt_or_int intify;
  if (a.f > b.f)
    return 1;
  else if (a.f < b.f)
    return -1;
  return 0;
}
/*#endif

#ifdef L_mulsf3*/
int
_mulsf3 (a, b)
     union flt_or_int a, b;
{
  union flt_or_int intify;
  return INTIFY (a.f * b.f);
}
/*#endif

#ifdef L_divsf3*/
int
_divsf3 (a, b)
     union flt_or_int a, b;
{
  union flt_or_int intify;
  return INTIFY (a.f / b.f);
}
/*#endif

#ifdef L_truncdfsf2*/
int
_truncdfsf2 (a)
     double a;
{
  union flt_or_int intify;
  return INTIFY (a);
}
/*#endif

#ifdef L_extendsfdf2*/
double
_extendsfdf2 (a)
     union flt_or_int a;
{
  union flt_or_int intify;
  return a.f;
}
/*#endif*/
