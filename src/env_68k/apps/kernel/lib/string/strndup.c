#include <stdlib.h>
#include <string.h>

char *strndup(const char *s, size_t n) {
    size_t len = strnlen(s, n);
    char *new = malloc(len + 1);
    if (new == NULL) return NULL;
    memcpy(new, s, len);
    new[len] = '\0';
    return new;
}
