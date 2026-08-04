#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "vga_primitives.h"

typedef struct {
    uint32_t magic;        // 0x68000ACE
    uint8_t  video_mode;   // 0=320x240, 1=640x480, etc.
    uint8_t  text_color;   // Cor padrão de boot
    uint8_t  bg_color;     // Cor de fundo
    uint8_t  reserved[58]; // Espaço para chegar perto do tamanho da página (opcional)
} sys_config_t;

void pico_i2c_init();
void eeprom_ping(); 
void eeprom_load_config(sys_config_t *config);
void eeprom_save_config(sys_config_t *config);
void eeprom_write_byte(uint8_t addr, uint8_t data);
uint8_t eeprom_read_byte(uint8_t addr);
void reset();
void print_mem_status();
void i2c_scanner();

extern sys_config_t vga_nvc_config;

#endif