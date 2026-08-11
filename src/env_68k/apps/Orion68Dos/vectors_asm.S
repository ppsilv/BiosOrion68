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

//TRAP2
.extern vid_writechar
.extern vid_clrscr
.extern vid_gohome
.extern vid_gotoxy
.extern vid_setcolor


// Importa os símbolos externos do C e outras partes do sistema
.extern TRAP14_Handler
.extern systemTick
.extern cconin
.extern cconout
.extern ring_buf_put
.extern ring_buf_put1
.extern tick_count
.extern file_open
.extern file_close
.extern file_read
.extern file_write
.extern file_sync
.extern file_unlink
.extern file_lseek
.extern file_tell
.extern file_size
.extern file_eof
.extern file_rename
.extern file_stat
.extern file_opendir
.extern file_readdir
.extern file_closedir
.extern file_mkdir
.extern file_mount
.extern file_unmount
.extern file_list
.extern file_save
.extern file_load
| teste
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
    TRAP 2  All VGA video funcionts.
   ========================================================================= */
Trap2Handler:
/*
Convenção de Registradores Exemplo
D1: Número da Syscall (1 = Read, 2 = Write)
A0: Ponteiro para FIL (arquivo)
A1: Ponteiro para o buffer de dados
D0: Character
A2: Ponteiro para variável de bytes processados (br / bw)
Retorno (D0): Resultado FRESULT    

Na convenção padrão do GCC para m68k (C ABI), os parâmetros são empilhados
 da direita para a esquerda (32 bits / 4 bytes cada) e a limpeza da pilha 
 é feita pelo chamador (caller).
*/

        movem.l %d1-%d7/%a0-%a6,-(%sp)

        /* Avalia a função passada em D1 */
        cmp.w   #1,%d1
        beq     vid_writechar
        cmp.w   #2,%d1
        beq     vid_gotoxy
        cmp.w   #3,%d1
        beq     vid_clrscr
        cmp.w   #4,%d1
        beq     vid_gohome
        cmp.w   #5,%d1
        beq     vid_setcolor
 
        /* Restaura o mundo (D0 contém o retorno de cconin se foi chamada) */
        movem.l (%sp)+,%d1-%d7/%a0-%a6
        rte


/* =========================================================================
    TRAP 12  All diskdriver funcionts.
   ========================================================================= */
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

        /* Avalia a função passada em D1 */
        cmp.w   #1,%d1
        beq     file_open
        cmp.w   #2,%d1
        beq     file_close
        cmp.w   #3,%d1
        beq     file_read
        cmp.w   #4,%d1
        beq     file_write
        cmp.w   #5,%d1
        beq     file_sync
        cmp.w   #6,%d1
        beq     file_unlink
        cmp.w   #7,%d1
        beq     file_lseek
        cmp.w   #8,%d1
        beq     file_tell
        cmp.w   #9,%d1
        beq     file_size
        cmp.w   #10,%d1
        beq     file_eof
        cmp.w   #11,%d1
        beq     file_rename
        cmp.w   #12,%d1
        beq     file_stat
        cmp.w   #13,%d1
        beq     file_opendir
        cmp.w   #14,%d1
        beq     file_readdir
        cmp.w   #15,%d1
        beq     file_closedir
        cmp.w   #16,%d1
        beq     file_mkdir
        cmp.w   #17,%d1
        beq     file_mount
        cmp.w   #18,%d1
        beq     file_unmount
        cmp.w   #19,%d1
        beq     file_list
        cmp.w   #20,%d1
        beq     file_save
        cmp.w   #21,%d1
        beq     file_load

        /* Restaura o mundo (D0 contém o retorno de cconin se foi chamada) */
        movem.l (%sp)+,%d1-%d7/%a0-%a6
        rte

/* =========================================================================
    TRAP 14 (A FAMOSA TRAP DO XMODEM / SERIAL)
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


/*
*/

/*Dispatcher*/
/*

trap_dispatcher:
    move.w  d0,d7               ; Salva ID da função
    cmpi.w  #MAX_SYSCALLS,d7    ; Valida se a chamada existe
    bhs     sys_bad_id

    lsl.w   #2,d7               ; Multiplica por 4 (tamanho de ponteiro .L)
    lea     syscall_table,a6
    move.l  (a6,d7.w),a6        ; Carrega endereço da função
    jsr     (a6)                ; Executa
    rte

sys_bad_id:
    move.q  #-1,d0              ; Retorna erro
    rte

syscall_table:
    dc.l    sys_con_read        ; ID 0: Ler caractere
    dc.l    sys_con_write       ; ID 1: Escrever caractere
    dc.l    sys_disk_read_sec   ; ID 2: Ler setor do disco
    dc.l    sys_video_set_mode  ; ID 3: Configurar modo de vídeo

*/
