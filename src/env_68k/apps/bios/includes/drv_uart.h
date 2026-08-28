#ifndef __DRVUART_H__
#define __DRVUART_H__

#define DRV_UART0_BASE 0xFF4000
#define DRV_UART1_BASE 0xFF4100
#define DRV_UART2_BASE 0xFF4200
#define DRV_UART3_BASE 0xFF4300
#define RHR 0x01   //receive holding register (read)
#define THR 0x01   //transmit holding register (write)
#define IER 0x03   //interrupt enable register
#define ISR 0x05   //interrupt status register (read)
#define FCR 0x05   //FIFO control register (write)
#define LCR 0x07   //line control register
#define MCR 0x09   //modem control register
#define LSR 0x0B   //line status register
#define MSR 0x0D   //modem status register
#define SPR 0x0F   //scratchpad register (reserved for system use)
#define DLL 0x01   //divisor latch LSB
#define DLM 0x03   //divisor latch MSB
//aliases for register names (used by different manufacturers)cd ..       
#define RBR 0x01   //receive buffer register
#define IIR 0x05   //interrupt identification register
#define SCR 0x0F   //scratch register


void uart0_init(void);
void uart_putchar(unsigned char ch);
void uart0_flush();

void uart0_init();
void uart1_init();
void uart2_init();
void uart3_init();

unsigned int uart0_read();
void uart0_write(unsigned char ch);
unsigned int uart0_read_timeout();
void uart0_write_string(char * str);
int _inbyte(int delay);
void _outbyte(unsigned char ch);

#endif