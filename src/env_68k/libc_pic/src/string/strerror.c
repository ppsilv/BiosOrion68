#include <string.h>

char *strerror(int errnum) {
    // Implementação simplificada para fins demonstrativos
    static char *errors[] = {
        "Success",
        "Operation not permitted",
        "No such file or directory"
        // ... completaria com todos os códigos de erro do sistema
    };

    if (errnum < 0 || errnum >= 3) {
        return "Unknown error";
    }
    return errors[errnum];
}
