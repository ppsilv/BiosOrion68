#include <stddef.h>
#include <string.h>

char *strchr(char *str, int ch)
{
	for (; *str; str++) {
		if (*str == ch)
			return str;
	}
	return NULL;
}

