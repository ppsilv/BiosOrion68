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

/* ---------------------------------------------------------------------
 * ATENCAO -- confirme estes valores contra o datasheet do SCN68681C1N40
 * antes de gravar, principalmente:
 *   - ACR[6:4] = 011 deve corresponder a "Counter mode, clock = X1/CLK
 *     direto" (nao X1/CLK/16, o que mudaria o calculo por um fator de 16)
 *   - CSR = 0xF em cada nibble deve selecionar "clock vindo do C/T" como
 *     fonte de 16x para aquele canal
 * ------------------------------------------------------------------- */

/*
 * Configura o Counter/Timer do chip (registrador GLOBAL, vale para os
 * dois canais) para gerar 115200*16 = 1.843.200 Hz a partir do cristal
 * de 14.7456MHz:
 *   freq_saida = X1 / (2*(N+1))
 *   1.843.200  = 14.745.600 / (2*(N+1))  ->  N = 3
 *
 * Chame isso UMA VEZ, antes de duart_uartA_init()/duart_uartB_init().
 */
void duart_clock_115200_setup(void)
{
    CTUR = 0x00;
    CTLR = 0x03;              /* N = 3 */

    ACR = 0x30;                /* bits 6-4 = 011: Counter mode, clock = X1/CLK direto */

    (void)START_CT;            /* leitura deste endereco = comando "Start Counter" */
}

/*
 * Inicializa o canal A: 8 bits, sem paridade, 1 stop bit, 115200 baud
 * (usando o clock do Counter/Timer configurado por duart_clock_115200_setup).
 * Chame duart_clock_115200_setup() ANTES desta funcao.
 */
void duart_uartA_init(void)
{
    CRA = 0x10;   /* Reset MR pointer -- garante que a proxima escrita va para MR1A */
    CRA = 0x20;   /* Reset Receiver */
    CRA = 0x30;   /* Reset Transmitter */

    MR1A_2 = 0x13;   /* MR1A: RxRDY (1 caractere), sem paridade, 8 bits */
    MR1A_2 = 0x07;   /* MR2A: 1 stop bit */

    CSRA = 0xFF;      /* Rx clock e Tx clock = saida do Counter/Timer (115200) */

    IMR = 0x02;       /* Habilita IRQ para RxRDYA */

    CRA = 0x05;       /* Enable Tx (bit0) + Enable Rx (bit2) */    
}

/*
 * Inicializa o canal B: mesma configuracao do canal A.
 * NAO reescreve ACR aqui -- e' registrador global, ja configurado por
 * duart_clock_115200_setup(); reescrever mudaria o canal A tambem.
 */
void duart_uartB_init(void)
{
    CRB = 0x10;   /* Reset MR pointer -- garante que a proxima escrita va para MR1B */
    CRB = 0x20;   /* Reset Receiver */
    CRB = 0x30;   /* Reset Transmitter */

    MR1B_2 = 0xE3;   /* 1a escrita -> MR1B: no parity, 8 data bits */
    MR1B_2 = 0x07;   /* 2a escrita -> MR2B: 1 stop bit */

    CSRB = 0xFF;      /* Rx clock e Tx clock = saida do Counter/Timer (115200) */

    CRB = 0x05;       /* Enable Tx (bit0) + Enable Rx (bit2) */
}

/* ===================== I/O canal A ===================== */

void duart_uartA_putc(uint8_t c)
{
    while (!(SRA & 0x04))   /* espera bit TxRDY (bit 2) */
        ;
    THRA = c;
}

uint8_t duart_uartA_getc(void)
{
    while (!(SRA & 0x01))   /* espera bit RxRDY (bit 0) */
        ;
    return RHRA;
}

/* ===================== I/O canal B ===================== */

void duart_uartB_putc(uint8_t c)
{
    while (!(SRB & 0x04))   /* espera bit TxRDY (bit 2) */
        ;
    THRB = c;
}

uint8_t duart_uartB_getc(void)
{
    while (!(SRB & 0x01))   /* espera bit RxRDY (bit 0) */
        ;
    return RHRB;
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

/* ===================== Exemplo de uso ===================== */
/*
void exemplo_init_completo(void)
{
    duart_clock_115200_setup();   // UMA VEZ, para o chip inteiro
    duart_uartA_init();
    duart_uartB_init();
    duart_opr_init();             // UMA VEZ, antes de usar o LED

    duart_uartA_putc('A');
    duart_uartB_putc('B');

    duart_led_op3_on();
}
*/
