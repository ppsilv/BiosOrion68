/*
 *  w5100.c      library of target-independent AVR support routines
 *               for the Wiznet W5100 Ethernet interface device
 *
 *  This file is derived from the excellent work found here:
 *  www.ermicro.com/blog/?p=1773
 *  by RWB.  I am leaving the header from the original file intact below,
 *  but you need to remember the rest of the source here is fairly
 *  heavily modified.  Go to the above site for the original.
 */
 
#include "w5100.h"

extern void _delay_ms();

void delay_ms(unsigned int delay)
{
    while(delay--){
        _delay_ms();
    }
}

/*
Como configurar o W5100 no modo Direto/Paralelo
Como você vai ligar todos os 15 pinos de endereço, os pinos de 
dados (D0-D7), RD, WR e o CS gerado pela sua GAL, para começar 
a usar o chip você precisa:

Habilitar o Modo Direto no hardware: Certifique-se de que o 
pino físico de configuração do W5100 chamado SEN (SPI Enable) 
esteja ligado ao GND (ou nível lógico baixo). Isso desativa o 
SPI e ativa o barramento paralelo.

Definir o endereço base no código C:
*/
// Exemplo: W5100 mapeado na faixa de $E00000 pela sua GAL
#define W51_REG_BASE  ((volatile unsigned char *)0x00E00000)

// Escreve apenas em endereços ÍMPARES (LDS)
void W51_write(unsigned int addr, unsigned char data) {
    // Multiplica o offset por 2 e soma 1 para cair sempre no LDS (D0-D7)
    W51_REG_BASE[(addr * 2) + 1] = data;
}

// Lê apenas de endereços ÍMPARES (LDS)
unsigned char W51_read(unsigned int addr) {
    // Multiplica o offset por 2 e soma 1 para cair sempre no LDS (D0-D7)
    return W51_REG_BASE[(addr * 2) + 1];
}


#include "w5100.h"

#define S0_BASE 0x0400

// ==========================================
// 1. ABRIR SOCKET
// ==========================================
unsigned char Socket_Abrir(void) {
    W51_write(S0_BASE + 0x0000, 0x01); // Modo = TCP
    W51_write(S0_BASE + 0x0004, 0xC0); // Porta Local MSB (0xC000)
    W51_write(S0_BASE + 0x0005, 0x00); // Porta Local LSB
    W51_write(S0_BASE + 0x0001, 0x01); // Comando = OPEN

    // Aguarda Status = SOCK_INIT
    while (W51_read(S0_BASE + 0x0003) != 0x13);
    return 1;
}

// ==========================================
// 2. CONECTAR AO SERVIDOR
// ==========================================
unsigned char Socket_Conectar(unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned char ip4, unsigned int porta) {
    W51_write(S0_BASE + 0x000C, ip1);
    W51_write(S0_BASE + 0x000D, ip2);
    W51_write(S0_BASE + 0x000E, ip3);
    W51_write(S0_BASE + 0x000F, ip4);

    W51_write(S0_BASE + 0x0010, (porta >> 8) & 0xFF); // Porta Destino MSB
    W51_write(S0_BASE + 0x0011, porta & 0xFF);        // Porta Destino LSB

    W51_write(S0_BASE + 0x0001, 0x04); // Comando = CONNECT

    // Aguarda conectar (SOCK_ESTABLISHED = 0x17) ou falhar (SOCK_CLOSED = 0x00)
    while (1) {
        unsigned char status = W51_read(S0_BASE + 0x0003);
        if (status == 0x17) return 1; // Conectado com sucesso
        if (status == 0x00) return 0; // Falhou
    }
}

// ==========================================
// 3. ENVIAR DADOS
// ==========================================
void Socket_Enviar(unsigned char *buf, unsigned int tam) {
    unsigned int ptr = (W51_read(S0_BASE + 0x0024) << 8) | W51_read(S0_BASE + 0x0025);

    for (unsigned int i = 0; i < tam; i++) {
        W51_write(0x4000 + ((ptr + i) & 0x07FF), buf[i]);
    }

    ptr += tam;
    W51_write(S0_BASE + 0x0024, (ptr >> 8) & 0xFF);
    W51_write(S0_BASE + 0x0025, ptr & 0xFF);

    W51_write(S0_BASE + 0x0001, 0x20); // Comando = SEND
    while (W51_read(S0_BASE + 0x0001) != 0x00); // Aguarda envio completar
}

// ==========================================
// 4. RECEBER DADOS
// ==========================================
unsigned int Socket_Receber(unsigned char *buf_destino) {
    unsigned int tam = (W51_read(S0_BASE + 0x0026) << 8) | W51_read(S0_BASE + 0x0027);
    if (tam == 0) return 0; 

    unsigned int ptr = (W51_read(S0_BASE + 0x0028) << 8) | W51_read(S0_BASE + 0x0029);

    for (unsigned int i = 0; i < tam; i++) {
        buf_destino[i] = W51_read(0x6000 + ((ptr + i) & 0x07FF));
    }

    ptr += tam;
    W51_write(S0_BASE + 0x0028, (ptr >> 8) & 0xFF);
    W51_write(S0_BASE + 0x0029, ptr & 0xFF);

    W51_write(S0_BASE + 0x0001, 0x40); // Comando = RECV (libera buffer)
    return tam;
}

// ==========================================
// 5. FECHAR SOCKET
// ==========================================
void Socket_Fechar(void) {
    W51_write(S0_BASE + 0x0001, 0x08); // Comando = DISCON
    while (W51_read(S0_BASE + 0x0003) != 0x00); // Aguarda Status = SOCK_CLOSED
}




