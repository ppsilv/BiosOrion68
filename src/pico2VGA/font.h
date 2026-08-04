#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>
#include "font_5x7.h"


typedef struct
{
    uint8_t *name;     // Name of the font
    uint8_t width;        // Width of the font in pixels
    uint8_t height;       // Height of the font in pixels
    uint8_t size;    // ASCII code of the first character
    const uint8_t *data;  // Pointer to the font data array
} font_t;

#define     FONTE_8X12      0x0A
#define     FONTE_8X14      0x0B
#define     FONTE_8X16      0x0C
#define     FONTE_8X18      0x0D

extern font_t * set_font(uint8_t fonte);

extern const uint8_t font_8x12[16 * 128];
extern const unsigned char font_8x14[16 * 128];
extern const unsigned char font_8x16[16*130];
extern const unsigned char font_8x18[32 * 128];

#endif