// Converte uma string inteira para minúsculas
void tolower_str(char *str) {
    while (*str) {
        if (*str >= 'A' && *str <= 'Z') {
            *str += ('a' - 'A'); // Soma 32 para converter
        }
        str++;
    }
}


int tolower(int c) {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A'); // Ou c + 32, que dá o mesmo resultado na tabela ASCII
    }
    return c;
}
