#include "drv_uart.h"
#include "timers.h"
#include <stdio.h>
#include <stdlib.h>

#define FIFO_ENABLE     0x01
#define RX_FIFO_RESET   0x02
#define TX_FIFO_RESET   0x04
#define DMA_MODE_SEL    0x08
#define RX_TRIG_LEVEL   0x00  //00 - triguer de 1 byte | 01 triguer de 4 bytes  | 10 triguer 8 bytes |  11 triquer de 14 bytes 

extern unsigned long get_system_tick(void);

/* TRAP 14
AH = 0x00: Inicializa a porta (Baud rate, paridade, stop bits).
AH = 0x01: Escreve um caractere (Equivalente ao seu UART_WriteChar).
AH = 0x02: Lê um caractere (Equivalente ao seu UART_ReadChar).
AH = 0x03: Pega o status da porta (Para fazer Polling).
*/

/*RX_TRIG_LEVEL
Bit 7	Bit 6	Valor em Hex 	Nível de Gatilho 	Comportamento no Hardware
                                (Trigger Level)
0	       0	   0x00	             1 Byte	         Assim que 1 único byte entra no FIFO, o hardware avisa. É o modo mais responsivo para digitação passo a passo (Polling/Keyboard).
0	       1	   0x40	             4 Bytes	     O hardware espera acumular 4 bytes no buffer antes de avisar a CPU.
1	       0	   0x80	             8 Bytes	     O hardware espera acumular 8 bytes (metade do buffer) antes de avisar a CPU.
1	       1	   0xC0	            14 Bytes                            
*/

void uart0_debug_char(char ch){
    printf("D0[%02x]\n",ch & 0xFF);
}

void uart0_init(){
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART0_BASE;

    // 1. Entra no modo DLAB para configurar Baud Rate
    *(uart_reg + FCR) = 0x07;
    *(uart_reg + LCR) = 0x83;
    *(uart_reg + DLL) = 8; // Divisor para 115200 com 14.7456MHz
    *(uart_reg + DLM) = 0;
    // 2. Sai do modo DLAB e define 8N1 (MUITO IMPORTANTE: usar 0x03)
    *(uart_reg + LCR) = 0x03;
    *(uart_reg + SCR) = 0x00;

    // 3. Configura FIFO (Habilita, limpa buffers e seta trigger de 14 bytes)
    //*(uart_reg + FCR0) = 0xC7;
    // 3. Configura FIFO (Habilita, limpa buffers e seta trigger de 1 byte)

    // 4. Limpa registradores de controle
    //*(uart_reg + MCR0) = 0x00;
    //*(uart_reg + IER0) = 0x00; // Garante que interrupções estão desligadas
}

void uart0_write(unsigned char ch){
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART0_BASE;
    while (!(*(uart_reg + LSR) & 0x20)) ;
    *(uart_reg + THR) = ch;
}

unsigned int uart0_read(){
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART0_BASE;
    unsigned char ch;
    while (!(*(uart_reg + LSR) & 0x01)) ;
    ch = (unsigned char)*(uart_reg + RHR);
    return ch;
}

unsigned int uart0_read_timeout(){
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART0_BASE;
    unsigned char ch;
    unsigned long time_now = get_system_tick();
    unsigned long timeout = time_now + 3000;
    while (!(*(uart_reg + LSR) & 0x01)){
        if(timeout < get_system_tick()) {
            return 0;
        }
    }
    ch = (unsigned char)*(uart_reg + RHR);
    return ch;
}

int _inbyte(int ptimeout) {
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART0_BASE;
    unsigned char ch;
    unsigned long timeout = ptimeout *10,time=0;
    while ( !( *(uart_reg + LSR) & 0x01 ) ){
        time++;
        if ( time > timeout ){
            printf("Timeout returning \n ");
            return -1;
        }
    }
    ch = (unsigned char)*(uart_reg + RHR);
    printf(" returning \n ");
    return (int)ch;
}
void _outbyte(unsigned char ch){
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART0_BASE;
    
    while (!(*(uart_reg + LSR) & 0x05)) ;
    *(uart_reg + THR) = ch;
}

void uart0_write_string(char * str){

    while(*str){
        uart0_write(*str++);
    }
}

void uart0_flush(){
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART0_BASE;
    unsigned char ch;
  
 
    return;
    while ((*(uart_reg + LSR) & 0x01)){
        ch = (unsigned char)*(uart_reg + RHR);

        printf("uart0_flush[%c]",ch);
    }

}
