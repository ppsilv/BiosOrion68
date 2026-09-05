/*
          _      _
     ____/ \----/ \
  _ /   |         |
 / |    |   °  °  |
   |     \_/----\_/
    \  ____ \  /
     | |  | |()
     |_|  |_| \\//


 */


#include <stdint.h>
#include <stdio.h>
#include "duart.h"
#include "fileio.h"


struct lixo{
    int a;
    char b;
};
int contador=0xffff;
int *ptr_contador = &contador;

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

void duart_a_init_38400(void) {
    DUART_ACR = 0x00;              /* seleciona baud Set 1 */
    DUART_SRA_CSRA = 0xCC;         /* Rx/Tx = 38400 baud (cristal 3.6864MHz) */

    DUART_CRA = 0x10;              /* reseta ponteiro MR -> aponta p/ MR1A */
    DUART_MR1A_MR2A = 0x13;        /* MR1A: 8 bits, sem paridade, char mode -- CORRIGIDO */
    DUART_MR1A_MR2A = 0x07;        /* MR2A: modo normal, 1 stop bit */

    DUART_CRA = 0x20;              /* reset receiver */
    DUART_CRA = 0x30;              /* reset transmitter */
    DUART_CRA = 0x05;              /* habilita Rx e Tx */
}
/* PARA USAR ESSE É PRECISO POR UM OSCILADOR EXTERNO NO VALOR 1.8432 NO PINO X1 DO CLOCK DA DUART
 *
 * Códigos 0x0 a 0xD no CSR: o multiplexador escolhe a saída do BRG (Baud Rate Generator)
 * — um circuito divisor interno que pega o que estiver oscilando em X1/X2 e divide em
 * várias razões fixas pra gerar as ~18 baud rates padrão da tabela (9600, 19200, 38400...).
 * Códigos 0xE e 0xF: o multiplexador pula o BRG inteiro e conecta o canal direto no sinal
 * bruto que está entrando pelo pino X1/CLK, sem nenhuma divisão pelas tabelas padrão
 * (só a divisão fixa por 16, no caso do 0xE).
 *
 * Ou seja: é o valor que você escreve no CSR que diz ao chip "use o caminho A" ou
 * "use o caminho B" — não existe nenhum sensor de "isso aqui é cristal" vs "isso
 * aqui é clock digital".
 *
 */
void duart_a_init_115200(void) {
    DUART_ACR = 0x00;              /* Set1/Set2 é irrelevante aqui — modo de clock externo ignora o BRG */
    DUART_SRA_CSRA = 0xEE;         /* Rx e Tx = código 0xE = clock externo ÷16 via pino X1/CLK */
    DUART_CRA = 0x10;              /* reseta ponteiro MR -> aponta p/ MR1A */
    DUART_MR1A_MR2A = 0x13;        /* MR1A: 8 bits, sem paridade, char mode */
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
void cputs(char *st){
    char *p=st;
    while(*p){
        putchar(*p);
        p++;
    }
}

void cputss(char *st){
    char *p=st;
    while(*p){
        duart_a_send_char(*p);
        p++;
    }
}
//int contador=0xa55a;

void dumphex(const char *label, const void *buf, size_t len) {
    const uint8_t *p = (const uint8_t *)buf;
    const uint8_t *p1 = (const uint8_t *)buf;
    printf("--- %s (%zu bytes) ---\n", label, len);
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", p[i]);
        if ((i + 1) % 16 == 0){
            for (size_t j = 16; j > 0; j--){
                if ( p[i-j] < 0x20 ){
                    puts(".");
                }else{
                    printf("%c", p[i-j]);
                }
            }
            printf("\n");
        }
    }
    printf("\n");
}
extern void print();
int main(void) {
    FIL fd;
    char buf[256];
    int bytesRead;

  //  printf("Iniciando o programa duart\n");

   // contador=1;

    //while (1) {
     // 1. Abre o arquivo ELF no disco
    if (fopen(&fd, "tb99.bas", FA_READ) != FR_OK) {
        cputs("Erro ao tentar abrir o arquivo\n");
        return 0;
    }
    cputs("Arquivo aberto\n");

    flseek(&fd, 0);
    if (fread(&fd, buf, sizeof(buf), &bytesRead) != FR_OK || bytesRead != sizeof(buf)) {
        cputs("Nao foi possivel ler o header do ELF.\n");
        return 1;
    }

    dumphex("tb99.bas",buf,bytesRead);



    fclose(&fd);
        cputss("Serial-Arquivo aberto\n");
        //uint8_t recebido = duart_a_recv_char();

        //printf("recebido '%c' (0x%02X)\n", recebido, recebido);
        //putchar(recebido);
        //delay_short(); /* só para dar um intervalo visível entre caracteres no osciloscopio */
        //putchar(getchar());
    //}
    printf("contador[%04x] *ptr_contador[%04x]\n",contador,*ptr_contador);
    print();
    return 0;
}
