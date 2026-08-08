#include "pico/stdlib.h"
#include "hardware/uart.h"

#define UART_ID       uart1
#define BAUD_RATE     115200

#define UART_TX_PIN   20
#define UART_RX_PIN   21

void setup_uart1(void)
{
    /* 1. Inicializa a UART1 com o baud rate desejado */
    uart_init(UART_ID, BAUD_RATE);

    /* 2. Seleciona a função de periférico UART para os GPIOs 20 e 21 */
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    /* 3. Configura formato de frame: 8 bits de dados, 1 stop bit, sem paridade (8N1) */
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);

    /* 4. Desabilita controle de fluxo por hardware (RTS/CTS) */
    uart_set_hw_flow(UART_ID, false, false);

    /* 5. Habilita a FIFO interna (padrão 32 bytes) */
    uart_set_fifo_enabled(UART_ID, true);
}

/* =========================================================================
   FUNÇÕES DE ESCRITA (TRANSMISSÃO)
   ========================================================================= */

/* Envia um único caractere */
void uart1_putc(char c)
{
    //if (uart_is_writable(uart1)) {
    //    uart_putc_raw(uart1, 'A'); /* Envia sem esperar */
    //}    
    uart_putc(UART_ID, c);
}

/* Envia uma string terminada em '\0' */
void uart1_puts(const char *str)
{
    uart_puts(UART_ID, str);
}

/* Envia um buffer de bytes com tamanho definido */
void uart1_write_raw(const uint8_t *buffer, size_t len)
{
    uart_write_blocking(UART_ID, buffer, len);
}

/* =========================================================================
   FUNÇÕES DE LEITURA (RECEPÇÃO)
   ========================================================================= */

/* Verifica se há dados na FIFO de recepção sem bloquear (retorna true/false) */
bool uart1_has_data(void)
{
    return uart_is_readable(UART_ID);
}

/* Lê um caractere (Bloqueante: trava a execução até receber algo) */
char uart1_getc(void)
{
    return uart_getc(UART_ID);
}

/* Lê um caractere (Não-bloqueante: retorna o byte ou -1 se não houver dado) */
int uart1_read_nonblocking(void)
{
    if (uart_is_readable(UART_ID)) {
        return (int)uart_getc(UART_ID);
    }
    return -1;
}