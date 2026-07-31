#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "pico/stdlib.h"
#include "hw_config.h"
#include "sd.h"

static SECTOR_DEF sd;   // static: evita colisão de símbolo com outros arquivos .c

SECTOR_DEF * init_sdcard(void) {
    sd_init_driver();                   // 1. Liga os periféricos de SPI/GPIO
    sd.card = sd_get_by_num(0);        // 2. Atribui o ponteiro do cartão 0

    if (sd.card != NULL) {
        sd.card->init(sd.card);       // 3. Executa a negociação inicial com o SD
        printf("sdcard initialized...\n");
    } else {
        printf("Error initialing sdcard\n");
        return NULL;
    }

    sd.size = 512;
    sd.sector = 0;
    return &sd;
}

SECTOR_DEF * get_sdcard_instance(void){
    if (sd.card != NULL) {
        return &sd;
    }
    return init_sdcard();   // pode retornar NULL se a inicialização falhar — chamador DEVE checar
}

void sd_sector_read(SECTOR_DEF * inst){
    if (inst == NULL || inst->card == NULL) {
        printf("sd_sector_read: instancia invalida\n");
        return;
    }
    inst->card->read_blocks(inst->card, inst->buffer, inst->sector, 1);
    inst->size = sizeof(inst->buffer);
}

void sd_sector_write(SECTOR_DEF * inst){
    if (inst == NULL || inst->card == NULL) {
        printf("sd_sector_write: instancia invalida\n");
        return;
    }
    inst->card->write_blocks(inst->card, inst->buffer, inst->sector, 1);
}
