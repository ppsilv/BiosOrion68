#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fileio.h>

#define W5100_BASE       0xFE0000UL
#define W5100_REG(off)   ((volatile uint8_t *)(W5100_BASE + ((off) * 2) + 1))
#define BUFFER_ARQUIVO   ((uint8_t *)0x00092000UL)

/* Tabela de registradores comuns */
#define W5100_MR    0x0000 /* Mode Register */
#define W5100_GAR   0x0001 /* Gateway Address (4 bytes) */
#define W5100_SUBR  0x0005 /* Subnet Mask (4 bytes) */
#define W5100_SHAR  0x0009 /* MAC Address (6 bytes) */
#define W5100_SIPR  0x000F /* Source IP Address (4 bytes) */
#define W5100_RMSR  0x001A /* RX Memory Size Register */
#define W5100_TMSR  0x001B /* TX Memory Size Register */


#define S0_MR            0x0400
#define S0_CR            0x0401
#define S0_SR            0x0403
#define S0_PORT          0x0404
#define S0_TX_WR         0x0424
#define S0_RX_RSR        0x0426
#define S0_RX_RD         0x0428

#define CR_OPEN          0x01
#define CR_LISTEN        0x02
#define CR_DISCON        0x08
#define CR_SEND          0x20
#define CR_RECV          0x40

#define SOCK_CLOSED      0x00
#define SOCK_INIT        0x13
#define SOCK_LISTEN      0x14
#define SOCK_ESTABLISHED 0x17
#define SOCK_CLOSE_WAIT  0x1C

#define S0_TX_BASE       0x4000
#define S0_RX_BASE       0x6000
#define S0_MASK          0x07FF

static uint32_t tamanho_total = 0;


void w5100_write(uint16_t reg, uint8_t val) {
    *W5100_REG(reg) = val;
}

uint8_t w5100_read(uint16_t reg) {
    return *W5100_REG(reg);
}


static inline void w5100_write16(uint16_t reg, uint16_t val) {
    *W5100_REG(reg)     = (val >> 8) & 0xFF;
    *W5100_REG(reg + 1) = val & 0xFF;
}

static inline uint16_t w5100_read16(uint16_t reg) {
    return (*W5100_REG(reg) << 8) | (*W5100_REG(reg + 1));
}

/* Leitura dupla obrigatoria do tamanho de RX (requisito W5100) */
static uint16_t w5100_get_rx_size(void) {
    uint16_t val1 = 0, val2 = 0;
    do {
        val1 = w5100_read16(S0_RX_RSR);
        if (val1 != 0) {
            val2 = w5100_read16(S0_RX_RSR);
        } else {
            val2 = 0;
        }
    } while (val1 != val2);
    return val1;
}

static inline void w5100_cmd(uint8_t cmd) {
    *W5100_REG(S0_CR) = cmd;
    while (*W5100_REG(S0_CR));
}

static void w5100_recv_bytes(uint8_t *buf, uint16_t len) {
    uint16_t ptr = w5100_read16(S0_RX_RD);
    for (uint16_t i = 0; i < len; i++) {
        uint16_t addr = S0_RX_BASE + ((ptr + i) & S0_MASK);
        buf[i] = *W5100_REG(addr);
    }
    w5100_write16(S0_RX_RD, ptr + len);
    w5100_cmd(CR_RECV);
}

static void w5100_send_bytes(const uint8_t *buf, uint16_t len) {
    uint16_t ptr = w5100_read16(S0_TX_WR);
    for (uint16_t i = 0; i < len; i++) {
        uint16_t addr = S0_TX_BASE + ((ptr + i) & S0_MASK);
        *W5100_REG(addr) = buf[i];
    }
    w5100_write16(S0_TX_WR, ptr + len);
    w5100_cmd(CR_SEND);
}
void dump_hex(const char *label, const void *buf, size_t len) {
    const uint8_t *p = (const uint8_t *)buf;
    printf("--- %s (%zu bytes) ---\n", label, len);
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", p[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}
int w5100_getter_loop() {
    uint8_t status = *W5100_REG(S0_SR);
    static uint8_t cabecalho_raw[21];
    static enum { ESTADO_HEADER, ESTADO_PAYLOAD } estado = ESTADO_HEADER;
    static uint32_t bytes_recebidos = 0;

    switch (status) {
        case SOCK_CLOSED:
            *W5100_REG(S0_MR) = 0x01; /* TCP */
            w5100_write16(S0_PORT, 4243);
            w5100_cmd(CR_OPEN);
            estado = ESTADO_HEADER;
            bytes_recebidos = 0;
            printf("Listening on 4243\n");
            break;

        case SOCK_INIT:
            w5100_cmd(CR_LISTEN);
            break;

        case SOCK_ESTABLISHED: {
            uint16_t rx_avail = w5100_get_rx_size();

            /* FASE 1: Recebe o Cabecalho de 21 bytes */
            if (estado == ESTADO_HEADER) {
                if (rx_avail >= 21) {
                    w5100_recv_bytes(cabecalho_raw, 21);

                    /* Decodifica Little-Endian byte a byte (seguro para o 68000) */
                    tamanho_total = ((uint32_t)cabecalho_raw[3] << 24) |
                                    ((uint32_t)cabecalho_raw[2] << 16) |
                                    ((uint32_t)cabecalho_raw[1] << 8)  |
                                     (uint32_t)cabecalho_raw[0];
                    printf("Tamanho total a ser lido[%d]\n",tamanho_total);
                    bytes_recebidos = 21;
                    estado = ESTADO_PAYLOAD;
                    //dump_hex("Cabecalho",cabecalho_raw,21);
                    memcpy(BUFFER_ARQUIVO,cabecalho_raw,21);
                }
                break;
            }

            /* FASE 2: Grava direto em 0x00092000 */
            if (estado == ESTADO_PAYLOAD) {
                //printf("rx_avail[%d]\n",rx_avail);
                if (rx_avail > 0) {
                    uint32_t restante = tamanho_total - bytes_recebidos;
                    uint16_t bloco = (rx_avail < restante) ? rx_avail : (uint16_t)restante;
                    //printf("bytes_recebidos[%d]\n",bytes_recebidos);
                    w5100_recv_bytes(&BUFFER_ARQUIVO[bytes_recebidos], bloco);
                    bytes_recebidos += bloco;

                    /* FASE 3: Envia ACK e desconecta */
                    if (bytes_recebidos >= tamanho_total) {
                        const char *ack = "Arq lido";
                        w5100_send_bytes((const uint8_t *)ack, 8);

                        estado = ESTADO_HEADER;
                        w5100_cmd(CR_DISCON);
                        return 0;
                    }
                }
            }
            break;
        }

        case SOCK_CLOSE_WAIT:
            w5100_cmd(CR_DISCON);
            printf("Socket closed..\n");
            return 0;
            break;
    }
    return 1;
}
void w5100_init(void) {
   // uint8_t mac[6]     = {0x00, 0x08, 0xDC, 0x01, 0x02, 0x03}; /* MAC local */
    uint8_t gateway[4] = {192, 168, 1, 1};                 /* IP do seu Roteador */
    uint8_t subnet[4]  = {255, 255, 255, 0};               /* Máscara de Rede */
 //   uint8_t ip[4]      = {192, 168, 1, 48};                /* IP do Orion68K */
    int i;

    /* 1. Software Reset */
 //   w5100_write(W5100_MR, 0x80);
 //   for (volatile int d = 0; d < 10000; d++); /* Pequeno atraso pós-reset */

    /* 2. Configura Gateway */
    for (i = 0; i < 4; i++) {
        w5100_write(W5100_GAR + i, gateway[i]);
    }

    /* 3. Configura Máscara de Rede */
    for (i = 0; i < 4; i++) {
        w5100_write(W5100_SUBR + i, subnet[i]);
    }

    /* 4. Configura Endereço MAC */
//    for (i = 0; i < 6; i++) {
//        w5100_write(W5100_SHAR + i, mac[i]);
//    }

    /* 5. Configura IP do W5100 */
//    for (i = 0; i < 4; i++) {
//        w5100_write(W5100_SIPR + i, ip[i]);
//    }

    /* 6. Aloca 2 KB de RX e TX por Socket (Sockets 0, 1, 2 e 3) */
    w5100_write(W5100_RMSR, 0x55);
    w5100_write(W5100_TMSR, 0x55);
}

void saving_file(){
    FIL     arquivo;
    FRESULT fr;
    UINT    bytes_escritos;

    size_t tamanho;
    tamanho = tamanho_total;
    char * dado = (char *)(BUFFER_ARQUIVO+21);
    char * nome_arquivo = (char *)(BUFFER_ARQUIVO+8);
    printf("Nome arquivo: [%s]\n",nome_arquivo);
    printf("Tam. arquivo: [%d]\n",tamanho);

    if (dado == NULL || nome_arquivo == NULL || tamanho == 0) {
        printf("dump_memoria_para_arquivo: parametros invalidos\n");
        return;
    }
    puts("Creating file...\n");
    fr = fopen(&arquivo, nome_arquivo, FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        printf("dump_memoria_para_arquivo: falha ao abrir '%s' (erro %d)\n", nome_arquivo, fr);
        return;
    }

    puts("Writting file...\n");
    fr = fwrite(&arquivo, dado, (UINT)tamanho, &bytes_escritos);
    if (fr != FR_OK || bytes_escritos != tamanho) {
        printf("dump_memoria_para_arquivo: falha ao escrever '%s' (erro %d, escrito %u de %u bytes)\n", nome_arquivo, fr, bytes_escritos, (unsigned)tamanho);
        fclose(&arquivo);
        return;
    }
    puts("Closing file...\n");
    fr = fclose(&arquivo);
    if (fr != FR_OK) {
        printf("dump_memoria_para_arquivo: falha ao fechar '%s' (erro %d)\n", nome_arquivo, fr);
        return;
    }
    printf("\n");
}

int main(){
    printf("Initialing w5100\n");
    w5100_init();
    printf("Calling w5100_getter_loop()\n");
    char resp= w5100_getter_loop();
    while(resp){
        resp = w5100_getter_loop();
    }
    saving_file();
    return(0);
}
