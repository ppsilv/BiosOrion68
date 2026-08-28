#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define W5100_BASE   0xFE0000UL
/* Aponta direto para GAR0 (0xFE0003) */
#define W5100_GAR0   ((volatile uint8_t *)(W5100_BASE + 0x0003))

/* Acesso aos registradores em endereços ímpares: (offset * 2) + 1 */
#define W5100_REG(offset) ((volatile uint8_t *)(W5100_BASE + ((offset) * 2) + 1))

/* Tabela de registradores comuns */
#define W5100_MR    0x0000 /* Mode Register */
#define W5100_GAR   0x0001 /* Gateway Address (4 bytes) */
#define W5100_SUBR  0x0005 /* Subnet Mask (4 bytes) */
#define W5100_SHAR  0x0009 /* MAC Address (6 bytes) */
#define W5100_SIPR  0x000F /* Source IP Address (4 bytes) */
#define W5100_RMSR  0x001A /* RX Memory Size Register */
#define W5100_TMSR  0x001B /* TX Memory Size Register */

extern void tcp_echo_server_process(uint16_t port);

void w5100_write(uint16_t reg, uint8_t val) {
    *W5100_REG(reg) = val;
}

uint8_t w5100_read(uint16_t reg) {
    return *W5100_REG(reg);
}

void w5100_init(void) {
    uint8_t mac[6]     = {0x00, 0x08, 0xDC, 0x01, 0x02, 0x03}; /* MAC local */
    uint8_t gateway[4] = {192, 168, 1, 1};                 /* IP do seu Roteador */
    uint8_t subnet[4]  = {255, 255, 255, 0};               /* Máscara de Rede */
    uint8_t ip[4]      = {192, 168, 1, 48};                /* IP do Orion68K */
    int i;

    /* 1. Software Reset */
    w5100_write(W5100_MR, 0x80);
    for (volatile int d = 0; d < 10000; d++); /* Pequeno atraso pós-reset */

    /* 2. Configura Gateway */
    for (i = 0; i < 4; i++) {
        w5100_write(W5100_GAR + i, gateway[i]);
    }

    /* 3. Configura Máscara de Rede */
    for (i = 0; i < 4; i++) {
        w5100_write(W5100_SUBR + i, subnet[i]);
    }

    /* 4. Configura Endereço MAC */
    for (i = 0; i < 6; i++) {
        w5100_write(W5100_SHAR + i, mac[i]);
    }

    /* 5. Configura IP do W5100 */
    for (i = 0; i < 4; i++) {
        w5100_write(W5100_SIPR + i, ip[i]);
    }

    /* 6. Aloca 2 KB de RX e TX por Socket (Sockets 0, 1, 2 e 3) */
    w5100_write(W5100_RMSR, 0x55);
    w5100_write(W5100_TMSR, 0x55);
}
int w5100_teste_barramento(void) {
    uint8_t padrao_escrita[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t leitura[4];
    int i, ok = 1;

    /* Escreve em GAR0, GAR1, GAR2 e GAR3 */
    for (i = 0; i < 4; i++) {
        W5100_GAR0[i * 2] = padrao_escrita[i];
    }

    /* Le de volta */
    for (i = 0; i < 4; i++) {
        leitura[i] = W5100_GAR0[i * 2];
        if (leitura[i] != padrao_escrita[i]) {
            ok = 0;
        }
    }

    for (i = 0; i < 4; i++) {
        printf("GAR[%d] = 0x%03d\n", i, leitura[i]);
    }

    return ok;
}

int main(void) {
    printf("Teste do W5100\n");

    if (w5100_teste_barramento()) {
        printf("Tudo ok...\n");
    } else {
        printf("Deu merda DAI!!!...\n");
    }
    printf("Initializing W5100\n");
    w5100_init();
    printf("Echo on port 7\n");
    tcp_echo_server_process(7);
    return 0;
}
