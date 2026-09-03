#include <string.h>

void *memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen) {
    // Se a agulha for vazia, retorna o início do palheiro
    if (needlelen == 0) return (void *)haystack;

    // Se a agulha for maior que o palheiro, impossível encontrar
    if (needlelen > haystacklen) return NULL;

    const unsigned char *h = (const unsigned char *)haystack;
    const unsigned char *n = (const unsigned char *)needle;

    // Percorre o palheiro, parando onde a agulha não caberia mais
    for (size_t i = 0; i <= haystacklen - needlelen; i++) {
        // Verifica se a sequência bate
        if (memcmp(h + i, n, needlelen) == 0) {
            return (void *)(h + i);
        }
    }

    return NULL;
}
