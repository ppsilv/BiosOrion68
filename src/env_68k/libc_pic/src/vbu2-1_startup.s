.section .text
.global _start

.global UartWrCh
.global uart0_initialize

.equ RHR, 0x01
.equ THR, 0x01
.equ FCR, 0x05
.equ LCR, 0x07
.equ LSR, 0x0B
.equ DLL, 0x01
.equ DLM, 0x03

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

        /* 1. Limpa a RAM (bss) */
        LEA     0x080000,%A0
        LEA     0x100000,%A1
        MOVE.L  #0x0,%D0
.ClearLoop:
        MOVE.L  %D0,(%A0)+
        CMPA.L  %A0,%A1
        BHI     .ClearLoop

        /* 2. Carrega seção .data da ROM para a RAM */
        lea     __data_load_start, %a0
        lea     __data_start, %a1
        lea     __data_end, %a2
copy_data:
        cmpa.l  %a2, %a1
        bge.s   go_ahead
        move.b  (%a0)+, (%a1)+
        bra.s   copy_data
go_ahead:

        /* Uart 0 init */
        LEA     0xFF4000,%A1
        move.b  #0x07,FCR(%a1)
        move.b  #0x83,LCR(%a1)
        move.b  #0x08,DLL(%A1)
        move.b  #0x00,DLM(%A1)
        bclr.b  #0x07,LCR(%a1)

        lea     bootstrap_string,%a0
        jsr     print_string

        jsr     main

.dead:
        bra.s   .dead

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

print_string:
        move.b  (%A0)+,%d0
        beq     .end_print_string
        jsr     UartWrCh
        jsr     PicoWrCh
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

.section .text
.align 2
bootstrap_string:
        .ascii  "PDS317 - Bootstrap 2026 V1.0"
        .byte   13, 10, 0
.align 2        /* <--- CRÍTICO: Garante que o PRÓXIMO arquivo linkado comece em endereço PAR */

/*
        MEMORY ADDRESS

0x00000000      TO      0x00000002      =       RESET VECTOR
0x00000004      TO      0x0000003E      =       INTERRUPT VECTORS
0x00000040      TO      0x0000007E      =       TRAP VECTORS




*/
