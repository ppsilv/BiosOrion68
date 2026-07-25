#ifndef __PICO_PS2_H__
#define __PICO_PS2_H__
#include <stdint.h>
#include <stdbool.h>


#define PICO_PS2_BASE 0x00FF9100
#define PICO_PS2_DATA_REG   (*((volatile unsigned char *)(PICO_PS2_BASE + (9*2)+1)))

void kb_init(void);
bool kb_put(uint8_t data);
bool kb_get(uint8_t *data);
bool kb_available(void);

unsigned int ps2_getchar();

#endif