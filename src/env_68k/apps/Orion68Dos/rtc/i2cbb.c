#include <stdint.h>
#include <stdio.h>
#include <orion_rtc.h>

/* ====================================================================
   MAPEAMENTO DA MC68681 DUART (Orion68K)
   ==================================================================== */
#define DUART_IPR     (*(volatile uint8_t *)0xFF901B) /* Input Port (IP0 = Pino 7) */
#define DUART_SOPR    (*(volatile uint8_t *)0xFF901D) /* Set Output Port */
#define DUART_ROPR    (*(volatile uint8_t *)0xFF901F) /* Reset Output Port */

/* Pinos na DUART (MC68681 DIP-40) */
#define SCL_BIT       0x20  // Pino 14 (OP5)
#define SDA_OUT_BIT   0x80  // Pino 15 (OP7)
#define SDA_IN_BIT    0x01  // Pino 7  (IP0 - Leitura)



/* Endereços I2C (7 bits) */
#define DS3231_ADDR   0x68
#define EEPROM_ADDR   0x50

/* ====================================================================
   PRIMITIVAS I2C (Bit-Bang)
   ==================================================================== */
static void i2c_delay(void) {
    for (volatile int i = 0; i < 20; i++) { __asm__("nop"); }
}

static inline void scl_high(void) { DUART_ROPR = SCL_BIT; }
static inline void scl_low(void)  { DUART_SOPR = SCL_BIT; }

static inline void sda_high(void) { DUART_ROPR = SDA_OUT_BIT; } /* Libera linha (HIGH via pull-up) */
static inline void sda_low(void)  { DUART_SOPR = SDA_OUT_BIT; } /* Puxa linha para GND */

static inline uint8_t sda_read(void) {
    return (DUART_IPR & SDA_IN_BIT) ? 1 : 0;
}

void i2c_start(void) {
    sda_high();
    scl_high();
    i2c_delay();
    sda_low();
    i2c_delay();
    scl_low();
    i2c_delay();
}

void i2c_stop(void) {
    sda_low();
    scl_high();
    i2c_delay();
    sda_high();
    i2c_delay();
}

uint8_t i2c_write_byte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        if (data & 0x80) sda_high();
        else             sda_low();
        i2c_delay();
        scl_high();
        i2c_delay();
        scl_low();
        data <<= 1;
    }
    
    /* Leitura de ACK */
    sda_high();
    i2c_delay();
    scl_high();
    i2c_delay();
    uint8_t ack = sda_read();
    scl_low();
    return ack; /* 0 = ACK, 1 = NACK */
}

uint8_t i2c_read_byte(uint8_t ack) {
    uint8_t data = 0;
    sda_high();
    
    for (int i = 0; i < 8; i++) {
        data <<= 1;
        scl_high();
        i2c_delay();
        if (sda_read()) data |= 1;
        scl_low();
        i2c_delay();
    }
    
    /* Envia ACK/NACK */
    if (ack) sda_low();  /* ACK */
    else     sda_high(); /* NACK */
    i2c_delay();
    scl_high();
    i2c_delay();
    scl_low();
    sda_high();
    
    return data;
}

/* ====================================================================
   FUNÇÕES BCD
   ==================================================================== */
static inline uint8_t bcd2bin(uint8_t val) { return ((val >> 4) * 10) + (val & 0x0F); }
static inline uint8_t bin2bcd(uint8_t val) { return ((val / 10) << 4) | (val % 10); }

/* ====================================================================
   DRIVER DS3231 (RTC)
   ==================================================================== */

/* Recebe o ponteiro alocado pela aplicação/caller */
uint8_t ds3231_read_time(rtc_time_t *t) {
    if (!t) return 1;

    i2c_start();
    if (i2c_write_byte((DS3231_ADDR << 1) | 0)) {
        i2c_stop();
        return 1; /* Erro de ACK */
    }
    i2c_write_byte(0x00); /* Registrador 0x00 (Segundos) */
    
    i2c_start();
    i2c_write_byte((DS3231_ADDR << 1) | 1); /* Leitura */
    
    t->sec  = bcd2bin(i2c_read_byte(1) & 0x7F);
    t->min  = bcd2bin(i2c_read_byte(1));
    t->hour = bcd2bin(i2c_read_byte(1) & 0x3F);
    i2c_read_byte(1); /* Pula dia da semana */
    t->mday = bcd2bin(i2c_read_byte(1));
    t->mon  = bcd2bin(i2c_read_byte(1) & 0x1F);
    t->year = bcd2bin(i2c_read_byte(0)); /* NACK no último byte */
    
    i2c_stop();
    return 0; /* Sucesso */
}

void ds3231_set_time(const rtc_time_t *t) {
    if (!t) return;

    i2c_start();
    if (i2c_write_byte((DS3231_ADDR << 1) | 0)) {
        i2c_stop();
        return;
    }
    i2c_write_byte(0x00);
    
    i2c_write_byte(bin2bcd(t->sec));
    i2c_write_byte(bin2bcd(t->min));
    i2c_write_byte(bin2bcd(t->hour));
    i2c_write_byte(0x01); /* Dia da semana padrão = 1 */
    i2c_write_byte(bin2bcd(t->mday));
    i2c_write_byte(bin2bcd(t->mon));
    i2c_write_byte(bin2bcd(t->year));
    
    i2c_stop();
}

/* ====================================================================
   DRIVER AT24C32 (EEPROM NVM de 4KB)
   ==================================================================== */
void eeprom_write_byte(uint16_t addr, uint8_t data) {
    i2c_start();
    if (i2c_write_byte((EEPROM_ADDR << 1) | 0)) {
        i2c_stop();
        return;
    }
    i2c_write_byte((uint8_t)(addr >> 8));   /* Endereço High */
    i2c_write_byte((uint8_t)(addr & 0xFF)); /* Endereço Low */
    i2c_write_byte(data);
    i2c_stop();
}

/* Retorna o byte lido diretamente (valor em D0 para TRAP) */
uint8_t eeprom_read_byte(uint16_t addr) {
    uint8_t val = 0xFF;
    
    i2c_start();
    if (i2c_write_byte((EEPROM_ADDR << 1) | 0)) {
        i2c_stop();
        return 0xFF;
    }
    i2c_write_byte((uint8_t)(addr >> 8));
    i2c_write_byte((uint8_t)(addr & 0xFF));
    
    i2c_start();
    i2c_write_byte((EEPROM_ADDR << 1) | 1);
    val = i2c_read_byte(0);
    i2c_stop();
    
    return val;
}

/* ====================================================================
   DETECÇÃO E INICIALIZAÇÃO
   ==================================================================== */

/* Retorna 1 se o RTC respondeu (ACK), 0 se não respondeu (NACK) */
uint8_t ds3231_probe(void) {
    i2c_start();
    uint8_t ack = i2c_write_byte((DS3231_ADDR << 1) | 0);
    i2c_stop();
    
    return (ack == 0);
}

/* Função de inicialização no boot do sistema */
uint8_t rtc_init(void) {
    /* 1. Coloca o barramento I2C em IDLE */
    scl_high();
    sda_high();
    i2c_delay();

    /* 2. Verifica se o hardware responde */
    if (!ds3231_probe()) {
        return 0; /* RTC Não encontrado */
    }

    /* 3. Garante oscilador ativo na bateria (Reg 0x0E = 0x00) */
    i2c_start();
    if (i2c_write_byte((DS3231_ADDR << 1) | 0) == 0) {
        i2c_write_byte(0x0E); /* Ponteiro no reg de Controle */
        i2c_write_byte(0x00); /* EOSC=0, INTCN=0, SQW desativado */
    }
    i2c_stop();

    return 1; /* RTC OK e Inicializado */
}