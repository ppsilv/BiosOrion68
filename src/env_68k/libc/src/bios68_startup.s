.section .text
.global _start

.global UartWrCh
.global uart0_initialize
.global _delay_ms
.global printString
.global UartWriteCh

.equ RHR, 0x01
.equ THR, 0x01
.equ FCR, 0x05
.equ LCR, 0x07
.equ LSR, 0x0B
.equ DLL, 0x01
.equ DLM, 0x03
.equ WR_CHAR, 0xFF912B

_vectors:
          .long    0x000FFFF0                   /*0x00000000*//* Pilha no topo real da RAM (ajuste se seu limite for outro) */
          .long    _start                       /*0x00000004*/
          .long    SvcBusError                  /*0x00000008*/
          .long    Svcaddress_err               /*0x0000000C*/
          .long    SvcIllegalIns                /*0x00000010*/
          .long    SvcDiv0_handler              /*0x00000014*/
          .long    SvcChkIns                    /*0x00000018*/
          .long    SvcTrapvIns                  /*0x0000001C*/
          .long    SvcPrivViolation             /*0x00000020*/
          .long    SvcTrace                     /*0x00000024*/
          .long    SvcLineA                     /*0x00000028*/
          .long    SvcLineF                     /*0x0000002C*/
          .long    bad_exception                /*0x00000030*/
          .long    bad_exception                /*0x00000034*/
          .long    bad_exception                /*0x00000038*/
          .long    bad_exception                /*0x0000003C*/
          .long    bad_exception                /*0x00000040*/
          .long    bad_exception                /*0x00000044*/
          .long    bad_exception                /*0x00000048*/
          .long    bad_exception                /*0x0000004C*/
          .long    bad_exception                /*0x00000050*/
          .long    bad_exception                /*0x00000054*/
          .long    bad_exception                /*0x00000058*/
          .long    bad_exception                /*0x0000005C*/
          .long    SpuriousHandler              /*0x00000060*/
          .long    Int1Handler                  /*0x00000064*/
          .long    Int2Handler                  /*0x00000068*/
          .long    Int3Handler                  /*0x0000006C*/
          .long    Int4Handler                  /*0x00000070*/
          .long    Int5Handler                  /*0x00000074*/
          .long    Int6Handler                  /*0x00000078*/
          .long    Int7Handler                  /*0x0000007C*/
          .long    Trap0Handler                 /*0x00000080*/
          .long    Trap1Handler                 /*0x00000084*/
          .long    Trap2Handler                 /*0x00000088*/
          .long    Trap3Handler                 /*0x0000008C*/
          .long    Trap4Handler                 /*0x00000090*/
          .long    Trap5Handler                 /*0x00000094*/
          .long    Trap6Handler                 /*0x00000098*/
          .long    Trap7Handler                 /*0x0000009C*/
          .long    Trap8Handler                 /*0x000000A0*/
          .long    Trap9Handler                 /*0x000000A4*/
          .long    TrapAHandler                 /*0x000000A8*/
          .long    TrapBHandler                 /*0x000000AC*/
          .long    TrapCHandler                 /*0x000000B0*/
          .long    TrapDHandler                 /*0x000000B4*/
          .long    TrapEHandler                 /*0x000000B8*/
          .long    TrapFHandler                 /*0x000000BC*/


_start:
        ORI.W   #0x0700,%SR         /* Desabilita interrupções no boot */
        LEA     0x000FFFF0,%A7      /* Garante o ponteiro da pilha limpo */

        MOVE.L  #3,%D1        /* espera 5 segundos */
        JSR     delay_seconds

        MOVE.B  #'1',WR_CHAR
/*
; Busca de RAM para m68k (Orion68)
; Retorna em A0 o endereço onde a RAM termina.
*/
/************************************************************************************* */
/* Find the total of SRAM */
FIND_RAM:
    MOVE.L  #0x00080000,%A0    /*; A0 = Endereço inicial da busca (Ajuste para o seu mapa)*/
    MOVE.L  #0x55AA55AA,%D1    /*; D1 = Padrão de teste (alternando 0s e 1s)*/
    MOVE.L  #0x00000400,%D2    /*; D2 = Incremento de salto (ex: 1KB)*/

    MOVE.B  #'2',WR_CHAR

.busca_loop:
    ADD.L   %D2,%A0            /*; Avança o ponteiro de memória*/
    MOVE.L  (%A0),%D0          /*; Salva o dado original da memória no D0*/
    MOVE.L  %D1,(%A0)          /*; Escreve o padrão de teste na RAM*/

    CMP.L   (%A0),%D1          /*; Lê da RAM e compara com o padrão*/
    BNE.S   .fim_ram          /*; Se não for igual (não há RAM), sai do loop*/

    MOVE.L  %D0,(%A0)          /*; Restaura o dado original (é RAM válida)*/
    BRA.S   .busca_loop       /*; Repete para o próximo bloco*/

    MOVE.B  #'3',WR_CHAR

.fim_ram:
        /*; A0 contém o endereço do fim da RAM.*/
        MOVE.L  %A0,%A4
        /*; A4 contém o endereço do fim da RAM.*/

    MOVE.B  #'4',WR_CHAR

/********************************************/
/* Clear the entire SRAM */
        /* 1. Limpa a RAM (bss) */
        LEA     0x080000,%A0
        MOVE.L  %A4,%A1
        MOVE.L  #0x0,%D0
.ClearLoop:
        MOVE.L  %D0,(%A0)+
        CMPA.L  %A0,%A1
        BHI     .ClearLoop

        LEA     0x080000,%A0
        MOVE.L  %A4,(%A0)

        MOVE.B  #'5',WR_CHAR

/*        MOVE.L  %A4,%D0
//        ANDI.L  #0xFFFFFFFC,%D0
//        MOVE.L  %D0,%A7
*/
    MOVE.B  #'6',WR_CHAR

/********************************************/
/* 2. Carrega seção .data da ROM para a RAM */
        lea     __data_load_start, %A0
        lea     __data_start, %A1
        lea     __data_end, %A2
copy_data:
        cmpa.l  %A2, %A1
        bge.s   go_ahead
        move.b  (%A0)+, (%A1)+
        bra.s   copy_data
go_ahead:

        MOVE.B  #'7',WR_CHAR

        /* Uart 0 init */
        LEA     0xFF4000,%A1
        move.b  #0x07,FCR(%a1)
        move.b  #0x83,LCR(%a1)
        move.b  #0x08,DLL(%A1)
        move.b  #0x00,DLM(%A1)
        bclr.b  #0x07,LCR(%a1)

        MOVE.B  #'8',WR_CHAR

        lea     bootstrap_string,%a0
        jsr     print_string

        MOVE.B  #'9',WR_CHAR

        jsr     main

.dead:
        bra.s   .dead

.global UartWrCh

UartWriteCh:
        MOVE.B  7(%A7),%D0     /* pega o argumento (char) da pilha */
UartWrCh:
        move.l  #0xFF4000,%A1
.WaitTx:
        btst    #5,LSR(%A1)
        beq     .WaitTx
        move.b  %D0,THR(%A1)
        RTS

PicoWrCh:
        LEA     0x00FF8001,%A1
        MOVE.B  %D0,(%A1)
        NOP
        NOP
        NOP
        NOP
        RTS

uart0_initialize:
        LEA     0xFF4000,%A1
        move.b  #0x07,FCR(%a1)
        move.b  #0x83,LCR(%a1)
        move.b  #0x08,DLL(%A1)
        move.b  #0x00,DLM(%A1)
        bclr.b  #0x07,LCR(%a1)
        RTS

uart1_initialize:
        LEA     0xFF4100,%A1
        move.b  #0x07,FCR(%a1)
        move.b  #0x83,LCR(%a1)
        move.b  #0x08,DLL(%A1)
        move.b  #0x00,DLM(%A1)
        bclr.b  #0x07,LCR(%a1)
        RTS


printString:
        MOVE.L  4(%A7),%A0     /* pega o 1º argumento da pilha (ponteiro da string) e põe em A0 */
        MOVE.L  %A0,-(%A7)     /* preserva A0 original do chamador (é caller-saved, mas por segurança) */
        ADDQ.L  #4,%A7
print_string:
        move.b  (%A0)+,%d0
        beq     .end_print_string
        jsr     UartWrCh
        /*jsr     PicoWrCh*/
        bra     print_string
.end_print_string:
        RTS

startup_delay:
        move.l  #2000,%d1
.outer_loop:
        move.l  #400,%d2
.inner_loop:
        subq.l  #1,%d2
        bne     .inner_loop
        subq.l  #1,%d1
        bne     .outer_loop
        rts
/*
; ---------------------------------------------------------
; DELAY_1MS: Gera um atraso de aproximadamente 1 ms
; Frequência da CPU: 16 MHz (Período do clock = 62.5 ns)
; ---------------------------------------------------------
*/
_delay_ms:
    MOVE.L  %D0, -(%A7)
    MOVE.L  #1333,%D0
.loop1:
    DBRA    %D0, .loop1

    MOVE.L  (%A7)+,%D0
    RTS
/*
; ---------------------------------------------------------
; DELAY_1MS_8MHZ: Gera um atraso de aproximadamente 1 ms
; Frequência da CPU: 8 MHz (Período do clock = 125 ns)
; ---------------------------------------------------------
*/
DELAY_1MS_8MHZ:
    MOVE.L  %D0,-(%A7)
    MOVE.L  #666,%D0
.loop2:
    DBRA    %D0, .loop2
    MOVE.L  (%A7)+,%D0
    RTS
/*
; ---------------------------------------------------------
; DELAY_1MS_4MHZ: Gera um atraso de aproximadamente 1 ms
; Frequência da CPU: 4 MHz (Período do clock = 250 ns)
; ---------------------------------------------------------
*/
DELAY_1MS_4MHZ:
    MOVE.L  %D0,-(%A7)
    MOVE.L  #333,%D0
.loop3:
    DBRA    %D0, .loop3
    MOVE.L  (%A7)+,%D0
    RTS

/*
; ---------------------------------------------------------
; DELAY_SECONDS: Gera um atraso de N segundos
; Entrada: D1 = número de segundos (0 = retorna imediatamente)
; Usa _delay_ms internamente (assume 16MHz, ajuste se mudar clock)
; ---------------------------------------------------------
*/
delay_seconds:
        MOVE.L  %D0,-(%A7)
        MOVE.L  %D1,-(%A7)

        TST.L   %D1
        BEQ     .delay_seconds_done   /* se pediram 0 segundos, sai direto */

.loop_outer:
        MOVE.L  #550,%D0             /* 1000 x 1ms = 1 segundo */
.loop_inner:
        JSR     DELAY_1MS_8MHZ
        SUBQ.L  #1,%D0
        BNE     .loop_inner

        SUBQ.L  #1,%D1
        BNE     .loop_outer

.delay_seconds_done:
        MOVE.L  (%A7)+,%D1
        MOVE.L  (%A7)+,%D0
        RTS


.section .text
.align 2
bootstrap_string:
        .byte   13, 10
        .ascii  "PDS317 - Bootstrap 2026 V1.0"
        .byte   13, 10, 0
.align 2        /* <--- CRÍTICO: Garante que o PRÓXIMO arquivo linkado comece em endereço PAR */

/*
        MEMORY ADDRESS

0x00000000      TO      0x00000002      =       RESET VECTOR
0x00000004      TO      0x0000003E      =       INTERRUPT VECTORS
0x00000040      TO      0x0000007E      =       TRAP VECTORS

*/
/*        MOVE.L %A4,0x80000*/
        /*; 2. Alinha o endereço (Garante que é múltiplo de 4)*/
        /*; Se A0 terminar em um endereço ímpar ou não alinhado a 32 bits,*/
        /*; esta operação arredonda o valor para baixo (área segura da RAM).*/
/*        MOVE.L  %A4,%D0 */
/*        ANDI.L  #0xFFFFFFFC,%D0*/    /*; Limpa os 2 bits menos significativos (alinha a 4 bytes)*/
/*        MOVE.L  %D0,%A7 */           /*; Seta o Stack Pointer (A7 / SP) com o valor seguro*/
