#ifndef __PICO_W_H__
#define __PICO_W_H__


#define SECTOR_LOW_REG         0x0A
#define SECTOR_HIGH_REG        0x0B
#define SECTOR_SEC_LOAD_REG    0x0C
#define SECTOR_READ_REG        0x0D

extern void send_sector_low(uint8_t sectorl);
extern void send_sector_high(uint8_t sectorh);
extern void send_read_cmd();
extern void read_sector(uint8_t *destino_ram);



#endif