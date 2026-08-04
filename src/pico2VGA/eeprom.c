/*

📍 Mapeamento dos Pinos no Pico 2O RP2350 é bem flexível. Olhando os pinos que você tem livres,
 aqui estão as opções para o barramento I2C:Pinos Disponíveis
 Opção I2C (SDA / SCL)Bloco I2C
 GPIO 14 / 15I2C1 SDA / I2C1 SCL Opção Ideal
 GPIO 26 / 27I2C1 SDA / I2C1 SCL Alternativa
 GPIO 21 / 22I2C0 SDA / I2C0 SCL Alternativa
 Minha recomendação: Use os GPIO 14 (SDA) e GPIO 15 (SCL). Eles estão juntos e são mapeados nativamente para o controlador i2c1.

*/

#include <stdio.h>
#include <string.h>
#include "hardware/i2c.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/watchdog.h"
#include "screen_mode.h"
#include "eeprom.h"

#define I2C_PORT i2c1
#define EEPROM_ADDR 0x50 // Endereço padrão das 24LC
// Variável global para evitar tentativas de escrita inúteis
bool eeprom_present = false;

void pico_i2c_init() {
    // Inicializa I2C a 400kHz (Fast Mode)
    i2c_init(I2C_PORT, 400 * 1000);
    
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    
    // Pull-ups internos (como garantia, mas use externos se puder)
    //gpio_pull_up(14);
    //gpio_pull_up(15);
}

// Função de "Ping" para ver se o chip responde ao endereço
void eeprom_ping() {
    uint8_t dummy;
    // O i2c_read_blocking retorna PICO_ERROR_GENERIC (-1) se houver NACK
    int result = i2c_read_blocking(I2C_PORT, EEPROM_ADDR, &dummy, 1, false);
    
    if (result < 0) {
        eeprom_present = false; // Hardware não respondeu
    }else{
        eeprom_present = true; // Hardware não respondeu
    }
}

void print_mem_status(){
    if(eeprom_present){
        get_vga()->printString("Memoria i2c existente no bus...\n");
    }else{
        get_vga()->printString("Memoria i2c ausente do bus...\n");
        get_vga()->printString("Loading standard configurations\n");
    }
}

// Escrita de 1 byte para 24C08
void eeprom_write_byte(uint8_t addr, uint8_t data) {
    uint8_t buf[2];
    buf[0] = addr; // Apenas 1 byte de endereço!
    buf[1] = data;
    i2c_write_blocking(I2C_PORT, EEPROM_ADDR, buf, 2, false);
    sleep_ms(10); 
}

// Leitura de 1 byte para 24C08
uint8_t eeprom_read_byte(uint8_t addr) {
    uint8_t data;
    i2c_write_blocking(I2C_PORT, EEPROM_ADDR, &addr, 1, true); // Envia 1 byte
    i2c_read_blocking(I2C_PORT, EEPROM_ADDR, &data, 1, false);
    return data;
}
void reset(){
    watchdog_reboot(0, 0, 0);
}


// Grava a struct campo por campo para evitar problemas de página na 24C08
void eeprom_save_config(sys_config_t *config) {
    if (!eeprom_present) return;

    uint8_t *ptr = (uint8_t *)config;
    for (uint16_t i = 0; i < sizeof(sys_config_t); i++) {
        eeprom_write_byte(i, ptr[i]); // Usa a sua função que funciona!
    }
}

// Lê a struct sequencialmente
void eeprom_load_config(sys_config_t *config) {
    if (!eeprom_present) {
        config->magic = 0;
        config->video_mode = MODE_TEXT_80_S;
        return;
    }

    uint8_t *ptr = (uint8_t *)config;

    for (uint16_t i = 0; i < sizeof(sys_config_t); i++) {
        *(ptr+i)=eeprom_read_byte(i); // Usa a sua função que funciona!
      //  get_vga()->printString2(" ",*(ptr+i));
    }
    //get_vga()->printString2(" ",config->magic);
    if (config->magic != 0x68000ACE) {
        //get_vga()->printString2("Magic number NAO conferi ",config->magic);
        eeprom_write_byte(0x68, 0);
        eeprom_write_byte(0x00, 1);
        eeprom_write_byte(0x00, 2);
        eeprom_write_byte(0x00, 3);
        config->magic = 0x68000ACE;
        config->video_mode = MODE_TEXT_80_S;
    }
}
void i2c_scanner(){
    int encontrado = 0;

    for (int addr = 0; addr < (1 << 7); ++addr) {
        uint8_t rxdata;
        // Tenta ler 1 byte do endereço atual
        // Se retornar < 0, o dispositivo não respondeu (NACK)
        int ret = i2c_read_blocking(I2C_PORT, addr, &rxdata, 1, false);

        if (ret >= 0) {
            get_vga()->printString1("Dispositivo encontrado no endereço: ", addr);
            encontrado++;
        }
    }

    if (encontrado == 0) {
        get_vga()->printString("Nenhum dispositivo I2C detectado. Verifique a fiação e os resistores!\n");
    } else {
        get_vga()->printString1("Varredura finalizada. dispositivo(s) encontrado(s)", encontrado);
    }    
}

void dump(uint8_t addr,uint8_t size){
    uint8_t j=0,data;
    get_vga()->printString2("Dumping ",size); 
    get_vga()->printString(" Hexa bytes from i2c memory.\n"); 
    for(uint8_t i=0; i < size; i++){
        data = eeprom_read_byte(i);
        if ( data <= 0x0F){
            get_vga()->printString("0"); 
        }
        get_vga()->printString2("", data);
        get_vga()->printString(" "); 
        j++;
        if( j==8 ){
            get_vga()->printString("\n");
            j=0;
        }
    }
}