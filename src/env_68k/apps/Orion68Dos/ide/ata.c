#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "interrupt.h"
#include "endian.h"
#include "ata.h"

struct ata_drive drives[ATA_MAX_DRIVES];



#define ATA_REG_BASE 0x00FF4400

#define ATA_DELAY(x)		{ for (int delay = 0; delay < (x); delay++) { asm volatile(""); } }
#define ATA_WAIT_FOR_DATA()	{ while (!((*ATA_REG_STATUS) & ATA_ST_DATA_READY)) { } }
#define ATA_WAIT()		{ ATA_DELAY(4); while (*ATA_REG_STATUS & ATA_ST_BUSY) { } }
#define LOCK(saved) {                   \
    asm("move.w %%sr, %0\n" : "=dm" ((saved))); \
    m68k_disable_all_interrupts();                 \
}

#define UNLOCK(saved) {                     \
    m68k_enable_all_interrupts(); \
}


#define ATA_REG_DEV_CONTROL	((volatile uint8_t *) (ATA_REG_BASE + 0x1c + 1))
#define ATA_REG_DEV_ADDRESS	((volatile uint8_t *) (ATA_REG_BASE + 0x1e + 1))

#define ATA_REG_DATA		((volatile uint16_t *) (ATA_REG_BASE + 0x0 + 1))
#define ATA_REG_DATA_BYTE	((volatile uint8_t *) (ATA_REG_BASE + 0x0 + 1))
#define ATA_REG_FEATURE		((volatile uint8_t *) (ATA_REG_BASE + 0x2 + 1))
#define ATA_REG_ERROR		((volatile uint8_t *) (ATA_REG_BASE + 0x2 + 1))
#define ATA_REG_SECTOR_COUNT	((volatile uint8_t *) (ATA_REG_BASE + 0x4 + 1))
#define ATA_REG_SECTOR_NUM	((volatile uint8_t *) (ATA_REG_BASE + 0x6 + 1))
#define ATA_REG_CYL_LOW		((volatile uint8_t *) (ATA_REG_BASE + 0x8 + 1))
#define ATA_REG_CYL_HIGH	((volatile uint8_t *) (ATA_REG_BASE + 0xa + 1))
#define ATA_REG_DRIVE_HEAD	((volatile uint8_t *) (ATA_REG_BASE + 0xc + 1))
#define ATA_REG_STATUS		((volatile uint8_t *) (ATA_REG_BASE + 0xe + 1))
#define ATA_REG_COMMAND		((volatile uint8_t *) (ATA_REG_BASE + 0xe + 1))

#define ATA_CMD_READ_SECTORS	0x20
#define ATA_CMD_WRITE_SECTORS	0x30
#define ATA_CMD_IDENTIFY	0xEC
#define ATA_CMD_SET_FEATURE	0xEF

#define ATA_ST_BUSY			0x80
#define ATA_ST_DATA_READY	0x08
#define ATA_ST_ERROR		0x01

#define log_notice printf
#define log_info   printf
#define log_error  printf

char ide_bus_mode=0;

void set_ide_bus_mode(char mode){
	if(mode > 1 || mode <0){
		printf("This mode does not exist...[%d]\n",mode);
		return;
	}
	ide_bus_mode=mode;
}
void go_8bits_mode()
{
	if(ide_bus_mode == 0){
		// Set 8-bit mode
		(*ATA_REG_FEATURE) = 0x01;
		(*ATA_REG_COMMAND) = ATA_CMD_SET_FEATURE;
		ATA_WAIT();
	}
	if(ide_bus_mode == 1){ // Ou a lógica que você usar para mudar para 16-bit
		// Set 16-bit mode (Disable 8-bit PIO)
		(*ATA_REG_FEATURE) = 0x81; 
		(*ATA_REG_COMMAND) = ATA_CMD_SET_FEATURE;
		ATA_WAIT();
	}	
}

int ata_detect(void)
{
	uint8_t status;

	status = *ATA_REG_STATUS;
	// If the busy bit is already set, or the two bits that are always 0, then perhaps nothing is connected
	if (status & (ATA_ST_BUSY | 0x06)){
		return 0;
    }
	ATA_DELAY(10);

	// Reset the IDE bus
	(*ATA_REG_COMMAND) = ATA_CMD_IDENTIFY;

	for (int i = 0; i < 1000; i++) {
		ATA_DELAY(10);

		status = *ATA_REG_STATUS;
		// If it becomes unbusy within the timeout then a drive is connected
		if (!(status & ATA_ST_BUSY)) {
			if (status & ATA_ST_DATA_READY) {
				ATA_DELAY(100);
				return 1;
			} else {
				printf("ata: data not ready\n");
				return 0;
			}
		}
	}
	printf("ata: timeout waiting for identify command\n");
	return 0;
}

int ata_read_sector(int sector, char *buffer)
{
	short saved_status;

	//printf("ata.c: Reading sector[%d] to buffer[%ld]\n",sector,(long *)buffer);

	LOCK(saved_status);

	(*ATA_REG_DRIVE_HEAD) = 0xE0;
	//(*ATA_REG_DRIVE_HEAD) = 0xE0 | (uint8_t) ((sector >> 24) & 0x0F);

	go_8bits_mode();
	/*
	// Set 8-bit mode
	(*ATA_REG_FEATURE) = 0x01;
	(*ATA_REG_COMMAND) = ATA_CMD_SET_FEATURE;
	ATA_WAIT();
	*/

	// Read a sector
	(*ATA_REG_CYL_HIGH) = (uint8_t) (sector >> 16);
	(*ATA_REG_CYL_LOW) = (uint8_t) (sector >> 8);
	(*ATA_REG_SECTOR_NUM) = (uint8_t) sector;
	(*ATA_REG_SECTOR_COUNT) = 1;
	(*ATA_REG_COMMAND) = ATA_CMD_READ_SECTORS;
	ATA_WAIT();

	char status = (*ATA_REG_STATUS);
	if (status & 0x01) {
		log_error("Error while reading ata: %x\n", (*ATA_REG_ERROR));
		UNLOCK(saved_status);
		return 0;
	}

	ATA_WAIT();
	ATA_WAIT_FOR_DATA();

	if( ide_bus_mode == 0 ){
		for (int i = 0; i < 512; i++ ) {
			buffer[i] = (*ATA_REG_DATA_BYTE);

			ATA_WAIT();
		}
	}else{
		for (int i = 0; i < 256; i++ ) {
			((uint16_t *) buffer)[i] = (*ATA_REG_DATA);
			asm volatile("rol.w	#8, %0\n" : "+g" (((uint16_t *) buffer)[i]));

			ATA_WAIT();
		}
	}
	UNLOCK(saved_status);
	return 512;
}

int ata_write_sector(int sector, const char *buffer)
{
	short saved_status;

	LOCK(saved_status);

	go_8bits_mode();
	/*
	// Set 8-bit mode
	(*ATA_REG_FEATURE) = 0x01;
	(*ATA_REG_COMMAND) = ATA_CMD_SET_FEATURE;
	ATA_WAIT();
	*/

	// Read a sector
	(*ATA_REG_DRIVE_HEAD) = 0xE0;
	//(*ATA_REG_DRIVE_HEAD) = 0xE0 | (uint8_t) ((sector >> 24) & 0x0F);
	(*ATA_REG_CYL_HIGH) = (uint8_t) (sector >> 16);
	(*ATA_REG_CYL_LOW) = (uint8_t) (sector >> 8);
	(*ATA_REG_SECTOR_NUM) = (uint8_t) sector;
	(*ATA_REG_SECTOR_COUNT) = 1;
	(*ATA_REG_COMMAND) = ATA_CMD_WRITE_SECTORS;
	ATA_WAIT();

	char status = (*ATA_REG_STATUS);
	//printk("IDE: %x\n", status);
	if (status & 0x01) {
		log_error("Error while writing ata: %x\n", (*ATA_REG_ERROR));
		UNLOCK(saved_status);
		return 0;
	}

	ATA_DELAY(100);
	ATA_WAIT();
	ATA_WAIT_FOR_DATA();

	for (int i = 0; i < 512; i++) {
		ATA_WAIT();
		//while (((*ATA_REG_STATUS) & ATA_ST_BUSY) || !((*ATA_REG_STATUS) & ATA_ST_DATA_READY)) { }

		//((uint16_t *) buffer)[i] = (*ATA_REG_DATA);
		//asm volatile("rol.w	#8, %0\n" : "+g" (((uint16_t *) buffer)[i]));
		(*ATA_REG_DATA_BYTE) = buffer[i];
	}

	ATA_WAIT();

	if (*ATA_REG_STATUS & ATA_ST_ERROR) {
		log_error("Error writing sector %d: %x\n", sector, *ATA_REG_ERROR);
	}

	UNLOCK(saved_status);

	return 512;
}



struct partition_entry {
    uint8_t status;
    uint8_t chs_start[3];
    uint8_t fstype;
    uint8_t chs_end[3];
    uint32_t lba_start;
    uint32_t lba_sectors;
};


int read_partition_table(char *buffer, struct partition *devices)
{
    struct partition_entry *table;

    table = (struct partition_entry *) &buffer[0x1BE];

    for (short i = 0; i < 4; i++) {
        devices[i].base = le32toh(table[i].lba_start);
        devices[i].size = le32toh(table[i].lba_sectors);
        devices[i].fstype = table[i].fstype;
        devices[i].flags = table[i].status;
    }

    return 0;
}

int ata_init(void)
{

	for (short i = 0; i < PARTITION_MAX; i++) {
		drives[0].parts[i].base = 0;
	}

	// TODO this doesn't work very well
	if (!ata_detect()) {
		log_info("ata: no device detected\n");
		return 0;
	}else{
		//log_info("ata: device detected\n");
	}

	char *buffer=( char *)0x82000;

	ata_read_sector(0, buffer);
	read_partition_table(buffer, drives[0].parts);

	for (short i = 0; i < PARTITION_MAX; i++) {
		if (drives[0].parts[i].size) {
			//log_notice("ata%d: found partition with %d sectors\n", i, drives[0].parts[i].size);
		}
	}

	return 1;
}

int ata_disk_status(){
	if( ata_detect() ){
		return 0; //RES_OK
	}
	return 1; //RES_ERROR
}

int ata_disk_initialize(){
	if( ata_init() ){
		return 0; //RES_OK
	}
	return 1; //RES_ERROR
}

void dump_memory(void * addr,int size);
#define ATA_TIMEOUT             100000

int ata_wait_not_busy()
{
    volatile uint32_t timeout = ATA_TIMEOUT;
    do
    {
        uint8_t status = (*ATA_REG_STATUS);
        if ((status & ATA_ST_BUSY) == 0)
            return 0;
    }
    while (timeout--);
    printf("ata_wait_not_busy: timeout\n");
    return 1;
}

int ata_wait_for_data()
{
    volatile uint32_t timeout = ATA_TIMEOUT;
    do
    {
        uint8_t status = (*ATA_REG_STATUS);
        if ((status & ATA_ST_BUSY) == 0)
            return 0;
    }
    while (timeout--);

    printf("ata_wait_for_data: timeout\n");
    return 1;
}


int ata_read_identity(void)
{

    printf("ata_read_identity: function entry\n");

	if (ata_wait_not_busy())
    {
        printf("ata_read_identity: ata_wait_for_data (1) error\n");
        return 1;
    }

	(*ATA_REG_SECTOR_COUNT) = 1;
	(*ATA_REG_COMMAND) = ATA_CMD_IDENTIFY;
	if (ata_wait_for_data())
    {
        printf("ata_read_identity: ata_wait_for_data (1) error\n");
        return 1;
    }

	uint8_t status = (*ATA_REG_STATUS);
	if (status & ATA_ST_ERROR)
    {
        printf("ata_read_identity: disk status error - status = %02X\n", status);
        return 1;
    }

	if (ata_wait_not_busy())
    {
        printf("ata_read_identity: ata_wait_not_busy (2) error\n");
        return 1;
    }

	printf("ata_read_identity: reading data....");
	uint8_t drives[1024];
    uint8_t *ptr8 = (uint8_t *)&drives;

    for (int i = 0; i < 512; i++)
    {
        ptr8[i] = (*ATA_REG_DATA_BYTE);
        ATA_WAIT();
    }
    for (int i = 0; i < 512; i += 2)
    {
        uint8_t tmp = ptr8[i];
        ptr8[i]     = ptr8[i + 1];
        ptr8[i + 1] = tmp;
    }

    
    dump_memory((void *)ptr8, 512);
    printf("done\n");
	return 0;
}

