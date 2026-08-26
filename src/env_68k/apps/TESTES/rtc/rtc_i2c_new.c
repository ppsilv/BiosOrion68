#include <stdint.h>
#include <stdio.h>

// Endereço base da MC68681
//#define MC68681_REG   (*(volatile uint16_t *)0xFF9010)
#define DUART_IPR     (*(volatile uint8_t *)0xFF901B) /* Input Port (IP0 = Pino 7) */
#define DUART_SOPR    (*(volatile uint8_t *)0xFF901D) /* Set Output Port */
#define DUART_ROPR    (*(volatile uint8_t *)0xFF901F) /* Reset Output Port */

// Mapeamento dos pinos (máscaras de bit)
#define PIN_SDA_IN    0x01   // Pino 7  - Entrada de Dados
#define PIN_SCL       0x20  // Pino 14 - Clock
#define PIN_SDA_OUT   0x80  // Pino 15 - Saída de Dados

/*
#define SCL_BIT       0x20  // Pino 14 (OP5)
#define SDA_OUT_BIT   0x80  // Pino 15 (OP7)
#define SDA_IN_BIT    0x01  // Pino 7  (IP0 - Leitura)
*/

#define AT24C32_ADDR  0xA0       // Endereço I2C base (Escrita: 0xA0, Leitura: 0xA1)

// Delay para estabilização dos sinais
static void i2c_delay(void) {
    for (volatile int i = 0; i < 50; i++);
}

// Manipulação dos pinos
static void set_scl(uint8_t val) {
    if (val) DUART_ROPR |= PIN_SCL;
    else     DUART_SOPR |= PIN_SCL;
    i2c_delay();
}

static void set_sda_out(uint8_t val) {
    if (val) DUART_ROPR |= PIN_SDA_OUT;
    else     DUART_SOPR |= PIN_SDA_OUT;
    i2c_delay();
}

static uint8_t get_sda_in(void) {
    return (DUART_IPR & PIN_SDA_IN) ? 1 : 0;    
}

// Primitivas I2C
void i2c_start(void) {
    set_sda_out(1);
    set_scl(1);
    set_sda_out(0);
    set_scl(0);
}

void i2c_stop(void) {
    set_sda_out(0);
    set_scl(1);
    set_sda_out(1);
}

uint8_t i2c_write_byte(uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        set_sda_out((byte >> i) & 0x01);
        set_scl(1);
        set_scl(0);
    }
    
    // Leitura do ACK
    set_sda_out(1); // Libera a linha
    set_scl(1);
    uint8_t ack = get_sda_in();
    set_scl(0);
    
    return (ack == 0); // Returns 1 se recebeu ACK
}

uint8_t i2c_read_byte(uint8_t send_ack) {
    uint8_t byte = 0x5a;
    set_sda_out(1); // Libera a linha de saída
    
    for (int i = 7; i >= 0; i--) {
        set_scl(1);
        if (get_sda_in()) {
            byte |= (1 << i);
            printf("1");
        }else{
            printf("0");
        }
        set_scl(0);
    }
    
    // Envio de ACK / NACK
    set_sda_out(send_ack ? 0 : 1);
    set_scl(1);
    set_scl(0);
    set_sda_out(1);
    
    return byte;
}

// Funções da EEPROM AT24C32 (Endereçamento de 16 bits)
uint8_t at24c32_write_byte(uint16_t mem_addr, uint8_t data) {
    i2c_start();
    
    if (!i2c_write_byte(AT24C32_ADDR | 0)) goto fail; // Comando de Escrita
    if (!i2c_write_byte((uint8_t)(mem_addr >> 8))) goto fail; // Endereço MSB
    if (!i2c_write_byte((uint8_t)(mem_addr & 0xFF))) goto fail; // Endereço LSB
    if (!i2c_write_byte(data)) goto fail; // Dado
    
    i2c_stop();
    return 1; // Sucesso

fail:
    i2c_stop();
    return 0; // Falha
}

uint8_t at24c32_read_byte(uint16_t mem_addr, uint8_t *data) {
    i2c_start();
    
    // Dummy Write para posicionar o ponteiro de memória
    if (!i2c_write_byte(AT24C32_ADDR | 0)) goto fail;
    if (!i2c_write_byte((uint8_t)(mem_addr >> 8))) goto fail;
    if (!i2c_write_byte((uint8_t)(mem_addr & 0xFF))) goto fail;
    
    // Start repetido para leitura
    i2c_start();
    if (!i2c_write_byte(AT24C32_ADDR | 1)) goto fail; // Comando de Leitura
    
    *data = i2c_read_byte(0); // NACK para encerrar leitura de 1 byte
    i2c_stop();
    return 1; // Sucesso

fail:
    i2c_stop();
    return 0; // Falha
}

int main(void) {
    uint16_t endereco = 0x0010;
    uint8_t dado_escrito = 0xA5;
    uint8_t dado_lido = 0x00;



    // Escrita na EEPROM
    if (at24c32_write_byte(endereco, dado_escrito)) {
        // Aguarda tempo interno de escrita da EEPROM (~5ms)
        for (volatile int i = 0; i < 50000; i++);
        printf("Escrito com sucesso\n");
    }else{
        printf("Erro na escrita\n");
    }
DENOVO:
    // Leitura da EEPROM
    if(at24c32_read_byte(endereco, &dado_lido)){
        printf("Lido com sucesso\n");
        goto DENOVO;
    }else{
        printf("Erro na leitura\n");
    }

    printf("Endereco: [%04d] dado: [%02x]\n",endereco,dado_lido);
    return 0;
}