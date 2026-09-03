
char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while (*src) {
        *d++ = *src++;
    }
    *d = '\0'; // Garante o caractere nulo no final
    return dest;
}
