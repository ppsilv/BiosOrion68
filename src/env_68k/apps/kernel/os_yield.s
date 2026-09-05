|
| OS_Yield -- cede a CPU voluntariamente, sem esperar o timer.
|
| A sacada: a instrucao TRAP #n gera uma excecao de software que a
| CPU trata EXATAMENTE como uma interrupcao de hardware -- ela empilha
| SR (2 bytes) e PC (4 bytes) sozinha, e desvia pro vetor
| correspondente (vetor 32+n da tabela de excecoes). E' o MESMO
| formato de frame que OS_TickISR ja usa (por isso 'rte' funciona
| igual nos dois).
|
| Ou seja: OS_YieldTrap e' quase uma copia de OS_TickISR -- a unica
| diferenca e' COMO se chegou ali (TRAP explicito no codigo, em vez de
| interrupcao de hardware do timer).
|
| PRECISA REGISTRAR NO BOOT: o vetor de excecao 32+15 = 47 (TRAP #15)
| tem que apontar pra OS_YieldTrap na sua tabela de vetores. Se voce
| ja usa alguma TRAP #15 pra outra coisa, troque o numero -- qualquer
| TRAP #0-#15 livre serve, so' precisa ser consistente entre aqui e a
| tabela de vetores.
|
.global OS_Yield
.global OS_YieldTrap
.extern current_task
.extern OS_Schedule

|
| OS_Yield -- chamada NORMAL de C (extern void OS_Yield(void);)
| Ela mesma nao faz o trabalho -- so' dispara o TRAP, que e' quem
| realmente troca de contexto. Quando essa task for escolhida de novo
| pelo scheduler, a execucao volta a partir do 'rts' abaixo, como se
| nada tivesse acontecido (do ponto de vista da task, e' so' uma
| chamada de funcao normal que "demorou" um pouco).
|
OS_Yield:
    trap    #15
    rts

|
| OS_YieldTrap -- o handler de verdade, apontado pelo vetor 47.
| Estrutura identica ao OS_TickISR que voce ja tem.
|
OS_YieldTrap:
    movem.l %D0-%D7/%A0-%A6, -(%A7)   | salva contexto da task que cedeu
    move.l  current_task,%A0
    move.l  %A7, (%A0)                 | tcb->sp = %A7

    jsr     OS_Schedule               | escolhe a proxima task pronta

    move.l  current_task,%A0
    move.l  (%A0),%A7                 | %A7 = tcb->sp da task escolhida
    movem.l (%A7)+, %D0-%D7/%A0-%A6     | restaura contexto dela
    rte                                | PC+SR da task escolhida, retoma ela
