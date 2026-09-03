|
| ctx_switch_68000.s -- troca de contexto para CPU 68000 PURO
|
| Frame de excecao do 68000 (SEM format/vector word):
|   SP+0: SR   (2 bytes)
|   SP+2: PC   (4 bytes)
|   -- total 6 bytes --
|
| Use este arquivo se seu alvo de build for o 68000. Para 68010,
| use ctx_switch_68010.s em vez deste (nao linke os dois juntos).
|

.global OS_StartHighRdy
.global OS_TickISR

.extern current_task
.extern OS_Schedule

OS_StartHighRdy:
    move.l  current_task,%A0       | %A0 = ponteiro pro TCB atual
    move.l  (%A0), %A7               | %A7 (SP) = tcb->sp (offset 0 da struct)
    movem.l (%A7)+, %D0-%D7/%A0-%A6     | restaura %D0-%D7, %A0-%A6
    rte                            | restaura PC e SR (6 bytes), salta pra tarefa

OS_TickISR:
    movem.l %D0-%D7/%A0-%A6, -(%A7)     | salva contexto da tarefa interrompida
    move.l  current_task, %A0
    move.l  %A7, (%A0)               | tcb->sp = %A7

    jsr     OS_Schedule            | escolhe a proxima tarefa, atualiza current_task

    move.l  current_task, %A0
    move.l  (%A0), %A7               | %A7 = tcb->sp da tarefa escolhida
    movem.l (%A7)+, %D0-%D7/%A0-%A6     | restaura contexto dela
    rte                            | volta a executar de onde ela parou
