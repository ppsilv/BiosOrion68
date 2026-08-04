#include "font.h"

// Instância da fonte 8x12
static font_t font_8x12_info = {
    .name = "Font 8x12",
    .height = 12,
    .width = 8,
    .size = 1,            
    .data = font_8x12
};

// Instância da fonte 8x14
static font_t font_8x14_info = {
    .name = "Font 8x14",
    .height = 14,
    .width = 8,
    .size = 1,            
    .data = font_8x14
};

// Instância da fonte 8x16
static font_t font_8x16_info = {
    .name = "Font 8x16",
    .height = 16,
    .width = 8,
    .size = 1,            
    .data = font_8x16
};

// Instância da fonte 8x18 (para seus títulos legais)
static font_t font_8x18_info = {
    .name = "Font 8x18",
    .height = 18,
    .width = 8,
    .size = 1,            
    .data = font_8x18
};

font_t * set_font(uint8_t fonte){
    switch(fonte){
        case FONTE_8X12:
             return &font_8x12_info;
             break;
        case FONTE_8X14:
             return &font_8x14_info;
             break;
        case FONTE_8X16:
             return &font_8x16_info;
             break;
        case FONTE_8X18:
             return &font_8x18_info;
             break;
        default:
            return 0;             
    }
}