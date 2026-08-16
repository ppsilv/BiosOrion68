#include <stdint.h>
#include "duart.h"


void duart_b_init_9600(void) {
    /* ACR é compartilhado entre os 2 canais (seleciona o Baud Set).
       Já deve estar em 0x00 se você inicializou o canal A antes,
       mas reescrever aqui não tem efeito colateral. */
    DUART_ACR = 0x00;

    DUART_SRB_CSRB = 0xBB;   /* Rx e Tx a 9600 baud */

    DUART_CRB = 0x10;        /* reseta ponteiro MR -> aponta p/ MR1B */
    DUART_MR1B_MR2B = 0x13;  /* MR1B: 8 bits, sem paridade, char mode */
    DUART_MR1B_MR2B = 0x07;  /* MR2B: modo normal, 1 stop bit */

    DUART_CRB = 0x20;        /* reset receiver */
    DUART_CRB = 0x30;        /* reset transmitter */
    DUART_CRB = 0x05;        /* habilita Rx e Tx */
}
void duart_b_send_char(uint8_t c) {
    while ((DUART_SRB_CSRB & SRB_TXRDY) == 0) {
        /* espera THR ficar livre */
    }
    DUART_RHRB_THRB = c;
}
uint8_t duart_b_recv_char(void) {
    while ((DUART_SRB_CSRB & SRB_RXRDY) == 0) {
        /* espera RxRDY */
    }
    return DUART_RHRB_THRB;
}
uint8_t duart_b_char_available(void) {
    return (DUART_SRB_CSRB & SRB_RXRDY) ? 1 : 0;
}