#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "fat_filelib.h"
#include "ata.h"


//-----------------------------------------------------------------
// main
//-----------------------------------------------------------------
int main(int argc, char *argv[])
{

    // Initialise SD interface
    if (ata_disk_initialize() > 0)
    {
        printf("ERROR: Cannot init SD card\n");
        return -1;
    }

    // Initialise File IO Library
    fl_init();

    // Attach media access functions to library
    if (fl_attach_media(ata_read_sector_multi, ata_write_sector_multi) != FAT_INIT_OK)
    {
        printf("ERROR: Failed to init file system\n");
        return -1;
    }
   
    // List the root directory
    fl_listdirectory("/");

    return 0;
}

