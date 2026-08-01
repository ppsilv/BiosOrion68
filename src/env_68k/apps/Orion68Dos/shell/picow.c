#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "timers.h"
#include "orion68.h"
#include "picow.h"

// Definição dos registradores mapeados na memória do m68k
// Usamos 'volatile uint8_t' para obrigar o m68k a ler o hardware toda vez
#define PICO_DATA_REG    (*(volatile uint8_t *)0xFF9101)
#define PICO_STATUS_REG  (*(volatile uint8_t *)0xFF9103)
#define PICO_SIZE_HIGH   (*(volatile uint8_t *)0xFF9105)
#define PICO_SIZE_LOW    (*(volatile uint8_t *)0xFF9107)
#define PICO_CRC_REG3    (*(volatile uint8_t *)0xFF9109)
#define PICO_CRC_REG2    (*(volatile uint8_t *)0xFF910B)
#define PICO_CRC_REG1    (*(volatile uint8_t *)0xFF910D)
#define PICO_CRC_REG0    (*(volatile uint8_t *)0xFF910F)

// Estados do STATUS REGISTER que definimos na PIO do Pico
#define PICO_STATE_IDLE       0x00
#define PICO_STATE_HAS_FILE   0x01
#define PICO_STATE_EOF        0x02

/**
 * Faz o polling do Pico 2 W e, se houver um arquivo pronto,
 * captura o tamanho e descarrega os bytes para um buffer na RAM do m68k.
 * 
 * @param destino_ram Ponteiro para onde o arquivo deve ser copiado na RAM do m68k
 * @return uint16_t O tamanho do arquivo recebido (0 se não houver arquivo)
 */
 extern void _delay_ms();
uint16_t receber_arquivo_do_pico(uint8_t *destino_ram, uint8_t preg) {
    volatile uint8_t *reg = (volatile uint8_t *)(0xFF9100 + preg);
    printf("reg=[%08X]\n",reg);
    uint8_t status = PICO_STATUS_REG;
    // 1. Polling: Aguarda até que o Pico mude o status para "HAS_FILE" (0x01)
    // Se o status for IDLE (0x00), o m68k fica preso aqui esperando o Wi-Fi
    while ( status == PICO_STATE_IDLE) {
        status = PICO_STATUS_REG;
    }
    // Se o Pico respondeu com algo diferente ou deu EOF direto, aborta
    if (status < 1 ) {
        return 0;
    }
    printf("Pico respondeu[%02x]\n",status);

    delay10ms(1);
    // 2. Captura o tamanho do arquivo enviado pelo Pico (16 bits fatiados em 2 bytes)
    uint16_t tamanho_arquivo=0;
    uint8_t tamanho_high = PICO_SIZE_HIGH;
    delay10ms(1);
    tamanho_high = PICO_SIZE_HIGH;
    printf("Tamanho HIGH do arquivo=[%04x]\n",tamanho_high);
    delay10ms(1);
    uint8_t tamanho_low = PICO_SIZE_LOW;
    delay10ms(1);
    tamanho_low = PICO_SIZE_LOW;
    printf("Tamanho low do arquivo=[%04x]\n",tamanho_low);
    delay10ms(1);
    
    tamanho_arquivo = (tamanho_high <<8) | tamanho_low;

    printf("Tamanho do arquivo=[%04x][%04d]\n",tamanho_arquivo,tamanho_arquivo);
    if (tamanho_arquivo == 0) {
        return 0;
    }

    printf("Reading file...\n");
    // 3. Loop de leitura dos dados do arquivo
    for (uint16_t i = 0; i < tamanho_arquivo; i++) {       
        destino_ram[i] = PICO_DATA_REG; 
        _delay_ms();
    }
    delay10ms(1);
    uint32_t arq_crc_rec = ((uint32_t)PICO_CRC_REG3 << 24);
    printf("arq_crc_rec3 [%08X]\n",arq_crc_rec);
    arq_crc_rec |= ((uint32_t)PICO_CRC_REG2 << 16) & 0x00FF0000;
    printf("arq_crc_rec2 [%08X]\n",arq_crc_rec);
    arq_crc_rec |= ((uint32_t)PICO_CRC_REG1 << 8)  & 0x0000FF00; // Corrigido: 0x0000FF00
    printf("arq_crc_rec1 [%08X]\n",arq_crc_rec);
    arq_crc_rec |= (uint32_t)PICO_CRC_REG0 & 0xFF;
    printf("arq_crc_rec0 [%08X]\n",arq_crc_rec);

    printf("Orion68: crc32 recebido[%08X]\n", arq_crc_rec);
    uint32_t arq_crc = crc32_calculate((const uint8_t *)destino_ram, tamanho_arquivo);
    printf("Orion68: crc32 calculado[%08X]\n", arq_crc);

    return tamanho_arquivo;
}


//#define SECTOR_LOW_REG         0x0A
//#define SECTOR_HIGH_REG        0x0B
//#define SECTOR_SEC_LOAD_REG    0x0C
//#define SECTOR_READ_REG        0x0D

#define MIO_BASE_ADDRESS    0xFF9100
#define MIO_SECTOR_LOW_REG      (*(volatile uint8_t *)(MIO_BASE_ADDRESS + (SECTOR_LOW_REG*2) +1))
#define MIO_SECTOR_HIGH_REG     (*(volatile uint8_t *)(MIO_BASE_ADDRESS + (SECTOR_HIGH_REG*2)+1))
#define MIO_SECTOR_SEC_LOAD_REG (*(volatile uint8_t *)(MIO_BASE_ADDRESS + (SECTOR_SEC_LOAD_REG*2)+1))
//#define MIO_SECTOR_READ_REG     (*(volatile uint8_t *)(MIO_BASE_ADDRESS + (SECTOR_READ_REG*2)+1))
#define MIO_SECTOR_READ_REG     (*(volatile uint8_t *)0xFF911B)

//#define MIO_BASE_ADDRESS    0xFF9100
/* Helper macro: computes address, casts, dereferences — write once, reuse for every register */
//#define MIO_REG(offset)  (*(volatile uint8_t *)(MIO_BASE_ADDRESS + ((offset) * 2) + 1))
//#define MIO_SECTOR_LOW_REG       MIO_REG(SECTOR_LOW_REG)
//#define MIO_SECTOR_HIGH_REG      MIO_REG(SECTOR_HIGH_REG)
//#define MIO_SECTOR_SEC_LOAD_REG  MIO_REG(SECTOR_SEC_LOAD_REG)
//#define MIO_SECTOR_READ_REG      MIO_REG(SECTOR_READ_REG)

void send_sector_low(uint8_t sectorl){
    printf("Sending low register value\n");
    MIO_SECTOR_LOW_REG = sectorl;
}
void send_sector_high(uint8_t sectorh){
    printf("Sending high register value\n");
    MIO_SECTOR_HIGH_REG = sectorh;
}
void send_read_cmd(){
    volatile uint8_t dummy=0;
    printf("Sending sector load command value\n");
    MIO_SECTOR_SEC_LOAD_REG=0xA5;
}
void read_sector(uint8_t *destino_ram){
    printf("Reading sector...to [%04x]\n",destino_ram);
    for (uint16_t i = 0; i < 512; i++) {       
        destino_ram[i] = MIO_SECTOR_READ_REG;
        printf("%02x|",destino_ram[i]);
        _delay_ms();
    }    
}

uint16_t receber_setor_do_pico(uint8_t *destino_ram, uint8_t preg) {
//    volatile uint8_t *reg = (volatile uint8_t *)(0xFF9100 + preg);
//    printf("reg=[%08X]\n",reg);
//    uint8_t status = PICO_STATUS_REG;
//    // 1. Polling: Aguarda até que o Pico mude o status para "HAS_FILE" (0x01)
//    // Se o status for IDLE (0x00), o m68k fica preso aqui esperando o Wi-Fi
//    while ( status == PICO_STATE_IDLE) {
//        status = PICO_STATUS_REG;
//    }
//    // Se o Pico respondeu com algo diferente ou deu EOF direto, aborta
//    if (status < 1 ) {
//        return 0;
//    }
//    printf("Pico respondeu[%02x]\n",status);

//    delay10ms(1);
    // 2. Captura o tamanho do arquivo enviado pelo Pico (16 bits fatiados em 2 bytes)
    printf("Sending low register value\n");
    MIO_SECTOR_LOW_REG = 0x00;
    delay10ms(1);

    printf("Sending high register value\n");
    MIO_SECTOR_HIGH_REG = 0x00;
    delay10ms(1);
    
    printf("Sending sector load command value\n");
    MIO_SECTOR_SEC_LOAD_REG=0xA5;
    delay10ms(1);

    printf("Reading file...\n");
    // 3. Loop de leitura dos dados do arquivo
    for (uint16_t i = 0; i < 512; i++) {       
        destino_ram[i] = MIO_SECTOR_READ_REG; 
        _delay_ms();
    }
    delay10ms(1);
//    uint32_t arq_crc_rec = ((uint32_t)PICO_CRC_REG3 << 24);
//    printf("arq_crc_rec3 [%08X]\n",arq_crc_rec);
//    arq_crc_rec |= ((uint32_t)PICO_CRC_REG2 << 16) & 0x00FF0000;
//    printf("arq_crc_rec2 [%08X]\n",arq_crc_rec);
//    arq_crc_rec |= ((uint32_t)PICO_CRC_REG1 << 8)  & 0x0000FF00; // Corrigido: 0x0000FF00
//    printf("arq_crc_rec1 [%08X]\n",arq_crc_rec);
//    arq_crc_rec |= (uint32_t)PICO_CRC_REG0 & 0xFF;
//    printf("arq_crc_rec0 [%08X]\n",arq_crc_rec);
//
//    printf("Orion68: crc32 recebido[%08X]\n", arq_crc_rec);
//    uint32_t arq_crc = crc32_calculate((const uint8_t *)destino_ram, tamanho_arquivo);
//    printf("Orion68: crc32 calculado[%08X]\n", arq_crc);

    return 512;
}
