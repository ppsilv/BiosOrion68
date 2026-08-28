#ifndef __ATA_H__
#define __ATA_H__

#define PARTITION_MAX   4

#define ATA_MAX_DRIVES		1
typedef unsigned long sector_t;

struct partition {
    sector_t base;                                                                                                                               
    sector_t size;
    uint8_t fstype;
    uint8_t flags;
};


struct ata_drive {
	struct partition parts[PARTITION_MAX];
};



int ata_detect(void);
int ata_read_sector(int sector, char *buffer);
int ata_write_sector(int sector, const char *buffer);
int read_partition_table(char *buffer, struct partition *devices);
int ata_init(void);
int ata_disk_status(void);
int ata_disk_initialize(void);

int ata_read_sector_multi (uint32_t sector, char *buffer, uint32_t count);
int ata_write_sector_multi(uint32_t sector, char *buffer, uint32_t count);


#endif