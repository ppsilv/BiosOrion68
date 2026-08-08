#include <stdio.h>
#include <pico/stdlib.h>
#include <./FatFs_SPI/sd_driver/sd_card.h>
#include "hw_config.h"
#include "sd.h"

//uint8_t buffer[512];

void sdtest(){

 //   sd_init_driver();
 //   sd_card_t* card = sd_get_by_num(0);
 //   int result = card->init(card);
 //   printf("\nInit result: %i.\n", result);
 //   uint8_t buffer[512];
 //   card->read_blocks(card, buffer, 0, 1);

    // --- Uso ---
    SECTOR_DEF *sd0;
    init_sdcard();

    sd0 = get_sdcard_instance();

    if (sd0->card) /* && sd0.card->mounted) */{    // Garante que o cartão inicializou com sucesso
        sd0->sector = 0;
        // Usa o próprio buffer da estrutura:
        printf("Calling read blocks\n");
        sd0->card->read_blocks(sd0->card, sd0->buffer, sd0->sector, 1);
    }

    printf("Sector %d:\n",sd0->sector);
    printf("00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F|0123456789ABCDEF\n");
    printf("-----------------------------------------------|----------------\n");
    for (int i = 0; i < 512; i++)
    {
        printf("%2.2x ", sd0->buffer[i]);

        if ((i % 16) == 15){
            for(int j=(i-15);j<=i;j++){
                if (sd0->buffer[j] > 0x20 && sd0->buffer[j] < 0x80 )
                    printf("%c", sd0->buffer[j] );
                else    
                    printf("." );
            }
            printf("\n");
        }
    }

}