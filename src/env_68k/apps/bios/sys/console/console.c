#include "io.h"

// Inicializa o ponteiro global apontando para NULL (ou para uma função padrão)
getchar_func cconin  = 0;
putchar_func cconout = 0;

// Função para mudar a saída em tempo de execução
void set_console_output(putchar_func nova_saida) {
    if (nova_saida != 0) {
        cconout = nova_saida;
    }
}
void set_console_input(getchar_func nova_entrada){
    if (nova_entrada != 0) {
        cconin = nova_entrada;
    }
}
void enable_interrupts(){
    __asm__ __volatile__ ("move.w #0x2000, %sr");
}
/*
// Uma função de print genérica que não quer saber quem é o hardware
void print_string(const char *str) {
    while (*str) {
        // Se o ponteiro for válido, chama a função para a qual ele aponta!
        if (cconout) {
            cconout(*str); 
        }
        str++;
    }
}
*/