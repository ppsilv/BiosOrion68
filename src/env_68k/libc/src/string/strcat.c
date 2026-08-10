
#include <string.h>
char *strcat(char *dest, const char *src)
{
    char *ret = dest;

    // 1. Anda com o ponteiro dest até o final da string original
    while (*dest) {
        dest++;
    }

    // 2. Copia a string src incluindo o '\0' final
    while ((*dest++ = *src++)) {
        // Bloco vazio: a própria atribuição copia e testa o '\0'
    }

    return ret;
}


