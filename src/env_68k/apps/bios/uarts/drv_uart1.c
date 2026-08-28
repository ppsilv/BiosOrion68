#include "drv_uart.h"
#include "timers.h"


void uart1_init(){
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART1_BASE;

    // 1. Entra no modo DLAB para configurar Baud Rate
    *(uart_reg + LCR1) = 0x83;
    *(uart_reg + DLL1) = 8; // Divisor para 115200 com 14.7456MHz
    *(uart_reg + DLM1) = 0;
    // 2. Sai do modo DLAB e define 8N1 (MUITO IMPORTANTE: usar 0x03)
    *(uart_reg + LCR1) = 0x03;

    // 3. Configura FIFO (Habilita, limpa buffers e seta trigger de 14 bytes)
    //*(uart_reg + FCR1) = 0xC7;
    // 3. Configura FIFO (Habilita, limpa buffers e seta trigger de 1 byte)
    *(uart_reg + FCR1) = 0x07;

    // 4. Limpa registradores de controle
    *(uart_reg + MCR1) = 0x00;
    *(uart_reg + IER1) = 0x00; // Garante que interrupções estão desligadas
}


unsigned char uart1_read(){
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART1_BASE;
    unsigned char ch;
    while (!(*(uart_reg + LSR1) & 0x01)) ;
    ch = (unsigned char)*(uart_reg + RHR1);

    return ch;
}
void uart1_write(unsigned char ch){
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART1_BASE;
    
    while (!(*(uart_reg + LSR1) & 0x05)) ;
    *(uart_reg + THR1) = ch;
}

void uart1_write_string(char * str){

    while(*str){
        uart1_write(*str++);
    }
}
