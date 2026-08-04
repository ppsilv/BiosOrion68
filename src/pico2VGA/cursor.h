#ifndef __CURSOR_H__
#define __CURSOR_H__

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "vga_primitives.h"
#include "colors.h"

typedef enum {
    CURSOR_BAR=0x80,
    CURSOR_UNDERLINE,
    CURSOR_BLOCK
} cursorShape_t ;

struct cursor{
    int16_t x;
    int16_t y;
    bool visible;
    bool blink;
    uint16_t blink_interval;
    cursorShape_t shape ;// 0x80 = bar, 0x81 = underline
    //Methods
    void (*set_x)(uint16_t x);
    void (*set_y)(uint16_t y);
    void (*set_position)(uint16_t x, uint16_t y);
    void (*set_visible)(bool v);
    void (*set_blink)(bool b);
    void (*set_blink_interval)(uint16_t interval);
    void (*set_shape)( cursorShape_t shape);
    uint16_t (*get_x)();
    uint16_t (*get_y)();
    bool (*is_visible)();
    uint16_t (*get_blink_interval)(void);
};
typedef struct cursor cursor_t;
static cursor_t *default_cursor = NULL;//{0,0,true,true,500,CURSOR_BAR} ;
static cursor_t * this = NULL ;




cursor_t* create_default_cursor();

#endif