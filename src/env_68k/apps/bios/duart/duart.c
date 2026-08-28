#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define DUART_BASE   0xFF9000UL

/* ===================== Canal A ===================== */
#define MR1A_2   (*(volatile uint8_t *)(DUART_BASE + 0x01))   /* MR1A e MR2A compartilham este endereco (ponteiro interno) */
#define SRA      (*(volatile uint8_t *)(DUART_BASE + 0x03))   /* leitura: Status Register A */
#define CSRA     (*(volatile uint8_t *)(DUART_BASE + 0x03))   /* escrita: Clock Select Register A */
#define CRA      (*(volatile uint8_t *)(DUART_BASE + 0x05))   /* Command Register A */
#define RHRA     (*(volatile uint8_t *)(DUART_BASE + 0x07))   /* leitura: Receive Holding Register A */
#define THRA     (*(volatile uint8_t *)(DUART_BASE + 0x07))   /* escrita: Transmit Holding Register A */

/* ===================== Canal B (mesmo padrao de offsets, deslocado) ===================== */
#define MR1B_2   (*(volatile uint8_t *)(DUART_BASE + 0x11))   /* MR1B e MR2B compartilham este endereco */
#define SRB      (*(volatile uint8_t *)(DUART_BASE + 0x13))
#define CSRB     (*(volatile uint8_t *)(DUART_BASE + 0x13))
#define CRB      (*(volatile uint8_t *)(DUART_BASE + 0x15))
#define RHRB     (*(volatile uint8_t *)(DUART_BASE + 0x17))
#define THRB     (*(volatile uint8_t *)(DUART_BASE + 0x17))

/* ===================== Registradores GLOBAIS do chip (um so, compartilhado por A e B) ===================== */
#define ACR      (*(volatile uint8_t *)(DUART_BASE + 0x09))   /* Auxiliary Control Register */
#define IMR      (*(volatile uint8_t *)(DUART_BASE + 0x0B))   /* Auxiliary Control Register */
#define CTUR     (*(volatile uint8_t *)(DUART_BASE + 0x0D))   /* Counter/Timer Upper (escrita) -- CORRIGIDO, estava em 0x1C */
#define CTLR     (*(volatile uint8_t *)(DUART_BASE + 0x0F))   /* Counter/Timer Lower (escrita) -- CORRIGIDO, estava em 0x1E */
#define START_CT (*(volatile uint8_t *)(DUART_BASE + 0x1D))   /* leitura deste endereco = comando "Start Counter" */

/* ===================== Output Port (GPIO) -- OP0-OP7 ===================== */
#define OPCR     (*(volatile uint8_t *)(DUART_BASE + 0x1B))   /* escrita: Output Port Configuration Register */
#define SOPBC    (*(volatile uint8_t *)(DUART_BASE + 0x1D))   /* escrita: Set Output Port Bits Command (1 = liga o bit correspondente) */
#define ROPBC    (*(volatile uint8_t *)(DUART_BASE + 0x1F))   /* escrita: Reset Output Port Bits Command (1 = desliga o bit correspondente) */

#include <stdint.h>

/* Delay curto para respeitar o tempo interno da DUART entre escritas no CR */
static inline void duart_delay(void) {
    for (volatile int i = 0; i < 10; i++) {
        __asm__ volatile("nop");
    }
}

void duart_init_canal_a(void) {
    /* 1. Desabilita TX e RX do Canal A */
    CRA = 0x0A; 
    duart_delay();

    /* 2. Reseta estados e o ponteiro dos registradores MR */
    CRA = 0x20; duart_delay(); /* Reset RX */
    CRA = 0x30; duart_delay(); /* Reset TX */
    CRA = 0x10; duart_delay(); /* APONTA ponteiro interno para MR1A */

    /* 3. Configura formato do quadro: 8N1 */
    /* Primeira escrita grava no MR1A (Ponteiro avança automaticamente para MR2A) */
    MR1A_2 = 0x13;             /* MR1A: Sem paridade, 8 bits por caractere */
    
    /* Segunda escrita grava no MR2A */
    MR1A_2 = 0x07;             /* MR2A: Modo normal, 1 Stop bit */

    /* 4. Seleciona a Tabela 1 de Baud Rates (Bit 7 do ACR = 0) */
    ACR = 0x00;

    /* 5. Configura 38400 bps para RX (nibble alto) e TX (nibble baixo) */
    /* Na Tabela 1, o código 0xC (1100) corresponde a 38400 baud */
    CSRA = 0xCC;

    /* 6. Habilita Receptor e Transmissor do Canal A */
    CRA = 0x05; 
    duart_delay();
}

/* Funções básicas de E/S para teste */

void duart_putc(char c) {
    /* Aguarda o buffer de transmissão ficar vazio (SRA Bit 2 = TxRDY) */
    while (!(SRA & 0x04));
    THRA = c;
}

char duart_getc(void) {
    /* Aguarda chegar um caractere (SRA Bit 0 = RxRDY) */
    while (!(SRA & 0x01));
    return RHRA;
}

/* ===================== Output Port / LED em OP3 ===================== */

/*
 * Inicializa o Output Port como GPIO de propósito geral (sem funções
 * especiais como TxC/RxC nos pinos OP2-OP7). Chame uma vez, antes de
 * usar duart_led_op3_on()/off()/toggle().
 */
void duart_opr_init(void)
{
    OPCR = 0x00;   /* todos os OP2-OP7 como saida de proposito geral (sem funcao especial) */
    ROPBC = 0xFF;  /* garante estado inicial conhecido: todos os OPs em 0 (desligados) */
}

void duart_led_op3_on(void)
{
    printf("led on\n");
    SOPBC = 0x1C;   /* bit 3 = OP3 -> escreve 1 no bit correspondente para ligar */
}

void duart_led_op3_off(void)
{
    printf("led off\n");
    ROPBC = 0x1C;   /* bit 3 = OP3 -> escreve 1 no bit correspondente para desligar */
}

void duart_led_op3_toggle(void)
{
    /* O 68681 nao tem leitura direta do OPR (so das entradas IP) --
       entao toggle precisa de estado guardado em software. */
    static uint8_t estado = 0;

    estado ^= 1;
    if (estado)
        duart_led_op3_on();
    else
        duart_led_op3_off();
}
