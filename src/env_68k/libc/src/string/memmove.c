#include <stddef.h>

void *memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    if (d > s && d < s + n) {
        // Sobreposição: copia de trás para frente
        d += n;
        s += n;
        while (n--) *--d = *--s;
    } else {
        // Sem sobreposição ou d <= s: copia normal
        while (n--) *d++ = *s++;
    }
    return dest;
}
