
#ifndef NULL
#define NULL 0
#endif


static int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

char *strtok(char *str, const char *delim) {
    static char *p = NULL;
    if (str != NULL) p = str;
    if (p == NULL) return NULL;

    char *start = p;
    while (*p) {
        int i = 0;
        while (delim[i]) {
            if (*p == delim[i]) {
                *p = '\0';
                p++;
                return start;
            }
            i++;
        }
        p++;
    }
    if (start == p) return NULL;
    return start;
}
