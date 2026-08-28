#include "drv_uart.h"
#include "timers.h"


void uart2_init(){
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART2_BASE;

    // 1. Entra no modo DLAB para configurar Baud Rate
    *(uart_reg + LCR2) = 0x83;
    *(uart_reg + DLL2) = 8; // Divisor para 115200 com 14.7456MHz
    *(uart_reg + DLM2) = 0;
    // 2. Sai do modo DLAB e define 8N1 (MUITO IMPORTANTE: usar 0x03)
    *(uart_reg + LCR2) = 0x03;

    // 3. Configura FIFO (Habilita, limpa buffers e seta trigger de 14 bytes)
    //*(uart_reg + FCR2) = 0xC7;
    // 3. Configura FIFO (Habilita, limpa buffers e seta trigger de 1 byte)
    *(uart_reg + FCR2) = 0x07;

    // 4. Limpa registradores de controle
    *(uart_reg + MCR2) = 0x00;
    *(uart_reg + IER2) = 0x00; // Garante que interrupções estão desligadas
}


unsigned char uart2_read(){
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART2_BASE;
    unsigned char ch;
    while (!(*(uart_reg + LSR2) & 0x01)) ;
    ch = (unsigned char)*(uart_reg + RHR2);

    return ch;
}
void uart2_write(unsigned char ch){
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART2_BASE;
    
    while (!(*(uart_reg + LSR2) & 0x05)) ;
    *(uart_reg + THR2) = ch;
}

void uart2_write_string(char * str){

    while(*str){
        uart2_write(*str++);
    }
}
