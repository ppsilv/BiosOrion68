|
| call_entry_with_stack.s
|
| int call_entry_with_stack(int (*entry)(int, char**), int argc,
|                            char **argv, uint32_t new_sp,
|                            uint32_t a5_value);
|
| Chama 'entry(argc, argv)' rodando numa pilha PROPRIA (new_sp),
| separada da pilha de quem chamou, com A5 = a5_value (base de
| dados/GOT). Restaura tudo antes de retornar -- depois desta
| chamada, a pilha de quem chamou (kernel/teste) volta ao normal.
|
| Parametros empilhados pelo chamador (ABI padrao, sem -mrtd):
|   8(a6)  = entry
|  12(a6)  = argc
|  16(a6)  = argv
|  20(a6)  = new_sp
|  24(a6)  = a5_value
|
    .global call_entry_with_stack

call_entry_with_stack:
    link    %a6,#0
    movem.l %d2/%a2-%a5,-(%a7)     | preserva registradores que vamos usar

    move.l  8(%a6),%a2              | a2 = entry
    move.l  12(%a6),%d2             | d2 = argc
    move.l  16(%a6),%a3             | a3 = argv
    move.l  20(%a6),%a4             | a4 = new_sp (deve ser par/word-aligned)
    move.l  24(%a6),%a5             | a5 = a5_value -- FICA setado durante toda a chamada

    move.l  %a7,%a1                 | a1 = pilha atual (do kernel) -- guardada num registrador, NAO na pilha
    move.l  %a4,%a7                 | troca pra pilha nova, dentro do slot

    move.l  %a3,-(%a7)              | empilha argv
    move.l  %d2,-(%a7)              | empilha argc
    jsr     (%a2)                   | chama entry(argc, argv) -- retorno vem em d0
    addq.l  #8,%a7                  | desempilha argc/argv (convencao: quem chama limpa)

    move.l  %a1,%a7                 | restaura a pilha do kernel -- A PARTIR DAQUI e' seguro usar a7 de novo

    movem.l (%a7)+,%d2/%a2-%a5
    unlk    %a6
    rts
