#include <stdint.h>

// Definição do tipo para as chamadas de sistema
typedef long (*sys_call_t)(void);

// Protótipos das suas funções de sistema
long sys_fput(long fd, char *buf);
long sys_getpid(void);
// ... outras funções

// Tabela de despacho direta (índice = número da syscall)
const sys_call_t sys_call_table[] = {
    (sys_call_t) sys_fput,      // Syscall 0
    (sys_call_t) sys_getpid,    // Syscall 1
    // ... adicione os ponteiros na ordem exata dos números de trap
};

#define NUM_SYSCALLS (sizeof(sys_call_table) / sizeof(sys_call_t))

// Função principal chamada pelo tratador de exceção do 68k
long kernel_syscall_dispatcher(long trap_num, long *stack_args) {
    if (trap_num < 0 || trap_num >= NUM_SYSCALLS) {
        // Tratar erro de syscall inválida
        return -1;
    }

    // Pega o ponteiro da função direto na tabela
    sys_call_t func = sys_call_table[trap_num];

    if (!func) {
        return -1;
    }

    // Executa diretamente. A própria assinatura da função em C
    // dita quantos argumentos ela consome da pilha/ponteiro.
    return func();
}
