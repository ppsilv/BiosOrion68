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
.extern ring_buf_put
.extern ring_buf_put1
.extern tick_count
.extern do_save_basic
.extern do_load_basic

/* =========================================================================
   1. EXCEÇÕES E INTERRUPÇÕES QUE NÃO FAZEM NADA (STUBS)
   ========================================================================= */
SvcBusError:
Svcaddress_err:
        /* Frame de bus/address error no 68000 tem 14 bytes (SR+PC+4 words extras).
           NUNCA fazer rte direto aqui -- desempilhar o frame inteiro e travar,
           já que não há retry de barramento implementado. */
        ADDA.L  #10,%A7        /* remove os 5 words extras (Format/Vector, Access Addr, IR, SW)
                                   deixando só SR+PC no topo, formato que RTE entende */
        /* opcional: acender um LED de diagnóstico específico de bus/address error aqui,
           antes de travar, pra diferenciar esse caso de um HALT genérico */
.trava_bus_error:
        bra.s   .trava_bus_error   /* trava aqui de propósito -- não faz rte */

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
        move.l  %a0,-(%sp)

        /* Incrementa a variável de 32 bits do C */
        move.l  #systemTick,%a0
        addq.l  #1,(%a0)
/*
    addq.w  #1,tick_count       
    cmpi.w  #400,tick_count     
    beq.s   .action_100
    cmpi.w  #800,tick_count     
    beq.s   .action_200
    move.l  (%sp)+,%a0
    rte                         

.action_100:
    move.b  #0x1C,0xFF901D        
    move.l  (%sp)+,%a0
    rte

.action_200:
    move.b  #0x1C,0xFF901F        
    clr.w   tick_count          
*/
    move.l  (%sp)+,%a0
    rte


Int3Handler:
        movem.l %d0-%d7/%a0-%a6,-(%sp)

        MOVE.B  0x00FF9113,%D0

        ANDI.L  #0x000000FF,%D0  /*; Limpa a parte alta (garante o unsigned)*/
        MOVE.L  %D0, -(%A7)       /*; Empilha 4 bytes (mantém A7 alinhado em par)*/
        
        jsr     ring_buf_put
        
        ADDQ.L  #4,%A7          /*; Limpa os 4 bytes da pilha!*/

        movem.l (%sp)+,%d0-%d7/%a0-%a6
        rte

Int4Handler:
        rte


/* =========================================================================
   4. TRAP 1 (BIOS CONSOLE IO)
   ========================================================================= */
Trap1Handler:
        /* Salva o contexto da aplicação, mas DEIXA O D0 LIVRE para o retorno! */
        movem.l %d1-%d7/%a0-%a6,-(%sp)

        /* Avalia a função passada em D0 */
        cmp.w   #1,%d1
        beq     .trap1_cconin
        cmp.w   #2,%d1
        beq     .trap1_cconout
        bra     .trap1_exit

.trap1_cconin:
        /* Verifica se o ponteiro da função cconin não é nulo */
        move.l  #cconin,%a0
        move.l  (%a0),%d0
        tst.l   %d0
        beq     .trap1_exit        
        /* Chama a função em C. O retorno virá em D0 naturalmente */
        move.l  %d0,%a0
        jsr     (%a0)
        bra     .trap1_exit

.trap1_cconout:
        /* Verifica se o ponteiro da função cconout não é nulo */
        move.l  #cconout,%a0
        move.l  (%a0),%d2
        tst.l   %d2
        beq     .trap1_exit

        /* Empilha o caractere que veio em D0 (argumento para cconout(char)) */
        /* O C espera um int/word alinhado na pilha, empilhamos como Long */
        and.l   #0x000000FF,%d0
        move.l  %d0,-(%sp)
        
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

TrapCHandler:
/*
Convenção de Registradores Exemplo
D1: Número da Syscall (1 = Read, 2 = Write)
A0: Ponteiro para FIL (arquivo)
A1: Ponteiro para o buffer de dados
D0: Quantidade de bytes (btr / btw)
A2: Ponteiro para variável de bytes processados (br / bw)
Retorno (D0): Resultado FRESULT    

Na convenção padrão do GCC para m68k (C ABI), os parâmetros são empilhados
 da direita para a esquerda (32 bits / 4 bytes cada) e a limpeza da pilha 
 é feita pelo chamador (caller).
*/

        movem.l %d1-%d7/%a0-%a6,-(%sp)

        /* Avalia a função passada em D0 */
        cmp.w   #1,%d1
        beq     .file_read
        cmp.w   #2,%d1
        beq     .file_write
        cmp.w   #3,%d1
        beq     .file_list
        bra     .trapC_exit

.file_list:
    /* 1. Monta a tabela argv[] na própria pilha (argv[0], argv[1], argv[2])*/
    move.l  %a0,-(%sp)    /*; argv[0] = Ponteiro FIL / "teste.bas"*/
    
    move.l  %sp,%a2       /*; A2 = Endereço do início do vetor argv[]*/

    /*; 2. Empilha os parâmetros para do_save(argc, argv) - da direita para a esquerda*/
    move.l  %a2,-(%sp)    /*; 2º parâmetro: char *argv[]*/
    move.l  #1,-(%sp)    /*; 1º parâmetro: int argc (3 argumentos)*/

    /*; 3. Chama a função*/
    jsr     do_ls

    /*; 4. Limpa a pilha (8 bytes dos parâmetros + 12 bytes da tabela argv = 20 bytes)*/
    lea     12(%sp),%sp
    bra     .trapC_exit


.file_write:
    /* 1. Monta a tabela argv[] na própria pilha (argv[0], argv[1], argv[2])*/
    move.l  %A2,-(%sp)    /*; argv[3] = bytes_escritos*/
    move.l  %d0,-(%sp)    /*; argv[2] = Quantidade de bytes*/
    move.l  %a1,-(%sp)    /*; argv[1] = Ponteiro do buffer*/
    move.l  %a0,-(%sp)    /*; argv[0] = Ponteiro FIL / "teste.bas"*/
    
    move.l  %sp,%a2       /*; A2 = Endereço do início do vetor argv[]*/

    /*; 2. Empilha os parâmetros para do_save(argc, argv) - da direita para a esquerda*/
    move.l  %a2,-(%sp)    /*; 2º parâmetro: char *argv[]*/
    move.l  #4,-(%sp)    /*; 1º parâmetro: int argc (3 argumentos)*/

    /*; 3. Chama a função*/
    jsr     do_save_basic

    /*; 4. Limpa a pilha (8 bytes dos parâmetros + 12 bytes da tabela argv = 20 bytes)*/
    lea     24(%sp),%sp
    bra     .trapC_exit

.file_read:
    /* 1. Monta a tabela argv[] na própria pilha (argv[0], argv[1], argv[2])*/
    move.l  %A2,-(%sp)    /*; argv[3] = bytes_escritos*/
    move.l  %d0,-(%sp)    /*; argv[2] = Quantidade de bytes*/
    move.l  %a1,-(%sp)    /*; argv[1] = Ponteiro do buffer de dados*/
    move.l  %a0,-(%sp)    /*; argv[0] = "teste.bas"*/
    */
    move.l  %sp,%a2       /*; A2 = Endereço do início do vetor argv[]*/

    /*; 2. Empilha os parâmetros para do_save(argc, argv) - da direita para a esquerda*/
    move.l  %a2,-(%sp)    /*; 2º parâmetro: char *argv[]*/
    move.l  #4, -(%sp)    /*; 1º parâmetro: int argc (3 argumentos)*/

    /*; 3. Chama a função*/
    jsr     do_load_basic

    /*; 4. Limpa a pilha (8 bytes dos parâmetros + 12 bytes da tabela argv = 20 bytes)*/
    lea     24(%sp),%sp
.trapC_exit:
        /* Restaura o mundo (D0 contém o retorno de cconin se foi chamada) */
        movem.l (%sp)+,%d1-%d7/%a0-%a6
        rte
