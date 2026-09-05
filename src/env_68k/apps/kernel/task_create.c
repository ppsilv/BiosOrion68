/*
 * task_create.c
 *
 * Cria uma task real a partir de um ELF PIC ja carregado na memoria
 * (task_base preenchido, GOT patcheada), montando na propria stack
 * dela o "frame falso" que OS_StartHighRdy espera encontrar.
 *
 * IMPORTANTE - AJUSTE ESSAS PREMISSAS PRO SEU CODIGO REAL ANTES DE USAR:
 *
 *   1) struct TCB       -> troque pelo layout real do seu scheduler.
 *                          O UNICO requisito duro (por causa do ctx_switch.s
 *                          que voce mostrou) e que o campo 'sp' seja o
 *                          PRIMEIRO membro da struct (offset 0), porque
 *                          OS_StartHighRdy faz:
 *                              move.l  current_task,%A0
 *                              move.l  (%A0), %A7    <- le offset 0 direto
 *
 *   2) SLOT_SIZE         -> troque pelo tamanho real do seu slot (voce
 *                          mencionou 256K = 0x40000).
 *
 *   3) A ORDEM exata que 'movem.l %D0-%D7/%A0-%A6,-(%A7)' empilha os
 *      registradores. Pela documentacao do 68k, movem com essa lista de
 *      registradores e modo predecremento empilha SEMPRE na ordem
 *      D0,D1,...,D7,A0,A1,...,A6 (independente da ordem escrita no
 *      opcode), com D0 ficando no ENDERECO MAIS ALTO (foi o ultimo a ser
 *      decrementado/gravado) e A6 no ENDERECO MAIS BAIXO (foi o primeiro).
 *      Ou seja, no layout em memoria, do menor endereco pro maior:
 *
 *          [SP mais baixo]  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0  [SP mais alto]
 *
 *      Isso e' exatamente o que este codigo monta abaixo. CONFIRA no seu
 *      assembler/manual se bate -- alguns assemblers/CPUs tem pegadinhas
 *      aqui, entao vale um teste isolado antes de confiar cegamente.
 *
 *   4) O valor inicial de SR (0x2000 = modo supervisor, interrupcoes
 *      habilitadas, sem trace) -- ajuste se seu kernel usa outra
 *      convencao (ex: rodar tasks de usuario em modo nao-supervisor).
 */

#include <stdint.h>
#include <string.h>
#include "slots.h"     /* Slots_Alloc, Slots_BaseAddr, Slots_Free */

/* ---- ajuste pro seu scheduler real ---- */
#define SLOT_SIZE 0x40000u   /* 256K -- confirme com seu slots.h */

typedef struct TCB {
    uint32_t sp;        /* OFFSET 0 -- obrigatorio, ctx_switch.s depende disso */
    struct TCB *next;   /* resto da struct: adapte pro seu scheduler real */
    int       state;
    /* ... demais campos do seu TCB real (prioridade, nome, etc) ... */
} TCB;

/* Indices dentro do bloco de 15 registradores (D0-D7,A0-A6), na ordem
 * de MEMORIA que o movem.l predecremento produz (ver premissa 3 acima).
 * index 0 = endereco mais baixo (A6) ... index 14 = endereco mais alto (D0)
 */
#define REGBLK_A6   0
#define REGBLK_A5   1
#define REGBLK_A4   2
#define REGBLK_A3   3
#define REGBLK_A2   4
#define REGBLK_A1   5
#define REGBLK_A0   6
#define REGBLK_D7   7
#define REGBLK_D6   8
#define REGBLK_D5   9
#define REGBLK_D4   10
#define REGBLK_D3   11
#define REGBLK_D2   12
#define REGBLK_D1   13
#define REGBLK_D0   14
#define REGBLK_NREGS 15

/*
 * Copia argc/argv para DENTRO do slot da task nova (nao pode confiar
 * que a memoria original do chamador (shell) continue viva/estavel --
 * ver discussao anterior sobre isso).
 *
 * Estrategia simples: escreve as strings e o array de ponteiros logo
 * ABAIXO de onde a stack vai comecar a crescer, reservando um espaco
 * fixo pra isso no topo do slot. Ajuste ARGS_AREA_SIZE se seus
 * programas usarem linhas de comando bem longas.
 */
#define ARGS_AREA_SIZE 512u  /* bytes reservados pra strings + argv[] */

static char *copy_args_into_slot(uint32_t area_top, int argc, char *argv[],
                                  char ***argv_copy_out)
{
    /* area_top = topo da regiao reservada pros argumentos (endereco alto) */
    uint32_t cursor = area_top;

    /* 1) reserva espaco pro array de ponteiros char*[argc+1] (NULL no fim) */
    cursor -= (uint32_t)(argc + 1) * sizeof(char *);
    cursor &= ~0x3u; /* alinhamento de 4 bytes */
    char **argv_copy = (char **)cursor;

    /* 2) copia cada string, de tras pra frente, e preenche argv_copy[i] */
    uint32_t str_cursor = cursor; /* strings ficam abaixo do array de ponteiros */
    for (int i = argc - 1; i >= 0; i--) {
        size_t len = strlen(argv[i]) + 1;
        str_cursor -= (uint32_t)len;
        memcpy((void *)str_cursor, argv[i], len);
        argv_copy[i] = (char *)str_cursor;
    }
    argv_copy[argc] = NULL;

    *argv_copy_out = argv_copy;
    return (char *)str_cursor; /* menor endereco usado -- pra checar overflow se quiser */
}

/*
 * Monta o frame inicial de uma task nova, no formato que
 * OS_StartHighRdy / OS_TickISR (ctx_switch.s) esperam encontrar,
 * e devolve o TCB pronto pra ser inserido na fila do scheduler.
 *
 * Parametros:
 *   task_base        - base do slot ja alocado (Slots_Alloc/Slots_BaseAddr)
 *   entry             - endereco de entrada DENTRO do slot (ja relativo,
 *                        sem somar task_base -- este codigo soma)
 *   got_addr_saved    - endereco da GOT relativo ao slot (idem load_pic_elf_standalone)
 *   argc, argv        - argumentos originais (serao copiados pro slot)
 *
 * Retorna um TCB* pronto (aloque com seu alocador real de TCBs -- aqui
 * uso malloc so' pra ilustrar, troque pelo seu).
 */
extern void *kmalloc(size_t size); /* troque pelo seu alocador real do kernel */

TCB *Task_CreateFromElf(uint32_t task_base, uint32_t entry,
                         uint32_t got_addr_saved, int argc, char *argv[])
{
    TCB *tcb = (TCB *)kmalloc(sizeof(TCB));
    if (!tcb)
        return NULL;

    uint32_t slot_top = task_base + SLOT_SIZE;

    /* --- 1) copia argv/strings pro topo do slot --- */
    uint32_t args_area_top = slot_top;
    uint32_t args_area_bottom = slot_top - ARGS_AREA_SIZE;
    char **argv_copy;
    copy_args_into_slot(args_area_top, argc, argv, &argv_copy);

    /* a partir daqui, a "stack de verdade" comeca logo abaixo da area
     * reservada pros argumentos, e cresce pra enderecos menores */
    uint32_t sp = args_area_bottom;
    sp &= ~0x3u; /* garante alinhamento de 4 bytes antes de empilhar */

    /* --- 2) empilha argv, argc e um "retorno" falso ---
     * ABI m68k padrao: argumentos empilhados da direita pra esquerda,
     * ultimo empilhado = primeiro argumento (fica no topo/menor offset
     * a partir do SP apos o "rts" que a entry() faria).
     *
     * Como quem vai "chamar" entry() e' o 'rte' do OS_StartHighRdy (nao
     * um jsr normal), precisamos deixar manualmente um endereco de
     * retorno na pilha, exatamente como um jsr teria deixado.
     */
    sp -= 4; *(uint32_t *)sp = (uint32_t)argv_copy;     /* argv */
    sp -= 4; *(uint32_t *)sp = (uint32_t)argc;          /* argc */
    sp -= 4; *(uint32_t *)sp = 0xDEADBEEFu;             /* endereco de retorno
                                                            fake -- entry() nunca
                                                            deveria dar 'rts' aqui;
                                                            se der, e' bug (ou
                                                            trate esse endereco
                                                            no seu handler de
                                                            excecao como
                                                            "task terminou") */

    /* --- 3) monta o frame de excecao (SR + PC) que 'rte' vai consumir --- */
    sp -= 6; /* 2 bytes SR + 4 bytes PC */
    uint16_t *sr_ptr = (uint16_t *)sp;
    uint32_t *pc_ptr = (uint32_t *)(sp + 2);
    *sr_ptr = 0x2000;                 /* modo supervisor, sem trace -- ajuste
                                          se suas tasks de usuario rodam em
                                          modo nao-supervisor */
    *pc_ptr = task_base + entry;      /* PC inicial = entry point da task */

    /* --- 4) monta o bloco de 15 registradores (D0-D7,A0-A6) que
     *        'movem.l (%A7)+, %D0-%D7/%A0-%A6' vai restaurar --- */
    sp -= REGBLK_NREGS * 4;
    uint32_t *regs = (uint32_t *)sp;
    memset(regs, 0, REGBLK_NREGS * 4);

    /* A5 = base da GOT desta task -- e' isso que resolve o problema do
     * PIC em multitarefa: a partir da primeira troca de contexto pra
     * FORA desta task, o valor certo de A5 ja fica salvo/restaurado
     * automaticamente pelo ctx_switch.s, sem nenhum truque extra. */
    regs[REGBLK_A5] = task_base + got_addr_saved;

    /* demais registradores ficam zerados -- isso e' seguro, o programa
     * nao deveria depender do valor inicial deles. */

    /* --- 5) SP final -- e' isso que fica gravado no TCB --- */
    tcb->sp = sp;
    tcb->next = NULL;
    tcb->state = 0; /* ex: TASK_READY -- ajuste pro seu enum real */

    return tcb;
}

/*
 * Exemplo de como plugar isso no load_pic_elf_standalone existente:
 * em vez de chamar call_with_a5() de forma sincrona no final da funcao,
 * troque por algo tipo:
 *
 *   TCB *new_task = Task_CreateFromElf(task_base, entry, got_addr_saved,
 *                                       argc, argv);
 *   if (!new_task) { kprintf("Falha criando task\n"); goto fail; }
 *   Scheduler_AddReady(new_task);   // insere na fila do seu OS_Schedule
 *   return 0; // loader nao espera mais o programa terminar
 *
 * Note que isso muda a semantica do loader: ele passa a ser
 * "fire and forget" (spawna e retorna na hora), nao mais bloqueante.
 * Se voce quiser MANTER o comportamento sincrono atual pra quando
 * multitarefa nao estiver em uso, vale deixar as duas opcoes
 * disponiveis (ex: um parametro 'int async' na funcao de load).
 */
