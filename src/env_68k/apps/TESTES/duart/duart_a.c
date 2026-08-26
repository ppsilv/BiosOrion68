#include <stdint.h>
#include <stdio.h>
#include "duart.h"

//static void delay_short(void) {
//    for (volatile int i = 0; i < 20; i++) { __asm__("nop"); }
//}
void duart_a_init_9600(void) {
    DUART_ACR = 0x00;              /* seleciona baud Set 1 */
    DUART_SRA_CSRA = 0xBB;         /* Rx/Tx = 9600 baud (cristal 3.6864MHz) */

    DUART_CRA = 0x10;              /* reseta ponteiro MR -> aponta p/ MR1A */
    DUART_MR1A_MR2A = 0x13;        /* MR1A: 8 bits, sem paridade, char mode -- CORRIGIDO */
    DUART_MR1A_MR2A = 0x07;        /* MR2A: modo normal, 1 stop bit */

    DUART_CRA = 0x20;              /* reset receiver */
    DUART_CRA = 0x30;              /* reset transmitter */
    DUART_CRA = 0x05;              /* habilita Rx e Tx */
}

/* Bloqueante: espera até chegar um caractere e retorna ele */
uint8_t duart_a_recv_char(void) {
    while ((DUART_SRA_CSRA & SRA_RXRDY) == 0) {
        /* espera RxRDY */
    }
    return DUART_RHRA_THRA;
}
void duart_a_send_char(uint8_t c) {
    while ((DUART_SRA_CSRA & SRA_TXRDY) == 0) {
        /* espera THR ficar livre (TxRDY) */
    }
    DUART_RHRA_THRA = c;
}

/* Não-bloqueante: retorna 1 se tem caractere disponível, 0 caso contrário */
uint8_t duart_a_char_available(void) {
    return (DUART_SRA_CSRA & SRA_RXRDY) ? 1 : 0;
}

int main(void) {
    duart_a_init_9600();

    while (1) {
        duart_a_send_char('A');
        uint8_t recebido = duart_a_recv_char();

        if (recebido == 'A') {
            printf("Loopback OK: recebido '%c' (0x%02X)\n", recebido, recebido);
        } else {
            printf("Loopback FALHOU: recebido 0x%02X (esperado 0x41)\n", recebido);
        }        
        //delay_short(); /* só para dar um intervalo visível entre caracteres no osciloscopio */
    }

    return 0;
}