/* ==============================================================================
   Esqueleto da BIOS do CP/M-68K (Sintaxe GNU Assembler / m68k-elf-as)
   ============================================================================== */

    .text
    .even
    .global bios_entry
    .type bios_entry, @function

/* --- Mapeamento de Hardware --- */
    .equ UART_STATUS,  0x00FF0000
    .equ UART_DATA,    0x00FF0002

    .equ RX_READY_BIT, 0
    .equ TX_READY_BIT, 1

/* ==============================================================================
   1. JUMP TABLE (Entrada da BIOS)
   ============================================================================== */
bios_entry:
    bra.w   bios_init      /* Offset 0x00: Boot Frio */
    bra.w   bios_wboot     /* Offset 0x04: Boot Quente */
    bra.w   bios_const     /* Offset 0x08: Status do Teclado */
    bra.w   bios_conin     /* Offset 0x0C: Entrada de Caractere */
    bra.w   bios_conout    /* Offset 0x10: Saída de Caractere */
    bra.w   bios_lstout    /* Offset 0x14: Impressora */
    bra.w   bios_punch     /* Offset 0x18: Auxiliar Out */
    bra.w   bios_reader    /* Offset 0x1C: Auxiliar In */
    bra.w   bios_home      /* Offset 0x20: Disco -> Trilha 0 */
    bra.w   bios_seldsk    /* Offset 0x24: Seleciona Drive */
    bra.w   bios_settrk    /* Offset 0x28: Define Trilha */
    bra.w   bios_setsec    /* Offset 0x2C: Define Setor */
    bra.w   bios_setdma    /* Offset 0x30: Define Endereço do Buffer RAM */
    bra.w   bios_read      /* Offset 0x34: Lê Setor */
    bra.w   bios_write     /* Offset 0x38: Escreve Setor */
    bra.w   bios_sectran   /* Offset 0x3C: Tradução de Setor */

/* ==============================================================================
   2. ROTINAS DE CONSOLE
   ============================================================================== */

/* ------------------------------------------------------------------------------
   CONST - Status do Teclado
   Retorno em %d0: 0xFFFFFFFF se houver caractere; 0x00000000 se vazio.
   ------------------------------------------------------------------------------ */
    .global bios_const
    .type bios_const, @function
bios_const:
    move.b  (UART_STATUS), %d0
    btst    #RX_READY_BIT, %d0
    bne     .L_has_char
    moveq   #0, %d0             /* Retorna 0 (Sem caractere) */
    rts

.L_has_char:
    moveq   #-1, %d0            /* Retorna -1 / 0xFFFFFFFF (Caractere pronto) */
    rts

/* ------------------------------------------------------------------------------
   CONIN - Leitura do Teclado (Bloqueante)
   Retorno em %d0: Caractere lido em 32 bits
   ------------------------------------------------------------------------------ */
    .global bios_conin
    .type bios_conin, @function
bios_conin:
    move.b  (UART_STATUS), %d0
    btst    #RX_READY_BIT, %d0
    beq     bios_conin          /* Loop de polling */

    moveq   #0, %d0             /* Limpa %d0 */
    move.b  (UART_DATA), %d0    /* Lê o byte da UART */
    andi.l  #0x7F, %d0          /* Mascara ASCII de 7 bits */
    rts

/* ------------------------------------------------------------------------------
   CONOUT - Escrita de Caractere
   Entrada: %d1.b = Caractere a ser enviado
   ------------------------------------------------------------------------------ */
    .global bios_conout
    .type bios_conout, @function
bios_conout:
    move.b  (UART_STATUS), %d0
    btst    #TX_READY_BIT, %d0
    beq     bios_conout         /* Loop de espera de TX */

    move.b  %d1, (UART_DATA)    /* Transmite o byte */
    rts

/* ==============================================================================
   3. STUBS / INICIALIZAÇÃO
   ============================================================================== */
    .global bios_init
    .type bios_init, @function
bios_init:
bios_wboot:
    rts

bios_lstout:
bios_punch:
bios_reader:
bios_home:
bios_seldsk:
bios_settrk:
bios_setsec:
bios_setdma:
bios_read:
bios_write:
bios_sectran:
    moveq   #0, %d0             /* Retorna OK/0 */
    rts
    