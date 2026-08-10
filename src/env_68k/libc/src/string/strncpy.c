
char *strncpy(char *dest, const char *src, int n) {
    char *d = dest;
    while (n--){
        *d++ = *src++;
    }
    return dest;
}
