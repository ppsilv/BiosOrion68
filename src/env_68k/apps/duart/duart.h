#ifndef __DUART_DRIVER_H__
#define __DUART_DRIVER_H__

#define DUART_BASE 0x00FF9000UL

/* Registradores do Canal A (offset = indice*2 + 1, byte baixo do barramento) */
#define DUART_MR1A_MR2A (*(volatile uint8_t *)(DUART_BASE + 0x01)) /* write: MR1A depois MR2A (auto-toggle) */
#define DUART_SRA_CSRA  (*(volatile uint8_t *)(DUART_BASE + 0x03)) /* read: SRA / write: CSRA (baud rate) */
#define DUART_CRA       (*(volatile uint8_t *)(DUART_BASE + 0x05)) /* write only: comando canal A */
#define DUART_RHRA_THRA (*(volatile uint8_t *)(DUART_BASE + 0x07)) /* read: RHRA / write: THRA (dado Tx) */
#define DUART_ACR       (*(volatile uint8_t *)(DUART_BASE + 0x09)) /* write only: aux control register */

/* Registradores do Canal B (mesmo padrão de offset do canal A) */
#define DUART_MR1B_MR2B (*(volatile uint8_t *)(DUART_BASE + 0x11)) /* write: MR1B depois MR2B */
#define DUART_SRB_CSRB  (*(volatile uint8_t *)(DUART_BASE + 0x13)) /* read: SRB / write: CSRB */
#define DUART_CRB       (*(volatile uint8_t *)(DUART_BASE + 0x15)) /* write only: comando canal B */
#define DUART_RHRB_THRB (*(volatile uint8_t *)(DUART_BASE + 0x17)) /* read: RHRB / write: THRB */

#define SRB_TXRDY 0x04
#define SRB_RXRDY 0x01

#define SRA_TXRDY 0x04
#define SRA_RXRDY 0x01


#endif
