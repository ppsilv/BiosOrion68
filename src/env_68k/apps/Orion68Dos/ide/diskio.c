/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "ff.h"			/* Basic definitions of FatFs */
#include "diskio.h"		/* Declarations FatFs MAI */
#include "ata.h"

typedef unsigned long sector_t;
extern struct ata_drive drives[];
extern int ata_read_sector(int sector, char *buffer);

/* Example: Declarations of the platform and disk functions in the project */
#include "platform.h"
#include "storage.h"

/* Example: Mapping of physical drive number for each drive */
#define DEV_FLASH	0	/* Map FTL to physical drive 0 */
#define DEV_MMC		1	/* Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Map USB MSD to physical drive 2 */
#define DEV_MIDE    3   /* Solid State disk*/

#define MAX_DRIVES	10		/* Max number of physical drives to be used */
#define	SZ_BLOCK	128		/* Erase block size to be returned by GET_BLOCK_SIZE command */

#define SZ_RAMDISK	135		/* Size of RAM disk (0) [MiB] */
#define SS_RAMDISK	512		/* Initial sector size of RAM disk (0) [byte] */

#define MIN_READ_ONLY	1	/* Read-only drive from */
#define	MAX_READ_ONLY	2	/* Read-only drive to */


typedef struct {
	DSTATUS	status;
	WORD sz_sector;
	LBA_t n_sectors;

} STAT;

static volatile STAT Stat[MAX_DRIVES];

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (BYTE pdrv)		/* Physical drive nmuber to identify the drive */
{
	return ata_disk_status();
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DRESULT disk_initialize ( BYTE pdrv	)			/* Physical drive nmuber to identify the drive */
{
	return ata_disk_initialize();
}

/* * ESTA É A PONTE: A FatFs vai chamar essa função sempre que precisar de dados.
 * Você deve plugar isso dentro do arquivo 'diskio.c' da FatFs ou mapear adequadamente.
 */
DRESULT disk_read (
    BYTE pdrv,    /* Physical drive nmuber to identify the drive */
    BYTE *buff,   /* Data buffer to store read data */
    LBA_t sector, /* Start sector number (LBA) */
    UINT count    /* Number of sectors to read */
) {
    if (pdrv != 0) {
        printf("disk_write: [%d] this drive does not exist.\n",pdrv);
        return RES_PARERR;
    }
    //printf("disk_read: pdrv[%d] sector[%ld]",pdrv,sector);

    // Soma a base da partição ativa que o seu MBR leu
    sector_t physical_sector = drives[0].parts[0].base + sector;
    //printf(" physical_sector[%ld]\n",physical_sector);

    // Lê quantos setores a FatFs pedir
    for (UINT i = 0; i < count; i++) {
        if (!ata_read_sector(physical_sector + i, (char *)buff + (i * 512))) {
            return RES_ERROR; // Erro físico no CompactFlash
        }
    }
    return RES_OK;
} 


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0
DRESULT disk_write (
    BYTE pdrv,          /* Physical drive number to identify the drive */
    const char *buff,   /* Data to be written */
    LBA_t sector,       /* Start sector in LBA */
    UINT count          /* Number of sectors to write */
)
{
    if (pdrv != 0) {
        printf("disk_write: [%d] this drive does not exist.\n",pdrv);
        return RES_PARERR;
    }
    //printf("disk_write: pdrv[%d] sector[%ld] count[%d]", pdrv, sector, count);

    // Soma a base da partição ativa na escrita também!
    sector_t physical_sector = drives[0].parts[0].base + sector;

    //printf(" physical_sector[%ld]\n",physical_sector);

    // Grava quantos setores a FatFs pedir (suportando o count)
    for (UINT i = 0; i < count; i++) {
        if (!ata_write_sector(physical_sector + i, buff + (i * 512))) {
            return RES_ERROR; // Erro físico na escrita do CompactFlash
        }
    }
    
    return RES_OK;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res=0;

	switch (pdrv) {
	case DEV_MIDE :
		return res;

	case DEV_MMC :
		return res;

	case DEV_USB :
		return res;
	}

	switch (cmd) {
	case CTRL_SYNC:			/* Nothing to do */
		res = RES_OK;
		break;

	case GET_SECTOR_COUNT:	/* Get number of sectors on the drive */
		*(LBA_t*)buff = Stat[pdrv].n_sectors;
		res = RES_OK;
		break;

	case GET_SECTOR_SIZE:	/* Get size of sector for generic read/write */
		*(WORD*)buff = Stat[pdrv].sz_sector;
		res = RES_OK;
		break;

	case GET_BLOCK_SIZE:	/* Get internal block size in unit of sector */
		*(DWORD*)buff = SZ_BLOCK;
		res = RES_OK;
		break;
	}




	return res;
}
