#include <stdint.h>

#define W5100_BASE       0xFE0000UL
#define W5100_REG(off)   ((volatile uint8_t *)(W5100_BASE + ((off) * 2) + 1))

/* Registradores do Socket 0 */
#define S0_MR            0x0400
#define S0_CR            0x0401
#define S0_SR            0x0403
#define S0_PORT          0x0404
#define S0_TX_FSR        0x0420
#define S0_TX_WR         0x0424
#define S0_RX_RSR        0x0426
#define S0_RX_RD         0x0428

/* Comandos S0_CR */
#define CR_OPEN          0x01
#define CR_LISTEN        0x02
#define CR_DISCON        0x08
#define CR_SEND          0x20
#define CR_RECV          0x40

/* Estados S0_SR */
#define SOCK_CLOSED      0x00
#define SOCK_INIT        0x13
#define SOCK_LISTEN      0x14
#define SOCK_ESTABLISHED 0x17
#define SOCK_CLOSE_WAIT  0x1C

/* Offsets de memória (Alocação padrão de 2 KB por socket) */
#define S0_TX_BASE       0x4000
#define S0_RX_BASE       0x6000
#define S0_MASK          0x07FF

/* Funções Auxiliares de 16 bits */
static inline void w5100_write16(uint16_t reg, uint16_t val) {
    *W5100_REG(reg)     = (val >> 8) & 0xFF;
    *W5100_REG(reg + 1) = val & 0xFF;
}

static inline uint16_t w5100_read16(uint16_t reg) {
    return (*W5100_REG(reg) << 8) | (*W5100_REG(reg + 1));
}

static inline void w5100_cmd(uint8_t cmd) {
    *W5100_REG(S0_CR) = cmd;
    while (*W5100_REG(S0_CR)); /* Aguarda a execução do comando */
}

/* Copia dados do buffer de recepção do W5100 */
static void w5100_recv_data(uint8_t *buf, uint16_t len) {
    uint16_t ptr = w5100_read16(S0_RX_RD);
    for (uint16_t i = 0; i < len; i++) {
        uint16_t addr = S0_RX_BASE + ((ptr + i) & S0_MASK);
        buf[i] = *W5100_REG(addr);
    }
    w5100_write16(S0_RX_RD, ptr + len);
    w5100_cmd(CR_RECV);
}

/* Copia dados para o buffer de transmissão e dispara o envio */
static void w5100_send_data(const uint8_t *buf, uint16_t len) {
    uint16_t ptr = w5100_read16(S0_TX_WR);
    for (uint16_t i = 0; i < len; i++) {
        uint16_t addr = S0_TX_BASE + ((ptr + i) & S0_MASK);
        *W5100_REG(addr) = buf[i];
    }
    w5100_write16(S0_TX_WR, ptr + len);
    w5100_cmd(CR_SEND);
}

/* Processa a máquina de estados do TCP Echo Server */
void tcp_echo_server_process(uint16_t port) {
    uint8_t status = *W5100_REG(S0_SR);
    static uint8_t buffer[512];

    switch (status) {
        case SOCK_CLOSED:
            *W5100_REG(S0_MR) = 0x01; /* Modo TCP */
            w5100_write16(S0_PORT, port);
            w5100_cmd(CR_OPEN);
            break;

        case SOCK_INIT:
            w5100_cmd(CR_LISTEN);
            break;

        case SOCK_ESTABLISHED: {
            uint16_t rx_size = w5100_read16(S0_RX_RSR);
            if (rx_size > 0) {
                if (rx_size > sizeof(buffer)) rx_size = sizeof(buffer);
                w5100_recv_data(buffer, rx_size);
                w5100_send_data(buffer, rx_size);
            }
            break;
        }

        case SOCK_CLOSE_WAIT:
            w5100_cmd(CR_DISCON);
            break;
    }
}

