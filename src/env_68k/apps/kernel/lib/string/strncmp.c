

#include <string.h>

int strncmp(const char *str1, const char *str2, size_t max)
{
	while (max) {
		if (*str1 > *str2)
			return 1;
		else if (*str1 < *str2)
			return -1;
		else if (*str1 == *str2 && *str1 == '\0')
			return 0;
		str1++;
		str2++;
		max--;
	}
	return 0;
}
/*
int strncmp(const char *s1, const char *s2, int n) {
    while (*s1 && (*s1 == *s2) && n--) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}
*/