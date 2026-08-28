#include <stddef.h>
#include <string.h>
/*
static inline char _search(const char *str2, char ch)
{
	for (size_t j = 0; str2[j]; j++) {
		if (str2[j] == ch)
			return 1;
	}
	return 0;
}

size_t strcspn(const char *str1, const char *str2)
{
	size_t i;

	for (i = 0; str1[i]; i++) {
		if (_search(str2, str1[i]))
			break;
	}
	return i;
}
*/
#include <stddef.h>

/**
 * strcspn - Versão otimizada para m68k
 *
 * Usa aritmética de ponteiros para melhor performance
 */
size_t strcspn(const char *s, const char *reject) {
    const char *p = s;
    const char *r;

    // Enquanto não chegamos ao fim da string
    while (*p) {
        // Verifica se o caractere atual está em 'reject'
        r = reject;
        while (*r) {
            if (*p == *r) {
                return (size_t)(p - s);  // Distância entre ponteiros
            }
            r++;
        }
        p++;
    }

    return (size_t)(p - s);  // Tamanho total da string
}
