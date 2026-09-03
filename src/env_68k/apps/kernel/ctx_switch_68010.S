|
| ctx_switch_68010.s -- troca de contexto para CPU 68010
|
| Frame de excecao do 68010 (COM format/vector word extra):
|   SP+0: SR              (2 bytes)
|   SP+2: PC               (4 bytes)
|   SP+6: Format/Vector    (2 bytes)
|   -- total 8 bytes --
|
| IMPORTANTE: as instrucoes deste arquivo sao IDENTICAS as de
| ctx_switch_68000.s. Isso nao e' engano -- a instrucao `rte` do 68010
| ja sabe ler o format/vector word e tratar o frame de 8 bytes
| corretamente em hardware; o codigo que voce escreve nao precisa (e
| nem consegue) manipular isso na mao.
|
| A DIFERENCA REAL fica em scheduler.c: ao montar a pilha inicial de
| uma tarefa nova (OS_TaskCreate), pro 68010 e' preciso incluir o
| format/vector word (valor 0x0000, formato 0) alem de SR e PC --
| senao o `rte` vai ler lixo dali e pular pro lugar errado. Por isso
| o scheduler.c usa a macro OS_CPU_68010 pra decidir isso.
|
| Mantido em arquivo separado, como pedido, pra deixar claro qual
| variante voce esta linkando em cada build -- mas se um dia isso
| divergir (por exemplo se voce passar a usar frames longos/format
| diferente de excecao no 68010), e' aqui que a mudanca entraria.
|

    .global OS_StartHighRdy
    .global OS_TickISR

    .extern current_task
    .extern OS_Schedule

OS_StartHighRdy:
    move.l  current_task, a0       | a0 = ponteiro pro TCB atual
    move.l  (a0), a7               | a7 (SP) = tcb->sp (offset 0 da struct)
    movem.l (a7)+, d0-d7/a0-a6     | restaura D0-D7, A0-A6
    rte                            | le SR, PC e format/vector (8 bytes), salta pra tarefa

OS_TickISR:
    movem.l d0-d7/a0-a6, -(a7)     | salva contexto da tarefa interrompida
    move.l  current_task, a0
    move.l  a7, (a0)               | tcb->sp = a7

    jsr     OS_Schedule            | escolhe a proxima tarefa, atualiza current_task

    move.l  current_task, a0
    move.l  (a0), a7               | a7 = tcb->sp da tarefa escolhida
    movem.l (a7)+, d0-d7/a0-a6     | restaura contexto dela
    rte                            | volta a executar de onde ela parou
