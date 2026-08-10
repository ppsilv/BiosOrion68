#define TYPE unsigned long long
#define NAME strtoull

#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>

TYPE NAME(const char *nptr, char **endptr, int base)
{
	return (TYPE) strntoumax(nptr, endptr, base, ~(size_t) 0);
}
