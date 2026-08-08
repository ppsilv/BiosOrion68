#ifndef __PICO_PS2_H__
#define __PICO_PS2_H__
#include <stdint.h>
#include <stdbool.h>

void kb_init(void);
bool kb_put(uint8_t data);
bool kb_get(uint8_t *data);
bool kb_available(void);

#endif