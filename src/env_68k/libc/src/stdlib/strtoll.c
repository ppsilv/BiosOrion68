#define TYPE signed long long
#define NAME strtoll

#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>

signed long long strtoll(const char *nptr, char **endptr, int base)
{
	return (TYPE) strntoumax(nptr, endptr, base, ~(size_t) 0);
}
