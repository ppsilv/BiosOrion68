#ifndef OS_SCHED_H
#define OS_SCHED_H

#include <stdint.h>

#define OS_MAX_TASKS   8

typedef enum {
    TASK_UNUSED = 0,
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED     /* esperando algo externo (I/O, evento) -- fora da fila de prontas */
} task_state_t;

/*
 * IMPORTANTE: 'sp' TEM que ser o primeiro campo da struct.
 * O assembly de context switch acessa o stack pointer salvo
 * fazendo move.l (a0), a7 -- ou seja, ele assume offset 0.
 * Se voce reordenar os campos, o assembly quebra silenciosamente.
 */
typedef struct tcb {
    uint32_t     *sp;          /* stack pointer salvo desta tarefa */
    task_state_t  state;
    struct tcb   *next;        /* proxima tarefa na fila circular de PRONTAS (NULL se nao estiver na fila) */
    void         *arg;         /* argumento da tarefa, lido via OS_TaskArg() */
    uint8_t      *stack_base;
    uint32_t      stack_size;
    int           id;
} tcb_t;

/* current_task precisa ser global e visivel pro assembly (ctx_switch_*.s usa .extern current_task) */
extern tcb_t *current_task;

void   OS_Init(void);

/*
 * Cria uma tarefa. 'entry' NAO recebe argumentos (void entry(void)) --
 * de proposito, pra nao depender de detalhes da ABI de passagem de
 * parametro do seu compilador. Dentro da tarefa, leia o argumento com
 * OS_TaskArg(). A tarefa ja entra na fila de prontas.
 */
tcb_t *OS_TaskCreate(void (*entry)(void), void *arg,
                      uint8_t *stack, uint32_t stack_size, int id);

/*
 * Igual a OS_TaskCreate, mas tambem preseta o registrador A5 antes
 * da tarefa comecar a rodar. Feito pra programas PIC (-msep-data):
 * A5 precisa apontar pro endereco real onde o .data do programa foi
 * carregado -- ver elf_pic_loader.c.
 */
tcb_t *OS_TaskCreateWithA5(void (*entry)(void), void *arg, uint32_t a5_value,
                            uint8_t *stack, uint32_t stack_size, int id);

void   OS_Start(void);        /* nunca retorna -- entra na primeira tarefa pronta */
void   OS_Schedule(void);     /* chamado pelo OS_TickISR (asm) a cada tick de timer */
void   OS_TaskExit(void);     /* para onde uma tarefa cai se a funcao dela retornar */
void  *OS_TaskArg(void);      /* le o 'arg' da tarefa atualmente rodando */

/*
 * OS_TaskBlock: tira 'task' da fila de prontas. Ela para de receber
 * tempo de CPU ate alguem chamar OS_TaskWake pra ela. Pensada pra ser
 * chamada por um driver (serial, ATA/FatFS) quando uma tarefa fica
 * esperando um evento externo -- assim o scheduler nem perde tempo
 * escalando ela enquanto ela nao tem nada pra fazer.
 *
 * LIMITACAO ATUAL: se a propria tarefa chamar isso em si mesma
 * (bloqueio voluntario), ela so vai realmente ceder a CPU no PROXIMO
 * tick do timer -- ainda nao existe um OS_Yield/trap pra forcar troca
 * de contexto imediata. Funciona, so nao e' instantaneo.
 */
void   OS_TaskBlock(tcb_t *task);

/* OS_TaskWake: devolve 'task' pra fila de prontas (ex: driver recebeu o dado esperado). */
void   OS_TaskWake(tcb_t *task);

#endif
