#include <stddef.h>
#include <stdlib.h>
#include <string.h>
/*
char *strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *new = malloc(len);
    if (new == NULL) return NULL;
    return memcpy(new, s, len);
}
*/
char* strdup(const char* str)
{
	char* copy = NULL;

	if(str)
	{
		size_t len = strlen(str) + 1;

		if((copy = malloc(len)) == NULL)
		{
			return (NULL);
		}

		memcpy(copy, str, len);
	}

	return (copy);
}
