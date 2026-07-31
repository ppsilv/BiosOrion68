#ifndef __ORION_BUS_H__
#define __ORION_BUS_H__

#include <./FatFs_SPI/sd_driver/sd_card.h>


typedef struct{
    sd_card_t* card;
    uint8_t buffer[512];
    uint16_t size;
    uint16_t sector;
} SECTOR_DEF;

extern SECTOR_DEF * init_sdcard(void);
extern void sd_sector_read(SECTOR_DEF * sd);
extern void sd_sector_write(SECTOR_DEF * sd);
extern SECTOR_DEF * get_sdcard_instance(void);

#endif
