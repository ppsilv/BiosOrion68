

#include <stdio.h>
#include <kprintf.h>
#include "scheduler.h"

/*
 * Teste isolado: duas tarefas bobas, cada uma so incrementando um
 * contador proprio e imprimindo de vez em quando. Se elas alternarem
 * no terminal, o context switch esta funcionando.
 *
 * Cada tarefa precisa da SUA PROPRIA pilha, alocada estaticamente
 * (sem malloc). O tamanho e' um chute razoavel pra uma tarefa simples
 * -- se sua tarefa usar buffers grandes na pilha (arrays locais, etc)
 * aumente esse valor. Estourar a pilha aqui e' silencioso (sem MMU),
 * entao se algo comportar estranho, suspeite disso primeiro.
 */
#define STACK_SIZE 1024

static uint8_t stack_task_a[STACK_SIZE];
static uint8_t stack_task_b[STACK_SIZE];

static void TaskA(void)
{
    uint32_t count = 0;
    for (;;) {
        count++;
        if ((count % 100000) == 0)
            kprintf("Tarefa A: %lu\n", (unsigned long)count);
    }
}

static void TaskB(void)
{
    uint32_t count = 0;
    for (;;) {
        count++;
        if ((count % 100000) == 0)
            kprintf("Tarefa B: %lu\n", (unsigned long)count);
    }
}

void run_scheduler_test(void)
{
    OS_Init();

    /* arg = NULL porque essas tarefas nao precisam de nenhum dado externo */
    OS_TaskCreate(TaskA, NULL, stack_task_a, STACK_SIZE, /*id=*/1);
    OS_TaskCreate(TaskB, NULL, stack_task_b, STACK_SIZE, /*id=*/2);

    OS_Start();   /* nunca retorna -- a partir daqui quem manda e' o OS_TickISR */

    /* nunca chega aqui */
}

extern void run_scheduler_test(void);

int main(){
    run_scheduler_test();
    return 0;
}