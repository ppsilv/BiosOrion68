.section .text
.align 2

.equ DUART_BASE, 0x00FF9000

.global duart_a_init_38400
.global duart_a_recv_char
.global duart_a_send_char
.global duart_send_char
.global duart_a_char_available

// void duart_a_init_38400(void)
duart_a_init_38400:
    // DUART_ACR = 0x00; (seleciona baud Set 1)
    move.b #0x00, (DUART_BASE + 0x09).l
    
    // DUART_SRA_CSRA = 0xCC; (Rx/Tx = 38400 baud, cristal 3.6864MHz)
    move.b #0xCC, (DUART_BASE + 0x03).l

    // DUART_CRA = 0x10; (reseta ponteiro MR -> aponta p/ MR1A)
    move.b #0x10, (DUART_BASE + 0x05).l

    // DUART_MR1A_MR2A = 0x13; (MR1A: 8 bits, sem paridade, char mode)
    move.b #0x13, (DUART_BASE + 0x01).l

    // DUART_MR1A_MR2A = 0x07; (MR2A: modo normal, 1 stop bit)
    move.b #0x07, (DUART_BASE + 0x01).l

    // DUART_CRA = 0x20; (reset receiver)
    move.b #0x20, (DUART_BASE + 0x05).l

    // DUART_CRA = 0x30; (reset transmitter)
    move.b #0x30, (DUART_BASE + 0x05).l

    // DUART_CRA = 0x05; (habilita Rx e Tx)
    move.b #0x05, (DUART_BASE + 0x05).l

    rts


// uint8_t duart_a_recv_char(void)
duart_a_recv_char:
.L_recv_wait:
    // while ((DUART_SRA_CSRA & SRA_RXRDY) == 0)
    move.b (DUART_BASE + 0x03).l, %d0
    and.b #0x01, %d0
    beq .L_recv_wait

    // return DUART_RHRA_THRA;
    move.b (DUART_BASE + 0x07).l, %d0
    rts


// void duart_a_send_char(uint8_t c)
duart_a_send_char:
    // DUART_RHRA_THRA = c; (argumento c na pilha)
    move.l 4(%sp), %d0
duart_send_char:
.L_send_wait:
    // while ((DUART_SRA_CSRA & SRA_TXRDY) == 0)
    move.b (DUART_BASE + 0x03).l, %d1
    and.b #0x04, %d1
    beq .L_send_wait

    move.b %d0, (DUART_BASE + 0x07).l
    rts


// uint8_t duart_a_char_available(void)
duart_a_char_available:
    move.b (DUART_BASE + 0x03).l, %d0
    and.b #0x01, %d0
    beq .L_not_avail
    moveq #1, %d0
    rts
.L_not_avail:
    moveq #0, %d0
    rts