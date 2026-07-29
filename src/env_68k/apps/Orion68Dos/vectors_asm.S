/*
 * vectors.S - Tratadores de Exceção e Interrupção para o Orion68K
 */

.section .text
.align 2

// Exporta os pontos de entrada para o arquivo de vetores principal
.global SvcBusError, Svcaddress_err, SvcIllegalIns, SvcDiv0_handler
.global SvcChkIns, SvcTrapvIns, SvcPrivViolation, SvcTrace, SvcLineA, SvcLineF
.global bad_exception, SpuriousHandler, Int1Handler, Int2Handler, Int3Handler
.global Int4Handler, Int5Handler, Int6Handler, Int7Handler
.global Trap0Handler, Trap1Handler, Trap2Handler, Trap3Handler, Trap4Handler
.global Trap5Handler, Trap6Handler, Trap7Handler, Trap8Handler, Trap9Handler
.global TrapAHandler, TrapBHandler, TrapCHandler, TrapDHandler, TrapEHandler, TrapFHandler

// Importa os símbolos externos do C e outras partes do sistema
.extern TRAP14_Handler
.extern systemTick
.extern cconin
.extern cconout
.extern kb_put

/* =========================================================================
   1. EXCEÇÕES E INTERRUPÇÕES QUE NÃO FAZEM NADA (STUBS)
   ========================================================================= */
SvcBusError:
Svcaddress_err:
SvcIllegalIns:
SvcDiv0_handler:
SvcChkIns:
SvcTrapvIns:
SvcPrivViolation:
SvcTrace:
SvcLineA:
SvcLineF:
bad_exception:
SpuriousHandler:
Int1Handler:
Int4Handler:
Int5Handler:
Int6Handler:
Int7Handler:
Trap0Handler:
Trap2Handler:
Trap3Handler:
Trap4Handler:
Trap5Handler:
Trap6Handler:
Trap7Handler:
Trap8Handler:
Trap9Handler:
TrapAHandler:
TrapBHandler:
TrapCHandler:
TrapFHandler:
        rte         /* Só retorna sem quebrar nada */

/* =========================================================================
   2. TRAP 13 (0xD) - STUB DE SEGURANÇA
   ========================================================================= */
TrapDHandler:
        rte

/* =========================================================================
   3. INTERRUPÇÃO DE HARDWARE NÍVEL 2 - TIMER (SYSTEM TICK)
   ========================================================================= */
Int2Handler:
        /* Como mexe em registrador para somar na RAM, salva o D0 e A0 */
        move.l  %d0,-(%sp)
        move.l  %a0,-(%sp)

        /* Incrementa a variável de 32 bits do C */
        move.l  #systemTick,%a0
        addq.l  #1,(%a0)

        /* Restaura e sai com RTE */
        move.l  (%sp)+,%a0
        move.l  (%sp)+,%d0
        rte
Int3Handler:
        movem.l %d0-%d7/%a0-%a6,-(%sp)

        MOVE.B  0x00FF9113,%D0

        ANDI.L  #0x000000FF,%D0  /*; Limpa a parte alta (garante o unsigned)*/
        MOVE.L  %D0, -(%A7)       /*; Empilha 4 bytes (mantém A7 alinhado em par)*/
        
        jsr     kb_put
        
        ADDQ.L  #4,%A7          /*; Limpa os 4 bytes da pilha!*/

        movem.l (%sp)+,%d0-%d7/%a0-%a6
        rte

/* =========================================================================
   4. TRAP 1 (BIOS CONSOLE IO)
   ========================================================================= */
Trap1Handler:
        /* Salva o contexto da aplicação, mas DEIXA O D0 LIVRE para o retorno! */
        movem.l %d1-%d7/%a0-%a6,-(%sp)

        /* Avalia a função passada em D0 */
        cmp.w   #1,%d0
        beq     .trap1_cconin
        cmp.w   #2,%d0
        beq     .trap1_cconout
        bra     .trap1_exit

.trap1_cconin:
        /* Verifica se o ponteiro da função cconin não é nulo */
        move.l  #cconin,%a0
        move.l  (%a0),%d1
        tst.l   %d1
        beq     .trap1_exit
        
        /* Chama a função em C. O retorno virá em D0 naturalmente */
        move.l  %d1,%a0
        jsr     (%a0)
        bra     .trap1_exit

.trap1_cconout:
        /* Verifica se o ponteiro da função cconout não é nulo */
        move.l  #cconout,%a0
        move.l  (%a0),%d2
        tst.l   %d2
        beq     .trap1_exit

        /* Empilha o caractere que veio em D1 (argumento para cconout(char)) */
        /* O C espera um int/word alinhado na pilha, empilhamos como Long */
        and.l   #0x000000FF,%d1
        move.l  %d1,-(%sp)
        
        move.l  %d2,%a0
        jsr     (%a0)
        addq.l  #4,%sp       /* Limpa o argumento da pilha */

.trap1_exit:
        /* Restaura o mundo (D0 contém o retorno de cconin se foi chamada) */
        movem.l (%sp)+,%d1-%d7/%a0-%a6
        rte

/* =========================================================================
   5. TRAP 14 (A FAMOSA TRAP DO XMODEM / SERIAL)
   ========================================================================= */
TrapEHandler:
        /* 1. Salva o contexto na pilha para restaurar depois */
        movem.l %d1-%d7/%a0-%a6,-(%sp)

        /* 2. Empurra os valores REAIS de D0 e D1 para a pilha como argumentos para o C */
        move.l  %d1,-(%sp)    /* Segundo argumento de TRAP14_Handler */
        move.l  %d0,-(%sp)    /* Primeiro argumento de TRAP14_Handler */

        /* 3. Chama o despachante em C */
        jsr     TRAP14_Handler

        /* 4. Remove os dois argumentos (4 bytes cada = 8 bytes) da pilha */
        addq.l  #8,%sp

        /* 5. Restaura o contexto original */
        movem.l (%sp)+,%d1-%d7/%a0-%a6
        rte