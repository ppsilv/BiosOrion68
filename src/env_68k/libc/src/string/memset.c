


void memset(void *s, int c, int n) {
    char *p = (char *)s;
    while (n--) *p++ = c;
}
