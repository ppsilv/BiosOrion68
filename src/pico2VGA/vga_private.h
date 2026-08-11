#ifndef __VGA_PRIVATE_H__
#define __VGA_PRIVATE_H__

#include "cursor.h"
#include "colors.h"
#include "font.h"


typedef struct  {
    uint16_t width;  //320, 640
    uint16_t height; //240, 480
    cursor_t *cursor ;
    screenMode_t video_mode ;
    font_t font ;
    color_t textcolor ;
    color_t textbgcolor ;

    uint32_t txcount;
    uint16_t topmask;
    uint16_t bottommask;
    uint8_t tabspace;    
    uint8_t* vga_data_array;    
}vga_text_private_t;

#endif