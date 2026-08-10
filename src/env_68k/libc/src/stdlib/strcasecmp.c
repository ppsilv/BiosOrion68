
/*
int strcasecmp(const char *s1, const char *s2) {
    unsigned char c1, c2;
    do {
        c1 = (unsigned char)tolower((unsigned char)*s1++);
        c2 = (unsigned char)tolower((unsigned char)*s2++);
    } while (c1 && c1 == c2);

    return c1 - c2;
}
*/

int strcasecmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        char c1 = (*s1 >= 'A' && *s1 <= 'Z') ? (*s1 + 32) : *s1;
        char c2 = (*s2 >= 'A' && *s2 <= 'Z') ? (*s2 + 32) : *s2;
        if (c1 != c2) return (int)c1 - (int)c2;
        s1++;
        s2++;
    }
    return (int)*s1 - (int)*s2;
}
