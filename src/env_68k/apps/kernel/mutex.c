/*
 * mutex.c
 *
 * Mutex simples com fila de espera FIFO, pensado pra se encaixar no
 * modelo de scheduler que voce ja tem (TCB com 'sp' no offset 0,
 * current_task, OS_Schedule).
 *
 * IMPORTANTE - PREMISSAS QUE VOCE PRECISA AJUSTAR:
 *
 *  1) Isso pressupoe que seu TCB ganhe um novo campo (estado da task:
 *     PRONTA, RODANDO, BLOQUEADA) e um ponteiro 'next' pra formar
 *     filas encadeadas (fila de prontos E fila de espera do mutex sao
 *     a MESMA estrutura de lista, so' que listas diferentes).
 *
 *  2) As regioes marcadas como CRITICO (Lock_CriticalEnter/Exit) usam
 *     a tecnica que voce ja decidiu usar hoje -- desabilitar
 *     interrupcoes -- so' que aqui elas protegem operacoes MUITO
 *     curtas (poucas instrucoes: testar+marcar uma flag, mexer numa
 *     lista encadeada), nao a operacao de disco inteira. E' essa a
 *     diferenca pratica entre "disable interrupts" cru e mutex: o
 *     mutex tambem usa disable-interrupts por baixo, mas so' pra
 *     proteger o proprio mecanismo de fila, nao o trabalho todo.
 *
 *  3) OS_Schedule() e' o que voce ja tem: escolhe a proxima task
 *     PRONTA e faz current_task apontar pra ela. Aqui eu assumo que
 *     existe uma forma de "ceder a CPU agora" (chamar o scheduler de
 *     dentro de codigo C, nao so' via interrupcao de timer) -- se seu
 *     scheduler hoje so' troca de contexto via OS_TickISR, isso e'
 *     uma peca que precisa existir tambem (uma especie de yield()
 *     manual). Deixei isso como funcao extern separada.
 */

#include <stdint.h>

/* --- coisas que devem vir do seu scheduler real, ajuste os nomes --- */
typedef struct TCB {
    uint32_t sp;          /* offset 0 -- obrigatorio pro ctx_switch.s */
    struct TCB *next;     /* proximo na fila (prontos OU espera de mutex) */
    int state;             /* TASK_READY, TASK_RUNNING, TASK_BLOCKED */
} TCB;

#define TASK_READY    0
#define TASK_RUNNING  1
#define TASK_BLOCKED  2

extern TCB *current_task;
extern void OS_Schedule(void);       /* ja existe no seu kernel */
extern void OS_Yield(void);          /* CEDE a CPU agora mesmo, sem
                                         esperar o timer -- se ainda nao
                                         existir, e' facil: so' chama
                                         OS_Schedule() e forca uma troca
                                         de contexto software (trap ou
                                         jsr pro mesmo lugar que
                                         OS_TickISR usa) */
extern void Scheduler_AddReady(TCB *t); /* insere t na fila de prontos */

/* --- regiao critica curta: protege so' o mecanismo do mutex em si --- */
static uint16_t Lock_CriticalEnter(void) {
    uint16_t old_sr;
    __asm__ volatile (
        "move.w %%sr,%0\n\t"
        "ori.w  #0x0700,%%sr"
        : "=d"(old_sr) :: "memory"
    );
    return old_sr;
}
static void Lock_CriticalExit(uint16_t old_sr) {
    __asm__ volatile ("move.w %0,%%sr" :: "d"(old_sr) : "memory");
}

/* --- o mutex em si --- */
typedef struct {
    int    locked;      /* 0 = livre, 1 = travado */
    TCB   *owner;        /* quem esta segurando (util p/ debug/deadlock) */
    TCB   *wait_head;    /* fila de quem esta esperando, FIFO */
    TCB   *wait_tail;
} Mutex;

void Mutex_Init(Mutex *m) {
    m->locked = 0;
    m->owner = NULL;
    m->wait_head = m->wait_tail = NULL;
}

void Mutex_Lock(Mutex *m) {
    uint16_t sr = Lock_CriticalEnter();

    if (!m->locked) {
        /* caminho feliz: ninguem segurando, pega na hora */
        m->locked = 1;
        m->owner = current_task;
        Lock_CriticalExit(sr);
        return;
    }

    /* mutex ja travado por outra task -- entra na fila de espera */
    TCB *me = current_task;
    me->next = NULL;
    if (m->wait_tail) {
        m->wait_tail->next = me;
        m->wait_tail = me;
    } else {
        m->wait_head = m->wait_tail = me;
    }
    me->state = TASK_BLOCKED;

    Lock_CriticalExit(sr);

    /* cede a CPU AGORA -- quando essa task for escolhida de novo pelo
     * scheduler, e' porque Mutex_Unlock() ja deu o mutex pra ela e ja
     * marcou ela como TASK_READY. Ao "acordar" daqui, ela ja e' a
     * dona do mutex, nao precisa tentar de novo. */
    OS_Yield();
}

void Mutex_Unlock(Mutex *m) {
    uint16_t sr = Lock_CriticalEnter();

    if (m->wait_head) {
        /* tem gente esperando: passa o mutex direto pro primeiro da
         * fila, sem nunca marcar 'locked = 0' no meio do caminho --
         * assim ninguem mais consegue "furar a fila" entre o unlock
         * e o proximo lock. */
        TCB *next_owner = m->wait_head;
        m->wait_head = next_owner->next;
        if (!m->wait_head) m->wait_tail = NULL;

        m->owner = next_owner;
        next_owner->state = TASK_READY;
        Scheduler_AddReady(next_owner);
        /* m->locked continua 1 -- so' mudou o dono */
    } else {
        /* ninguem esperando: libera de verdade */
        m->locked = 0;
        m->owner = NULL;
    }

    Lock_CriticalExit(sr);
}

/*
 * Exemplo de uso -- protegendo o driver ATA que discutimos:
 *
 *   static Mutex disk_mutex;
 *   Mutex_Init(&disk_mutex);   // uma vez, no boot
 *
 *   void diskread(...) {
 *       Mutex_Lock(&disk_mutex);
 *       // ... programa comando, faz o loop PIO, le os dados ...
 *       // pode ser preemptada pelo timer no meio, sem problema --
 *       // o mutex continua "segurado" mesmo com a task fora da CPU
 *       Mutex_Unlock(&disk_mutex);
 *   }
 *
 * Se a Task B chamar diskread() enquanto A ainda esta dentro,
 * Mutex_Lock() da Task B bloqueia ela e devolve a CPU pra outras
 * tasks prontas -- diferente do disable-interrupts puro, o resto do
 * sistema (RTC, teclado, outras tasks que NAO mexem no disco)
 * continua respondendo normalmente enquanto isso.
 */
