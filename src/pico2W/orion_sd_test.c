#include <stdio.h>
#include <pico/stdlib.h>
#include </home/pdsilva/project/Orion68/src/picolibs/no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI/sd_driver/sd_card.h>
#include "hw_config.h"


void sdtest()
{
   
printf("sd_init driver antes\n");
    sd_init_driver();
printf("sd_init driver depois\n");

    sd_card_t* card = sd_get_by_num(0);

    int result = card->init(card);
printf("sd_initialized\n");
    printf("\nInit result: %i.\n", result);
   // printf("The card has %u sectors.\n\n", card->get_num_sectors(card));

    uint8_t buffer[512];
printf("read blocks\n");
    card->read_blocks(card, buffer, 0, 1);

    printf("Sector 0:\n");
    printf("00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F|0123456789ABCDEF\n");
    printf("-----------------------------------------------|----------------\n");
    for (int i = 0; i < 512; i++)
    {
        printf("%2.2x ", buffer[i]);

        if ((i % 16) == 15){
            for(int j=(i-15);j<=i;j++){
                if (buffer[j] > 0x20 && buffer[j] < 0x80 )
                    printf("%c", buffer[j] );
                else    
                    printf("." );
            }
            printf("\n");
        }
    }

}