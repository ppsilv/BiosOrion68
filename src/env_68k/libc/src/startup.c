#include <stdlib.h>

/* Declarar a função main ORIGINAL */
extern int main(void);

void _main_wrapper(void) {
    int result = main();  // ✅ Agora com nome correto
    exit(result);
}
