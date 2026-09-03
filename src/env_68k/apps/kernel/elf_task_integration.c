/*
 * elf_task_integration.c
 *
 * Mostra a mudanca conceitual no load_elf_executable: em vez de
 *
 *     int (*entry)(int, char**) = (int (*)(int, char**))header.entry;
 *     int ret = (*entry)(argc, argv);   // bloqueia aqui ate o programa retornar
 *
 * o loader passa a CRIAR UMA TAREFA com entry point = header.entry, e
 * devolve o controle pro scheduler. O programa carregado roda como
 * mais uma tarefa round-robin, junto com o shell e outras tarefas do
 * kernel.
 *
 * LIMITACAO que ja discutimos antes: como nao ha MMU e o linker.ld do
 * seu programa carregavel fixa o endereco em 0x00082000, so pode
 * existir UMA tarefa "programa de usuario carregado" por vez ocupando
 * essa faixa de memoria -- mesmo com o scheduler rodando. Multitasking
 * aqui vale sobretudo pra tarefas do PROPRIO kernel/shell (que moram
 * em enderecos fixos, fora da USERRAM) rodarem concorrentemente com
 * o programa carregado. Rodar dois .elf carregados ao mesmo tempo
 * exigiria resolver relocacao/multiplas regioes de USERRAM, que e' um
 * problema separado.
 */

#include "scheduler.h"
#include "kernel.h"

/* empacota o que a tarefa vai precisar pra chamar entry(argc, argv) */
typedef struct {
    int    (*entry)(int, char **);
    int      argc;
    char   **argv;
    int      ret;   /* onde a tarefa guarda o valor de retorno, se alguem precisar consultar depois */
} elf_task_args_t;

static elf_task_args_t g_elf_args; /* simplificado pra 1 elf ativo por vez -- combina com a limitacao acima */

/*
 * Trampolim: e' esta funcao (sem argumentos) que vira o "entry" da
 * tarefa no scheduler. Ela le os dados reais via OS_TaskArg() e so
 * entao chama o entry point do ELF com a assinatura certa.
 */
static void ELF_TaskTrampoline(void)
{
    elf_task_args_t *a = (elf_task_args_t *)OS_TaskArg();
    a->ret = a->entry(a->argc, a->argv);
    OS_TaskExit();   /* a tarefa termina aqui -- nunca "retorna" pro chamador original */
}

/*
 * Substitui o trecho final de load_elf_executable(). Chame isso no
 * lugar de `entry(argc, argv)` direto, depois de validar o header e
 * carregar os segmentos PT_LOAD (essa parte nao muda nada).
 *
 * 'stack' e 'stack_size': a pilha que essa tarefa vai usar. Pode ser
 * uma regiao separada da USERRAM do programa (recomendado), alocada
 * pelo kernel/shell antes de chamar isso.
 */
int load_elf_as_task(int argc, char *argv[], uint32_t entry_addr,
                      uint8_t *task_stack, uint32_t task_stack_size)
{
    g_elf_args.entry = (int (*)(int, char **))entry_addr;
    g_elf_args.argc  = argc;
    g_elf_args.argv  = argv;

    tcb_t *t = OS_TaskCreate(ELF_TaskTrampoline, &g_elf_args,
                              task_stack, task_stack_size, /*id=*/1);
    if (t == NULL) {
        kprintf("Nao foi possivel criar tarefa pro programa ELF.\n");
        return -1;
    }

    /*
     * Repare: essa funcao NAO chama o entry point diretamente e nao
     * bloqueia esperando retorno. Quem chamou load_elf_as_task segue
     * seu fluxo normal -- o programa carregado so vai rodar de fato
     * quando o scheduler (via OS_TickISR) escalar essa tarefa pra
     * TASK_RUNNING pela primeira vez.
     */
    return 0;
}
