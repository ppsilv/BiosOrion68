#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"

#include "cursor.h"




static void set_x(uint16_t x){
    if (x < 640) {
        this->x = x;
    }
}
static void set_y(uint16_t y)
{
    if (y < 480) {
        this->y = y;
    }
}
static void set_position(uint16_t x, uint16_t y)
{
    set_x(x);
    set_y(y);
    }
static void set_visible(bool v)
{
    this->visible = v;
}
static void set_blink(bool b)
{
    this->blink = b;
}
static void set_blink_interval(uint16_t interval)
{
    this->blink_interval = interval;
}
static uint16_t get_blink_interval(void)
{
    return this->blink_interval;
}
static void set_shape( cursorShape_t shape)
{
    this->shape = shape;
}
static uint16_t get_x()
{
    return this->x;
}
static uint16_t get_y()
{
    return this->y;
}
static bool is_visible()
{
    return this->visible;
}

cursor_t* create_default_cursor()
{
    default_cursor = (cursor_t*)malloc(sizeof(cursor_t));
    if (!default_cursor) {
        return NULL;
    }
    this = default_cursor ;
    this->x = 0 ;
    this->y = 0 ;
    this->visible = true ;
    this->blink = true ;
    this->blink_interval = 150 ;
    this->shape = CURSOR_BAR ;
    this->set_x = set_x ;
    this->set_y = set_y ;
    this->set_position = set_position ;
    this->set_visible = set_visible ;
    this->set_blink = set_blink ;
    this->set_blink_interval = set_blink_interval ;
    this->get_blink_interval = get_blink_interval ;
    this->set_shape = set_shape ;
    this->get_x = get_x ;
    this->get_y = get_y ;
    this->is_visible = is_visible ;


    return (cursor_t*) this ;
}

// "Destrutor" - libera a memória
void cursor_destroy(cursor_t* cursor)
{
    if (cursor) {
        free(cursor);
    }
}