#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/spi.h"

// Definições de velocidade (ajuste conforme seu hardware)
#define SERIAL_BAUD_RATE 115200
#define SD_SPI_FREQ      12000000  // 12 MHz para o SD Card
#define WIZ_SPI_FREQ     20000000  // 20 MHz para o Wiznet (WIZ810MJ aguenta bem)

void inicializar_uart0(void) {
    // 1. Inicializa o periférico UART0 com o Baud Rate definido
    uart_init(uart0, SERIAL_BAUD_RATE);

    // 2. Mapeia as funções de hardware para os pinos GPIO 13 e 14
    gpio_set_function(13, GPIO_FUNC_UART); // GPIO 13 vira UART0 TX
    gpio_set_function(14, GPIO_FUNC_UART); // GPIO 14 vira UART0 RX
    
    // Opcional: Ativa os buffers FIFO internos da UART
    uart_set_fifo_enabled(uart0, true);
}

void inicializar_spi0_sdcard(void) {
    // 1. Inicializa o periférico SPI0 na frequência desejada
    spi_init(spi0, SD_SPI_FREQ);

    // 2. Mapeia as funções nativas de SPI para os pinos escolhidos
    gpio_set_function(16, GPIO_FUNC_SPI); // SPI0 RX (MISO)
    gpio_set_function(18, GPIO_FUNC_SPI); // SPI0 SCK (CLK)
    gpio_set_function(19, GPIO_FUNC_SPI); // SPI0 TX (MOSI)

    // 3. O pino de Chip Select (CS) do SD Card é melhor controlado via software (GPIO comum)
    // Isso evita problemas com algumas bibliotecas de FAT/SD Card
    gpio_init(17);
    gpio_set_dir(17, GPIO_OUT);
    gpio_put(17, 1); // Coloca em nível ALTO (Desativado) por padrão
}

void inicializar_spi1_wiznet(void) {
    // 1. Inicializa o periférico SPI1 na frequência desejada
    spi_init(spi1, WIZ_SPI_FREQ);

    // 2. Configura o formato de dados: 8 bits, polaridade e fase padrão (Modo 0)
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // 3. Mapeia as funções nativas de SPI para os pinos do Wiznet
    gpio_set_function(20, GPIO_FUNC_SPI); // SPI1 RX (MISO)
    gpio_set_function(22, GPIO_FUNC_SPI); // SPI1 SCK (CLK)
    gpio_set_function(26, GPIO_FUNC_SPI); // SPI1 TX (MOSI)

    // 4. Chip Select (CS) do Wiznet controlado via software (GPIO comum)
    gpio_init(21);
    gpio_set_dir(21, GPIO_OUT);
    gpio_put(21, 1); // Coloca em nível ALTO (Desativado) por padrão
}

// Função centralizada para você chamar no início da sua 'main()'
void inicializar_perifericos_pico2(void) {
    inicializar_uart0();
    inicializar_spi0_sdcard();
    inicializar_spi1_wiznet();
}


#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/spi.h"

/**
 * @brief Envia uma string de texto pela UART0 (Dispositivo Serial).
 * @param texto Ponteiro para a string terminada em '\0' (null-terminated).
 */
void uart0_escrever_string(const char *texto) {
    // Envia a string inteira, aguardando o buffer se necessário
    uart_puts(uart0, texto);
}

/**
 * @brief Envia um bloco de bytes pela UART0 (Dados brutos).
 * @param dados Ponteiro para o array de bytes.
 * @param tamanho Quantidade de bytes a enviar.
 */
void uart0_escrever_bytes(const uint8_t *dados, size_t tamanho) {
    for (size_t i = 0; i < tamanho; i++) {
        uart_putc(uart0, dados[i]);
    }
}

/**
 * @brief Escreve um bloco de dados no SD Card via SPI0.
 * @param dados Ponteiro para o buffer de dados que serão enviados.
 * @param tamanho Quantidade de bytes a serem transmitidos.
 */
void spi0_sdcard_escrever(const uint8_t *dados, size_t tamanho) {
    // 1. Ativa o SD Card colocando o pino CS (GPIO 17) em nível lógico BAIXO
    gpio_put(17, 0);
    
    // 2. Transmite os dados em alta velocidade via hardware
    // spi_write_blocking envia os dados e aguarda a finalização da transmissão
    spi_write_blocking(spi0, dados, tamanho);
    
    // 3. Desativa o SD Card colocando o pino CS em nível lógico ALTO
    gpio_put(17, 1);
}

/**
 * @brief Escreve um bloco de dados no Wiznet via SPI1.
 * @param dados Ponteiro para o buffer de dados (comandos/payload).
 * @param tamanho Quantidade de bytes a serem transmitidos.
 */
void spi1_wiznet_escrever(const uint8_t *dados, size_t tamanho) {
    // 1. Ativa o Wiznet colocando o pino CS (GPIO 21) em nível lógico BAIXO
    gpio_put(21, 0);
    
    // 2. Transmite os dados via hardware
    spi_write_blocking(spi1, dados, tamanho);
    
    // 3. Desativa o Wiznet colocando o pino CS em nível lógico ALTO
    gpio_put(21, 1);
}



#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/spi.h"

/**
 * @brief Lê dados da UART0 (Serial), se houver algo disponível.
 * @param buffer_destino Ponteiro para onde os bytes recebidos serão salvos.
 * @param tamanho_maximo Quantidade máxima de bytes que deseja ler.
 * @return size_t Quantidade de bytes efetivamente lidos.
 */
size_t uart0_ler_bytes(uint8_t *buffer_destino, size_t tamanho_maximo) {
    size_t bytes_lidos = 0;
    
    // Lê os bytes enquanto houver dados na FIFO e não estourar o tamanho máximo
    while (uart_is_readable(uart0) && bytes_lidos < tamanho_maximo) {
        buffer_destino[bytes_lidos] = uart_getc(uart0);
        bytes_lidos++;
    }
    
    return bytes_lidos;
}

/**
 * @brief Lê um bloco de dados do SD Card via SPI0.
 * @param buffer_destino Ponteiro onde os dados vindos do SD serão armazenados.
 * @param tamanho Quantidade de bytes a serem lidos.
 */
void spi0_sdcard_ler(uint8_t *buffer_destino, size_t tamanho) {
    // 1. Ativa o SD Card (CS em nível BAIXO)
    gpio_put(17, 0);
    
    // 2. O SDK envia bytes repetidos (0xFF) automaticamente para gerar o clock
    // e captura as respostas salvando-as no buffer_destino
    spi_read_blocking(spi0, 0xFF, buffer_destino, tamanho);
    
    // 3. Desativa o SD Card (CS em nível ALTO)
    gpio_put(17, 1);
}

/**
 * @brief Lê um bloco de dados do Wiznet via SPI1.
 * @param buffer_destino Ponteiro onde os dados vindos do Wiznet serão armazenados.
 * @param tamanho Quantidade de bytes a serem lidos.
 */
void spi1_wiznet_ler(uint8_t *buffer_destino, size_t tamanho) {
    // 1. Ativa o Wiznet (CS em nível BAIXO)
    gpio_put(21, 0);
    
    // 2. Realiza a leitura síncrona por hardware
    spi_read_blocking(spi1, 0xFF, buffer_destino, tamanho);
    
    // 3. Desativa o Wiznet (CS em nível ALTO)
    gpio_put(21, 1);
}
