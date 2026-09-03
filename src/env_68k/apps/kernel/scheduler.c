#include "scheduler.h"
#include <string.h>

/*
 * Defina OS_CPU_68010 na linha de compilacao (-DOS_CPU_68010) se for
 * linkar com ctx_switch_68010.s. Se nao definir, assume 68000 puro e
 * este arquivo deve ser linkado com ctx_switch_68000.s.
 */

tcb_t *current_task = NULL;

static tcb_t task_pool[OS_MAX_TASKS];   /* armazenamento fixo dos TCBs -- sem malloc */
static int   task_count = 0;

/*
 * ready_head: cabeca da fila circular de tarefas PRONTAS (TASK_READY
 * ou TASK_RUNNING). So essas tarefas estao encadeadas via 'next'.
 * Uma tarefa TASK_BLOCKED e' removida daqui e nao participa do
 * round robin ate ser desbloqueada.
 */
static tcb_t *ready_head = NULL;

extern void OS_StartHighRdy(void); /* implementada em ctx_switch_68000.s OU ctx_switch_68010.s */

/* insere 't' no fim da fila circular de prontas */
static void ready_insert(tcb_t *t)
{
    if (ready_head == NULL) {
        ready_head = t;
        t->next    = t;             /* fila de 1 elemento aponta pra si mesma */
        return;
    }
    tcb_t *tail = ready_head;
    while (tail->next != ready_head)
        tail = tail->next;
    tail->next = t;
    t->next    = ready_head;
}

/* remove 't' da fila circular de prontas, se ela estiver la */
static void ready_remove(tcb_t *t)
{
    if (ready_head == NULL)
        return;

    if (t->next == t) {
        /* 't' e' a unica tarefa da fila */
        if (ready_head == t)
            ready_head = NULL;
        t->next = NULL;
        return;
    }

    tcb_t *prev = ready_head;
    while (prev->next != t) {
        prev = prev->next;
        if (prev == ready_head) /* deu a volta inteira e nao achou -- 't' nao esta na fila */
            return;
    }

    prev->next = t->next;
    if (ready_head == t)
        ready_head = t->next;
    t->next = NULL;
}

void OS_Init(void)
{
    memset(task_pool, 0, sizeof(task_pool));
    task_count  = 0;
    current_task = NULL;
    ready_head   = NULL;
}

void *OS_TaskArg(void)
{
    return current_task ? current_task->arg : NULL;
}

/* escreve 4 bytes big-endian e devolve o novo topo da pilha (que cresce pra baixo) */
static uint8_t *push32(uint8_t *sp, uint32_t val)
{
    sp -= 4;
    sp[0] = (uint8_t)(val >> 24);
    sp[1] = (uint8_t)(val >> 16);
    sp[2] = (uint8_t)(val >> 8);
    sp[3] = (uint8_t)(val);
    return sp;
}

static uint8_t *push16(uint8_t *sp, uint16_t val)
{
    sp -= 2;
    sp[0] = (uint8_t)(val >> 8);
    sp[1] = (uint8_t)(val);
    return sp;
}

/*
 * Monta a pilha inicial da tarefa como se ela ja tivesse sido
 * interrompida uma vez pelo OS_TickISR -- o mesmo codigo de
 * restauracao (movem.l (a7)+, d0-d7/a0-a6 ; rte) funciona igual pra
 * tarefa nova e pra tarefa retomada.
 *
 *   68000 (OS_CPU_68010 NAO definido):
 *     [ D0..D7, A0..A6 ]   60 bytes
 *     [ SR ]                2 bytes
 *     [ PC ]                4 bytes
 *
 *   68010 (OS_CPU_68010 definido):
 *     [ D0..D7, A0..A6 ]   60 bytes
 *     [ SR ]                2 bytes
 *     [ PC ]                4 bytes
 *     [ Format/Vector ]     2 bytes  (0x0000 = formato 0, frame curto)
 */
tcb_t *OS_TaskCreate(void (*entry)(void), void *arg,
                      uint8_t *stack, uint32_t stack_size, int id)
{
    if (task_count >= OS_MAX_TASKS || entry == NULL || stack == NULL)
        return NULL;

    tcb_t *tcb = &task_pool[task_count++];
    tcb->stack_base = stack;
    tcb->stack_size = stack_size;
    tcb->arg        = arg;
    tcb->id         = id;
    tcb->next       = NULL;

    uint8_t *sp = stack + stack_size;
    sp = (uint8_t *)((uint32_t)sp & ~1u);   /* pilha alinhada em word, exigencia do 68k */

#ifdef OS_CPU_68010
    sp = push16(sp, 0x0000);                /* format/vector: formato 0 -- so o 68010 exige isso */
#endif
    sp = push32(sp, (uint32_t)entry);       /* PC = ponto de entrada da tarefa */
    sp = push16(sp, 0x2000);                /* SR: modo supervisor, mascara de interrupcao 0 (todas habilitadas) */

    for (int i = 14; i >= 0; i--)
        sp = push32(sp, 0);                 /* D0-D7, A0-A6 comecam zerados */

    tcb->sp    = (uint32_t *)sp;
    tcb->state = TASK_READY;
    ready_insert(tcb);

    return tcb;
}

void OS_TaskExit(void)
{
    if (current_task) {
        ready_remove(current_task);       /* sai do round robin de vez */
        current_task->state = TASK_UNUSED;
    }
    for (;;)
        ;   /* proxima interrupcao de timer escala outra tarefa; esta nunca mais volta */
}

void OS_Schedule(void)
{
    if (ready_head == NULL)
        return;   /* nenhuma tarefa pronta -- nao deveria acontecer em uso normal, mas nao trava */

    if (current_task && current_task->state == TASK_RUNNING)
        current_task->state = TASK_READY;

    current_task = ready_head;
    ready_head   = ready_head->next;   /* proxima chamada comeca dali -- efetiva o round robin */
    current_task->state = TASK_RUNNING;
}

void OS_Start(void)
{
    if (ready_head == NULL)
        return;   /* nenhuma tarefa criada ainda -- nada pra rodar */

    current_task = ready_head;
    ready_head   = ready_head->next;
    current_task->state = TASK_RUNNING;

    OS_StartHighRdy();   /* nao retorna */
}

void OS_TaskBlock(tcb_t *task)
{
    if (task == NULL)
        return;
    if (task->state != TASK_READY && task->state != TASK_RUNNING)
        return;   /* ja bloqueada ou nao existe -- nao faz nada */

    ready_remove(task);
    task->state = TASK_BLOCKED;

    /*
     * Se 'task' e' a que esta rodando agora (bloqueio voluntario), ela
     * so vai realmente sair da CPU no proximo tick -- ver aviso no
     * scheduler.h sobre a falta de um OS_Yield/trap pra reschedule
     * imediato.
     */
}

void OS_TaskWake(tcb_t *task)
{
    if (task == NULL || task->state != TASK_BLOCKED)
        return;

    task->state = TASK_READY;
    ready_insert(task);
}
