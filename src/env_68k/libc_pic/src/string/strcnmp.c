




int strncmp(const char *s1, const char *s2, int n) {
    while (*s1 && (*s1 == *s2) && n--) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}
