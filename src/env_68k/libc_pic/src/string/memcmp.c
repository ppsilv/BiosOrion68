
#include <stdio.h>
#include <stdlib.h>




int memcmp(const char *s1, const char *s2, int n) {
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;

    while (n > 0) {
        if (*p1 != *p2) {
            return (*p1 - *p2);
        }
        p1++;
        p2++;
        n--;
    }

    return 0; // Tudo igual!
}

