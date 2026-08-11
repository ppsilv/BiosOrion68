// Define um tipo chamado "putchar_func" que aceita um char
typedef void (*putchar_func)(char);
typedef unsigned int (*getchar_func)(void);

// Declara a variável global que vai apontar para a saída ativa
extern putchar_func cconout;
extern getchar_func cconin;

void print_string(const char *str);
// Declara as funções principais de entrada e saída do sistema
void set_console_input(getchar_func nova_entrada);
void set_console_output(putchar_func nova_saida);

unsigned char get_key();

