#include <stdlib.h>
#include <ctype.h>

double strtod(const char *nptr, char **endptr) {
    const char *p = nptr;
    while (isspace(*p)) p++;

    int sign = 1;
    if (*p == '-') { sign = -1; p++; }
    else if (*p == '+') { p++; }

    double val = 0.0;
    while (isdigit(*p)) {
        val = val * 10.0 + (*p - '0');
        p++;
    }

    if (*p == '.') {
        p++;
        double frac = 0.1;
        while (isdigit(*p)) {
            val += (*p - '0') * frac;
            frac *= 0.1;
            p++;
        }
    }

    if (endptr) *endptr = (char *)p;
    return val * sign;
}

// atof: Versão simplificada que não retorna o endptr
double atof(const char *nptr) {
    return strtod(nptr, NULL);
}

// strtof: Converte para float (simples precisão)
float strtof(const char *nptr, char **endptr) {
    return (float)strtod(nptr, endptr);
}

// strtold: Converte para long double
long double strtold(const char *nptr, char **endptr) {
    return (long double)strtod(nptr, endptr);
}
