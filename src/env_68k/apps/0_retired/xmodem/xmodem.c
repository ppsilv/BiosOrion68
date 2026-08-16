#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vbug2-1.h"
#include "drv_uart.h"
#include "sysflags.h"

// main.c

// Declaração da função externa em Assembly
extern int xmodem_receive(void);

// Se você quiser ler a flag que o Assembly altera:
extern unsigned char flag_pgm_loaded; 

void xmodem_loader() {
    printf("Aguardando transmissão XMODEM...\n");
    
    // Chama a sua rotina em Assembly!
    if( xmodem_receive() ){
        sys_flags.bit.xmodem_error = 0;
    }else{
        sys_flags.bit.xmodem_error = 1;
    }

    
    // Quando der o RTS no Assembly, o fluxo volta para cá suavemente
    if (sys_flags.bit.xmodem_error == 0) {
        printf("Sucesso! Programa carregado em 0x00082000\n");
    } else {
        printf("Erro na recepção.\n");
    }
}


















/*************************************************************************************
#define SOH     0x01
#define EOT     0x04
#define ACK     0x06
#define NACK    0x15
#define CAN     0x18

unsigned char buf_xmodem[128];
#define DEBUG_NIVEL_0   0x00
#define DEBUG_NIVEL_1   0x01
#define DEBUG_NIVEL_2   0x02
#define DEBUG_NIVEL_3   0x03
unsigned char debug=DEBUG_NIVEL_0;

void xmodem_receive(){
    //volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART0_BASE;
    volatile unsigned char ch;
    volatile unsigned char total_bytes=127;    
    volatile unsigned char counter=0;    
    unsigned char *buf_pgm = (unsigned char *)0x82000;
    unsigned char *pbuf; 
    volatile unsigned char chksum=0;

    pbuf = buf_pgm;
    clr_flg_program_loaded();
    printf("Aguardando o programa de xmodem no PC\n");
    ch = uart0_read();
    printf("Starting sending NACK\n");
    if( debug >= DEBUG_NIVEL_1) printf("Sending NACK\n");
    uart0_write(NACK);
receivePacote:
    ch = uart0_read();
    if( (ch != SOH) && (ch != EOT) )
        goto receivePacote;
    if( debug >= DEBUG_NIVEL_1) printf("Receiving SOH[%02x]\n",ch);
    if( ch == EOT ){
        goto Transfer_Complete;
    }
    ch = uart0_read();
    if( debug >= DEBUG_NIVEL_0) printf("Receiving PKT[%02x]\n",ch);
    ch = uart0_read();
    if( debug >= DEBUG_NIVEL_1) printf("Receiving PKT INV[%02x]\n",ch);
    counter=0;    
    total_bytes=128;
    chksum=0;
    if( debug >= DEBUG_NIVEL_1) printf("Recebendo os dados\n");
centoEvinteOito:
    ch = uart0_read();
    chksum += ch;
    buf_xmodem[counter++] = ch;
    //printf("%02x",ch);
    if( counter < total_bytes ){
        goto centoEvinteOito;
    }
    if( debug >= DEBUG_NIVEL_1) printf("Recebendo os checKsum\n");
    ch = uart0_read();
    if( debug >= DEBUG_NIVEL_2) printf("\nChecksum recebido[%02x] calculado[%02x]\n",ch,chksum);
    if( ch == chksum ){
        chksum = 0;
        memcpy(pbuf,buf_xmodem,128);
        pbuf+=128;
        if( debug >= DEBUG_NIVEL_1) printf("Sending ACK\n");
        uart0_write(ACK);
        if( debug >= DEBUG_NIVEL_2) printf("Sent ACK\n");
        goto receivePacote;
    }else{
        chksum = 0;
        counter--;  //corrigindo a posição do caracter no buffer 
        if( debug >= DEBUG_NIVEL_1) printf("Sending NACK\n");
        uart0_write(NACK);
        if( debug >= DEBUG_NIVEL_2) printf("Sent NACK\n");
        goto receivePacote;
    }    

Transfer_Complete:
    set_flg_program_loaded();
    printf("Transfer finished...\n");
}

******************************************************************************************/