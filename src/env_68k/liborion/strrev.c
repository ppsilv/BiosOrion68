/*
 * Reverse a string.
 */

strrev (s)
    register char *s;
{
    register char *p, c;

    for (p = s + strlen (s) - 1 ; s < p ; p--)
    {
	c = *s;
	*s++ = *p;
	*p = c;
    }
}
