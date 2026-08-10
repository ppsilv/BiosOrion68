#include <stddef.h>

void *memccpy(void *dest, const void *src, int c, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    unsigned char target = (unsigned char)c;

    for (size_t i = 0; i < n; i++) {
        *d = *s;
        if (*s == target) {
            return (void *)(d + 1);
        }
        d++;
        s++;
    }
    return NULL;
}
