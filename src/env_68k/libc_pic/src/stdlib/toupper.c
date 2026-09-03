



// Converte uma string inteira para maiúsculas
void toupper_str(char *str) {
    while (*str) {
        if (*str >= 'a' && *str <= 'z') {
            *str -= ('a' - 'A'); // Subtrai 32 para converter
        }
        str++;
    }
}

int toupper(int c) {
    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A'); // Ou c - 32, que puxa para a maiúscula equivalente na tabela ASCII
    }
    return c;
}
